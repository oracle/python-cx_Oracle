//-----------------------------------------------------------------------------
// Transforms.c
//   Provides methods for transforming Oracle data to Python objects or for
// setting Oracle data from Python objects.
//-----------------------------------------------------------------------------

#ifdef NATIVE_DATETIME
static udt_VariableType vt_Date;
#endif

//-----------------------------------------------------------------------------
// OracleDateToPythonDate()
//   Return a Python date object given an Oracle date.
//-----------------------------------------------------------------------------
static PyObject *OracleDateToPythonDate(
    udt_VariableType *varType,          // variable type
    OCIDate* value)                     // value to convert
{
    ub1 hour, minute, second, month, day;
    sb2 year;

    OCIDateGetDate(value, &year, &month, &day);
    OCIDateGetTime(value, &hour, &minute, &second);

#ifdef NATIVE_DATETIME
    if (varType == &vt_Date)
        return PyDate_FromDate(year, month, day);
    return PyDateTime_FromDateAndTime(year, month, day, hour, minute, second,
            0);
#else
    return ExternalDateTimeVar_NewFromC(&g_ExternalDateTimeVarType, year,
            month, day, hour, minute, second, 0);
#endif
}


//-----------------------------------------------------------------------------
// OracleTimestampToPythonDate()
//   Return a Python date object given an Oracle timestamp.
//-----------------------------------------------------------------------------
static PyObject *OracleTimestampToPythonDate(
    udt_Environment *environment,       // environment
    OCIDateTime* value)                 // value to convert
{
    ub1 hour, minute, second, month, day;
    sword status;
    ub4 fsecond;
    sb2 year;

    status = OCIDateTimeGetDate(environment->handle, environment->errorHandle,
            value, &year, &month, &day);
    if (Environment_CheckForError(environment, status,
            "OracleTimestampToPythonDate(): date portion") < 0)
        return NULL;
    status = OCIDateTimeGetTime(environment->handle, environment->errorHandle,
            value, &hour, &minute, &second, &fsecond);
    if (Environment_CheckForError(environment, status,
            "OracleTimestampToPythonDate(): time portion") < 0)
        return NULL;
#ifdef NATIVE_DATETIME
    return PyDateTime_FromDateAndTime(year, month, day, hour, minute, second,
            fsecond / 1000);
#else
    return ExternalDateTimeVar_NewFromC(&g_ExternalDateTimeVarType, year,
            month, day, hour, minute, second, fsecond / 1000);
#endif
}


//-----------------------------------------------------------------------------
// OracleNumberToPythonFloat()
//   Return a Python date object given an Oracle date.
//-----------------------------------------------------------------------------
static PyObject *OracleNumberToPythonFloat(
    udt_Environment *environment,       // environment
    OCINumber* value)                   // value to convert
{
    double doubleValue;
    sword status;

    status = OCINumberToReal(environment->errorHandle,
            value, sizeof(double), (dvoid*) &doubleValue);
    if (Environment_CheckForError(environment, status,
            "OracleNumberToPythonFloat()") < 0)
        return NULL;
    return PyFloat_FromDouble(doubleValue);
}

