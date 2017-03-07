//-----------------------------------------------------------------------------
// Copyright 2016, 2017, Oracle and/or its affiliates. All rights reserved.
//
// Portions Copyright 2007-2015, Anthony Tuininga. All rights reserved.
//
// Portions Copyright 2001-2007, Computronix (Canada) Ltd., Edmonton, Alberta,
// Canada. All rights reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Environment.c
//   Environment handling.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// structure for the Python type
//-----------------------------------------------------------------------------
typedef struct {
    PyObject_HEAD
    OCIEnv *handle;
    OCIError *errorHandle;
    int maxBytesPerCharacter;
    int nmaxBytesPerCharacter;
    char *encoding;
    char *nencoding;
    ub2 charsetId;
    ub2 ncharsetId;
    PyObject *cloneEnv;
    udt_Buffer numberToStringFormatBuffer;
    udt_Buffer numberFromStringFormatBuffer;
    udt_Buffer nlsNumericCharactersBuffer;
} udt_Environment;

//-----------------------------------------------------------------------------
// forward declarations
//-----------------------------------------------------------------------------
static void Environment_Free(udt_Environment*);
static int Environment_CheckForError(udt_Environment*, sword, const char*);

//-----------------------------------------------------------------------------
// declaration of Python type
//-----------------------------------------------------------------------------
static PyTypeObject g_EnvironmentType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "OracleEnvironment",                // tp_name
    sizeof(udt_Environment),            // tp_basicsize
    0,                                  // tp_itemsize
    (destructor) Environment_Free,      // tp_dealloc
    0,                                  // tp_print
    0,                                  // tp_getattr
    0,                                  // tp_setattr
    0,                                  // tp_compare
    0,                                  // tp_repr
    0,                                  // tp_as_number
    0,                                  // tp_as_sequence
    0,                                  // tp_as_mapping
    0,                                  // tp_hash
    0,                                  // tp_call
    0,                                  // tp_str
    0,                                  // tp_getattro
    0,                                  // tp_setattro
    0,                                  // tp_as_buffer
    Py_TPFLAGS_DEFAULT,                 // tp_flags
    0                                   // tp_doc
};


#include "Error.c"


//-----------------------------------------------------------------------------
// Environment_New()
//   Create a new environment object.
//-----------------------------------------------------------------------------
static udt_Environment *Environment_New(
    OCIEnv *handle)                     // handle to use
{
    udt_Environment *env;
    udt_Error *errorObj;
    sword status;

    // create a new object for the Oracle environment
    env = (udt_Environment*) g_EnvironmentType.tp_alloc(&g_EnvironmentType, 0);
    if (!env)
        return NULL;
    env->handle = handle;
    env->maxBytesPerCharacter = 1;
    env->nmaxBytesPerCharacter = 4;
    cxBuffer_Init(&env->numberToStringFormatBuffer);
    cxBuffer_Init(&env->numberFromStringFormatBuffer);
    cxBuffer_Init(&env->nlsNumericCharactersBuffer);

    // create the error handle
    status = OCIHandleAlloc(handle, (dvoid**) &env->errorHandle,
            OCI_HTYPE_ERROR, 0, 0);
    if (status != OCI_SUCCESS) {
        errorObj = Error_InternalNew(env,
                "Environment_New(): create error handle", OCI_HTYPE_ENV,
                handle);
        if (!errorObj) {
            Py_DECREF(env);
            return NULL;
        }
        PyErr_SetObject(g_DatabaseErrorException, (PyObject*) errorObj);
        Py_DECREF(env);
        return NULL;
    }

    return env;
}


//-----------------------------------------------------------------------------
// Environment_GetCharacterSetName()
//   Retrieve and store the IANA character set name for the attribute.
//-----------------------------------------------------------------------------
static int Environment_GetCharacterSetName(
    udt_Environment *self,              // environment object
    ub2 attribute,                      // attribute to fetch
    const char *overrideValue,          // override value, if specified
    char **result,                      // place to store result
    ub2 *charsetId)                     // character set ID (OUT)
{
    char charsetName[OCI_NLS_MAXBUFSZ], ianaCharsetName[OCI_NLS_MAXBUFSZ];
    sword status;

    // get character set id
    status = OCIAttrGet(self->handle, OCI_HTYPE_ENV, charsetId, NULL,
            attribute, self->errorHandle);
    if (Environment_CheckForError(self, status,
            "Environment_GetCharacterSetName(): get charset id") < 0)
        return -1;

    // if override value specified, use it
    if (overrideValue) {
        *result = PyMem_Malloc(strlen(overrideValue) + 1);
        if (!*result)
            return -1;
        strcpy(*result, overrideValue);
        return 0;
    }

    // get character set name
    status = OCINlsCharSetIdToName(self->handle, (text*) charsetName,
            OCI_NLS_MAXBUFSZ, *charsetId);
    if (Environment_CheckForError(self, status,
            "Environment_GetCharacterSetName(): get Oracle charset name") < 0)
        return -1;

    // get IANA character set name
    status = OCINlsNameMap(self->handle, (oratext*) ianaCharsetName,
            OCI_NLS_MAXBUFSZ, (oratext*) charsetName, OCI_NLS_CS_ORA_TO_IANA);
    if (Environment_CheckForError(self, status,
            "Environment_GetCharacterSetName(): translate NLS charset") < 0)
        return -1;

    // store results
    *result = PyMem_Malloc(strlen(ianaCharsetName) + 1);
    if (!*result)
        return -1;
    strcpy(*result, ianaCharsetName);
    return 0;
}


