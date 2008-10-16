//-----------------------------------------------------------------------------
// Variable.c
//   Defines Python types for Oracle variables.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// define structure common to all variables
//-----------------------------------------------------------------------------
struct _udt_VariableType;
#define Variable_HEAD \
    PyObject_HEAD \
    OCIBind *bindHandle; \
    OCIDefine *defineHandle; \
    OCIStmt *boundCursorHandle; \
    PyObject *boundName; \
    PyObject *inConverter; \
    PyObject *outConverter; \
    ub4 boundPos; \
    udt_Environment *environment; \
    ub4 allocatedElements; \
    ub4 actualElements; \
    unsigned internalFetchNum; \
    int isArray; \
    int isAllocatedInternally; \
    sb2 *indicator; \
    ub2 *returnCode; \
    ub2 *actualLength; \
    ub4 maxLength; \
    struct _udt_VariableType *type;
typedef struct {
    Variable_HEAD
    void *data;
} udt_Variable;


//-----------------------------------------------------------------------------
// define function types for the common actions that take place on a variable
//-----------------------------------------------------------------------------
typedef int (*InitializeProc)(udt_Variable*, udt_Cursor*);
typedef void (*FinalizeProc)(udt_Variable*);
typedef int (*PreDefineProc)(udt_Variable*, OCIParam*);
typedef int (*PostDefineProc)(udt_Variable*);
typedef int (*IsNullProc)(udt_Variable*, unsigned);
typedef int (*SetValueProc)(udt_Variable*, unsigned, PyObject*);
typedef PyObject * (*GetValueProc)(udt_Variable*, unsigned);


//-----------------------------------------------------------------------------
// define structure for the common actions that take place on a variable
//-----------------------------------------------------------------------------
typedef struct _udt_VariableType {
    InitializeProc initializeProc;
    FinalizeProc finalizeProc;
    PreDefineProc preDefineProc;
    PostDefineProc postDefineProc;
    IsNullProc isNullProc;
    SetValueProc setValueProc;
    GetValueProc getValueProc;
    PyTypeObject *pythonType;
    ub2 oracleType;
    ub1 charsetForm;
    ub4 elementLength;
    int isCharacterData;
    int isVariableLength;
    int canBeCopied;
    int canBeInArray;
} udt_VariableType;


//-----------------------------------------------------------------------------
// Declaration of common variable functions.
//-----------------------------------------------------------------------------
static void Variable_Free(udt_Variable *);
static PyObject *Variable_Repr(udt_Variable *);
static PyObject *Variable_ExternalCopy(udt_Variable *, PyObject *);
static PyObject *Variable_ExternalSetValue(udt_Variable *, PyObject *);
static PyObject *Variable_ExternalGetValue(udt_Variable *, PyObject *,
        PyObject *);
static int Variable_InternalBind(udt_Variable *);
static int Variable_Resize(udt_Variable *, unsigned);


//-----------------------------------------------------------------------------
// declaration of members for variables
//-----------------------------------------------------------------------------
static PyMemberDef g_VariableMembers[] = {
    { "maxlength", T_INT, offsetof(udt_Variable, maxLength), READONLY },
    { "allocelems", T_INT, offsetof(udt_Variable, allocatedElements),
            READONLY },
    { "inconverter", T_OBJECT, offsetof(udt_Variable, inConverter), 0 },
    { "outconverter", T_OBJECT, offsetof(udt_Variable, outConverter), 0 },
    { NULL }
};


//-----------------------------------------------------------------------------
// declaration of methods for variables
//-----------------------------------------------------------------------------
static PyMethodDef g_VariableMethods[] = {
    { "copy", (PyCFunction) Variable_ExternalCopy, METH_VARARGS },
    { "setvalue", (PyCFunction) Variable_ExternalSetValue, METH_VARARGS },
    { "getvalue", (PyCFunction) Variable_ExternalGetValue,
              METH_VARARGS  | METH_KEYWORDS },
    { NULL }
};


//-----------------------------------------------------------------------------
// The base variable type
//-----------------------------------------------------------------------------
static PyTypeObject g_BaseVarType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "cx_Oracle._BASEVARTYPE",           // tp_name
    sizeof(udt_Variable),               // tp_basicsize
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
    0,                                  // tp_getattro
    0,                                  // tp_setattro
    0,                                  // tp_as_buffer
    Py_TPFLAGS_DEFAULT,                 // tp_flags
    0,                                  // tp_doc
    0,                                  // tp_traverse
    0,                                  // tp_clear
    0,                                  // tp_richcompare
    0,                                  // tp_weaklistoffset
    0,                                  // tp_iter
    0,                                  // tp_iternext
    g_VariableMethods,                  // tp_methods
    g_VariableMembers                   // tp_members
};


#ifndef NATIVE_DATETIME
#include "ExternalDateTimeVar.c"
#endif
#include "Transforms.c"
#include "StringVar.c"
#include "LongVar.c"
#include "NumberVar.c"
#include "DateTimeVar.c"
#include "TimestampVar.c"
#include "LobVar.c"
#include "CursorVar.c"
#include "ObjectVar.c"


