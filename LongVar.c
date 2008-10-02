//-----------------------------------------------------------------------------
// LongVar.c
//   Defines the routines specific to the long type.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// long type
//-----------------------------------------------------------------------------
typedef struct {
    Variable_HEAD
    char *data;
} udt_LongVar;


//-----------------------------------------------------------------------------
// declaration of long variable functions.
//-----------------------------------------------------------------------------
static int LongVar_SetValue(udt_LongVar*, unsigned, PyObject*);
static PyObject *LongVar_GetValue(udt_LongVar*, unsigned);


//-----------------------------------------------------------------------------
// Python type declarations
//-----------------------------------------------------------------------------
static PyTypeObject g_LongStringVarType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "cx_Oracle.LONG_STRING",            // tp_name
    sizeof(udt_LongVar),                // tp_basicsize
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


static PyTypeObject g_LongBinaryVarType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "cx_Oracle.LONG_BINARY",            // tp_name
    sizeof(udt_LongVar),                // tp_basicsize
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
static udt_VariableType vt_LongString = {
    (InitializeProc) NULL,
    (FinalizeProc) NULL,
    (PreDefineProc) NULL,
    (PostDefineProc) NULL,
    (IsNullProc) NULL,
    (SetValueProc) LongVar_SetValue,
    (GetValueProc) LongVar_GetValue,
    &g_LongStringVarType,               // Python type
    SQLT_LVC,                           // Oracle type
    SQLCS_IMPLICIT,                     // charset form
    128 * 1024,                         // element length (default)
    1,                                  // is variable length
    1,                                  // can be copied
    0                                   // can be in array
};


static udt_VariableType vt_LongBinary = {
    (InitializeProc) NULL,
    (FinalizeProc) NULL,
    (PreDefineProc) NULL,
    (PostDefineProc) NULL,
    (IsNullProc) NULL,
    (SetValueProc) LongVar_SetValue,
    (GetValueProc) LongVar_GetValue,
    &g_LongBinaryVarType,               // Python type
    SQLT_LVB,                           // Oracle type
    SQLCS_IMPLICIT,                     // charset form
    128 * 1024,                         // element length (default)
    1,                                  // is variable length
    1,                                  // can be copied
    0                                   // can be in array
};


//-----------------------------------------------------------------------------
// LongVar_SetValue()
//   Set the value of the variable.
//-----------------------------------------------------------------------------
static int LongVar_SetValue(
    udt_LongVar *var,                   // variable to set value for
    unsigned pos,                       // array position to set
    PyObject *value)                    // value to set
{
    Py_ssize_t bufferSize;
    const void *buffer;
    char *ptr;

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

    // copy the string to the Oracle buffer
    if (bufferSize + sizeof(ub4) > var->maxLength) {
        if (Variable_Resize( (udt_Variable*) var,
                bufferSize + sizeof(ub4)) < 0)
            return -1;
    }
    ptr = var->data + var->maxLength * pos;
    *((ub4 *) ptr) = (ub4) bufferSize;
    if (bufferSize)
        memcpy(ptr + sizeof(ub4), buffer, bufferSize);

    return 0;
}


//-----------------------------------------------------------------------------
// LongVar_GetValue()
//   Returns the value stored at the given array position.
//-----------------------------------------------------------------------------
static PyObject *LongVar_GetValue(
    udt_LongVar *var,                   // variable to determine value for
    unsigned pos)                       // array position
{
    char *ptr;
    ub4 size;

    ptr = var->data + var->maxLength * pos;
    size = *((ub4 *) ptr);
    return PyString_FromStringAndSize(ptr + sizeof(ub4), size);
}

