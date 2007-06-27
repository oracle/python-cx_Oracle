//-----------------------------------------------------------------------------
// LobVar.c
//   Defines the routines for handling LOB variables.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// LOB type
//-----------------------------------------------------------------------------
typedef struct {
    Variable_HEAD
    OCILobLocator **data;
    udt_Connection *connection;
    int isFile;
} udt_LobVar;

#include "ExternalLobVar.c"

//-----------------------------------------------------------------------------
// Declaration of LOB variable functions.
//-----------------------------------------------------------------------------
static int LobVar_Initialize(udt_LobVar*, udt_Cursor*);
static void LobVar_Finalize(udt_LobVar*);
static PyObject *LobVar_GetValue(udt_LobVar*, unsigned);
static int LobVar_SetValue(udt_LobVar*, unsigned, PyObject*);


//-----------------------------------------------------------------------------
// Python type declarations
//-----------------------------------------------------------------------------
static PyTypeObject g_CLOBVarType = {
    PyObject_HEAD_INIT(NULL)
    0,                                  // ob_size
    "cx_Oracle.CLOB",                   // tp_name
    sizeof(udt_LobVar),                 // tp_basicsize
    0,                                  // tp_itemsize
    (destructor) Variable_Free,         // tp_dealloc
    0,                                  // tp_print
    0,                                  // tp_getattr
    0,                                  // tp_setattr
    0,                                  // tp_compare
    (reprfunc) Variable_Repr,           // tp_repr
    0,                                  // tp_as_number
    0,                                  // tp_as_sequence
    0,                                  // tp_as_mapping
    0,                                  // tp_hash
    0,                                  // tp_call
    0,                                  // tp_str
    (getattrofunc) Variable_GetAttr,    // tp_getattro
    0,                                  // tp_setattro
    0,                                  // tp_as_buffer
    Py_TPFLAGS_DEFAULT,                 // tp_flags
    0                                   // tp_doc
};


static PyTypeObject g_NCLOBVarType = {
    PyObject_HEAD_INIT(NULL)
    0,                                  // ob_size
    "cx_Oracle.NCLOB",                  // tp_name
    sizeof(udt_LobVar),                 // tp_basicsize
    0,                                  // tp_itemsize
    (destructor) Variable_Free,         // tp_dealloc
    0,                                  // tp_print
    0,                                  // tp_getattr
    0,                                  // tp_setattr
    0,                                  // tp_compare
    (reprfunc) Variable_Repr,           // tp_repr
    0,                                  // tp_as_number
    0,                                  // tp_as_sequence
    0,                                  // tp_as_mapping
    0,                                  // tp_hash
    0,                                  // tp_call
    0,                                  // tp_str
    (getattrofunc) Variable_GetAttr,    // tp_getattro
    0,                                  // tp_setattro
    0,                                  // tp_as_buffer
    Py_TPFLAGS_DEFAULT,                 // tp_flags
    0                                   // tp_doc
};


static PyTypeObject g_BLOBVarType = {
    PyObject_HEAD_INIT(NULL)
    0,                                  // ob_size
    "cx_Oracle.BLOB",                   // tp_name
    sizeof(udt_LobVar),                 // tp_basicsize
    0,                                  // tp_itemsize
    (destructor) Variable_Free,         // tp_dealloc
    0,                                  // tp_print
    0,                                  // tp_getattr
    0,                                  // tp_setattr
    0,                                  // tp_compare
    (reprfunc) Variable_Repr,           // tp_repr
    0,                                  // tp_as_number
    0,                                  // tp_as_sequence
    0,                                  // tp_as_mapping
    0,                                  // tp_hash
    0,                                  // tp_call
    0,                                  // tp_str
    (getattrofunc) Variable_GetAttr,    // tp_getattro
    0,                                  // tp_setattro
    0,                                  // tp_as_buffer
    Py_TPFLAGS_DEFAULT,                 // tp_flags
    0                                   // tp_doc
};


