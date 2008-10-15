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
#ifndef WITH_UNICODE
static int StringVar_PostDefine(udt_StringVar*);
#endif
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


#ifndef WITH_UNICODE
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
#endif


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


#ifndef WITH_UNICODE
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
#endif


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
    1,                                  // is character data
    1,                                  // is variable length
    1,                                  // can be copied
    1                                   // can be in array
};


#ifndef WITH_UNICODE
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
    1,                                  // is character data
    1,                                  // is variable length
    1,                                  // can be copied
    1                                   // can be in array
};
#endif


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
    1,                                  // is character data
    1,                                  // is variable length
    1,                                  // can be copied
    1                                   // can be in array
};


#ifndef WITH_UNICODE
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
    1,                                  // is character data
    1,                                  // is variable length
    1,                                  // can be copied
    1                                   // can be in array
};
#endif


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
    1,                                  // is character data
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
    MAX_BINARY_BYTES,                   // element length (default)
    0,                                  // is character data
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


#ifndef WITH_UNICODE
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
#endif


//-----------------------------------------------------------------------------
// StringVar_SetValue()
//   Set the value of the variable.
//-----------------------------------------------------------------------------
static int StringVar_SetValue(
    udt_StringVar *var,                 // variable to set value for
    unsigned pos,                       // array position to set
    PyObject *value)                    // value to set
{
    udt_StringBuffer buffer;

    // get the buffer data and size for binding
    StringBuffer_Init(&buffer);
#ifdef WITH_UNICODE
    if (!var->type->isCharacterData) {
#else
    if (var->type->charsetForm == SQLCS_IMPLICIT) {
#endif
        if (PyBytes_Check(value)) {
            StringBuffer_FromBytes(&buffer, value);
        } else if (PyBuffer_Check(value)) {
            if (PyObject_AsReadBuffer(value, &buffer.ptr, &buffer.size) < 0)
                return -1;
        } else {
            PyErr_SetString(PyExc_TypeError,
                    "expecting string or buffer data");
            return -1;
        }
        if (var->type->isCharacterData
                && buffer.size > var->environment->maxStringBytes) {
            StringBuffer_Clear(&buffer);
            PyErr_SetString(PyExc_ValueError, "string data too large");
            return -1;
        } else if (!var->type->isCharacterData
                && buffer.size > MAX_BINARY_BYTES) {
            StringBuffer_Clear(&buffer);
            PyErr_SetString(PyExc_ValueError, "binary data too large");
            return -1;
        }
    } else {
        if (!PyUnicode_Check(value)) {
            PyErr_SetString(PyExc_TypeError, "expecting unicode data");
            return -1;
        }
        if (PyUnicode_GET_SIZE(value) > MAX_STRING_CHARS) {
            PyErr_SetString(PyExc_ValueError, "unicode data too large");
            return -1;
        }
        if (StringBuffer_FromUnicode(&buffer, value) < 0)
            return -1;
    }

    // ensure that the buffer is not too large
    if (buffer.size > var->maxLength) {
        if (Variable_Resize( (udt_Variable*) var, buffer.size) < 0) {
            StringBuffer_Clear(&buffer);
            return -1;
        }
    }

    // keep a copy of the string
    var->actualLength[pos] = (ub2) buffer.size;
    if (buffer.size)
        memcpy(var->data + var->maxLength * pos, buffer.ptr, buffer.size);
    StringBuffer_Clear(&buffer);

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
#ifdef WITH_UNICODE
    if (var->type == &vt_Binary)
        return PyBytes_FromStringAndSize(data, var->actualLength[pos]);
    return cxString_FromEncodedString(data, var->actualLength[pos]);
#else
    if (var->type->charsetForm == SQLCS_IMPLICIT)
        return PyBytes_FromStringAndSize(data, var->actualLength[pos]);
    #ifdef Py_UNICODE_WIDE
    return PyUnicode_DecodeUTF16(data, var->actualLength[pos], NULL, NULL);
    #else
    return PyUnicode_FromUnicode((Py_UNICODE*) data,
            var->actualLength[pos] / 2);
    #endif
#endif
}

