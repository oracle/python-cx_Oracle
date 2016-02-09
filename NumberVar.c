//-----------------------------------------------------------------------------
// NumberVar.c
//   Defines the routines for handling numeric variables.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Number types
//-----------------------------------------------------------------------------
typedef struct {
    Variable_HEAD
    OCINumber *data;
} udt_NumberVar;


typedef struct {
    Variable_HEAD
    double *data;
} udt_NativeFloatVar;


typedef struct {
    Variable_HEAD
    long *data;
} udt_NativeIntVar;


//-----------------------------------------------------------------------------
// Declaration of number variable functions.
//-----------------------------------------------------------------------------
static int NumberVar_PreDefine(udt_NumberVar*, OCIParam*);
static int NumberVar_SetValue(udt_NumberVar*, unsigned, PyObject*);
static PyObject *NumberVar_GetValue(udt_NumberVar*, unsigned);
static int NativeFloatVar_SetValue(udt_NativeFloatVar*, unsigned, PyObject*);
static PyObject *NativeFloatVar_GetValue(udt_NativeFloatVar*, unsigned);
static int NativeIntVar_SetValue(udt_NativeIntVar*, unsigned, PyObject*);
static PyObject *NativeIntVar_GetValue(udt_NativeIntVar*, unsigned);


//-----------------------------------------------------------------------------
// Python type declaration
//-----------------------------------------------------------------------------
static PyTypeObject g_NumberVarType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "cx_Oracle.NUMBER",                 // tp_name
    sizeof(udt_NumberVar),              // tp_basicsize
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


static PyTypeObject g_NativeFloatVarType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "cx_Oracle.NATIVE_FLOAT",           // tp_name
    sizeof(udt_NativeFloatVar),         // tp_basicsize
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


static PyTypeObject g_NativeIntVarType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "cx_Oracle.NATIVE_INT",             // tp_name
    sizeof(udt_NativeIntVar),           // tp_basicsize
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
static udt_VariableType vt_Float = {
    (InitializeProc) NULL,
    (FinalizeProc) NULL,
    (PreDefineProc) NumberVar_PreDefine,
    (PostDefineProc) NULL,
    (PostBindProc) NULL,
    (PreFetchProc) NULL,
    (IsNullProc) NULL,
    (SetValueProc) NumberVar_SetValue,
    (GetValueProc) NumberVar_GetValue,
    (GetBufferSizeProc) NULL,
    &g_NumberVarType,                   // Python type
    SQLT_VNU,                           // Oracle type
    SQLCS_IMPLICIT,                     // charset form
    sizeof(OCINumber),                  // element length
    0,                                  // is character data
    0,                                  // is variable length
    1,                                  // can be copied
    1                                   // can be in array
};


static udt_VariableType vt_NativeFloat = {
    (InitializeProc) NULL,
    (FinalizeProc) NULL,
    (PreDefineProc) NULL,
    (PostDefineProc) NULL,
    (PostBindProc) NULL,
    (PreFetchProc) NULL,
    (IsNullProc) NULL,
    (SetValueProc) NativeFloatVar_SetValue,
    (GetValueProc) NativeFloatVar_GetValue,
    (GetBufferSizeProc) NULL,
    &g_NativeFloatVarType,              // Python type
    SQLT_BDOUBLE,                       // Oracle type
    SQLCS_IMPLICIT,                     // charset form
    sizeof(double),                     // element length
    0,                                  // is character data
    0,                                  // is variable length
    1,                                  // can be copied
    1                                   // can be in array
};


static udt_VariableType vt_NativeInteger = {
    (InitializeProc) NULL,
    (FinalizeProc) NULL,
    (PreDefineProc) NULL,
    (PostDefineProc) NULL,
    (PostBindProc) NULL,
    (PreFetchProc) NULL,
    (IsNullProc) NULL,
    (SetValueProc) NativeIntVar_SetValue,
    (GetValueProc) NativeIntVar_GetValue,
    (GetBufferSizeProc) NULL,
    &g_NativeIntVarType,                // Python type
    SQLT_INT,                           // Oracle type
    SQLCS_IMPLICIT,                     // charset form
    sizeof(long),                       // element length
    0,                                  // is character data
    0,                                  // is variable length
    1,                                  // can be copied
    1                                   // can be in array
};


