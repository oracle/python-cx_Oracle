//-----------------------------------------------------------------------------
// Copyright 2016, 2017, Oracle and/or its affiliates. All rights reserved.
//
// Portions Copyright 2007-2015, Anthony Tuininga. All rights reserved.
//
// Portions Copyright 2001-2007, Computronix (Canada) Ltd., Edmonton, Alberta,
// Canada. All rights reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// NumberVar.c
//   Defines the routines for handling numeric variables.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Declaration of number variable functions.
//-----------------------------------------------------------------------------
static PyObject *NumberVar_GetValueDecimal(udt_Variable*, dpiData*);
static PyObject *NumberVar_GetValueFloat(udt_Variable*, dpiData*);
static PyObject *NumberVar_GetValueInteger(udt_Variable*, dpiData*);
static PyObject *NumberVar_GetValueLongInteger(udt_Variable*, dpiData*);

static int NumberVar_SetValueDecimal(udt_Variable*, uint32_t, dpiData*,
        PyObject*);
static int NumberVar_SetValueFloat(udt_Variable*, uint32_t, dpiData*,
        PyObject*);
static int NumberVar_SetValueInteger(udt_Variable*, uint32_t, dpiData*,
        PyObject*);
static int NumberVar_SetValueLongInteger(udt_Variable*, uint32_t, dpiData*,
        PyObject*);


//-----------------------------------------------------------------------------
// Python type declarations
//-----------------------------------------------------------------------------
DECLARE_VARIABLE_TYPE(g_NumberVarType, NUMBER)
DECLARE_VARIABLE_TYPE(g_NativeFloatVarType, NATIVE_FLOAT)
DECLARE_VARIABLE_TYPE(g_NativeIntVarType, NATIVE_INT)


//-----------------------------------------------------------------------------
// variable type declarations
//-----------------------------------------------------------------------------
static udt_VariableType vt_NumberAsDecimal = {
    (SetValueProc) NumberVar_SetValueDecimal,
    (GetValueProc) NumberVar_GetValueDecimal,
    &g_NumberVarType,                   // Python type
    DPI_ORACLE_TYPE_NUMBER,             // Oracle type
    DPI_NATIVE_TYPE_BYTES,              // native type
    1000                                // element length
};

static udt_VariableType vt_NumberAsFloat = {
    (SetValueProc) NumberVar_SetValueFloat,
    (GetValueProc) NumberVar_GetValueFloat,
    &g_NumberVarType,                   // Python type
    DPI_ORACLE_TYPE_NUMBER,             // Oracle type
    DPI_NATIVE_TYPE_DOUBLE,             // native type
    0                                   // element length
};


static udt_VariableType vt_NumberAsInteger = {
    (SetValueProc) NumberVar_SetValueInteger,
    (GetValueProc) NumberVar_GetValueInteger,
    &g_NumberVarType,                   // Python type
    DPI_ORACLE_TYPE_NUMBER,             // Oracle type
    DPI_NATIVE_TYPE_INT64,              // native type
    0                                   // element length
};


static udt_VariableType vt_NumberAsLongInteger = {
    (SetValueProc) NumberVar_SetValueLongInteger,
    (GetValueProc) NumberVar_GetValueLongInteger,
    &g_NumberVarType,                   // Python type
    DPI_ORACLE_TYPE_NUMBER,             // Oracle type
    DPI_NATIVE_TYPE_BYTES,              // native type
    40                                  // element length
};


static udt_VariableType vt_NativeFloat = {
    (SetValueProc) NumberVar_SetValueFloat,
    (GetValueProc) NumberVar_GetValueFloat,
    &g_NativeFloatVarType,              // Python type
    DPI_ORACLE_TYPE_NATIVE_DOUBLE,      // Oracle type
    DPI_NATIVE_TYPE_DOUBLE,             // native type
    0                                   // element length
};



static udt_VariableType vt_NativeInteger = {
    (SetValueProc) NumberVar_SetValueInteger,
    (GetValueProc) NumberVar_GetValueInteger,
    &g_NativeIntVarType,                // Python type
    DPI_ORACLE_TYPE_NATIVE_INT,         // Oracle type
    DPI_NATIVE_TYPE_INT64,              // native type
    0                                   // element length
};