static PyTypeObject g_BFILEVarType = {
    PyObject_HEAD_INIT(NULL)
    0,                                  // ob_size
    "cx_Oracle.BFILE",                  // tp_name
    sizeof(udt_LobVar),                 // tp_basicsize
    0,                                  // tp_itemsize
    (destructor) Variable_Free,         // tp_dealloc
    0,                                  // tp_print
    0,                                  // tp_getattr
    0,                                  // tp_setattr
    0,                                  // tp_compare
    (reprfunc) Variable_Repr,           // tp_repr
    0,                                  // tp_as_number
    0,                                  // tp_as_sequence
    0,                                  // tp_as_mapping
    0,                                  // tp_hash
    0,                                  // tp_call
    0,                                  // tp_str
    (getattrofunc) Variable_GetAttr,    // tp_getattro
    0,                                  // tp_setattro
    0,                                  // tp_as_buffer
    Py_TPFLAGS_DEFAULT,                 // tp_flags
    0                                   // tp_doc
};


//-----------------------------------------------------------------------------
// variable type declarations
//-----------------------------------------------------------------------------
static udt_VariableType vt_CLOB = {
    (InitializeProc) LobVar_Initialize,
    (FinalizeProc) LobVar_Finalize,
    (PreDefineProc) NULL,
    (PostDefineProc) NULL,
    (IsNullProc) NULL,
    (SetValueProc) LobVar_SetValue,
    (GetValueProc) LobVar_GetValue,
    &g_CLOBVarType,                     // Python type
    SQLT_CLOB,                          // Oracle type
    SQLCS_IMPLICIT,                     // charset form
    sizeof(OCILobLocator*),             // element length
    0,                                  // is variable length
    0,                                  // can be copied
    0                                   // can be in array
};


static udt_VariableType vt_NCLOB = {
    (InitializeProc) LobVar_Initialize,
    (FinalizeProc) LobVar_Finalize,
    (PreDefineProc) NULL,
    (PostDefineProc) NULL,
    (IsNullProc) NULL,
    (SetValueProc) LobVar_SetValue,
    (GetValueProc) LobVar_GetValue,
    &g_NCLOBVarType,                    // Python type
    SQLT_CLOB,                          // Oracle type
    SQLCS_NCHAR,                        // charset form
    sizeof(OCILobLocator*),             // element length
    0,                                  // is variable length
    0,                                  // can be copied
    0                                   // can be in array
};


static udt_VariableType vt_BLOB = {
    (InitializeProc) LobVar_Initialize,
    (FinalizeProc) LobVar_Finalize,
    (PreDefineProc) NULL,
    (PostDefineProc) NULL,
    (IsNullProc) NULL,
    (SetValueProc) LobVar_SetValue,
    (GetValueProc) LobVar_GetValue,
    &g_BLOBVarType,                     // Python type
    SQLT_BLOB,                          // Oracle type
    SQLCS_IMPLICIT,                     // charset form
    sizeof(OCILobLocator*),             // element length
    0,                                  // is variable length
    0,                                  // can be copied
    0                                   // can be in array
};


static udt_VariableType vt_BFILE = {
    (InitializeProc) LobVar_Initialize,
    (FinalizeProc) LobVar_Finalize,
    (PreDefineProc) NULL,
    (PostDefineProc) NULL,
    (IsNullProc) NULL,
    (SetValueProc) LobVar_SetValue,
    (GetValueProc) LobVar_GetValue,
    &g_BFILEVarType,                    // Python type
    SQLT_BFILE,                         // Oracle type
    SQLCS_IMPLICIT,                     // charset form
    sizeof(OCILobLocator*),             // element length
    0,                                  // is variable length
    0,                                  // can be copied
    0                                   // can be in array
};