//-----------------------------------------------------------------------------
// Environment_SetBuffer()
//   Set the buffer in the environment from the specified string.
//-----------------------------------------------------------------------------
static int Environment_SetBuffer(
    udt_Buffer *buf,                    // buffer to set
    const char *value,                  // ASCII value to use
    const char *encoding)               // encoding to use
{
    PyObject *obj;

    obj = cxString_FromAscii(value);
    if (!obj)
        return -1;
    if (cxBuffer_FromObject(buf, obj, encoding) < 0) {
        Py_DECREF(obj);
        return -1;
    }
    Py_CLEAR(obj);

    return 0;
}


//-----------------------------------------------------------------------------
// Environment_LookupCharSet()
//   Look up an Oracle character set given the name. This can be either the
// Oracle character set name or the IANA encoding name. A pointer to an
// environment handle is passed and an environment created if needed in
// order to perform the lookup.
//-----------------------------------------------------------------------------
static int Environment_LookupCharSet(
    const char *name,                   // IANA or Oracle character set name
    OCIEnv **envHandle,                 // environment handle (IN/OUT)
    ub2 *charsetId)                     // Oracle character set id (OUT)
{
    char oraCharsetName[OCI_NLS_MAXBUFSZ];
    sword status;

    // if IANA name is null, use the charset 0 which tells Oracle to make use
    // of the NLS_LANG and NLS_NCHAR environment variables
    if (!name) {
        *charsetId = 0;
        return 0;
    }

    // create environment, if needed
    if (!*envHandle) {
        status = OCIEnvCreate(envHandle, OCI_DEFAULT, NULL, NULL, NULL, NULL,
                0, NULL);
        if (status != OCI_SUCCESS) {
            PyErr_SetString(g_InterfaceErrorException,
                    "Unable to acquire Oracle environment handle");
            return -1;
        }
    }

    // check for the Oracle character set name first
    // if that fails, lookup using the IANA character set name
    *charsetId = OCINlsCharSetNameToId(*envHandle, (oratext*) name);
    if (!*charsetId) {
        status = OCINlsNameMap(*envHandle, (oratext*) oraCharsetName,
                sizeof(oraCharsetName), (oratext*) name,
                OCI_NLS_CS_IANA_TO_ORA);
        if (status == OCI_ERROR) {
            PyErr_SetString(g_InterfaceErrorException,
                    "Invalid character set name");
            return -1;
        }
        *charsetId = OCINlsCharSetNameToId(*envHandle,
                (oratext*) oraCharsetName);
    }

    return 0;
}


