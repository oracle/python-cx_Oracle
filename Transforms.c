//-----------------------------------------------------------------------------
// Transforms.c
//   Provides methods for transforming Oracle data to Python objects or for
// setting Oracle data from Python objects.
//-----------------------------------------------------------------------------

static udt_VariableType vt_Date;

#if ORACLE_VERSION_HEX >= ORACLE_VERSION(12, 1)
//-----------------------------------------------------------------------------
// OracleBooleanToPythonBoolean()
//   Return a Python boolean object given an Oracle boolean.
//-----------------------------------------------------------------------------
static PyObject *OracleBooleanToPythonBoolean(
    boolean *value)                     // value to convert
{
    PyObject *pythonValue;

    pythonValue = (*value) ? Py_True : Py_False;
    Py_INCREF(pythonValue);
    return pythonValue;
}
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

    if (varType == &vt_Date)
        return PyDate_FromDate(year, month, day);
    return PyDateTime_FromDateAndTime(year, month, day, hour, minute, second,
            0);
}


//-----------------------------------------------------------------------------
// OracleIntervalToPythonDelta()
//   Return a Python delta object given an Oracle interval.
//-----------------------------------------------------------------------------
static PyObject *OracleIntervalToPythonDelta(
    udt_Environment *environment,       // environment
    OCIInterval *value)                 // value to convert
{
    sb4 days, hours, minutes, seconds, fseconds;
    sword status;

    status = OCIIntervalGetDaySecond(environment->handle,
            environment->errorHandle, &days, &hours, &minutes, &seconds,
            &fseconds, value);
    if (Environment_CheckForError(environment, status,
            "OracleIntervalToPythonDelta()") < 0)
        return NULL;
    seconds = hours * 60 * 60 + minutes * 60 + seconds;
    return PyDelta_FromDSU(days, seconds, fseconds / 1000);
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
    return PyDateTime_FromDateAndTime(year, month, day, hour, minute, second,
            fsecond / 1000);
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


#if ORACLE_VERSION_HEX >= ORACLE_VERSION(12, 1)
//-----------------------------------------------------------------------------
// PythonBooleanToOracleBoolean()
//   Transform a Python boolean into an Oracle boolean.
//-----------------------------------------------------------------------------
static int PythonBooleanToOracleBoolean(
    PyObject *pythonValue,              // Python value to convert
    boolean *oracleValue)               // value to convert
{
    *oracleValue = (pythonValue == Py_True);
    return 0;
}
#endif


//-----------------------------------------------------------------------------
// PythonBooleanToOracleNumber()
//   Transform a Python boolean into an Oracle number.
//-----------------------------------------------------------------------------
static int PythonBooleanToOracleNumber(
    udt_Environment *environment,       // environment
    PyObject *pythonValue,              // Python value to convert
    OCINumber *oracleValue)             // Oracle value to set
{
    long integerValue;
    sword status;

    integerValue = (pythonValue == Py_True);
    status = OCINumberFromInt(environment->errorHandle, &integerValue,
            sizeof(long), OCI_NUMBER_SIGNED, oracleValue);
    return Environment_CheckForError(environment, status,
            "PythonBooleanToOracleNumber()");
}


//-----------------------------------------------------------------------------
// PythonDateToOracleDate()
//   Transform a Python date into an Oracle date.
//-----------------------------------------------------------------------------
static int PythonDateToOracleDate(
    PyObject *pythonValue,              // Python value to convert
    OCIDate *oracleValue)               // Oracle value to set
{
    ub1 month, day, hour, minute, second;
    sb2 year;

    if (PyDateTime_Check(pythonValue)) {
        year = (short) PyDateTime_GET_YEAR(pythonValue);
        month = PyDateTime_GET_MONTH(pythonValue);
        day = PyDateTime_GET_DAY(pythonValue);
        hour = PyDateTime_DATE_GET_HOUR(pythonValue);
        minute = PyDateTime_DATE_GET_MINUTE(pythonValue);
        second = PyDateTime_DATE_GET_SECOND(pythonValue);
    } else if (PyDate_Check(pythonValue)) {
        year = (short) PyDateTime_GET_YEAR(pythonValue);
        month = PyDateTime_GET_MONTH(pythonValue);
        day = PyDateTime_GET_DAY(pythonValue);
        hour = minute = second = 0;
    } else {
        PyErr_SetString(PyExc_TypeError, "expecting date data");
        return -1;
    }

    OCIDateSetDate(oracleValue, year, month, day);
    OCIDateSetTime(oracleValue, hour, minute, second);

    return 0;
}


#if PY_MAJOR_VERSION < 3
//-----------------------------------------------------------------------------
// PythonIntegerToOracleNumber()
//   Transform a Python integer into an Oracle number.
//-----------------------------------------------------------------------------
static int PythonIntegerToOracleNumber(
    udt_Environment *environment,       // environment
    PyObject *pythonValue,              // Python value to convert
    OCINumber *oracleValue)             // Oracle value to set
{
    long integerValue;
    sword status;

    integerValue = PyInt_AS_LONG(pythonValue);
    status = OCINumberFromInt(environment->errorHandle, &integerValue,
            sizeof(long), OCI_NUMBER_SIGNED, oracleValue);
    return Environment_CheckForError(environment, status,
            "PythonIntegerToOracleNumber()");
}
#endif


//-----------------------------------------------------------------------------
// PythonFloatToOracleNumber()
//   Transform a Python float into an Oracle number.
//-----------------------------------------------------------------------------
static int PythonFloatToOracleNumber(
    udt_Environment *environment,       // environment
    PyObject *pythonValue,              // Python value to convert
    OCINumber *oracleValue)             // Oracle value to set
{
    double doubleValue;
    sword status;

    doubleValue = PyFloat_AS_DOUBLE(pythonValue);
    status = OCINumberFromReal(environment->errorHandle, &doubleValue,
            sizeof(double), oracleValue);
    return Environment_CheckForError(environment, status,
            "PythonFloatToOracleNumber()");
}


//-----------------------------------------------------------------------------
// PythonLongToOracleNumber()
//   Set the value of the variable from a Python long.
//-----------------------------------------------------------------------------
static int PythonLongToOracleNumber(
    udt_Environment *environment,       // environment
    PyObject *pythonValue,              // Python value to convert
    OCINumber *oracleValue)             // Oracle value to set
{
    udt_Buffer textBuffer;
    PyObject *textValue;
    sword status;

    textValue = PyObject_Str(pythonValue);
    if (!textValue)
        return -1;
    if (cxBuffer_FromObject(&textBuffer, textValue, environment->encoding) < 0)
        return -1;
    status = OCINumberFromText(environment->errorHandle,
            (text*) textBuffer.ptr, textBuffer.size,
            (text*) environment->numberFromStringFormatBuffer.ptr,
            environment->numberFromStringFormatBuffer.size, NULL, 0,
            oracleValue);
    cxBuffer_Clear(&textBuffer);
    Py_DECREF(textValue);
    return Environment_CheckForError(environment, status,
            "PythonLongToOracleNumber()");
}


//-----------------------------------------------------------------------------
// GetFormatAndTextFromPythonDecimal()
//   Return the number format and text to use for the Decimal object.
//-----------------------------------------------------------------------------
static int GetFormatAndTextFromPythonDecimal(
    PyObject *tupleValue,               // decimal as_tuple() value
    PyObject **textObj,                 // text string for conversion
    PyObject **formatObj)               // format for conversion
{
    long numDigits, scale, i, sign, length, digit;
    char *textValue, *format, *textPtr, *formatPtr;
    PyObject *digits;

    // acquire basic information from the value tuple
    sign = PyInt_AsLong(PyTuple_GET_ITEM(tupleValue, 0));
    if (PyErr_Occurred())
        return -1;
    digits = PyTuple_GET_ITEM(tupleValue, 1);
    scale = PyInt_AsLong(PyTuple_GET_ITEM(tupleValue, 2));
    if (PyErr_Occurred())
        return -1;
    numDigits = PyTuple_GET_SIZE(digits);

    // allocate memory for the string and format to use in conversion
    length = numDigits + abs(scale) + 3;
    textValue = textPtr = PyMem_Malloc(length);
    if (!textValue) {
        PyErr_NoMemory();
        return -1;
    }
    format = formatPtr = PyMem_Malloc(length);
    if (!format) {
        PyMem_Free(textValue);
        PyErr_NoMemory();
        return -1;
    }

    // populate the string and format
    if (sign)
        *textPtr++ = '-';
    for (i = 0; i < numDigits + scale; i++) {
        *formatPtr++ = '9';
        if (i < numDigits) {
            digit = PyInt_AsLong(PyTuple_GetItem(digits, i));
            if (PyErr_Occurred()) {
                PyMem_Free(textValue);
                return -1;
            }
        }
        else digit = 0;
        *textPtr++ = '0' + (char) digit;
    }
    if (scale < 0) {
        *formatPtr++ = 'D';
        *textPtr++ = '.';
        for (i = scale; i < 0; i++) {
            *formatPtr++ = '9';
            if (numDigits + i < 0)
                digit = 0;
            else {
                digit = PyInt_AsLong(PyTuple_GetItem(digits, numDigits + i));
                if (PyErr_Occurred()) {
                    PyMem_Free(textValue);
                    return -1;
                }
            }
            *textPtr++ = '0' + (char) digit;
        }
    }
    *formatPtr = '\0';
    *textPtr = '\0';
    *textObj = cxString_FromAscii(textValue);
    PyMem_Free(textValue);
    if (!*textObj) {
        PyMem_Free(format);
        return -1;
    }
    *formatObj = cxString_FromAscii(format);
    PyMem_Free(format);
    if (!*formatObj) {
        Py_DECREF(*textObj);
        return -1;
    }

    return 0;
}


//-----------------------------------------------------------------------------
// PythonDecimalToOracleNumber()
//   Transform a Python decimal object into an Oracle number.
//-----------------------------------------------------------------------------
static int PythonDecimalToOracleNumber(
    udt_Environment *environment,       // environment
    PyObject *pythonValue,              // Python value to convert
    OCINumber *oracleValue)             // Oracle value to set
{
    PyObject *textValue, *format, *tupleValue;
    udt_Buffer textBuffer, formatBuffer;
    sword status;

    tupleValue = PyObject_CallMethod(pythonValue, "as_tuple", NULL);
    if (!tupleValue)
        return -1;
    if (GetFormatAndTextFromPythonDecimal(tupleValue, &textValue,
            &format) < 0) {
        Py_DECREF(tupleValue);
        return -1;
    }
    Py_DECREF(tupleValue);
    if (cxBuffer_FromObject(&textBuffer, textValue, environment->encoding) < 0)
        return -1;
    if (cxBuffer_FromObject(&formatBuffer, format,
            environment->encoding) < 0) {
        cxBuffer_Clear(&textBuffer);
        return -1;
    }
    status = OCINumberFromText(environment->errorHandle,
            (text*) textBuffer.ptr, textBuffer.size, (text*) formatBuffer.ptr,
            formatBuffer.size, environment->nlsNumericCharactersBuffer.ptr,
            environment->nlsNumericCharactersBuffer.size, oracleValue);
    cxBuffer_Clear(&textBuffer);
    cxBuffer_Clear(&formatBuffer);
    Py_DECREF(textValue);
    Py_DECREF(format);
    return Environment_CheckForError(environment, status,
            "PythonDecimalToOracleNumber()");
}


//-----------------------------------------------------------------------------
// PythonNumberToOracleNumber()
//   Convert a Python number to an Oracle number.
//-----------------------------------------------------------------------------
static int PythonNumberToOracleNumber(
    udt_Environment *environment,       // environment
    PyObject *pythonValue,              // Python value
    OCINumber* oracleValue)             // Oracle value
{
#if PY_MAJOR_VERSION < 3
    if (PyInt_Check(pythonValue))
        return PythonIntegerToOracleNumber(environment, pythonValue,
                oracleValue);
#endif
    if (PyBool_Check(pythonValue))
        return PythonBooleanToOracleNumber(environment, pythonValue,
                oracleValue);
    if (PyLong_Check(pythonValue))
        return PythonLongToOracleNumber(environment, pythonValue, oracleValue);
    if (PyFloat_Check(pythonValue))
        return PythonFloatToOracleNumber(environment, pythonValue,
                oracleValue);
    if (Py_TYPE(pythonValue) == g_DecimalType)
        return PythonDecimalToOracleNumber(environment, pythonValue,
                oracleValue);
    PyErr_SetString(PyExc_TypeError, "expecting numeric data");
    return -1;
}


//-----------------------------------------------------------------------------
// PythonDateToOracleTimestamp()
//   Convert a Python date to an Oracle timestamp.
//-----------------------------------------------------------------------------
static int PythonDateToOracleTimestamp(
    udt_Environment *environment,       // environment
    PyObject *pythonValue,              // Python value
    OCIDateTime* oracleValue)           // Oracle value
{
    sword status;
    uword valid;

    // make sure a timestamp is being bound
    if (!PyDateTime_Check(pythonValue)) {
        PyErr_SetString(PyExc_TypeError, "expecting timestamp data");
        return -1;
    }

    // store a copy of the value
    status = OCIDateTimeConstruct(environment->handle,
            environment->errorHandle, oracleValue,
            (sb2) PyDateTime_GET_YEAR(pythonValue),
            PyDateTime_GET_MONTH(pythonValue),
            PyDateTime_GET_DAY(pythonValue),
            PyDateTime_DATE_GET_HOUR(pythonValue),
            PyDateTime_DATE_GET_MINUTE(pythonValue),
            PyDateTime_DATE_GET_SECOND(pythonValue),
            PyDateTime_DATE_GET_MICROSECOND(pythonValue) * 1000, NULL, 0);
    if (Environment_CheckForError(environment, status,
            "PythonDateToOracleTimestamp(): create structure") < 0)
        return -1;
    status = OCIDateTimeCheck(environment->handle, environment->errorHandle,
            oracleValue, &valid);
    if (Environment_CheckForError(environment, status,
            "PythonDateToOracleTimestamp(): check validity") < 0)
        return -1;
    if (valid != 0) {
        PyErr_SetString(g_DataErrorException, "invalid date");
        return -1;
    }

    return 0;
}

