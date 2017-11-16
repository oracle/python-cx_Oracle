//-----------------------------------------------------------------------------
// Copyright 2016, 2017, Oracle and/or its affiliates. All rights reserved.
//
// Portions Copyright 2007-2015, Anthony Tuininga. All rights reserved.
//
// Portions Copyright 2001-2007, Computronix (Canada) Ltd., Edmonton, Alberta,
// Canada. All rights reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// IntervalVar.c
//   Defines the routines for handling interval variables.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Declaration of interval variable functions.
//-----------------------------------------------------------------------------
static int IntervalVar_SetValue(udt_Variable*, uint32_t, dpiData*, PyObject*);
static PyObject *IntervalVar_GetValue(udt_Variable*, dpiData*);


//-----------------------------------------------------------------------------
// Python type declarations
//-----------------------------------------------------------------------------
DECLARE_VARIABLE_TYPE(g_IntervalVarType, INTERVAL)


//-----------------------------------------------------------------------------
// variable type declarations
//-----------------------------------------------------------------------------
static udt_VariableType vt_Interval = {
    (SetValueProc) IntervalVar_SetValue,
    (GetValueProc) IntervalVar_GetValue,
    &g_IntervalVarType,                 // Python type
    DPI_ORACLE_TYPE_INTERVAL_DS,        // Oracle type
    DPI_NATIVE_TYPE_INTERVAL_DS,        // native type
    0                                   // element length
};


// PyPy compatibility
#ifndef PyDateTime_DELTA_GET_DAYS
#define PyDateTime_DELTA_GET_DAYS(x) ((x)->days)
#endif

#ifndef PyDateTime_DELTA_GET_SECONDS
#define PyDateTime_DELTA_GET_SECONDS(x) ((x)->seconds)
#endif

#ifndef PyDateTime_DELTA_GET_MICROSECONDS
#define PyDateTime_DELTA_GET_MICROSECONDS(x) ((x)->microseconds)
#endif


//-----------------------------------------------------------------------------
// IntervalVar_SetValue()
//   Set the value of the variable.
//-----------------------------------------------------------------------------
static int IntervalVar_SetValue(udt_Variable *var, uint32_t pos, dpiData *data,
        PyObject *value)
{
    dpiIntervalDS *interval;
    PyDateTime_Delta *delta;
    int32_t delta_seconds;

    if (!PyDelta_Check(value)) {
        PyErr_SetString(PyExc_TypeError, "expecting timedelta data");
        return -1;
    }
    delta = (PyDateTime_Delta*) value;
    interval = &data->value.asIntervalDS;
    delta_seconds = PyDateTime_DELTA_GET_SECONDS(delta);
    interval->days = PyDateTime_DELTA_GET_DAYS(delta);
    interval->hours = delta_seconds / 3600;
    interval->seconds = delta_seconds % 3600;
    interval->minutes = interval->seconds / 60;
    interval->seconds = interval->seconds % 60;
    interval->fseconds = PyDateTime_DELTA_GET_MICROSECONDS(delta) * 1000;
    return 0;
}


//-----------------------------------------------------------------------------
// IntervalVar_GetValue()
//   Returns the value stored at the given array position.
//-----------------------------------------------------------------------------
static PyObject *IntervalVar_GetValue(udt_Variable *var, dpiData *data)
{
    dpiIntervalDS *interval;
    int32_t seconds;

    interval = &data->value.asIntervalDS;
    seconds = interval->hours * 60 * 60 + interval->minutes * 60 +
            interval->seconds;
    return PyDelta_FromDSU(interval->days, seconds, interval->fseconds / 1000);
}