//-----------------------------------------------------------------------------
// Environment_NewFromScratch()
//   Create a new environment object from scratch.
//-----------------------------------------------------------------------------
static udt_Environment *Environment_NewFromScratch(
    int threaded,                       // use threaded mode?
    int events,                         // use events mode?
    char *encoding,                     // override value for encoding
    char *nencoding)                    // override value for nencoding
{
    ub2 charsetId, ncharsetId;
    udt_Environment *env;
    OCIEnv *handle;
    sword status;
    ub4 mode;

    // turn threading mode on, if desired
    mode = OCI_OBJECT;
    if (threaded)
        mode |= OCI_THREADED;
    if (events)
        mode |= OCI_EVENTS;

    // perform lookup of character sets to use
    // keep track of any environment that needs to be created in order to
    // perform this lookup
    handle = NULL;
    status = Environment_LookupCharSet(encoding, &handle, &charsetId);
    if (status == 0)
        status = Environment_LookupCharSet(nencoding, &handle, &ncharsetId);
    if (handle) {
        OCIHandleFree(handle, OCI_HTYPE_ENV);
        handle = NULL;
    }
    if (status < 0)
        return NULL;

    // create the new environment handle
    status = OCIEnvNlsCreate(&handle, mode, NULL, NULL, NULL, NULL, 0, NULL,
            charsetId, ncharsetId);
    if (!handle ||
            (status != OCI_SUCCESS && status != OCI_SUCCESS_WITH_INFO)) {
        PyErr_SetString(g_InterfaceErrorException,
                "Unable to acquire Oracle environment handle");
        return NULL;
    }

    // create the environment object
    env = Environment_New(handle);
    if (!env) {
        OCIHandleFree(handle, OCI_HTYPE_ENV);
        return NULL;
    }

    // acquire max bytes per character
    status = OCINlsNumericInfoGet(env->handle, env->errorHandle,
            &env->maxBytesPerCharacter, OCI_NLS_CHARSET_MAXBYTESZ);
    if (Environment_CheckForError(env, status,
            "Environment_New(): get max bytes per character") < 0) {
        Py_DECREF(env);
        return NULL;
    }

    // determine encodings to use for Unicode values
    if (Environment_GetCharacterSetName(env, OCI_ATTR_ENV_CHARSET_ID,
            encoding, &env->encoding, &env->charsetId) < 0)
        return NULL;
    if (Environment_GetCharacterSetName(env, OCI_ATTR_ENV_NCHARSET_ID,
            nencoding, &env->nencoding, &env->ncharsetId) < 0)
        return NULL;

    // max bytes per character for NCHAR can be assigned only if it matches the
    // character set used for CHAR data; OCI does not provide a way of
    // determining it otherwise
    if (env->ncharsetId == env->charsetId)
        env->nmaxBytesPerCharacter = env->maxBytesPerCharacter;

    // fill buffers for number formats
    if (Environment_SetBuffer(&env->numberToStringFormatBuffer, "TM9",
            env->encoding) < 0)
        return NULL;
    if (Environment_SetBuffer(&env->numberFromStringFormatBuffer,
            "999999999999999999999999999999999999999999999999999999999999999",
            env->encoding) < 0)
        return NULL;
    if (Environment_SetBuffer(&env->nlsNumericCharactersBuffer,
            "NLS_NUMERIC_CHARACTERS='.,'", env->encoding) < 0)
        return NULL;

    return env;
}


//-----------------------------------------------------------------------------
// Environment_Clone()
//   Clone an existing environment which is used when acquiring a connection
// from a session pool, for example.
//-----------------------------------------------------------------------------
static udt_Environment *Environment_Clone(
    udt_Environment *cloneEnv)          // environment to clone
{
    udt_Environment *env;

    env = Environment_New(cloneEnv->handle);
    if (!env)
        return NULL;
    env->maxBytesPerCharacter = cloneEnv->maxBytesPerCharacter;
    Py_INCREF(cloneEnv);
    env->cloneEnv = (PyObject*) cloneEnv;
    env->encoding = cloneEnv->encoding;
    env->nencoding = cloneEnv->nencoding;
    env->charsetId = cloneEnv->charsetId;
    env->ncharsetId = cloneEnv->ncharsetId;
    cxBuffer_Copy(&env->numberToStringFormatBuffer,
            &cloneEnv->numberToStringFormatBuffer);
    cxBuffer_Copy(&env->numberFromStringFormatBuffer,
            &cloneEnv->numberFromStringFormatBuffer);
    cxBuffer_Copy(&env->nlsNumericCharactersBuffer,
            &cloneEnv->nlsNumericCharactersBuffer);
    return env;
}


//-----------------------------------------------------------------------------
// Environment_Free()
//   Deallocate the environment. Note that destroying the environment handle
// will automatically destroy any child handles that were created.
//-----------------------------------------------------------------------------
static void Environment_Free(
    udt_Environment *self)              // environment object
{
    if (self->errorHandle)
        OCIHandleFree(self->errorHandle, OCI_HTYPE_ERROR);
    if (self->handle && !self->cloneEnv)
        OCIHandleFree(self->handle, OCI_HTYPE_ENV);
    if (!self->cloneEnv) {
        if (self->encoding)
            PyMem_Free(self->encoding);
        if (self->nencoding)
            PyMem_Free(self->nencoding);
    }
    cxBuffer_Clear(&self->numberToStringFormatBuffer);
    cxBuffer_Clear(&self->numberFromStringFormatBuffer);
    cxBuffer_Clear(&self->nlsNumericCharactersBuffer);
    Py_CLEAR(self->cloneEnv);
    Py_TYPE(self)->tp_free((PyObject*) self);
}


//-----------------------------------------------------------------------------
// Environment_CheckForError()
//   Check for an error in the last call and if an error has occurred, raise a
// Python exception.
//-----------------------------------------------------------------------------
static int Environment_CheckForError(
    udt_Environment *environment,       // environment to raise error in
    sword status,                       // status of last call
    const char *context)                // context
{
    return Error_Check(environment, status, context, environment->errorHandle);
}