static udt_VariableType vt_Integer = {
    (InitializeProc) NULL,
    (FinalizeProc) NULL,
    (PreDefineProc) NumberVar_PreDefine,
    (PostDefineProc) NULL,
    (PostBindProc) NULL,
    (PreFetchProc) NULL,
    (IsNullProc) NULL,
    (SetValueProc) NumberVar_SetValue,
    (GetValueProc) NumberVar_GetValue,
    (GetBufferSizeProc) NULL,
    &g_NumberVarType,                   // Python type
    SQLT_VNU,                           // Oracle type
    SQLCS_IMPLICIT,                     // charset form
    sizeof(OCINumber),                  // element length
    0,                                  // is character data
    0,                                  // is variable length
    1,                                  // can be copied
    1                                   // can be in array
};


static udt_VariableType vt_LongInteger = {
    (InitializeProc) NULL,
    (FinalizeProc) NULL,
    (PreDefineProc) NumberVar_PreDefine,
    (PostDefineProc) NULL,
    (PostBindProc) NULL,
    (PreFetchProc) NULL,
    (IsNullProc) NULL,
    (SetValueProc) NumberVar_SetValue,
    (GetValueProc) NumberVar_GetValue,
    (GetBufferSizeProc) NULL,
    &g_NumberVarType,                   // Python type
    SQLT_VNU,                           // Oracle type
    SQLCS_IMPLICIT,                     // charset form
    sizeof(OCINumber),                  // element length
    0,                                  // is character data
    0,                                  // is variable length
    1,                                  // can be copied
    1                                   // can be in array
};


static udt_VariableType vt_NumberAsString = {
    (InitializeProc) NULL,
    (FinalizeProc) NULL,
    (PreDefineProc) NumberVar_PreDefine,
    (PostDefineProc) NULL,
    (PostBindProc) NULL,
    (PreFetchProc) NULL,
    (IsNullProc) NULL,
    (SetValueProc) NumberVar_SetValue,
    (GetValueProc) NumberVar_GetValue,
    (GetBufferSizeProc) NULL,
    &g_NumberVarType,                   // Python type
    SQLT_VNU,                           // Oracle type
    SQLCS_IMPLICIT,                     // charset form
    sizeof(OCINumber),                  // element length
    0,                                  // is character data
    0,                                  // is variable length
    1,                                  // can be copied
    1                                   // can be in array
};


#if ORACLE_VERSION_HEX < ORACLE_VERSION(12, 1)
static udt_VariableType vt_Boolean = {
    (InitializeProc) NULL,
    (FinalizeProc) NULL,
    (PreDefineProc) NumberVar_PreDefine,
    (PostDefineProc) NULL,
    (PostBindProc) NULL,
    (PreFetchProc) NULL,
    (IsNullProc) NULL,
    (SetValueProc) NumberVar_SetValue,
    (GetValueProc) NumberVar_GetValue,
    (GetBufferSizeProc) NULL,
    &g_NumberVarType,                   // Python type
    SQLT_VNU,                           // Oracle type
    SQLCS_IMPLICIT,                     // charset form
    sizeof(OCINumber),                  // element length
    0,                                  // is character data
    0,                                  // is variable length
    1,                                  // can be copied
    1                                   // can be in array
};
#endif


//-----------------------------------------------------------------------------
// NumberVar_PreDefine()
//   Set the type of value (integer, float or string) that will be returned
// when values are fetched from this variable.
//-----------------------------------------------------------------------------
static int NumberVar_PreDefine(
    udt_NumberVar *var,                 // variable to initialize
    OCIParam *param)                    // parameter handle
{
    static int maxLongSafeDigits = sizeof(long) >= 8 ? 18 : 9;
    sb2 precision;
    sword status;
    sb1 scale;

    // if the return type has not already been specified, check to see if the
    // number can fit inside an integer by looking at the precision and scale
    if (var->type == &vt_Float) {
        scale = 0;
        precision = 0;
        status = OCIAttrGet(param, OCI_HTYPE_DESCRIBE, (dvoid*) &scale, 0,
                OCI_ATTR_SCALE, var->environment->errorHandle);
        if (Environment_CheckForError(var->environment, status,
                "NumberVar_PreDefine(): scale") < 0)
            return -1;
        status = OCIAttrGet(param, OCI_HTYPE_DESCRIBE, (dvoid*) &precision, 0,
                OCI_ATTR_PRECISION, var->environment->errorHandle);
        if (Environment_CheckForError(var->environment, status,
                "NumberVar_PreDefine(): precision") < 0)
            return -1;
        if (scale == 0 || (scale == -127 && precision == 0)) {
            var->type = &vt_LongInteger;
            if (precision > 0 && precision <= maxLongSafeDigits)
                var->type = &vt_Integer;
        }
    }

    return 0;
}