//-----------------------------------------------------------------------------
// NumberVar_GetValueDecimal()
//   Returns the value of the variable as a decimal object.
//-----------------------------------------------------------------------------
static PyObject *NumberVar_GetValueDecimal(udt_Variable *var, dpiData *data)
{
    PyObject *stringObj, *result;
    dpiBytes *bytes;

    bytes = &data->value.asBytes;
    stringObj = cxString_FromEncodedString(bytes->ptr, bytes->length,
            bytes->encoding);
    if (!stringObj)
        return NULL;
    result = PyObject_CallFunctionObjArgs( (PyObject*) g_DecimalType,
            stringObj, NULL);
    Py_DECREF(stringObj);
    return result;
}


//-----------------------------------------------------------------------------
// NumberVar_GetValueFloat()
//   Returns the value of the variable as a float.
//-----------------------------------------------------------------------------
static PyObject *NumberVar_GetValueFloat(udt_Variable *var, dpiData *data)
{
    return PyFloat_FromDouble(data->value.asDouble);
}


//-----------------------------------------------------------------------------
// NumberVar_GetValueInteger()
//   Returns the value of the variable as an integer.
//-----------------------------------------------------------------------------
static PyObject *NumberVar_GetValueInteger(udt_Variable *var, dpiData *data)
{
    return PyInt_FromLong((long) data->value.asInt64);
}


//-----------------------------------------------------------------------------
// NumberVar_GetValueLongInteger()
//   Returns the value of the variable as a long integer (one that may not fit
// in a native integer).
//-----------------------------------------------------------------------------
static PyObject *NumberVar_GetValueLongInteger(udt_Variable *var,
        dpiData *data)
{
    PyObject *stringObj, *result;
    dpiBytes *bytes;

    bytes = &data->value.asBytes;
    stringObj = cxString_FromEncodedString(bytes->ptr, bytes->length,
            bytes->encoding);
    if (!stringObj)
        return NULL;
    result = PyNumber_Int(stringObj);
    if (!result && PyErr_ExceptionMatches(PyExc_ValueError)) {
        PyErr_Clear();
        result = PyNumber_Float(stringObj);
    }
    Py_DECREF(stringObj);
    return result;
}


//-----------------------------------------------------------------------------
// NumberVar_SetValueDecimal()
//   Sets the value of the variable from a decimal object.
//-----------------------------------------------------------------------------
static int NumberVar_SetValueDecimal(udt_Variable *var, uint32_t pos,
        dpiData *data, PyObject *value)
{
    PyObject *textValue;
    int status;

    if (Py_TYPE(value) != g_DecimalType) {
        PyErr_SetString(PyExc_TypeError, "expecting decimal");
        return -1;
    }
    textValue = PyObject_Str(value);
    if (!textValue)
        return -1;
    status = Variable_SetValueBytes(var, pos, data, textValue);
    Py_DECREF(textValue);
    return status;
}


//-----------------------------------------------------------------------------
// NumberVar_SetValueFloat()
//   Sets the value of the variable from a float.
//-----------------------------------------------------------------------------
static int NumberVar_SetValueFloat(udt_Variable *var, uint32_t pos,
        dpiData *data, PyObject *value)
{
    data->value.asDouble = PyFloat_AsDouble(value);
    if (PyErr_Occurred())
        return -1;
    return 0;
}


//-----------------------------------------------------------------------------
// NumberVar_SetValueInteger()
//   Sets the value of the variable from an integer.
//-----------------------------------------------------------------------------
static int NumberVar_SetValueInteger(udt_Variable *var, uint32_t pos,
        dpiData *data, PyObject *value)
{
#if PY_MAJOR_VERSION < 3
    if (PyInt_Check(value)) {
        data->value.asInt64 = PyInt_AS_LONG(value);
        return 0;
    }
#endif
    if (!PyLong_Check(value)) {
        PyErr_SetString(PyExc_TypeError, "expecting integer");
        return -1;
    }
    data->value.asInt64 = PyLong_AsLong(value);
    if (PyErr_Occurred())
        return -1;
    return 0;
}


//-----------------------------------------------------------------------------
// NumberVar_SetValueLongInteger()
//   Sets the value of the variable from a long integer.
//-----------------------------------------------------------------------------
static int NumberVar_SetValueLongInteger(udt_Variable *var, uint32_t pos,
        dpiData *data, PyObject *value)
{
    PyObject *textValue;
    int status;

#if PY_MAJOR_VERSION < 3
    if (!PyInt_Check(value) && !PyLong_Check(value)) {
#else
    if (!PyLong_Check(value)) {
#endif
        PyErr_SetString(PyExc_TypeError, "expecting integer");
        return -1;
    }
    textValue = PyObject_Str(value);
    if (!textValue)
        return -1;
    status = Variable_SetValueBytes(var, pos, data, textValue);
    Py_DECREF(textValue);
    return status;
}

