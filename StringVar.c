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
} udt_StringVar;


//-----------------------------------------------------------------------------
// Declaration of string variable functions.
//-----------------------------------------------------------------------------
static int StringVar_Initialize(udt_StringVar*, udt_Cursor*);
static int StringVar_PostDefine(udt_StringVar*);
static int StringVar_SetValue(udt_StringVar*, unsigned, PyObject*);
static PyObject *StringVar_GetValue(udt_StringVar*, unsigned);

//-----------------------------------------------------------------------------
// Python type declarations
//-----------------------------------------------------------------------------
static PyTypeObject g_StringVarType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "cx_Oracle.STRING",                 // tp_name
    sizeof(udt_StringVar),              // tp_basicsize
    0,                                  // tp_itemsize
    0,                                  // tp_dealloc
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


static PyTypeObject g_UnicodeVarType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "cx_Oracle.UNICODE",                // tp_name
    sizeof(udt_StringVar),              // tp_basicsize
    0,                                  // tp_itemsize
    0,                                  // tp_dealloc
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


static PyTypeObject g_FixedCharVarType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "cx_Oracle.FIXED_CHAR",             // tp_name
    sizeof(udt_StringVar),              // tp_basicsize
    0,                                  // tp_itemsize
    0,                                  // tp_dealloc
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


static PyTypeObject g_FixedUnicodeVarType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "cx_Oracle.FIXED_UNICODE",          // tp_name
    sizeof(udt_StringVar),              // tp_basicsize
    0,                                  // tp_itemsize
    0,                                  // tp_dealloc
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


static PyTypeObject g_RowidVarType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "cx_Oracle.ROWID",                  // tp_name
    sizeof(udt_StringVar),              // tp_basicsize
    0,                                  // tp_itemsize
    0,                                  // tp_dealloc
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


static PyTypeObject g_BinaryVarType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "cx_Oracle.BINARY",                 // tp_name
    sizeof(udt_StringVar),              // tp_basicsize
    0,                                  // tp_itemsize
    0,                                  // tp_dealloc
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


//-----------------------------------------------------------------------------
// variable type declarations
//-----------------------------------------------------------------------------
static udt_VariableType vt_String = {
    (InitializeProc) StringVar_Initialize,
    (FinalizeProc) NULL,
    (PreDefineProc) NULL,
    (PostDefineProc) NULL,
    (IsNullProc) NULL,
    (SetValueProc) StringVar_SetValue,
    (GetValueProc) StringVar_GetValue,
    &g_StringVarType,                   // Python type
    SQLT_CHR,                           // Oracle type
    SQLCS_IMPLICIT,                     // charset form
    MAX_STRING_CHARS,                   // element length (default)
    1,                                  // is variable length
    1,                                  // can be copied
    1                                   // can be in array
};


static udt_VariableType vt_NationalCharString = {
    (InitializeProc) StringVar_Initialize,
    (FinalizeProc) NULL,
    (PreDefineProc) NULL,
    (PostDefineProc) StringVar_PostDefine,
    (IsNullProc) NULL,
    (SetValueProc) StringVar_SetValue,
    (GetValueProc) StringVar_GetValue,
    &g_UnicodeVarType,                  // Python type
    SQLT_CHR,                           // Oracle type
    SQLCS_NCHAR,                        // charset form
    MAX_STRING_CHARS,                   // element length (default)
    1,                                  // is variable length
    1,                                  // can be copied
    1                                   // can be in array
};


static udt_VariableType vt_FixedChar = {
    (InitializeProc) StringVar_Initialize,
    (FinalizeProc) NULL,
    (PreDefineProc) NULL,
    (PostDefineProc) NULL,
    (IsNullProc) NULL,
    (SetValueProc) StringVar_SetValue,
    (GetValueProc) StringVar_GetValue,
    &g_FixedCharVarType,                // Python type
    SQLT_AFC,                           // Oracle type
    SQLCS_IMPLICIT,                     // charset form
    2000,                               // element length (default)
    1,                                  // is variable length
    1,                                  // can be copied
    1                                   // can be in array
};


static udt_VariableType vt_FixedNationalChar = {
    (InitializeProc) StringVar_Initialize,
    (FinalizeProc) NULL,
    (PreDefineProc) NULL,
    (PostDefineProc) StringVar_PostDefine,
    (IsNullProc) NULL,
    (SetValueProc) StringVar_SetValue,
    (GetValueProc) StringVar_GetValue,
    &g_FixedUnicodeVarType,             // Python type
    SQLT_AFC,                           // Oracle type
    SQLCS_NCHAR,                        // charset form
    2000,                               // element length (default)
    1,                                  // is variable length
    1,                                  // can be copied
    1                                   // can be in array
};


static udt_VariableType vt_Rowid = {
    (InitializeProc) StringVar_Initialize,
    (FinalizeProc) NULL,
    (PreDefineProc) NULL,
    (PostDefineProc) NULL,
    (IsNullProc) NULL,
    (SetValueProc) StringVar_SetValue,
    (GetValueProc) StringVar_GetValue,
    &g_RowidVarType,                    // Python type
    SQLT_CHR,                           // Oracle type
    SQLCS_IMPLICIT,                     // charset form
    18,                                 // element length (default)
    0,                                  // is variable length
    1,                                  // can be copied
    1                                   // can be in array
};


