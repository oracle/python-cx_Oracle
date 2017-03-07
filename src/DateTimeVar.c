//-----------------------------------------------------------------------------
// Copyright 2016, 2017, Oracle and/or its affiliates. All rights reserved.
//
// Portions Copyright 2007-2015, Anthony Tuininga. All rights reserved.
//
// Portions Copyright 2001-2007, Computronix (Canada) Ltd., Edmonton, Alberta,
// Canada. All rights reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// DateTimeVar.c
//   Defines the routines for handling date (time) variables.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Declaration of date/time variable functions.
//-----------------------------------------------------------------------------
static int DateTimeVar_SetValue(udt_Variable*, uint32_t, dpiData*, PyObject*);
static PyObject *DateTimeVar_GetValue(udt_Variable*, dpiData*);


//-----------------------------------------------------------------------------
// Python type declarations
//-----------------------------------------------------------------------------
DECLARE_VARIABLE_TYPE(g_DateTimeVarType, DATETIME)
DECLARE_VARIABLE_TYPE(g_TimestampVarType, TIMESTAMP)


//-----------------------------------------------------------------------------
// variable type declarations
//-----------------------------------------------------------------------------
static udt_VariableType vt_DateTime = {
    (SetValueProc) DateTimeVar_SetValue,
    (GetValueProc) DateTimeVar_GetValue,
    &g_DateTimeVarType,                 // Python type
    DPI_ORACLE_TYPE_DATE,               // Oracle type
    DPI_NATIVE_TYPE_TIMESTAMP,          // native type
    0                                   // element length
};


static udt_VariableType vt_Date = {
    (SetValueProc) DateTimeVar_SetValue,
    (GetValueProc) DateTimeVar_GetValue,
    &g_DateTimeVarType,                 // Python type
    DPI_ORACLE_TYPE_DATE,               // Oracle type
    DPI_NATIVE_TYPE_TIMESTAMP,          // native type
    0                                   // element length
};


static udt_VariableType vt_Timestamp = {
    (SetValueProc) DateTimeVar_SetValue,
    (GetValueProc) DateTimeVar_GetValue,
    &g_TimestampVarType,                // Python type
    DPI_ORACLE_TYPE_TIMESTAMP,          // Oracle type
    DPI_NATIVE_TYPE_TIMESTAMP,          // native type
    0                                   // element length
};


static udt_VariableType vt_TimestampLTZ = {
    (SetValueProc) DateTimeVar_SetValue,
    (GetValueProc) DateTimeVar_GetValue,
    &g_TimestampVarType,                // Python type
    DPI_ORACLE_TYPE_TIMESTAMP_LTZ,      // Oracle type
    DPI_NATIVE_TYPE_TIMESTAMP,          // native type
    0                                   // element length
};


//-----------------------------------------------------------------------------
// DateTimeVar_SetValue()
//   Set the value of the variable.
//-----------------------------------------------------------------------------
static int DateTimeVar_SetValue(udt_Variable *var, uint32_t pos, dpiData *data,
        PyObject *value)
{
    dpiTimestamp *timestamp;

    timestamp = &data->value.asTimestamp;
    if (PyDateTime_Check(value)) {
        timestamp->year = PyDateTime_GET_YEAR(value);
        timestamp->month = PyDateTime_GET_MONTH(value);
        timestamp->day = PyDateTime_GET_DAY(value);
        timestamp->hour = PyDateTime_DATE_GET_HOUR(value);
        timestamp->minute = PyDateTime_DATE_GET_MINUTE(value);
        timestamp->second = PyDateTime_DATE_GET_SECOND(value);
        timestamp->fsecond = PyDateTime_DATE_GET_MICROSECOND(value) * 1000;
    } else if (PyDate_Check(value)) {
        timestamp->year = PyDateTime_GET_YEAR(value);
        timestamp->month = PyDateTime_GET_MONTH(value);
        timestamp->day = PyDateTime_GET_DAY(value);
        timestamp->hour = 0;
        timestamp->minute = 0;
        timestamp->second = 0;
        timestamp->fsecond = 0;
    } else {
        PyErr_SetString(PyExc_TypeError, "expecting date data");
        return -1;
    }
    return 0;
}


//-----------------------------------------------------------------------------
// DateTimeVar_GetValue()
//   Returns the value stored at the given array position.
//-----------------------------------------------------------------------------
static PyObject *DateTimeVar_GetValue(udt_Variable *var, dpiData *data)
{
    dpiTimestamp *timestamp;

    timestamp = &data->value.asTimestamp;
    if (var->type == &vt_Date)
        return PyDate_FromDate(timestamp->year, timestamp->month,
                timestamp->day);
    return PyDateTime_FromDateAndTime(timestamp->year, timestamp->month,
            timestamp->day, timestamp->hour, timestamp->minute,
            timestamp->second, timestamp->fsecond / 1000);
}