//-----------------------------------------------------------------------------
// Variable_New()
//   Allocate a new variable.
//-----------------------------------------------------------------------------
static udt_Variable *Variable_New(
    udt_Cursor *cursor,                 // cursor to associate variable with
    unsigned numElements,               // number of elements to allocate
    udt_VariableType *type,             // variable type
    ub4 elementLength)                  // used only for variable length types
{
    unsigned PY_LONG_LONG dataLength;
    udt_Variable *var;
    ub4 i;

    // attempt to allocate the object
    var = PyObject_NEW(udt_Variable, type->pythonType);
    if (!var)
        return NULL;

    // perform basic initialization
    Py_INCREF(cursor->connection->environment);
    var->environment = cursor->connection->environment;
    var->boundCursorHandle = NULL;
    var->bindHandle = NULL;
    var->defineHandle = NULL;
    var->boundName = NULL;
    var->inConverter = NULL;
    var->outConverter = NULL;
    var->boundPos = 0;
    if (numElements < 1)
        var->allocatedElements = 1;
    else var->allocatedElements = numElements;
    var->actualElements = 0;
    var->internalFetchNum = 0;
    var->isArray = 0;
    var->isAllocatedInternally = 1;
    var->type = type;
    var->indicator = NULL;
    var->data = NULL;
    var->actualLength = NULL;
    var->returnCode = NULL;

    // set the maximum length of the variable, ensure that a minimum of
    // 2 bytes is allocated to ensure that the array size check works
    var->maxLength = type->elementLength;
    if (type->isVariableLength) {
        if (elementLength < sizeof(ub2))
            elementLength = sizeof(ub2);
        var->maxLength = elementLength;
    }
    if (type->isCharacterData) {
#ifdef WITH_UNICODE
        var->maxLength = var->maxLength * CXORA_BYTES_PER_CHAR;
#else
        if (type->charsetForm != SQLCS_IMPLICIT)
            var->maxLength =
                    var->maxLength * cursor->environment->maxBytesPerCharacter;
        else var->maxLength = elementLength * 2;
#endif
    }

    // allocate the indicator and data
    dataLength = (unsigned PY_LONG_LONG) numElements *
            (unsigned PY_LONG_LONG) var->maxLength;
    if (dataLength > INT_MAX) {
        PyErr_SetString(PyExc_ValueError, "array size too large");
        Py_DECREF(var);
        return NULL;
    }
    var->indicator = PyMem_Malloc(numElements * sizeof(sb2));
    var->data = PyMem_Malloc((size_t) dataLength);
    if (!var->indicator || !var->data) {
        PyErr_NoMemory();
        Py_DECREF(var);
        return NULL;
    }

    // ensure that all variable values start out NULL
    for (i = 0; i < numElements; i++)
        var->indicator[i] = OCI_IND_NULL;

    // for variable length data, also allocate the return code
    if (type->isVariableLength) {
        var->returnCode = PyMem_Malloc(numElements * sizeof(ub2));
        if (!var->returnCode) {
            PyErr_NoMemory();
            Py_DECREF(var);
            return NULL;
        }
    }

    // perform extended initialization
    if (var->type->initializeProc) {
        if ((*var->type->initializeProc)(var, cursor) < 0) {
            Py_DECREF(var);
            return NULL;
        }
    }

    return var;
}


//-----------------------------------------------------------------------------
// Variable_Free()
//   Free an existing variable.
//-----------------------------------------------------------------------------
static void Variable_Free(
    udt_Variable *var)                  // variable to free
{
    if (var->isAllocatedInternally) {
        if (var->type->finalizeProc)
            (*var->type->finalizeProc)(var);
        if (var->indicator)
            PyMem_Free(var->indicator);
        if (var->data)
            PyMem_Free(var->data);
        if (var->actualLength)
            PyMem_Free(var->actualLength);
        if (var->returnCode)
            PyMem_Free(var->returnCode);
    }
    Py_DECREF(var->environment);
    Py_XDECREF(var->boundName);
    Py_XDECREF(var->inConverter);
    Py_XDECREF(var->outConverter);
    PyObject_DEL(var);
}


//-----------------------------------------------------------------------------
// Variable_Resize()
//   Resize the variable.
//-----------------------------------------------------------------------------
static int Variable_Resize(
    udt_Variable *var,                  // variable to resize
    unsigned maxLength)                 // new length to use
{
    char *newData;
    ub4 i;

    // allocate new memory for the larger size
    newData = (char*) PyMem_Malloc(var->allocatedElements * maxLength);
    if (!newData) {
        PyErr_NoMemory();
        return -1;
    }

    // copy the data from the original array to the new array
    for (i = 0; i < var->allocatedElements; i++)
        memcpy(newData + maxLength * i,
                (void*) ( (char*) var->data + var->maxLength * i ),
                var->maxLength);
    PyMem_Free(var->data);
    var->data = newData;
    var->maxLength = maxLength;

    // force rebinding
    if (var->boundName || var->boundPos > 0) {
        if (Variable_InternalBind(var) < 0)
            return -1;
    }

    return 0;
}


//-----------------------------------------------------------------------------
// Variable_Check()
//   Returns a boolean indicating if the object is a variable.
//-----------------------------------------------------------------------------
static int Variable_Check(
    PyObject *object)                   // Python object to check
{
    return (Py_TYPE(object) == &g_CursorVarType ||
            Py_TYPE(object) == &g_DateTimeVarType ||
            Py_TYPE(object) == &g_BFILEVarType ||
            Py_TYPE(object) == &g_BLOBVarType ||
            Py_TYPE(object) == &g_CLOBVarType ||
            Py_TYPE(object) == &g_LongStringVarType ||
            Py_TYPE(object) == &g_LongBinaryVarType ||
            Py_TYPE(object) == &g_NumberVarType ||
            Py_TYPE(object) == &g_StringVarType ||
            Py_TYPE(object) == &g_FixedCharVarType ||
#ifndef WITH_UNICODE
            Py_TYPE(object) == &g_NCLOBVarType ||
            Py_TYPE(object) == &g_UnicodeVarType ||
            Py_TYPE(object) == &g_FixedUnicodeVarType ||
#endif
            Py_TYPE(object) == &g_RowidVarType ||
            Py_TYPE(object) == &g_BinaryVarType ||
            Py_TYPE(object) == &g_TimestampVarType
#ifdef SQLT_BFLOAT
            || Py_TYPE(object) == &g_NativeFloatVarType
#endif
            );
}


