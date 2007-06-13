//-----------------------------------------------------------------------------
// StringVar.c
//   Defines the routines specific to the string type.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// String type
//-----------------------------------------------------------------------------
typedef struct {
    Variable_HEAD
    char *data;
    ub1 charsetForm;
    ub2 charsetId;
} udt_StringVar;


//-----------------------------------------------------------------------------
// Declaration of string variable functions.
//-----------------------------------------------------------------------------
static int StringVar_Initialize(udt_StringVar*, udt_Cursor*);
static int StringVar_PreDefine(udt_StringVar*, OCIParam*);
static int StringVar_SetValue(udt_StringVar*, unsigned, PyObject*);
static PyObject *StringVar_GetValue(udt_StringVar*, unsigned);

//-----------------------------------------------------------------------------
// Python type declarations
//-----------------------------------------------------------------------------
static PyTypeObject g_StringVarType = {
    PyObject_HEAD_INIT(NULL)
    0,					// ob_size
    "cx_Oracle.STRING",			// tp_name
    sizeof(udt_StringVar),		// tp_basicsize
    0,					// tp_itemsize
    (destructor) Variable_Free,		// tp_dealloc
    0,					// tp_print
    0,					// tp_getattr
    0,					// tp_setattr
    0,					// tp_compare
    (reprfunc) Variable_Repr,		// tp_repr
    0,					// tp_as_number
    0,					// tp_as_sequence
    0,					// tp_as_mapping
    0,					// tp_hash
    0,					// tp_call
    0,					// tp_str
    (getattrofunc) Variable_GetAttr,	// tp_getattro
    0,					// tp_setattro
    0,					// tp_as_buffer
    Py_TPFLAGS_DEFAULT,			// tp_flags
    0					// tp_doc
};


static PyTypeObject g_FixedCharVarType = {
    PyObject_HEAD_INIT(NULL)
    0,					// ob_size
    "cx_Oracle.FIXED_CHAR",		// tp_name
    sizeof(udt_StringVar),		// tp_basicsize
    0,					// tp_itemsize
    (destructor) Variable_Free,		// tp_dealloc
    0,					// tp_print
    0,					// tp_getattr
    0,					// tp_setattr
    0,					// tp_compare
    (reprfunc) Variable_Repr,		// tp_repr
    0,					// tp_as_number
    0,					// tp_as_sequence
    0,					// tp_as_mapping
    0,					// tp_hash
    0,					// tp_call
    0,					// tp_str
    (getattrofunc) Variable_GetAttr,	// tp_getattro
    0,					// tp_setattro
    0,					// tp_as_buffer
    Py_TPFLAGS_DEFAULT,			// tp_flags
    0					// tp_doc
};


static PyTypeObject g_RowidVarType = {
    PyObject_HEAD_INIT(NULL)
    0,					// ob_size
    "cx_Oracle.ROWID",			// tp_name
    sizeof(udt_StringVar),		// tp_basicsize
    0,					// tp_itemsize
    (destructor) Variable_Free,		// tp_dealloc
    0,					// tp_print
    0,					// tp_getattr
    0,					// tp_setattr
    0,					// tp_compare
    (reprfunc) Variable_Repr,		// tp_repr
    0,					// tp_as_number
    0,					// tp_as_sequence
    0,					// tp_as_mapping
    0,					// tp_hash
    0,					// tp_call
    0,					// tp_str
    (getattrofunc) Variable_GetAttr,	// tp_getattro
    0,					// tp_setattro
    0,					// tp_as_buffer
    Py_TPFLAGS_DEFAULT,			// tp_flags
    0					// tp_doc
};


static PyTypeObject g_BinaryVarType = {
    PyObject_HEAD_INIT(NULL)
    0,					// ob_size
    "cx_Oracle.BINARY",			// tp_name
    sizeof(udt_StringVar),		// tp_basicsize
    0,					// tp_itemsize
    (destructor) Variable_Free,		// tp_dealloc
    0,					// tp_print
    0,					// tp_getattr
    0,					// tp_setattr
    0,					// tp_compare
    (reprfunc) Variable_Repr,		// tp_repr
    0,					// tp_as_number
    0,					// tp_as_sequence
    0,					// tp_as_mapping
    0,					// tp_hash
    0,					// tp_call
    0,					// tp_str
    (getattrofunc) Variable_GetAttr,	// tp_getattro
    0,					// tp_setattro
    0,					// tp_as_buffer
    Py_TPFLAGS_DEFAULT,			// tp_flags
    0					// tp_doc
};


//-----------------------------------------------------------------------------
// variable type declarations
//-----------------------------------------------------------------------------
static udt_VariableType vt_String = {
    (InitializeProc) StringVar_Initialize,
    (FinalizeProc) NULL,
    (PreDefineProc) StringVar_PreDefine,
    (PostDefineProc) NULL,
    (IsNullProc) NULL,
    (SetValueProc) StringVar_SetValue,
    (GetValueProc) StringVar_GetValue,
    &g_StringVarType,			// Python type
    SQLT_CHR,				// Oracle type
    SQLCS_IMPLICIT,			// charset form
    MAX_STRING_CHARS,			// element length (default)
    1,					// is variable length
    1,					// can be copied
    1					// can be in array
};