static udt_VariableType vt_Binary = {
    (InitializeProc) StringVar_Initialize,
    (FinalizeProc) NULL,
    (PreDefineProc) NULL,
    (PostDefineProc) NULL,
    (IsNullProc) NULL,
    (SetValueProc) StringVar_SetValue,
    (GetValueProc) StringVar_GetValue,
    &g_BinaryVarType,                   // Python type
    SQLT_BIN,                           // Oracle type
    SQLCS_IMPLICIT,                     // charset form
    MAX_STRING_CHARS,                   // element length (default)
    1,                                  // is variable length
    1,                                  // can be copied
    1                                   // can be in array
};


//-----------------------------------------------------------------------------
// StringVar_Initialize()
//   Initialize the variable.
//-----------------------------------------------------------------------------
static int StringVar_Initialize(
    udt_StringVar *var,                 // variable to initialize
    udt_Cursor *cursor)                 // cursor to use
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
// StringVar_PostDefine()
//   Set the character set information when values are fetched from this
// variable.
//-----------------------------------------------------------------------------
static int StringVar_PostDefine(
    udt_StringVar *var)                 // variable to initialize
{
    ub2 charsetId;
    sword status;

    status = OCIAttrSet(var->defineHandle, OCI_HTYPE_DEFINE,
            &var->type->charsetForm, 0, OCI_ATTR_CHARSET_FORM,
            var->environment->errorHandle);
    if (Environment_CheckForError(var->environment, status,
            "StringVar_PostDefine(): setting charset form") < 0)
        return -1;

    charsetId = OCI_UTF16ID;
    status = OCIAttrSet(var->defineHandle, OCI_HTYPE_DEFINE, &charsetId, 0,
            OCI_ATTR_CHARSET_ID, var->environment->errorHandle);
    if (Environment_CheckForError(var->environment, status,
            "StringVar_PostDefine(): setting charset id") < 0)
        return -1;

    return 0;
}


//-----------------------------------------------------------------------------
// StringVar_SetValue()
//   Set the value of the variable.
//-----------------------------------------------------------------------------
static int StringVar_SetValue(
    udt_StringVar *var,                 // variable to set value for
    unsigned pos,                       // array position to set
    PyObject *value)                    // value to set
{
    PyObject *encodedString;
    Py_ssize_t bufferSize;
    const void *buffer;
    ub2 actualLength;

    // get the buffer data and size for binding
    encodedString = NULL;
    if (var->type->charsetForm == SQLCS_IMPLICIT) {
        if (PyString_Check(value)) {
            buffer = PyString_AS_STRING(value);
            bufferSize = PyString_GET_SIZE(value);
        } else if (PyBuffer_Check(value)) {
            if (PyObject_AsReadBuffer(value, &buffer, &bufferSize) < 0)
                return -1;
        } else {
            PyErr_SetString(PyExc_TypeError,
                    "expecting string or buffer data");
            return -1;
        }
        actualLength = (ub2) bufferSize;
    } else {
        if (!PyUnicode_Check(value)) {
            PyErr_SetString(PyExc_TypeError, "expecting unicode data");
            return -1;
        }
#ifdef Py_UNICODE_WIDE
        int one = 1;
        int byteOrder = (IS_LITTLE_ENDIAN) ? -1 : 1;
        encodedString = PyUnicode_EncodeUTF16(PyUnicode_AS_UNICODE(value),
                PyUnicode_GET_SIZE(value), NULL, byteOrder);
        if (!encodedString)
            return -1;
        buffer = PyString_AS_STRING(encodedString);
        bufferSize = PyString_GET_SIZE(encodedString);
#else
        buffer = PyUnicode_AS_UNICODE(value);
        bufferSize = sizeof(Py_UNICODE) * PyUnicode_GET_SIZE(value);
#endif
        actualLength = bufferSize / 2;
    }

    // ensure that the buffer is not too large
    if (bufferSize > var->maxLength) {
        if (bufferSize > var->environment->maxStringBytes) {
            PyErr_SetString(PyExc_ValueError, "string data too large");
            Py_XDECREF(encodedString);
            return -1;
        }
        if (Variable_Resize( (udt_Variable*) var, bufferSize) < 0) {
            Py_XDECREF(encodedString);
            return -1;
        }
    }

    // keep a copy of the string
    var->actualLength[pos] = actualLength;
    if (bufferSize)
        memcpy(var->data + var->maxLength * pos, buffer, bufferSize);
    Py_XDECREF(encodedString);

    return 0;
}


//-----------------------------------------------------------------------------
// StringVar_GetValue()
//   Returns the value stored at the given array position.
//-----------------------------------------------------------------------------
static PyObject *StringVar_GetValue(
    udt_StringVar *var,                 // variable to determine value for
    unsigned pos)                       // array position
{
    char *data;

    data = var->data + pos * var->maxLength;
#ifdef WTIH_UNICODE
    if (var->type->charsetForm == SQLCS_IMPLICIT)
#else
    if (var->type == &vt_Binary)
#endif
        return PyString_FromStringAndSize(data, var->actualLength[pos]);
    return CXORA_TO_STRING_OBJ(data, var->actualLength[pos]);
}