//-----------------------------------------------------------------------------
// Variable_TypeByPythonType()
//   Return a variable type given a Python type object or NULL if the Python
// type does not have a corresponding variable type.
//-----------------------------------------------------------------------------
static udt_VariableType *Variable_TypeByPythonType(
    udt_Cursor* cursor,                 // cursor variable created for
    PyObject* type)                     // Python type
{
    if (type == (PyObject*) &g_StringVarType)
        return &vt_String;
    if (type == (PyObject*) cxString_Type)
        return &vt_String;
    if (type == (PyObject*) &g_FixedCharVarType)
        return &vt_FixedChar;
#ifndef WITH_UNICODE
    if (type == (PyObject*) &g_UnicodeVarType)
        return &vt_NationalCharString;
    if (type == (PyObject*) &PyUnicode_Type)
        return &vt_NationalCharString;
    if (type == (PyObject*) &g_FixedUnicodeVarType)
        return &vt_FixedNationalChar;
    if (type == (PyObject*) &g_NCLOBVarType)
        return &vt_NCLOB;
#endif
    if (type == (PyObject*) &g_RowidVarType)
        return &vt_Rowid;
    if (type == (PyObject*) &g_BinaryVarType)
        return &vt_Binary;
    if (type == (PyObject*) &PyBuffer_Type)
        return &vt_Binary;
    if (type == (PyObject*) &g_LongStringVarType)
        return &vt_LongString;
    if (type == (PyObject*) &g_LongBinaryVarType)
        return &vt_LongBinary;
    if (type == (PyObject*) &g_BFILEVarType)
        return &vt_BFILE;
    if (type == (PyObject*) &g_BLOBVarType)
        return &vt_BLOB;
    if (type == (PyObject*) &g_CLOBVarType)
        return &vt_CLOB;
    if (type == (PyObject*) &g_NumberVarType) {
        if (cursor->numbersAsStrings)
            return &vt_NumberAsString;
        return &vt_Float;
    }
    if (type == (PyObject*) &PyFloat_Type)
        return &vt_Float;
    if (type == (PyObject*) &PyInt_Type)
        return &vt_Integer;
    if (type == (PyObject*) &PyLong_Type)
        return &vt_LongInteger;
    if (type == (PyObject*) &PyBool_Type)
        return &vt_Boolean;
    if (type == (PyObject*) &g_DateTimeVarType)
        return &vt_DateTime;
#ifdef NATIVE_DATETIME
    if (type == (PyObject*) PyDateTimeAPI->DateType)
        return &vt_Date;
    if (type == (PyObject*) PyDateTimeAPI->DateTimeType)
        return &vt_DateTime;
#else
    if (type == (PyObject*) &g_ExternalDateTimeVarType)
        return &vt_DateTime;
#endif
    if (type == (PyObject*) &g_TimestampVarType)
        return &vt_Timestamp;
    if (type == (PyObject*) &g_CursorVarType)
        return &vt_Cursor;
#ifdef SQLT_BFLOAT
    if (type == (PyObject*) &g_NativeFloatVarType)
        return &vt_NativeFloat;
#endif

    PyErr_SetString(g_NotSupportedErrorException,
            "Variable_TypeByPythonType(): unhandled data type");
    return NULL;
}


//-----------------------------------------------------------------------------
// Variable_TypeByValue()
//   Return a variable type given a Python object or NULL if the Python
// object does not have a corresponding variable type.
//-----------------------------------------------------------------------------
static udt_VariableType *Variable_TypeByValue(
    PyObject* value,                    // Python type
    ub4* size,                          // size to use (OUT)
    unsigned *numElements)              // number of elements (OUT)
{
    udt_VariableType *varType;
    PyObject *elementValue;
    char buffer[200];
    int i, result;

    // handle scalars
    if (value == Py_None) {
        *size = 1;
        return &vt_String;
    }
    if (cxString_Check(value)) {
        *size = cxString_GetSize(value);
        if (*size > MAX_STRING_CHARS)
            return &vt_LongString;
        return &vt_String;
    }
#ifdef WITH_UNICODE
    if (PyBytes_Check(value)) {
        *size = PyBytes_GET_SIZE(value);
        if (*size > MAX_BINARY_BYTES)
            return &vt_LongBinary;
        return &vt_Binary;
    }
#else
    if (PyUnicode_Check(value)) {
        *size = PyUnicode_GET_SIZE(value);
        return &vt_NationalCharString;
    }
#endif
    if (PyInt_Check(value))
        return &vt_Integer;
    if (PyLong_Check(value))
        return &vt_LongInteger;
    if (PyFloat_Check(value))
        return &vt_Float;
    if (PyBuffer_Check(value)) {
        const void *buffer;
        if (PyObject_AsReadBuffer(value, &buffer, (int*) size) < 0)
            return NULL;
        if (*size > MAX_BINARY_BYTES)
            return &vt_LongBinary;
        return &vt_Binary;
    }
    if (PyBool_Check(value))
        return &vt_Boolean;
#ifdef NATIVE_DATETIME
    if (PyDateTime_Check(value))
        return &vt_DateTime;
    if (PyDate_Check(value))
        return &vt_DateTime;
#else
    if (Py_TYPE(value) == &g_ExternalDateTimeVarType)
        return &vt_DateTime;
#endif
    result = PyObject_IsInstance(value, (PyObject*) &g_CursorType);
    if (result < 0)
        return NULL;
    if (result)
        return &vt_Cursor;
    if (Py_TYPE(value) == g_DateTimeType)
        return &vt_DateTime;
    if (Py_TYPE(value) == g_DecimalType)
        return &vt_NumberAsString;

    // handle arrays
    if (PyList_Check(value)) {
        elementValue = Py_None;
        for (i = 0; i < PyList_GET_SIZE(value); i++) {
            elementValue = PyList_GET_ITEM(value, i);
            if (elementValue != Py_None)
                break;
        }
        varType = Variable_TypeByValue(elementValue, size, numElements);
        if (!varType)
            return NULL;
        *numElements = PyList_GET_SIZE(value);
        *size = varType->elementLength;
        return varType;
    }

    sprintf(buffer, "Variable_TypeByValue(): unhandled data type %.*s", 150,
            Py_TYPE(value)->tp_name);
    PyErr_SetString(g_NotSupportedErrorException, buffer);
    return NULL;
}