//-----------------------------------------------------------------------------
// NumberVar_SetValue()
//   Set the value of the variable.
//-----------------------------------------------------------------------------
static int NumberVar_SetValue(
    udt_NumberVar *var,                 // variable to set value for
    unsigned pos,                       // array position to set
    PyObject *value)                    // value to set
{
    return PythonNumberToOracleNumber(var->environment, value,
            &var->data[pos]);
}


//-----------------------------------------------------------------------------
// NumberVar_GetValue()
//   Returns the value stored at the given array position.
//-----------------------------------------------------------------------------
static PyObject *NumberVar_GetValue(
    udt_NumberVar *var,                 // variable to determine value for
    unsigned pos)                       // array position
{
    PyObject *result, *stringObj;
    char stringValue[200];
    long integerValue;
    ub4 stringLength;
    sword status;

    if (var->type == &vt_Boolean || var->type == &vt_Integer) {
        status = OCINumberToInt(var->environment->errorHandle, &var->data[pos],
                sizeof(long), OCI_NUMBER_SIGNED, (dvoid*) &integerValue);
        if (Environment_CheckForError(var->environment, status,
                "NumberVar_GetValue(): as integer") < 0)
            return NULL;

        if (var->type == &vt_Boolean)
            return PyBool_FromLong(integerValue);
#if PY_MAJOR_VERSION >= 3
        return PyLong_FromLong(integerValue);
#else
        return PyInt_FromLong(integerValue);
#endif
    }

    if (var->type == &vt_NumberAsString || var->type == &vt_LongInteger) {
        stringLength = sizeof(stringValue);
        status = OCINumberToText(var->environment->errorHandle,
                &var->data[pos],
                (text*) var->environment->numberToStringFormatBuffer.ptr,
                var->environment->numberToStringFormatBuffer.size, NULL, 0,
                &stringLength, (unsigned char*) stringValue);
        if (Environment_CheckForError(var->environment, status,
                "NumberVar_GetValue(): as string") < 0)
            return NULL;
        stringObj = cxString_FromEncodedString(stringValue, stringLength,
                var->environment->encoding);
        if (!stringObj)
            return NULL;
        if (var->type == &vt_NumberAsString)
            return stringObj;
#if PY_MAJOR_VERSION >= 3
        result = PyNumber_Long(stringObj);
#else
        result = PyNumber_Int(stringObj);
#endif
        Py_DECREF(stringObj);
        if (result || !PyErr_ExceptionMatches(PyExc_ValueError))
            return result;
        PyErr_Clear();
    }

    return OracleNumberToPythonFloat(var->environment, &var->data[pos]);
}


//-----------------------------------------------------------------------------
// NativeFloatVar_GetValue()
//   Returns the value stored at the given array position as a float.
//-----------------------------------------------------------------------------
static PyObject *NativeFloatVar_GetValue(
    udt_NativeFloatVar *var,            // variable to determine value for
    unsigned pos)                       // array position
{
    return PyFloat_FromDouble(var->data[pos]);
}


//-----------------------------------------------------------------------------
// NativeFloatVar_SetValue()
//   Set the value of the variable which should be a native double.
//-----------------------------------------------------------------------------
static int NativeFloatVar_SetValue(
    udt_NativeFloatVar *var,            // variable to set value for
    unsigned pos,                       // array position to set
    PyObject *value)                    // value to set
{
    if (!PyFloat_Check(value)) {
        PyErr_SetString(PyExc_TypeError, "expecting float");
        return -1;
    }
    var->data[pos] = PyFloat_AS_DOUBLE(value);
    return 0;
}


//-----------------------------------------------------------------------------
// NativeIntVar_GetValue()
//   Returns the value stored at the given array position as an integer.
//-----------------------------------------------------------------------------
static PyObject *NativeIntVar_GetValue(
    udt_NativeIntVar *var,              // variable to determine value for
    unsigned pos)                       // array position
{
//printf("Getting native integer (value %ld)\n", var->data[pos]);
    return PyInt_FromLong(var->data[pos]);
}


//-----------------------------------------------------------------------------
// NativeIntVar_SetValue()
//   Set the value of the variable which should be a native integer.
//-----------------------------------------------------------------------------
static int NativeIntVar_SetValue(
    udt_NativeIntVar *var,              // variable to set value for
    unsigned pos,                       // array position to set
    PyObject *value)                    // value to set
{
    if (!PyInt_Check(value)) {
        PyErr_SetString(PyExc_TypeError, "expecting integer");
        return -1;
    }
    var->data[pos] = PyInt_AsLong(value);
//printf("Setting native integer to value %ld:\n", var->data[pos]);
    if (PyErr_Occurred())
        return -1;
    return 0;
}

