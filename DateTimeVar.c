//-----------------------------------------------------------------------------
// DateTimeVar.c
//   Defines the routines for handling date (time) variables.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// DateTime type
//-----------------------------------------------------------------------------
typedef struct {
    Variable_HEAD
    OCIDate *data;
} udt_DateTimeVar;


//-----------------------------------------------------------------------------
// Declaration of date/time variable functions.
//-----------------------------------------------------------------------------
static int DateTimeVar_SetValue(udt_DateTimeVar*, unsigned, PyObject*);
static PyObject *DateTimeVar_GetValue(udt_DateTimeVar*, unsigned);


//-----------------------------------------------------------------------------
// Python type declarations
//-----------------------------------------------------------------------------
static PyTypeObject g_DateTimeVarType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "cx_Oracle.DATETIME",               // tp_name
    sizeof(udt_DateTimeVar),            // tp_basicsize
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
static udt_VariableType vt_DateTime = {
    (InitializeProc) NULL,
    (FinalizeProc) NULL,
    (PreDefineProc) NULL,
    (PostDefineProc) NULL,
    (IsNullProc) NULL,
    (SetValueProc) DateTimeVar_SetValue,
    (GetValueProc) DateTimeVar_GetValue,
    &g_DateTimeVarType,                 // Python type
    SQLT_ODT,                           // Oracle type
    SQLCS_IMPLICIT,                     // charset form
    sizeof(OCIDate),                    // element length (default)
    0,                                  // is variable length
    1,                                  // can be copied
    1                                   // can be in array
};


#ifdef NATIVE_DATETIME
static udt_VariableType vt_Date = {
    (InitializeProc) NULL,
    (FinalizeProc) NULL,
    (PreDefineProc) NULL,
    (PostDefineProc) NULL,
    (IsNullProc) NULL,
    (SetValueProc) DateTimeVar_SetValue,
    (GetValueProc) DateTimeVar_GetValue,
    &g_DateTimeVarType,                 // Python type
    SQLT_ODT,                           // Oracle type
    SQLCS_IMPLICIT,                     // charset form
    sizeof(OCIDate),                    // element length (default)
    0,                                  // is variable length
    1,                                  // can be copied
    1                                   // can be in array
};


//-----------------------------------------------------------------------------
// DateTimeVar_SetValue()
//   Set the value of the variable.
//-----------------------------------------------------------------------------
static int DateTimeVar_SetValue(
    udt_DateTimeVar *var,               // variable to set value for
    unsigned pos,                       // array position to set
    PyObject *value)                    // value to set
{
    ub1 month, day, hour, minute, second;
    short year;

    if (PyDateTime_Check(value)) {
        year = (short) PyDateTime_GET_YEAR(value);
        month = PyDateTime_GET_MONTH(value);
        day = PyDateTime_GET_DAY(value);
        hour = PyDateTime_DATE_GET_HOUR(value);
        minute = PyDateTime_DATE_GET_MINUTE(value);
        second = PyDateTime_DATE_GET_SECOND(value);
    } else if (PyDate_Check(value)) {
        year = (short) PyDateTime_GET_YEAR(value);
        month = PyDateTime_GET_MONTH(value);
        day = PyDateTime_GET_DAY(value);
        hour = minute = second = 0;
    } else {
        PyErr_SetString(PyExc_TypeError, "expecting date data");
        return -1;
    }

    // store a copy of the value
    OCIDateSetDate(&var->data[pos], year, month, day);
    OCIDateSetTime(&var->data[pos], hour, minute, second);

    return 0;
}

#else

//-----------------------------------------------------------------------------
// DateTimeVar_GetAttribute()
//   Get the attribute from the object and convert it to an integer.
//-----------------------------------------------------------------------------
static int DateTimeVar_GetAttribute(
    PyObject *value,                    // value to get attribute for
    char *name,                         // name to acquire
    unsigned *outputValue)              // output value
{
    PyObject *attrValue;

    attrValue = PyObject_GetAttrString(value, name);
    if (!attrValue)
        return -1;
    *outputValue = PyInt_AsLong(attrValue);
    if (PyErr_Occurred())
        return -1;

    return 0;
}

//-----------------------------------------------------------------------------
// DateTimeVar_SetValue()
//   Set the value of the variable.
//-----------------------------------------------------------------------------
static int DateTimeVar_SetValue(
    udt_DateTimeVar *var,               // variable to set value for
    unsigned pos,                       // array position to set
    PyObject *value)                    // value to set
{
    unsigned year, month, day, hour, minute, second;
    udt_ExternalDateTimeVar *dateValue;
    sword status;
    uword valid;

    // handle internal cx_Oracle date type
    if (Py_TYPE(value) == &g_ExternalDateTimeVarType) {
        dateValue = (udt_ExternalDateTimeVar*) value;
        year = dateValue->year;
        month = dateValue->month;
        day = dateValue->day;
        hour = dateValue->hour;
        minute = dateValue->minute;
        second = dateValue->second;

    // handle Python 2.3 datetime type
    } else if (Py_TYPE(value) == g_DateTimeType) {
        if (DateTimeVar_GetAttribute(value, "year", &year) < 0)
            return -1;
        if (DateTimeVar_GetAttribute(value, "month", &month) < 0)
            return -1;
        if (DateTimeVar_GetAttribute(value, "day", &day) < 0)
            return -1;
        if (DateTimeVar_GetAttribute(value, "hour", &hour) < 0)
            return -1;
        if (DateTimeVar_GetAttribute(value, "minute", &minute) < 0)
            return -1;
        if (DateTimeVar_GetAttribute(value, "second", &second) < 0)
            return -1;
    } else {
        PyErr_SetString(PyExc_TypeError, "expecting date data");
        return -1;
    }

    // store a copy of the value
    OCIDateSetDate(&var->data[pos], (sb2) year, (ub1) month, (ub1) day);
    OCIDateSetTime(&var->data[pos], (ub1) hour, (ub1) minute, (ub1) second);
    status = OCIDateCheck(var->environment->errorHandle, &var->data[pos],
            &valid);
    if (Environment_CheckForError(var->environment, status,
            "DateTimeVar_SetValue()") < 0)
        return -1;
    if (valid != 0) {
        PyErr_Format(g_DataErrorException, "invalid date: %d/%d/%d %d:%d:%d",
                year, month, day, hour, minute, second);
        return -1;
    }

    return 0;
}
#endif


//-----------------------------------------------------------------------------
// DateTimeVar_GetValue()
//   Returns the value stored at the given array position.
//-----------------------------------------------------------------------------
static PyObject *DateTimeVar_GetValue(
    udt_DateTimeVar *var,               // variable to determine value for
    unsigned pos)                       // array position
{
    return OracleDateToPythonDate(var->type, &var->data[pos]);
}