//-----------------------------------------------------------------------------
// Variable_TypeByOracleDataType()
//   Return a variable type given an Oracle data type or NULL if the Oracle
// data type does not have a corresponding variable type.
//-----------------------------------------------------------------------------
static udt_VariableType *Variable_TypeByOracleDataType (
    ub2 oracleDataType,                 // Oracle data type
    ub1 charsetForm)                    // character set form
{
    char buffer[100];

    switch(oracleDataType) {
        case SQLT_LNG:
            return &vt_LongString;
        case SQLT_AFC:
#ifndef WITH_UNICODE
            if (charsetForm == SQLCS_NCHAR)
                return &vt_FixedNationalChar;
#endif
            return &vt_FixedChar;
        case SQLT_CHR:
#ifndef WITH_UNICODE
            if (charsetForm == SQLCS_NCHAR)
                return &vt_NationalCharString;
#endif
            return &vt_String;
        case SQLT_RDD:
            return &vt_Rowid;
        case SQLT_BIN:
            return &vt_Binary;
        case SQLT_LBI:
            return &vt_LongBinary;
#ifdef SQLT_BFLOAT
        case SQLT_BFLOAT:
        case SQLT_IBFLOAT:
        case SQLT_BDOUBLE:
        case SQLT_IBDOUBLE:
            return &vt_NativeFloat;
#endif
        case SQLT_NUM:
        case SQLT_VNU:
            return &vt_Float;
        case SQLT_DAT:
        case SQLT_ODT:
            return &vt_DateTime;
        case SQLT_DATE:
        case SQLT_TIMESTAMP:
        case SQLT_TIMESTAMP_TZ:
        case SQLT_TIMESTAMP_LTZ:
            return &vt_Timestamp;
        case SQLT_CLOB:
#ifndef WITH_UNICODE
            if (charsetForm == SQLCS_NCHAR)
                return &vt_NCLOB;
#endif
            return &vt_CLOB;
        case SQLT_BLOB:
            return &vt_BLOB;
        case SQLT_BFILE:
            return &vt_BFILE;
        case SQLT_RSET:
            return &vt_Cursor;
        case SQLT_NTY:
            return &vt_Object;
    }

    sprintf(buffer, "Variable_TypeByOracleDataType: unhandled data type %d",
            oracleDataType);
    PyErr_SetString(g_NotSupportedErrorException, buffer);
    return NULL;
}


//-----------------------------------------------------------------------------
// Variable_TypeByOracleDescriptor()
//   Return a variable type given an Oracle descriptor.
//-----------------------------------------------------------------------------
static udt_VariableType *Variable_TypeByOracleDescriptor(
    OCIParam *param,                    // parameter to get type from
    udt_Environment *environment)       // environment to use
{
    ub1 charsetForm;
    ub2 dataType;
    sword status;

    // retrieve datatype of the parameter
    status = OCIAttrGet(param, OCI_HTYPE_DESCRIBE, (dvoid*) &dataType, 0,
            OCI_ATTR_DATA_TYPE, environment->errorHandle);
    if (Environment_CheckForError(environment, status,
            "Variable_TypeByOracleDescriptor(): data type") < 0)
        return NULL;

    // retrieve character set form of the parameter
    if (dataType != SQLT_CHR && dataType != SQLT_AFC &&
            dataType != SQLT_CLOB) {
        charsetForm = SQLCS_IMPLICIT;
    } else {
        status = OCIAttrGet(param, OCI_HTYPE_DESCRIBE, (dvoid*) &charsetForm,
                0, OCI_ATTR_CHARSET_FORM, environment->errorHandle);
        if (Environment_CheckForError(environment, status,
                "Variable_TypeByOracleDescriptor(): charset form") < 0)
            return NULL;
    }

    return Variable_TypeByOracleDataType(dataType, charsetForm);
}


//-----------------------------------------------------------------------------
// Variable_MakeArray()
//   Make the variable an array, ensuring that the type supports arrays.
//-----------------------------------------------------------------------------
static int Variable_MakeArray(
    udt_Variable *var)                  // variable to make an array
{
    if (!var->type->canBeInArray) {
        PyErr_SetString(g_NotSupportedErrorException,
                "Variable_MakeArray(): type does not support arrays");
        return -1;
    }
    var->isArray = 1;
    return 0;
}


//-----------------------------------------------------------------------------
// Variable_DefaultNewByValue()
//   Default method for determining the type of variable to use for the data.
//-----------------------------------------------------------------------------
static udt_Variable *Variable_DefaultNewByValue(
    udt_Cursor *cursor,                 // cursor to associate variable with
    PyObject *value,                    // Python value to associate
    unsigned numElements)               // number of elements to allocate
{
    udt_VariableType *varType;
    udt_Variable *var;
    ub4 size = 0;

    varType = Variable_TypeByValue(value, &size, &numElements);
    if (!varType)
        return NULL;
    var = Variable_New(cursor, numElements, varType, size);
    if (!var)
        return NULL;
    if (PyList_Check(value)) {
        if (Variable_MakeArray(var) < 0) {
            Py_DECREF(var);
            return NULL;
        }
    }

    return var;
}


//-----------------------------------------------------------------------------
// Variable_NewByInputTypeHandler()
//   Allocate a new variable by looking at the type of the data.
//-----------------------------------------------------------------------------
static udt_Variable *Variable_NewByInputTypeHandler(
    udt_Cursor *cursor,                 // cursor to associate variable with
    PyObject *inputTypeHandler,         // input type handler
    PyObject *value,                    // Python value to associate
    unsigned numElements)               // number of elements to allocate
{
    PyObject *var;

    var = PyObject_CallFunction(inputTypeHandler, "OOi", cursor, value,
            numElements);
    if (!var)
        return NULL;
    if (var != Py_None) {
        if (!Variable_Check(var)) {
            Py_DECREF(var);
            PyErr_SetString(PyExc_TypeError,
                    "expecting variable from input type handler");
            return NULL;
        }
        return (udt_Variable*) var;
    }
    Py_DECREF(var);
    return Variable_DefaultNewByValue(cursor, value, numElements);
}


//-----------------------------------------------------------------------------
// Variable_NewByValue()
//   Allocate a new variable by looking at the type of the data.
//-----------------------------------------------------------------------------
static udt_Variable *Variable_NewByValue(
    udt_Cursor *cursor,                 // cursor to associate variable with
    PyObject *value,                    // Python value to associate
    unsigned numElements)               // number of elements to allocate
{
    if (cursor->inputTypeHandler && cursor->inputTypeHandler != Py_None)
        return Variable_NewByInputTypeHandler(cursor, cursor->inputTypeHandler,
                value, numElements);
    if (cursor->connection->inputTypeHandler &&
            cursor->connection->inputTypeHandler != Py_None)
        return Variable_NewByInputTypeHandler(cursor,
                cursor->connection->inputTypeHandler, value, numElements);
    return Variable_DefaultNewByValue(cursor, value, numElements);
}