//-----------------------------------------------------------------------------
// LobVar_Initialize()
//   Initialize the variable.
//-----------------------------------------------------------------------------
static int LobVar_Initialize(
    udt_LobVar *var,                    // variable to initialize
    udt_Cursor *cursor)                 // cursor created by
{
    sword status;
    ub4 i;

    // initialize members
    Py_INCREF(cursor->connection);
    var->connection = cursor->connection;
    var->isFile = (var->type == &vt_BFILE);

    // initialize the LOB locators
    for (i = 0; i < var->allocatedElements; i++) {
        status = OCIDescriptorAlloc(var->environment->handle,
                (dvoid**) &var->data[i], OCI_DTYPE_LOB, 0, 0);
        if (Environment_CheckForError(var->environment, status,
                "LobVar_Initialize()") < 0)
            return -1;
    }

    return 0;
}


//-----------------------------------------------------------------------------
// LobVar_Finalize()
//   Prepare for variable destruction.
//-----------------------------------------------------------------------------
static void LobVar_Finalize(
    udt_LobVar *var)                    // variable to free
{
    boolean isTemporary;
    ub4 i;

    for (i = 0; i < var->allocatedElements; i++) {
        if (var->data[i]) {
            OCILobIsTemporary(var->environment->handle,
                    var->environment->errorHandle, var->data[i], &isTemporary);
            if (isTemporary)
                OCILobFreeTemporary(var->connection->handle,
                        var->environment->errorHandle, var->data[i]);
            OCIDescriptorFree(var->data[i], OCI_DTYPE_LOB);
        }
    }
    Py_DECREF(var->connection);
}


//-----------------------------------------------------------------------------
// LobVar_GetValue()
//   Returns the value stored at the given array position.
//-----------------------------------------------------------------------------
static PyObject *LobVar_GetValue(
    udt_LobVar *var,                    // variable to determine value for
    unsigned pos)                       // array position
{
    return ExternalLobVar_New(var, pos);
}


//-----------------------------------------------------------------------------
// LobVar_SetValue()
//   Sets the value stored at the given array position.
//-----------------------------------------------------------------------------
static int LobVar_SetValue(
    udt_LobVar *var,                    // variable to determine value for
    unsigned pos,                       // array position
    PyObject *value)                    // value to set
{
    boolean isTemporary;
    sword status;
    ub1 lobType;
    ub4 length;

    // only support strings
    if (!PyString_Check(value)) {
        PyErr_SetString(PyExc_TypeError, "expecting string data");
        return -1;
    }

    // make sure have temporary LOBs set up
    status = OCILobIsTemporary(var->environment->handle,
            var->environment->errorHandle, var->data[pos], &isTemporary);
    if (Environment_CheckForError(var->environment, status,
            "LobVar_SetValue(): is temporary?") < 0)
        return -1;
    if (!isTemporary) {
        if (var->type->oracleType == SQLT_BLOB)
            lobType = OCI_TEMP_BLOB;
        else lobType = OCI_TEMP_CLOB;
        status = OCILobCreateTemporary(var->connection->handle,
                var->environment->errorHandle, var->data[pos], OCI_DEFAULT,
                OCI_DEFAULT, lobType, FALSE, OCI_DURATION_SESSION);
        if (Environment_CheckForError(var->environment, status,
                "LobVar_SetValue(): create temporary") < 0)
            return -1;
    }

    // trim the current value
    status = OCILobTrim(var->connection->handle,
            var->environment->errorHandle, var->data[pos], 0);
    if (Environment_CheckForError(var->environment, status,
            "LobVar_SetValue(): trim") < 0)
        return -1;

    // set the current value
    length = PyString_GET_SIZE(value);
    if (length) {
        status = OCILobWrite(var->connection->handle,
                var->environment->errorHandle, var->data[pos],
                &length, 1, PyString_AS_STRING(value),
                PyString_GET_SIZE(value), OCI_ONE_PIECE, NULL, NULL, 0, 0);
        if (Environment_CheckForError(var->environment, status,
                "LobVar_SetValue(): write") < 0)
            return -1;
    }

    return 0;
}