static udt_VariableType vt_NationalCharString = {
    (InitializeProc) StringVar_Initialize,
    (FinalizeProc) NULL,
    (PreDefineProc) StringVar_PreDefine,
    (PostDefineProc) NULL,
    (IsNullProc) NULL,
    (SetValueProc) StringVar_SetValue,
    (GetValueProc) StringVar_GetValue,
    &g_StringVarType,			// Python type
    SQLT_CHR,				// Oracle type
    SQLCS_NCHAR,			// charset form
    MAX_STRING_CHARS,			// element length (default)
    1,					// is variable length
    1,					// can be copied
    1					// can be in array
};


static udt_VariableType vt_FixedChar = {
    (InitializeProc) StringVar_Initialize,
    (FinalizeProc) NULL,
    (PreDefineProc) StringVar_PreDefine,
    (PostDefineProc) NULL,
    (IsNullProc) NULL,
    (SetValueProc) StringVar_SetValue,
    (GetValueProc) StringVar_GetValue,
    &g_FixedCharVarType,		// Python type
    SQLT_AFC,				// Oracle type
    SQLCS_IMPLICIT,			// charset form
    2000,				// element length (default)
    1,					// is variable length
    1,					// can be copied
    1					// can be in array
};


static udt_VariableType vt_Rowid = {
    (InitializeProc) StringVar_Initialize,
    (FinalizeProc) NULL,
    (PreDefineProc) NULL,
    (PostDefineProc) NULL,
    (IsNullProc) NULL,
    (SetValueProc) StringVar_SetValue,
    (GetValueProc) StringVar_GetValue,
    &g_RowidVarType,			// Python type
    SQLT_CHR,				// Oracle type
    SQLCS_IMPLICIT,			// charset form
    18,					// element length (default)
    0,					// is variable length
    1,					// can be copied
    1					// can be in array
};


static udt_VariableType vt_Binary = {
    (InitializeProc) StringVar_Initialize,
    (FinalizeProc) NULL,
    (PreDefineProc) NULL,
    (PostDefineProc) NULL,
    (IsNullProc) NULL,
    (SetValueProc) StringVar_SetValue,
    (GetValueProc) StringVar_GetValue,
    &g_BinaryVarType,			// Python type
    SQLT_BIN,				// Oracle type
    SQLCS_IMPLICIT,			// charset form
    MAX_STRING_CHARS,			// element length (default)
    1,					// is variable length
    1,					// can be copied
    1					// can be in array
};


//-----------------------------------------------------------------------------
// StringVar_Initialize()
//   Initialize the variable.
//-----------------------------------------------------------------------------
static int StringVar_Initialize(
    udt_StringVar *var,			// variable to initialize
    udt_Cursor *cursor)			// cursor to use
{
    var->actualLength = (ub2*) PyMem_Malloc(var->allocatedElements *
            sizeof(ub2));
    if (!var->actualLength) {
        PyErr_NoMemory();
        return -1;
    }
    return 0;
}


//-----------------------------------------------------------------------------
// StringVar_PreDefine()
//   Set the character set information when values are fetched from this
// variable.
//-----------------------------------------------------------------------------
static int StringVar_PreDefine(
    udt_StringVar *var,			// variable to initialize
    OCIParam *param)			// parameter handle
{
    sword status;

    status = OCIAttrGet(param, OCI_HTYPE_DESCRIBE, (dvoid*) &var->charsetForm,
            0, OCI_ATTR_CHARSET_FORM, var->environment->errorHandle);
    if (Environment_CheckForError(var->environment, status,
            "StringVar_PreDefine(): getting charset form") < 0)
        return -1;
    status = OCIAttrGet(param, OCI_HTYPE_DESCRIBE, (dvoid*) &var->charsetId, 0,
            OCI_ATTR_CHARSET_ID, var->environment->errorHandle);
    if (Environment_CheckForError(var->environment, status,
            "StringVar_PreDefine(): getting charset id") < 0)
        return -1;

    return 0;
}


//-----------------------------------------------------------------------------
// StringVar_SetValue()
//   Set the value of the variable.
//-----------------------------------------------------------------------------
static int StringVar_SetValue(
    udt_StringVar *var,			// variable to set value for
    unsigned pos,			// array position to set
    PyObject *value)			// value to set
{
    Py_ssize_t bufferSize;
    const void *buffer;

    // get the buffer data and size for binding
    if (PyString_Check(value)) {
        buffer = PyString_AS_STRING(value);
        bufferSize = PyString_GET_SIZE(value);
    } else if (PyBuffer_Check(value)) {
        if (PyObject_AsReadBuffer(value, &buffer, &bufferSize) < 0)
            return -1;
    } else {
        PyErr_SetString(PyExc_TypeError, "expecting string or buffer data");
        return -1;
    }

    // ensure that the buffer is not too large
    if (bufferSize > var->maxLength) {
        if (bufferSize > var->environment->maxStringBytes) {
            PyErr_SetString(PyExc_ValueError, "string data too large");
            return -1;
        }
        if (Variable_Resize( (udt_Variable*) var, bufferSize) < 0)
            return -1;
    }

    // keep a copy of the string
    var->actualLength[pos] = (ub2) bufferSize;
    if (bufferSize)
        memcpy(var->data + var->maxLength * pos, buffer, bufferSize);

    return 0;
}


//-----------------------------------------------------------------------------
// StringVar_GetValue()
//   Returns the value stored at the given array position.
//-----------------------------------------------------------------------------
static PyObject *StringVar_GetValue(
    udt_StringVar *var,			// variable to determine value for
    unsigned pos)			// array position
{
    return PyString_FromStringAndSize(var->data + pos * var->maxLength,
            var->actualLength[pos]);
}