//-----------------------------------------------------------------------------
// Variable_NewArrayByType()
//   Allocate a new PL/SQL array by looking at the Python data type.
//-----------------------------------------------------------------------------
static udt_Variable *Variable_NewArrayByType(
    udt_Cursor *cursor,                 // cursor to bind variable to
    PyObject *value)                    // value to bind
{
    PyObject *type, *numElements;
    udt_VariableType *varType;
    udt_Variable *var;

    if (PyList_GET_SIZE(value) != 2) {
        PyErr_SetString(g_ProgrammingErrorException,
                "expecting an array of two elements [type, numelems]");
        return NULL;
    }

    type = PyList_GET_ITEM(value, 0);
    numElements = PyList_GET_ITEM(value, 1);
    if (!PyInt_Check(numElements)) {
        PyErr_SetString(g_ProgrammingErrorException,
                "number of elements must be an integer");
        return NULL;
    }

    varType = Variable_TypeByPythonType(cursor, type);
    if (!varType)
        return NULL;

    var = Variable_New(cursor, PyInt_AS_LONG(numElements), varType,
            varType->elementLength);
    if (!var)
        return NULL;
    if (Variable_MakeArray(var) < 0) {
        Py_DECREF(var);
        return NULL;
    }

    return var;
}


//-----------------------------------------------------------------------------
// Variable_NewByType()
//   Allocate a new variable by looking at the Python data type.
//-----------------------------------------------------------------------------
static udt_Variable *Variable_NewByType(
    udt_Cursor *cursor,                 // cursor to associate variable with
    PyObject *value,                    // Python data type to associate
    unsigned numElements)               // number of elements to allocate
{
    udt_VariableType *varType;
    int maxLength;

    // passing an integer is assumed to be a string
    if (PyInt_Check(value)) {
        maxLength = PyInt_AS_LONG(value);
        if (maxLength > MAX_STRING_CHARS)
            varType = &vt_LongString;
        else varType = &vt_String;
        return Variable_New(cursor, numElements, varType, maxLength);
    }

    // passing an array of two elements to define an array
    if (PyList_Check(value))
        return Variable_NewArrayByType(cursor, value);

    // handle directly bound variables
    if (Variable_Check(value)) {
        Py_INCREF(value);
        return (udt_Variable*) value;
    }

    // everything else ought to be a Python type
    varType = Variable_TypeByPythonType(cursor, value);
    if (!varType)
        return NULL;
    return Variable_New(cursor, numElements, varType, varType->elementLength);
}


//-----------------------------------------------------------------------------
// Variable_NewByOutputTypeHandler()
//   Create a new variable by calling the output type handler.
//-----------------------------------------------------------------------------
static udt_Variable *Variable_NewByOutputTypeHandler(
    udt_Cursor *cursor,                 // cursor to associate variable with
    OCIParam *param,                    // parameter descriptor
    PyObject *outputTypeHandler,        // method to call to get type
    udt_VariableType *varType,          // variable type already chosen
    ub4 maxLength,                      // maximum length of variable
    unsigned numElements)               // number of elements
{
    udt_Variable *var;
    PyObject *result;
    ub4 nameLength;
    sb2 precision;
    sword status;
    char *name;
    sb1 scale;

    // determine name of variable
    status = OCIAttrGet(param, OCI_HTYPE_DESCRIBE, (dvoid*) &name,
            &nameLength, OCI_ATTR_NAME, cursor->environment->errorHandle);
    if (Environment_CheckForError(cursor->environment, status,
            "Variable_NewByOutputTypeHandler(): get name") < 0)
        return NULL;

    // retrieve scale and precision of the parameter, if applicable
    scale = precision = 0;
    if (varType->pythonType == &g_NumberVarType) {
        status = OCIAttrGet(param, OCI_HTYPE_DESCRIBE, (dvoid*) &scale, 0,
                OCI_ATTR_SCALE, cursor->environment->errorHandle);
        if (Environment_CheckForError(cursor->environment, status,
                "Variable_NewByOutputTypeHandler(): get scale") < 0)
            return NULL;
        status = OCIAttrGet(param, OCI_HTYPE_DESCRIBE, (dvoid*) &precision, 0,
                OCI_ATTR_PRECISION, cursor->environment->errorHandle);
        if (Environment_CheckForError(cursor->environment, status,
                "Variable_NewByOutputTypeHandler(): get precision") < 0)
            return NULL;
    }

    // call method, passing parameters
    result = PyObject_CallFunction(outputTypeHandler, "Os#Oiii",
            cursor, name, nameLength, varType->pythonType, maxLength,
            precision, scale);
    if (!result)
        return NULL;

    // if result is None, assume default behavior
    if (result == Py_None) {
        Py_DECREF(result);
        return Variable_New(cursor, numElements, varType, maxLength);
    }

    // otherwise, verify that the result is an actual variable
    if (!Variable_Check(result)) {
        Py_DECREF(result);
        PyErr_SetString(PyExc_TypeError,
                "expecting variable from output type handler");
        return NULL;
    }

    // verify that the array size is sufficient to handle the fetch
    var = (udt_Variable*) result;
    if (var->allocatedElements < cursor->fetchArraySize) {
        Py_DECREF(result);
        PyErr_SetString(PyExc_TypeError,
                "expecting variable with array size large enough for fetch");
        return NULL;
    }

    return var;
}


//-----------------------------------------------------------------------------
// Variable_DefineHelper()
//   Helper routine for Variable_Define() used so that constant calls to
// OCIDescriptorFree() is not necessary.
//-----------------------------------------------------------------------------
static udt_Variable *Variable_DefineHelper(
    udt_Cursor *cursor,                 // cursor in use
    OCIParam *param,                    // parameter descriptor
    unsigned position,                  // position in define list
    unsigned numElements)               // number of elements to create
{
    udt_VariableType *varType;
    ub2 lengthFromOracle;
    udt_Variable *var;
    ub4 maxLength;
    sword status;

    // determine data type
    varType = Variable_TypeByOracleDescriptor(param, cursor->environment);
    if (!varType)
        return NULL;
    if (cursor->numbersAsStrings && varType == &vt_Float)
        varType = &vt_NumberAsString;

    // retrieve size of the parameter
    maxLength = varType->elementLength;
    if (varType->isVariableLength) {

        // determine the maximum length from Oracle
        status = OCIAttrGet(param, OCI_HTYPE_DESCRIBE,
                (dvoid*) &lengthFromOracle, 0, OCI_ATTR_DATA_SIZE,
                cursor->environment->errorHandle);
        if (Environment_CheckForError(cursor->environment, status,
                "Variable_Define(): data size") < 0)
            return NULL;

        // use the length from Oracle directly if available
        if (lengthFromOracle)
            maxLength = lengthFromOracle;

        // otherwise, use the value set with the setoutputsize() parameter but
        // since long strings have the length embedded in them, increase the
        // size by the size of an integer to make things work as expected
        else if (cursor->outputSize >= 0) {
            if (cursor->outputSizeColumn < 0 ||
                    (int) position == cursor->outputSizeColumn)
                maxLength = cursor->outputSize + sizeof(ub4);
        }
    }

    // create a variable of the correct type
    if (cursor->outputTypeHandler && cursor->outputTypeHandler != Py_None)
        var = Variable_NewByOutputTypeHandler(cursor, param,
                cursor->outputTypeHandler, varType, maxLength, numElements);
    else if (cursor->connection->outputTypeHandler &&
            cursor->connection->outputTypeHandler != Py_None)
        var = Variable_NewByOutputTypeHandler(cursor, param,
                cursor->connection->outputTypeHandler, varType, maxLength,
                numElements);
    else var = Variable_New(cursor, numElements, varType, maxLength);
    if (!var)
        return NULL;

    // call the procedure to set values prior to define
    if (var->type->preDefineProc) {
        if ((*var->type->preDefineProc)(var, param) < 0) {
            Py_DECREF(var);
            return NULL;
        }
    }

    // perform the define
    status = OCIDefineByPos(cursor->handle, &var->defineHandle,
            var->environment->errorHandle, position, var->data,
            var->maxLength, var->type->oracleType, var->indicator,
            var->actualLength, var->returnCode, OCI_DEFAULT);
    if (Environment_CheckForError(var->environment, status,
            "Variable_Define(): define") < 0) {
        Py_DECREF(var);
        return NULL;
    }

    // call the procedure to set values after define
    if (var->type->postDefineProc) {
        if ((*var->type->postDefineProc)(var) < 0) {
            Py_DECREF(var);
            return NULL;
        }
    }

    return var;
}


//-----------------------------------------------------------------------------
// Variable_Define()
//   Allocate a variable and define it for the given statement.
//-----------------------------------------------------------------------------
static udt_Variable *Variable_Define(
    udt_Cursor *cursor,                 // cursor to define for
    unsigned numElements,               // number of elements to create
    unsigned position)                  // position to define
{
    udt_Variable *var;
    OCIParam *param;
    sword status;

    // retrieve parameter descriptor
    status = OCIParamGet(cursor->handle, OCI_HTYPE_STMT,
            cursor->environment->errorHandle, (void**) &param, position);
    if (Environment_CheckForError(cursor->environment, status,
            "Variable_Define(): parameter") < 0)
        return NULL;

    // call the helper to do the actual work
    var = Variable_DefineHelper(cursor, param, position, numElements);
    OCIDescriptorFree(param, OCI_DTYPE_PARAM);
    return var;
}


//-----------------------------------------------------------------------------
// Variable_InternalBind()
//   Allocate a variable and bind it to the given statement.
//-----------------------------------------------------------------------------
static int Variable_InternalBind(
    udt_Variable *var)                  // variable to bind
{
    sword status;

    // perform the bind
    if (var->boundName) {
        udt_StringBuffer buffer;
        if (StringBuffer_Fill(&buffer, var->boundName) < 0)
            return -1;
        if (var->isArray) {
            status = OCIBindByName(var->boundCursorHandle, &var->bindHandle,
                    var->environment->errorHandle, (text*) buffer.ptr,
                    buffer.size, var->data, var->maxLength,
                    var->type->oracleType, var->indicator, var->actualLength,
                    var->returnCode, var->allocatedElements,
                    &var->actualElements, OCI_DEFAULT);
        } else {
            status = OCIBindByName(var->boundCursorHandle, &var->bindHandle,
                    var->environment->errorHandle, (text*) buffer.ptr,
                    buffer.size, var->data, var->maxLength,
                    var->type->oracleType, var->indicator, var->actualLength,
                    var->returnCode, 0, 0, OCI_DEFAULT);
        }
        StringBuffer_Clear(&buffer);
    } else {
        if (var->isArray) {
            status = OCIBindByPos(var->boundCursorHandle, &var->bindHandle,
                    var->environment->errorHandle, var->boundPos, var->data,
                    var->maxLength, var->type->oracleType, var->indicator,
                    var->actualLength, var->returnCode, var->allocatedElements,
                    &var->actualElements, OCI_DEFAULT);
        } else {
            status = OCIBindByPos(var->boundCursorHandle, &var->bindHandle,
                    var->environment->errorHandle, var->boundPos, var->data,
                    var->maxLength, var->type->oracleType, var->indicator,
                    var->actualLength, var->returnCode, 0, 0, OCI_DEFAULT);
        }
    }
    if (Environment_CheckForError(var->environment, status,
            "Variable_InternalBind()") < 0)
        return -1;

#ifndef WITH_UNICODE
    // set the charset form and id if applicable
    if (var->type->charsetForm != SQLCS_IMPLICIT) {
        ub2 charsetId = OCI_UTF16ID;
        status = OCIAttrSet(var->bindHandle, OCI_HTYPE_BIND,
                (dvoid*) &var->type->charsetForm, 0, OCI_ATTR_CHARSET_FORM,
                var->environment->errorHandle);
        if (Environment_CheckForError(var->environment, status,
                "Variable_InternalBind(): set charset form") < 0)
            return -1;
        status = OCIAttrSet(var->bindHandle, OCI_HTYPE_BIND,
                 (dvoid*) &charsetId, 0, OCI_ATTR_CHARSET_ID,
                 var->environment->errorHandle);
        if (Environment_CheckForError(var->environment, status,
                 "Variable_InternalBind(): setting charset Id") < 0)
            return -1;
        status = OCIAttrSet(var->bindHandle, OCI_HTYPE_BIND,
                (dvoid*) &var->maxLength, 0, OCI_ATTR_MAXDATA_SIZE,
                var->environment->errorHandle);
        if (Environment_CheckForError(var->environment, status,
                "Variable_InternalBind(): set max data size") < 0)
            return -1;
    }
#endif

    // set the max data size for strings
    if ((var->type == &vt_String || var->type == &vt_FixedChar)
            && var->maxLength > var->type->elementLength) {
        status = OCIAttrSet(var->bindHandle, OCI_HTYPE_BIND,
                (dvoid*) &var->type->elementLength, 0, OCI_ATTR_MAXDATA_SIZE,
                var->environment->errorHandle);
        if (Environment_CheckForError(var->environment, status,
                "Variable_InternalBind(): set max data size") < 0)
            return -1;
    }

    return 0;
}


//-----------------------------------------------------------------------------
// Variable_Bind()
//   Allocate a variable and bind it to the given statement.
//-----------------------------------------------------------------------------
static int Variable_Bind(
    udt_Variable *var,                  // variable to bind
    udt_Cursor *cursor,                 // cursor to bind to
    PyObject *name,                     // name to bind to
    ub4 pos)                            // position to bind to
{
    // nothing to do if already bound
    if (var->bindHandle && name == var->boundName && pos == var->boundPos)
        return 0;

    // set the instance variables specific for binding
    var->boundPos = pos;
    var->boundCursorHandle = cursor->handle;
    Py_XDECREF(var->boundName);
#if defined(WITH_UNICODE) && Py_MAJOR_VERSION < 3
    if (name && PyBytes_Check(name)) {
        var->boundName = PyObject_Unicode(name);
        if (!var->boundName)
            return -1;
        return Variable_InternalBind(var);
    }
#endif
    Py_XINCREF(name);
    var->boundName = name;

    // perform the bind
    return Variable_InternalBind(var);
}


//-----------------------------------------------------------------------------
// Variable_VerifyFetch()
//   Verifies that truncation or other problems did not take place on retrieve.
//-----------------------------------------------------------------------------
static int Variable_VerifyFetch(
  udt_Variable *var,                    // variable to check fetch for
  unsigned arrayPos)                    // array position
{
    udt_Error *error;

    if (var->type->isVariableLength) {
        if (var->returnCode[arrayPos] != 0) {
            error = Error_New(var->environment, "Variable_VerifyFetch()", 0);
            error->code = var->returnCode[arrayPos];
            error->message = PyString_FromFormat(
                    "column at array pos %d fetched with error: %d",
                    arrayPos, var->returnCode[arrayPos]);
            if (!error->message)
                Py_DECREF(error);
            else PyErr_SetObject(g_DatabaseErrorException, (PyObject*) error);
            return -1;
        }
    }
    return 0;
}


//-----------------------------------------------------------------------------
// Variable_GetSingleValue()
//   Return the value of the variable at the given position.
//-----------------------------------------------------------------------------
static PyObject *Variable_GetSingleValue(
    udt_Variable *var,                  // variable to get the value for
    unsigned arrayPos)                  // array position
{
    PyObject *value, *result;
    int isNull;

    // ensure we do not exceed the number of allocated elements
    if (arrayPos >= var->allocatedElements) {
        PyErr_SetString(PyExc_IndexError,
                "Variable_GetSingleValue: array size exceeded");
        return NULL;
    }

    // check for a NULL value
    if (var->type->isNullProc)
        isNull = (*var->type->isNullProc)(var, arrayPos);
    else isNull = (var->indicator[arrayPos] == OCI_IND_NULL);
    if (isNull) {
        Py_INCREF(Py_None);
        return Py_None;
    }

    // check for truncation or other problems on retrieve
    if (Variable_VerifyFetch(var, arrayPos) < 0)
        return NULL;

    // calculate value to return
    value = (*var->type->getValueProc)(var, arrayPos);
    if (value && var->outConverter && var->outConverter != Py_None) {
        result = PyObject_CallFunctionObjArgs(var->outConverter, value, NULL);
        Py_DECREF(value);
        return result;
    }

    return value;
}


//-----------------------------------------------------------------------------
// Variable_GetArrayValue()
//   Return the value of the variable as an array.
//-----------------------------------------------------------------------------
static PyObject *Variable_GetArrayValue(
    udt_Variable *var,                  // variable to get the value for
    ub4 numElements)                    // number of elements to include
{
    PyObject *value, *singleValue;
    ub4 i;

    value = PyList_New(numElements);
    if (!value)
        return NULL;

    for (i = 0; i < numElements; i++) {
        singleValue = Variable_GetSingleValue(var, i);
        if (!singleValue) {
            Py_DECREF(value);
            return NULL;
        }
        PyList_SET_ITEM(value, i, singleValue);
    }

    return value;
}


//-----------------------------------------------------------------------------
// Variable_GetValue()
//   Return the value of the variable.
//-----------------------------------------------------------------------------
static PyObject *Variable_GetValue(
    udt_Variable *var,                  // variable to get the value for
    unsigned arrayPos)                  // array position
{
    if (var->isArray)
        return Variable_GetArrayValue(var, var->actualElements);
    return Variable_GetSingleValue(var, arrayPos);
}


//-----------------------------------------------------------------------------
// Variable_SetSingleValue()
//   Set a single value in the variable.
//-----------------------------------------------------------------------------
static int Variable_SetSingleValue(
    udt_Variable *var,                  // variable to set value for
    unsigned arrayPos,                  // array position
    PyObject *value)                    // value to set
{
    PyObject *convertedValue = NULL;
    int result;

    // ensure we do not exceed the number of allocated elements
    if (arrayPos >= var->allocatedElements) {
        PyErr_SetString(PyExc_IndexError,
                "Variable_SetSingleValue: array size exceeded");
        return -1;
    }

    // convert value, if necessary
    if (var->inConverter && var->inConverter != Py_None) {
        convertedValue = PyObject_CallFunctionObjArgs(var->inConverter, value,
                NULL);
        if (!convertedValue)
            return -1;
        value = convertedValue;
    }

    // check for a NULL value
    if (value == Py_None) {
        var->indicator[arrayPos] = OCI_IND_NULL;
        Py_XDECREF(convertedValue);
        return 0;
    }

    var->indicator[arrayPos] = OCI_IND_NOTNULL;
    if (var->type->isVariableLength)
        var->returnCode[arrayPos] = 0;
    result = (*var->type->setValueProc)(var, arrayPos, value);
    Py_XDECREF(convertedValue);
    return result;
}


//-----------------------------------------------------------------------------
// Variable_SetArrayValue()
//   Set all of the array values for the variable.
//-----------------------------------------------------------------------------
static int Variable_SetArrayValue(
    udt_Variable *var,                  // variable to set value for
    PyObject *value)                    // value to set
{
    unsigned numElements;
    ub4 i;

    // ensure we have an array to set
    if (!PyList_Check(value)) {
        PyErr_SetString(PyExc_TypeError, "expecting array data");
        return -1;
    }

    // ensure we haven't exceeded the number of allocated elements
    numElements = PyList_GET_SIZE(value);
    if (numElements > var->allocatedElements) {
        PyErr_SetString(PyExc_IndexError,
                "Variable_SetArrayValue: array size exceeded");
        return -1;
    }

    // set all of the values
    var->actualElements = numElements;
    for (i = 0; i < var->actualElements; i++) {
        if (Variable_SetSingleValue(var, i, PyList_GET_ITEM(value, i)) < 0)
            return -1;
    }
    return 0;
}


//-----------------------------------------------------------------------------
// Variable_SetValue()
//   Set the value of the variable.
//-----------------------------------------------------------------------------
static int Variable_SetValue(
    udt_Variable *var,                  // variable to set
    unsigned arrayPos,                  // array position
    PyObject *value)                    // value to set
{
    if (var->isArray)
        return Variable_SetArrayValue(var, value);
    return Variable_SetSingleValue(var, arrayPos, value);
}


//-----------------------------------------------------------------------------
// Variable_ExternalCopy()
//   Copy the contents of the source variable to the destination variable.
//-----------------------------------------------------------------------------
static PyObject *Variable_ExternalCopy(
    udt_Variable *targetVar,            // variable to set
    PyObject *args)                     // arguments
{
    unsigned sourcePos, targetPos;
    udt_Variable *sourceVar;

    // parse arguments; verify that copy is possible
    if (!PyArg_ParseTuple(args, "Oii", &sourceVar, &sourcePos, &targetPos))
        return NULL;
    if (Py_TYPE(targetVar) != Py_TYPE(sourceVar)) {
        PyErr_SetString(g_ProgrammingErrorException,
                "source and target variable type must match");
        return NULL;
    }
    if (!sourceVar->type->canBeCopied) {
        PyErr_SetString(g_ProgrammingErrorException,
                "variable does not support copying");
        return NULL;
    }

    // ensure array positions are not violated
    if (sourcePos >= sourceVar->allocatedElements) {
        PyErr_SetString(PyExc_IndexError,
                "Variable_ExternalCopy: source array size exceeded");
        return NULL;
    }
    if (targetPos >= targetVar->allocatedElements) {
        PyErr_SetString(PyExc_IndexError,
                "Variable_ExternalCopy: target array size exceeded");
        return NULL;
    }

    // ensure target can support amount data from the source
    if (targetVar->maxLength < sourceVar->maxLength) {
        PyErr_SetString(g_ProgrammingErrorException,
                "target variable has insufficient space to copy source data");
        return NULL;
    }

    // handle null case directly
    if (sourceVar->indicator[sourcePos] == OCI_IND_NULL)
        targetVar->indicator[targetPos] = OCI_IND_NULL;

    // otherwise, copy data
    else {
        targetVar->indicator[targetPos] = OCI_IND_NOTNULL;
        if (Variable_VerifyFetch(sourceVar, sourcePos) < 0)
            return NULL;
        if (targetVar->actualLength)
            targetVar->actualLength[targetPos] =
                    sourceVar->actualLength[sourcePos];
        if (targetVar->returnCode)
            targetVar->returnCode[targetPos] =
                    sourceVar->returnCode[sourcePos];
        memcpy( (char*) targetVar->data + targetPos * targetVar->maxLength,
                (char*) sourceVar->data + sourcePos * sourceVar->maxLength,
                sourceVar->maxLength);
    }

    Py_INCREF(Py_None);
    return Py_None;
}


//-----------------------------------------------------------------------------
// Variable_ExternalSetValue()
//   Set the value of the variable at the given position.
//-----------------------------------------------------------------------------
static PyObject *Variable_ExternalSetValue(
    udt_Variable *var,                  // variable to set
    PyObject *args)                     // arguments
{
    PyObject *value;
    unsigned pos;

    if (!PyArg_ParseTuple(args, "iO", &pos, &value))
      return NULL;
    if (Variable_SetValue(var, pos, value) < 0)
      return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}


//-----------------------------------------------------------------------------
// Variable_ExternalGetValue()
//   Return the value of the variable at the given position.
//-----------------------------------------------------------------------------
static PyObject *Variable_ExternalGetValue(
    udt_Variable *var,                  // variable to set
    PyObject *args,                     // arguments
    PyObject *keywordArgs)              // keyword arguments
{
    static char *keywordList[] = { "pos", NULL };
    unsigned pos = 0;

    if (!PyArg_ParseTupleAndKeywords(args, keywordArgs, "|i", keywordList,
            &pos))
        return NULL;
    return Variable_GetValue(var, pos);
}


//-----------------------------------------------------------------------------
// Variable_Repr()
//   Return a string representation of the variable.
//-----------------------------------------------------------------------------
static PyObject *Variable_Repr(
    udt_Variable *var)                  // variable to return the string for
{
    PyObject *valueRepr, *value, *module, *name, *result;

    if (var->isArray)
        value = Variable_GetArrayValue(var, var->actualElements);
    else if (var->allocatedElements == 1)
        value = Variable_GetSingleValue(var, 0);
    else value = Variable_GetArrayValue(var, var->allocatedElements);
    if (!value)
        return NULL;
    valueRepr = PyObject_Repr(value);
    Py_DECREF(value);
    if (!valueRepr)
        return NULL;
    if (GetModuleAndName(Py_TYPE(var), &module, &name) < 0) {
        Py_DECREF(valueRepr);
        return NULL;
    }
    result = PyString_FromFormat("<%s.%s with value %s>",
            PyString_AS_STRING(module), PyString_AS_STRING(name),
            PyString_AS_STRING(valueRepr));
    Py_DECREF(valueRepr);
    Py_DECREF(module);
    Py_DECREF(name);
    return result;
}

