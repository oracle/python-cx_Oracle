//-----------------------------------------------------------------------------
// Copyright (c) 2018, 2020, Oracle and/or its affiliates. All rights reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// cxoTransform.c
//   Defines routines used to transform Python objects to database values and
// database values to Python objects.
//-----------------------------------------------------------------------------

#include "cxoModule.h"
#include "datetime.h"

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

// forward declarations
static Py_ssize_t cxoTransform_calculateSize(PyObject *value,
        cxoTransformNum transformNum);
static cxoTransformNum cxoTransform_getNumFromPythonType(PyTypeObject *type);
static PyObject *cxoTransform_toPythonFromJson(cxoConnection *connection,
        dpiJsonNode *node, const char *encodingErrors);


//-----------------------------------------------------------------------------
// Types
//-----------------------------------------------------------------------------
typedef struct {
    cxoTransformNum transformNum;
    dpiOracleTypeNum oracleTypeNum;
    dpiNativeTypeNum nativeTypeNum;
} cxoTransform;


//-----------------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------------
PyTypeObject *cxoPyTypeDate;
PyTypeObject *cxoPyTypeDateTime;
static PyTypeObject *cxoPyTypeDecimal;
static const cxoTransform cxoAllTransforms[] = {
    {
        CXO_TRANSFORM_NONE,
        DPI_ORACLE_TYPE_VARCHAR,
        DPI_NATIVE_TYPE_BYTES
    },
    {
        CXO_TRANSFORM_BINARY,
        DPI_ORACLE_TYPE_RAW,
        DPI_NATIVE_TYPE_BYTES
    },
    {
        CXO_TRANSFORM_BFILE,
        DPI_ORACLE_TYPE_BFILE,
        DPI_NATIVE_TYPE_LOB
    },
    {
        CXO_TRANSFORM_BLOB,
        DPI_ORACLE_TYPE_BLOB,
        DPI_NATIVE_TYPE_LOB
    },
    {
        CXO_TRANSFORM_BOOLEAN,
        DPI_ORACLE_TYPE_BOOLEAN,
        DPI_NATIVE_TYPE_BOOLEAN
    },
    {
        CXO_TRANSFORM_CLOB,
        DPI_ORACLE_TYPE_CLOB,
        DPI_NATIVE_TYPE_LOB
    },
    {
        CXO_TRANSFORM_CURSOR,
        DPI_ORACLE_TYPE_STMT,
        DPI_NATIVE_TYPE_STMT
    },
    {
        CXO_TRANSFORM_DATE,
        DPI_ORACLE_TYPE_DATE,
        DPI_NATIVE_TYPE_TIMESTAMP
    },
    {
        CXO_TRANSFORM_DATETIME,
        DPI_ORACLE_TYPE_DATE,
        DPI_NATIVE_TYPE_TIMESTAMP
    },
    {
        CXO_TRANSFORM_DECIMAL,
        DPI_ORACLE_TYPE_NUMBER,
        DPI_NATIVE_TYPE_BYTES
    },
    {
        CXO_TRANSFORM_FIXED_CHAR,
        DPI_ORACLE_TYPE_CHAR,
        DPI_NATIVE_TYPE_BYTES
    },
    {
        CXO_TRANSFORM_FIXED_NCHAR,
        DPI_ORACLE_TYPE_NCHAR,
        DPI_NATIVE_TYPE_BYTES
    },
    {
        CXO_TRANSFORM_FLOAT,
        DPI_ORACLE_TYPE_NUMBER,
        DPI_NATIVE_TYPE_BYTES
    },
    {
        CXO_TRANSFORM_INT,
        DPI_ORACLE_TYPE_NUMBER,
        DPI_NATIVE_TYPE_BYTES
    },
    {
        CXO_TRANSFORM_LONG_BINARY,
        DPI_ORACLE_TYPE_LONG_RAW,
        DPI_NATIVE_TYPE_BYTES
    },
    {
        CXO_TRANSFORM_LONG_STRING,
        DPI_ORACLE_TYPE_LONG_VARCHAR,
        DPI_NATIVE_TYPE_BYTES
    },
    {
        CXO_TRANSFORM_NATIVE_DOUBLE,
        DPI_ORACLE_TYPE_NATIVE_DOUBLE,
        DPI_NATIVE_TYPE_DOUBLE
    },
    {
        CXO_TRANSFORM_NATIVE_FLOAT,
        DPI_ORACLE_TYPE_NATIVE_FLOAT,
        DPI_NATIVE_TYPE_FLOAT
    },
    {
        CXO_TRANSFORM_NATIVE_INT,
        DPI_ORACLE_TYPE_NATIVE_INT,
        DPI_NATIVE_TYPE_INT64
    },
    {
        CXO_TRANSFORM_NCLOB,
        DPI_ORACLE_TYPE_NCLOB,
        DPI_NATIVE_TYPE_LOB
    },
    {
        CXO_TRANSFORM_NSTRING,
        DPI_ORACLE_TYPE_NVARCHAR,
        DPI_NATIVE_TYPE_BYTES
    },
    {
        CXO_TRANSFORM_OBJECT,
        DPI_ORACLE_TYPE_OBJECT,
        DPI_NATIVE_TYPE_OBJECT
    },
    {
        CXO_TRANSFORM_ROWID,
        DPI_ORACLE_TYPE_ROWID,
        DPI_NATIVE_TYPE_ROWID
    },
    {
        CXO_TRANSFORM_STRING,
        DPI_ORACLE_TYPE_VARCHAR,
        DPI_NATIVE_TYPE_BYTES
    },
    {
        CXO_TRANSFORM_TIMEDELTA,
        DPI_ORACLE_TYPE_INTERVAL_DS,
        DPI_NATIVE_TYPE_INTERVAL_DS
    },
    {
        CXO_TRANSFORM_TIMESTAMP,
        DPI_ORACLE_TYPE_TIMESTAMP,
        DPI_NATIVE_TYPE_TIMESTAMP
    },
    {
        CXO_TRANSFORM_TIMESTAMP_LTZ,
        DPI_ORACLE_TYPE_TIMESTAMP_LTZ,
        DPI_NATIVE_TYPE_TIMESTAMP
    },
    {
        CXO_TRANSFORM_TIMESTAMP_TZ,
        DPI_ORACLE_TYPE_TIMESTAMP_TZ,
        DPI_NATIVE_TYPE_TIMESTAMP
    },
    {
        CXO_TRANSFORM_JSON,
        DPI_ORACLE_TYPE_JSON,
        DPI_NATIVE_TYPE_JSON
    }
};


//-----------------------------------------------------------------------------
// cxoTransform_calculateSize()
//   Calculate the size to use with the specified transform and Python value.
// This function is only called by cxoTransform_getNumFromValue() and no
// attempt is made to verify the value further.
//-----------------------------------------------------------------------------
static Py_ssize_t cxoTransform_calculateSize(PyObject *value,
        cxoTransformNum transformNum)
{
    switch (transformNum) {
        case CXO_TRANSFORM_NONE:
            return 1;
        case CXO_TRANSFORM_BINARY:
            return PyBytes_GET_SIZE(value);
        case CXO_TRANSFORM_NSTRING:
        case CXO_TRANSFORM_STRING:
            return PyUnicode_GET_LENGTH(value);
        default:
            break;
    }
    return 0;
}


//-----------------------------------------------------------------------------
// cxoTransform_dateFromTicks()
//   Creates a date from ticks (number of seconds since Unix epoch).
//-----------------------------------------------------------------------------
PyObject *cxoTransform_dateFromTicks(PyObject *args)
{
    return PyDate_FromTimestamp(args);
}


//-----------------------------------------------------------------------------
// cxoTransform_fromPython()
//   Transforms a Python object into its corresponding database value.
//-----------------------------------------------------------------------------
int cxoTransform_fromPython(cxoTransformNum transformNum,
        dpiNativeTypeNum *nativeTypeNum, PyObject *pyValue,
        dpiDataBuffer *dbValue, cxoBuffer *buffer, const char *encoding,
        const char *nencoding, cxoVar *var, uint32_t arrayPos)
{
    cxoJsonBuffer jsonBuffer;
    dpiIntervalDS *interval;
    PyDateTime_Delta *delta;
    int32_t deltaSeconds;
    PyObject *tempValue;
    cxoObject *obj;
    cxoLob *lob;
    int status;

    switch (transformNum) {
        case CXO_TRANSFORM_BOOLEAN:
            dbValue->asBoolean = PyObject_IsTrue(pyValue);
            if (PyErr_Occurred())
                return -1;
            return 0;
        case CXO_TRANSFORM_BINARY:
        case CXO_TRANSFORM_FIXED_CHAR:
        case CXO_TRANSFORM_LONG_BINARY:
        case CXO_TRANSFORM_LONG_STRING:
        case CXO_TRANSFORM_STRING:
            if (cxoBuffer_fromObject(buffer, pyValue, encoding) < 0)
                return -1;
            dbValue->asBytes.ptr = (char*) buffer->ptr;
            dbValue->asBytes.length = buffer->size;
            return 0;
        case CXO_TRANSFORM_FIXED_NCHAR:
        case CXO_TRANSFORM_NSTRING:
            if (cxoBuffer_fromObject(buffer, pyValue, nencoding) < 0)
                return -1;
            dbValue->asBytes.ptr = (char*) buffer->ptr;
            dbValue->asBytes.length = buffer->size;
            return 0;
        case CXO_TRANSFORM_BLOB:
        case CXO_TRANSFORM_CLOB:
        case CXO_TRANSFORM_NCLOB:
            if (Py_TYPE(pyValue) == &cxoPyTypeLob) {
                lob = (cxoLob*) pyValue;
                if ((lob->dbType == cxoDbTypeBlob &&
                                transformNum != CXO_TRANSFORM_BLOB) ||
                        (lob->dbType == cxoDbTypeClob &&
                                transformNum != CXO_TRANSFORM_CLOB) ||
                        (lob->dbType == cxoDbTypeNclob &&
                                transformNum != CXO_TRANSFORM_NCLOB)) {
                    PyErr_SetString(PyExc_TypeError,
                            "LOB must be of the correct type");
                    return -1;
                }
                if (var) {
                    if (dpiVar_setFromLob(var->handle, arrayPos,
                            lob->handle) < 0)
                        return cxoError_raiseAndReturnInt();
                } else dbValue->asLOB = lob->handle;
                return 0;
            }
            if (transformNum == CXO_TRANSFORM_NCLOB)
                encoding = nencoding;
            if (cxoBuffer_fromObject(buffer, pyValue, encoding) < 0)
                return -1;
            if (var) {
                Py_BEGIN_ALLOW_THREADS
                status = dpiLob_setFromBytes(dbValue->asLOB, buffer->ptr,
                        buffer->size);
                Py_END_ALLOW_THREADS
                if (status < 0)
                    return cxoError_raiseAndReturnInt();
            } else {
                *nativeTypeNum = DPI_NATIVE_TYPE_BYTES;
                dbValue->asBytes.ptr = (char*) buffer->ptr;
                dbValue->asBytes.length = buffer->size;
            }
            return 0;
        case CXO_TRANSFORM_NATIVE_INT:
            if (PyBool_Check(pyValue)) {
                dbValue->asInt64 = (pyValue == Py_True);
                return 0;
            }
            if (!PyFloat_Check(pyValue) &&
                    !PyLong_Check(pyValue) &&
                    !PyObject_TypeCheck(pyValue, cxoPyTypeDecimal)) {
                PyErr_SetString(PyExc_TypeError,
                        "expecting number or boolean");
                return -1;
            }
            tempValue = PyObject_CallFunctionObjArgs((PyObject*) &PyLong_Type,
                    pyValue, NULL);
            if (!tempValue)
                return -1;
            dbValue->asInt64 = PyLong_AsLong(tempValue);
            status = (PyErr_Occurred()) ? -1 : 0;
            Py_DECREF(tempValue);
            return status;
        case CXO_TRANSFORM_INT:
        case CXO_TRANSFORM_DECIMAL:
        case CXO_TRANSFORM_FLOAT:
            if (PyBool_Check(pyValue)) {
                buffer->ptr = (pyValue == Py_True) ? "1" : "0";
                buffer->size = 1;
                buffer->numCharacters = 1;
            } else {
                if (!PyFloat_Check(pyValue) &&
                        !PyLong_Check(pyValue) &&
                        !PyObject_TypeCheck(pyValue, cxoPyTypeDecimal)) {
                    PyErr_SetString(PyExc_TypeError, "expecting number");
                    return -1;
                }
                tempValue = PyObject_Str(pyValue);
                if (!tempValue)
                    return -1;
                status = cxoBuffer_fromObject(buffer, tempValue, encoding);
                Py_DECREF(tempValue);
                if (status < 0)
                    return -1;
            }
            dbValue->asBytes.ptr = (char*) buffer->ptr;
            dbValue->asBytes.length = buffer->size;
            return 0;
        case CXO_TRANSFORM_NATIVE_DOUBLE:
        case CXO_TRANSFORM_NATIVE_FLOAT:
            if (!PyFloat_Check(pyValue) &&
                    !PyObject_TypeCheck(pyValue, cxoPyTypeDecimal) &&
                    !PyLong_Check(pyValue)) {
                PyErr_SetString(PyExc_TypeError, "expecting float");
                return -1;
            }
            if (transformNum == CXO_TRANSFORM_NATIVE_FLOAT)
                dbValue->asFloat = (float) PyFloat_AsDouble(pyValue);
            else dbValue->asDouble = PyFloat_AsDouble(pyValue);
            if (PyErr_Occurred())
                return -1;
            return 0;
        case CXO_TRANSFORM_OBJECT:
            if (Py_TYPE(pyValue) != &cxoPyTypeObject) {
                PyErr_SetString(PyExc_TypeError, "expecting cx_Oracle.Object");
                return -1;
            }
            obj = (cxoObject*) pyValue;
            if (var) {
                if (dpiVar_setFromObject(var->handle, arrayPos,
                        obj->handle) < 0)
                    return cxoError_raiseAndReturnInt();
            } else dbValue->asObject = obj->handle;
            return 0;
        case CXO_TRANSFORM_DATE:
        case CXO_TRANSFORM_DATETIME:
        case CXO_TRANSFORM_TIMESTAMP:
        case CXO_TRANSFORM_TIMESTAMP_LTZ:
        case CXO_TRANSFORM_TIMESTAMP_TZ:
            if (PyDateTime_Check(pyValue)) {
                memset(&dbValue->asTimestamp, 0, sizeof(dbValue->asTimestamp));
                dbValue->asTimestamp.year = PyDateTime_GET_YEAR(pyValue);
                dbValue->asTimestamp.month = PyDateTime_GET_MONTH(pyValue);
                dbValue->asTimestamp.day = PyDateTime_GET_DAY(pyValue);
                dbValue->asTimestamp.hour = PyDateTime_DATE_GET_HOUR(pyValue);
                dbValue->asTimestamp.minute =
                        PyDateTime_DATE_GET_MINUTE(pyValue);
                dbValue->asTimestamp.second =
                        PyDateTime_DATE_GET_SECOND(pyValue);
                dbValue->asTimestamp.fsecond =
                        PyDateTime_DATE_GET_MICROSECOND(pyValue) * 1000;
            } else if (PyDate_Check(pyValue)) {
                memset(&dbValue->asTimestamp, 0, sizeof(dbValue->asTimestamp));
                dbValue->asTimestamp.year = PyDateTime_GET_YEAR(pyValue);
                dbValue->asTimestamp.month = PyDateTime_GET_MONTH(pyValue);
                dbValue->asTimestamp.day = PyDateTime_GET_DAY(pyValue);
            } else {
                PyErr_SetString(PyExc_TypeError, "expecting date or datetime");
                return -1;
            }
            return 0;
        case CXO_TRANSFORM_TIMEDELTA:
            if (!PyDelta_Check(pyValue)) {
                PyErr_SetString(PyExc_TypeError, "expecting timedelta");
                return -1;
            }
            delta = (PyDateTime_Delta*) pyValue;
            interval = &dbValue->asIntervalDS;
            deltaSeconds = PyDateTime_DELTA_GET_SECONDS(delta);
            interval->days = PyDateTime_DELTA_GET_DAYS(delta);
            interval->hours = deltaSeconds / 3600;
            interval->seconds = deltaSeconds % 3600;
            interval->minutes = interval->seconds / 60;
            interval->seconds = interval->seconds % 60;
            interval->fseconds =
                    PyDateTime_DELTA_GET_MICROSECONDS(delta) * 1000;
            return 0;
        case CXO_TRANSFORM_JSON:
            status = cxoJsonBuffer_fromObject(&jsonBuffer, pyValue);
            if (status < 0) {
                cxoJsonBuffer_free(&jsonBuffer);
                return -1;
            }
            status = dpiJson_setValue(dbValue->asJson, &jsonBuffer.topNode);
            cxoJsonBuffer_free(&jsonBuffer);
            if (status < 0)
                return cxoError_raiseAndReturnInt();
            return 0;
        default:
            break;
    }

    cxoError_raiseFromString(cxoNotSupportedErrorException,
            "Python value cannot be converted to a database value");
    return -1;
}


//-----------------------------------------------------------------------------
// cxoTransform_getDefaultSize()
//   Return the default size for the specified transform.
//-----------------------------------------------------------------------------
uint32_t cxoTransform_getDefaultSize(cxoTransformNum transformNum)
{
    switch (transformNum) {
        case CXO_TRANSFORM_NONE:
            return 1;
        case CXO_TRANSFORM_BINARY:
        case CXO_TRANSFORM_NSTRING:
        case CXO_TRANSFORM_STRING:
            return 4000;
        case CXO_TRANSFORM_DECIMAL:
        case CXO_TRANSFORM_FLOAT:
        case CXO_TRANSFORM_INT:
            return 1000;
        case CXO_TRANSFORM_FIXED_CHAR:
        case CXO_TRANSFORM_FIXED_NCHAR:
            return 2000;
        case CXO_TRANSFORM_LONG_BINARY:
        case CXO_TRANSFORM_LONG_STRING:
            return 128 * 1024;
        default:
            break;
    }

    return 0;
}


//-----------------------------------------------------------------------------
// cxoTransform_getNumFromDataTypeInfo()
//   Get the default transformation to use for the specified data type.
//-----------------------------------------------------------------------------
cxoTransformNum cxoTransform_getNumFromDataTypeInfo(dpiDataTypeInfo *info)
{
    switch (info->oracleTypeNum) {
        case DPI_ORACLE_TYPE_VARCHAR:
            return CXO_TRANSFORM_STRING;
        case DPI_ORACLE_TYPE_NVARCHAR:
            return CXO_TRANSFORM_NSTRING;
        case DPI_ORACLE_TYPE_CHAR:
            return CXO_TRANSFORM_FIXED_CHAR;
        case DPI_ORACLE_TYPE_NCHAR:
            return CXO_TRANSFORM_FIXED_NCHAR;
        case DPI_ORACLE_TYPE_ROWID:
            return CXO_TRANSFORM_ROWID;
        case DPI_ORACLE_TYPE_RAW:
            return CXO_TRANSFORM_BINARY;
        case DPI_ORACLE_TYPE_NATIVE_DOUBLE:
            return CXO_TRANSFORM_NATIVE_DOUBLE;
        case DPI_ORACLE_TYPE_NATIVE_FLOAT:
            return CXO_TRANSFORM_NATIVE_FLOAT;
        case DPI_ORACLE_TYPE_NUMBER:
            if (info->scale == 0 ||
                    (info->scale == -127 && info->precision == 0))
                return CXO_TRANSFORM_INT;
            return CXO_TRANSFORM_FLOAT;
        case DPI_ORACLE_TYPE_NATIVE_INT:
            return CXO_TRANSFORM_NATIVE_INT;
        case DPI_ORACLE_TYPE_DATE:
            return CXO_TRANSFORM_DATETIME;
        case DPI_ORACLE_TYPE_TIMESTAMP:
            return CXO_TRANSFORM_TIMESTAMP;
        case DPI_ORACLE_TYPE_TIMESTAMP_TZ:
            return CXO_TRANSFORM_TIMESTAMP_TZ;
        case DPI_ORACLE_TYPE_TIMESTAMP_LTZ:
            return CXO_TRANSFORM_TIMESTAMP_LTZ;
        case DPI_ORACLE_TYPE_INTERVAL_DS:
            return CXO_TRANSFORM_TIMEDELTA;
        case DPI_ORACLE_TYPE_CLOB:
            return CXO_TRANSFORM_CLOB;
        case DPI_ORACLE_TYPE_NCLOB:
            return CXO_TRANSFORM_NCLOB;
        case DPI_ORACLE_TYPE_BLOB:
            return CXO_TRANSFORM_BLOB;
        case DPI_ORACLE_TYPE_BFILE:
            return CXO_TRANSFORM_BFILE;
        case DPI_ORACLE_TYPE_STMT: 
            return CXO_TRANSFORM_CURSOR;
        case DPI_ORACLE_TYPE_OBJECT:
            return CXO_TRANSFORM_OBJECT;
        case DPI_ORACLE_TYPE_LONG_VARCHAR:
            return CXO_TRANSFORM_LONG_STRING;
        case DPI_ORACLE_TYPE_LONG_RAW:
            return CXO_TRANSFORM_LONG_BINARY;
        case DPI_ORACLE_TYPE_BOOLEAN:
            return CXO_TRANSFORM_BOOLEAN;
        case DPI_ORACLE_TYPE_JSON:
            return CXO_TRANSFORM_JSON;
        default:
            break;
    }
    return CXO_TRANSFORM_UNSUPPORTED;
}


//-----------------------------------------------------------------------------
// cxoTransform_getNumFromPythonType()
//   Get the appropriate transformation to use for the specified Python type.
//-----------------------------------------------------------------------------
static cxoTransformNum cxoTransform_getNumFromPythonType(PyTypeObject *type)
{
    if (type == &PyUnicode_Type)
        return CXO_TRANSFORM_STRING;
    if (type == &PyBytes_Type)
        return CXO_TRANSFORM_BINARY;
    if (type == &PyFloat_Type)
        return CXO_TRANSFORM_FLOAT;
    if (type == &PyLong_Type)
        return CXO_TRANSFORM_INT;
    if (type == cxoPyTypeDecimal)
        return CXO_TRANSFORM_DECIMAL;
    if (type == &PyBool_Type)
        return CXO_TRANSFORM_BOOLEAN;
    if (type == PyDateTimeAPI->DateType)
        return CXO_TRANSFORM_DATE;
    if (type == PyDateTimeAPI->DateTimeType)
        return CXO_TRANSFORM_DATETIME;
    if (type == PyDateTimeAPI->DeltaType)
        return CXO_TRANSFORM_TIMEDELTA;

    return CXO_TRANSFORM_UNSUPPORTED;
}


//-----------------------------------------------------------------------------
// cxoTransform_getNumFromPythonValue()
//   Get the appropriate transformation to use for the specified Python value.
//-----------------------------------------------------------------------------
cxoTransformNum cxoTransform_getNumFromPythonValue(PyObject *value, int plsql)
{
    cxoLob *lob;

    if (value == Py_None)
        return CXO_TRANSFORM_NONE;
    if (PyBool_Check(value)) {
        if (cxoClientVersionInfo.versionNum < 12 || !plsql)
            return CXO_TRANSFORM_NATIVE_INT;
        return CXO_TRANSFORM_BOOLEAN;
    }
    if (PyUnicode_Check(value))
        return CXO_TRANSFORM_STRING;
    if (PyBytes_Check(value))
        return CXO_TRANSFORM_BINARY;
    if (PyLong_Check(value))
        return CXO_TRANSFORM_INT;
    if (PyFloat_Check(value))
        return CXO_TRANSFORM_FLOAT;
    if (PyDateTime_Check(value))
        return CXO_TRANSFORM_DATETIME;
    if (PyDate_Check(value))
        return CXO_TRANSFORM_DATE;
    if (PyDelta_Check(value))
        return CXO_TRANSFORM_TIMEDELTA;
    if (PyObject_TypeCheck(value, &cxoPyTypeCursor))
        return CXO_TRANSFORM_CURSOR;
    if (PyObject_TypeCheck(value, cxoPyTypeDecimal))
        return CXO_TRANSFORM_DECIMAL;
    if (PyObject_TypeCheck(value, &cxoPyTypeObject))
        return CXO_TRANSFORM_OBJECT;
    if (PyObject_TypeCheck(value, &cxoPyTypeLob)) {
        lob = (cxoLob*) value;
        return lob->dbType->defaultTransformNum;
    }
    return CXO_TRANSFORM_UNSUPPORTED;
}


//-----------------------------------------------------------------------------
// cxoTransform_getNumFromType()
//   Get the appropriate transformation to use for the specified type. This
// can be either a database type constant defined at the module level or a
// Python type.
//-----------------------------------------------------------------------------
int cxoTransform_getNumFromType(PyObject *type, cxoTransformNum *transformNum,
        cxoObjectType **objType)
{
    PyTypeObject *pyType;
    cxoApiType *apiType;
    cxoDbType *dbType;
    char message[250];
    int status;

    // check to see if a database type constant has been specified
    status = PyObject_IsInstance(type, (PyObject*) &cxoPyTypeDbType);
    if (status < 0)
        return -1;
    if (status == 1) {
        dbType = (cxoDbType*) type;
        *transformNum = dbType->defaultTransformNum;
        *objType = NULL;
        return 0;
    }

    // check to see if a DB API type constant has been specified
    status = PyObject_IsInstance(type, (PyObject*) &cxoPyTypeApiType);
    if (status < 0)
        return -1;
    if (status == 1) {
        apiType = (cxoApiType*) type;
        *transformNum = apiType->defaultTransformNum;
        *objType = NULL;
        return 0;
    }

    // check to see if an object type has been specified
    if (Py_TYPE(type) == &cxoPyTypeObjectType) {
        *transformNum = CXO_TRANSFORM_OBJECT;
        *objType = (cxoObjectType*) type;
        return 0;
    }

    // check to see if a Python type has been specified
    if (Py_TYPE(type) != &PyType_Type) {
        PyErr_SetString(PyExc_TypeError, "expecting type");
        return -1;
    }

    // check to see if the Python type is a supported type
    pyType = (PyTypeObject*) type;
    *objType = NULL;
    *transformNum = cxoTransform_getNumFromPythonType(pyType);
    if (*transformNum != CXO_TRANSFORM_UNSUPPORTED)
        return 0;

    // no valid type specified
    snprintf(message, sizeof(message), "Python type %s not supported.",
            pyType->tp_name);
    cxoError_raiseFromString(cxoNotSupportedErrorException, message);
    return -1;
}


//-----------------------------------------------------------------------------
// cxoTransform_getNumFromValue()
//   Get the appropriate transformation to use for the specified value. If the
// value is an array, determine the transformation that can be used for all of
// the elements in that array.
//-----------------------------------------------------------------------------
int cxoTransform_getNumFromValue(PyObject *value, int *isArray,
        Py_ssize_t *size, Py_ssize_t *numElements, int plsql,
        cxoTransformNum *transformNum)
{
    cxoTransformNum tempTransformNum;
    PyObject *elementValue;
    Py_ssize_t i, tempSize;
    char message[250];

    // initialization (except numElements which always has a valid value and is
    // only overridden when a an array is encountered)
    *size = 0;
    *isArray = 0;

    // handle arrays
    if (PyList_Check(value)) {
        *transformNum = CXO_TRANSFORM_NONE;
        for (i = 0; i < PyList_GET_SIZE(value); i++) {
            elementValue = PyList_GET_ITEM(value, i);
            tempTransformNum = cxoTransform_getNumFromPythonValue(elementValue,
                    1);
            if (tempTransformNum == CXO_TRANSFORM_UNSUPPORTED) {
                snprintf(message, sizeof(message),
                        "element %u value of type %s is not supported",
                        (unsigned) i, Py_TYPE(value)->tp_name);
                cxoError_raiseFromString(cxoNotSupportedErrorException,
                        message);
                return -1;
            } else if (*transformNum == CXO_TRANSFORM_NONE) {
                *transformNum = tempTransformNum;
            } else if (*transformNum != tempTransformNum) {
                snprintf(message, sizeof(message),
                        "element %u value is not the same type as previous "
                        "elements", (unsigned) i);
                cxoError_raiseFromString(cxoNotSupportedErrorException,
                        message);
                return -1;
            }
            tempSize = cxoTransform_calculateSize(elementValue, *transformNum);
            if (tempSize > *size)
                *size = tempSize;
        }
        *isArray = 1;
        *numElements = PyList_GET_SIZE(value);
        return 0;
    }

    // handle scalar values
    *transformNum = cxoTransform_getNumFromPythonValue(value, plsql);
    if (*transformNum == CXO_TRANSFORM_UNSUPPORTED) {
        snprintf(message, sizeof(message),
                "Python value of type %s not supported.",
                Py_TYPE(value)->tp_name);
        cxoError_raiseFromString(cxoNotSupportedErrorException, message);
        return -1;
    }
    *size = cxoTransform_calculateSize(value, *transformNum);
    return 0;
}


//-----------------------------------------------------------------------------
// cxoTransform_getTypeInfo()
//   Get type information for the specified transform. The transform number is
// assumed to be a valid value at this point (not CXO_TRANSFORM_UNSUPPORTED).
//-----------------------------------------------------------------------------
void cxoTransform_getTypeInfo(cxoTransformNum transformNum,
        dpiOracleTypeNum *oracleTypeNum, dpiNativeTypeNum *nativeTypeNum)
{
    const cxoTransform *transform;

    transform = &cxoAllTransforms[transformNum];
    *oracleTypeNum = transform->oracleTypeNum;
    *nativeTypeNum = transform->nativeTypeNum;
}


//-----------------------------------------------------------------------------
// cxoTransform_init()
//   Import the necessary modules for performing transformations.
//-----------------------------------------------------------------------------
int cxoTransform_init(void)
{
    PyObject *module;

    // import the datetime module for datetime support
    PyDateTime_IMPORT;
    if (PyErr_Occurred())
        return -1;
    cxoPyTypeDate = PyDateTimeAPI->DateType;
    cxoPyTypeDateTime = PyDateTimeAPI->DateTimeType;

    // import the decimal module for decimal support
    module = PyImport_ImportModule("decimal");
    if (!module)
        return -1;
    cxoPyTypeDecimal =
            (PyTypeObject*) PyObject_GetAttrString(module, "Decimal");
    Py_DECREF(module);
    if (!cxoPyTypeDecimal)
        return -1;

    return 0;
}


//-----------------------------------------------------------------------------
// cxoTransform_timestampFromTicks()
//   Creates a timestamp from ticks (number of seconds since Unix epoch).
//-----------------------------------------------------------------------------
PyObject *cxoTransform_timestampFromTicks(PyObject *args)
{
    return PyDateTime_FromTimestamp(args);
}


//-----------------------------------------------------------------------------
// cxoTransform_toPython()
//   Transforms a database value into its corresponding Python object.
//-----------------------------------------------------------------------------
PyObject *cxoTransform_toPython(cxoTransformNum transformNum,
        cxoConnection *connection, cxoObjectType *objType,
        dpiDataBuffer *dbValue, const char *encodingErrors)
{
    PyObject *stringObj, *result;
    dpiIntervalDS *intervalDS;
    dpiTimestamp *timestamp;
    dpiJsonNode *jsonNode;
    uint32_t rowidLength;
    cxoDbType *dbType;
    const char *rowid;
    cxoCursor *cursor;
    dpiBytes *bytes;
    int32_t seconds;

    switch (transformNum) {
        case CXO_TRANSFORM_BINARY:
        case CXO_TRANSFORM_LONG_BINARY:
            bytes = &dbValue->asBytes;
            return PyBytes_FromStringAndSize(bytes->ptr, bytes->length);
        case CXO_TRANSFORM_BFILE:
        case CXO_TRANSFORM_BLOB:
        case CXO_TRANSFORM_CLOB:
        case CXO_TRANSFORM_NCLOB:
            dbType = cxoDbType_fromTransformNum(transformNum);
            return cxoLob_new(connection, dbType, dbValue->asLOB);
        case CXO_TRANSFORM_BOOLEAN:
            if (dbValue->asBoolean)
                Py_RETURN_TRUE;
            Py_RETURN_FALSE;
        case CXO_TRANSFORM_CURSOR:
            cursor = (cxoCursor*) PyObject_CallMethod((PyObject*) connection,
                    "cursor", NULL);
            if (!cursor)
                return NULL;
            cursor->handle = dbValue->asStmt;
            dpiStmt_addRef(cursor->handle);
            cursor->fixupRefCursor = 1;
            return (PyObject*) cursor;
        case CXO_TRANSFORM_DATE:
            timestamp = &dbValue->asTimestamp;
            return PyDate_FromDate(timestamp->year, timestamp->month,
                    timestamp->day);
        case CXO_TRANSFORM_DATETIME:
        case CXO_TRANSFORM_TIMESTAMP:
        case CXO_TRANSFORM_TIMESTAMP_LTZ:
        case CXO_TRANSFORM_TIMESTAMP_TZ:
            timestamp = &dbValue->asTimestamp;
            return PyDateTime_FromDateAndTime(timestamp->year,
                    timestamp->month, timestamp->day, timestamp->hour,
                    timestamp->minute, timestamp->second,
                    timestamp->fsecond / 1000);
        case CXO_TRANSFORM_FIXED_CHAR:
        case CXO_TRANSFORM_FIXED_NCHAR:
        case CXO_TRANSFORM_LONG_STRING:
        case CXO_TRANSFORM_NSTRING:
        case CXO_TRANSFORM_STRING:
            bytes = &dbValue->asBytes;
            return PyUnicode_Decode(bytes->ptr, bytes->length, bytes->encoding,
                    encodingErrors);
        case CXO_TRANSFORM_NATIVE_DOUBLE:
            return PyFloat_FromDouble(dbValue->asDouble);
        case CXO_TRANSFORM_NATIVE_FLOAT:
            return PyFloat_FromDouble(dbValue->asFloat);
        case CXO_TRANSFORM_NATIVE_INT:
            return PyLong_FromLongLong(dbValue->asInt64);
        case CXO_TRANSFORM_DECIMAL:
        case CXO_TRANSFORM_INT:
        case CXO_TRANSFORM_FLOAT:
            bytes = &dbValue->asBytes;
            stringObj = PyUnicode_Decode(bytes->ptr, bytes->length,
                    bytes->encoding, encodingErrors);
            if (!stringObj)
                return NULL;
            if (transformNum == CXO_TRANSFORM_INT &&
                    memchr(bytes->ptr, '.', bytes->length) == NULL) {
                result = PyNumber_Long(stringObj);
            } else if (transformNum == CXO_TRANSFORM_DECIMAL) {
                result = PyObject_CallFunctionObjArgs(
                        (PyObject*) cxoPyTypeDecimal, stringObj, NULL);
            } else {
                result = PyNumber_Float(stringObj);
            }
            Py_DECREF(stringObj);
            return result;
        case CXO_TRANSFORM_OBJECT:
            return cxoObject_new(objType, dbValue->asObject);
        case CXO_TRANSFORM_ROWID:
            if (dpiRowid_getStringValue(dbValue->asRowid, &rowid,
                    &rowidLength) < 0)
                return cxoError_raiseAndReturnNull();
            return PyUnicode_Decode(rowid, rowidLength,
                    connection->encodingInfo.encoding, NULL);
        case CXO_TRANSFORM_JSON:
            if (dpiJson_getValue(dbValue->asJson,
                    DPI_JSON_OPT_NUMBER_AS_STRING, &jsonNode) < 0)
                return cxoError_raiseAndReturnNull();
            return cxoTransform_toPythonFromJson(connection, jsonNode,
                    encodingErrors);
        case CXO_TRANSFORM_TIMEDELTA:
            intervalDS = &dbValue->asIntervalDS; 
            seconds = intervalDS->hours * 60 * 60 + intervalDS->minutes * 60 +
                    intervalDS->seconds;
            return PyDelta_FromDSU(intervalDS->days, seconds,
                    intervalDS->fseconds / 1000);
        default:
            break;
    }

    return cxoError_raiseFromString(cxoNotSupportedErrorException,
            "Database value cannot be converted to a Python value");
}


//-----------------------------------------------------------------------------
// cxoTransform_toPythonFromJson()
//   Transforms a JSON node to its equivalent Python value.
//-----------------------------------------------------------------------------
static PyObject *cxoTransform_toPythonFromJson(cxoConnection *connection,
        dpiJsonNode *node, const char *encodingErrors)
{
    PyObject *result, *temp, *name;
    cxoTransformNum transformNum;
    dpiJsonArray *array;
    dpiJsonObject *obj;
    uint32_t i;

    // null is a special case
    if (node->nativeTypeNum == DPI_NATIVE_TYPE_NULL)
        Py_RETURN_NONE;

    switch (node->oracleTypeNum) {
        case DPI_ORACLE_TYPE_NUMBER:
            transformNum = (node->nativeTypeNum == DPI_NATIVE_TYPE_DOUBLE) ?
                    CXO_TRANSFORM_NATIVE_DOUBLE : CXO_TRANSFORM_DECIMAL;
            break;
        case DPI_ORACLE_TYPE_VARCHAR:
            transformNum = CXO_TRANSFORM_STRING;
            break;
        case DPI_ORACLE_TYPE_RAW:
            transformNum = CXO_TRANSFORM_BINARY;
            break;
        case DPI_ORACLE_TYPE_DATE:
        case DPI_ORACLE_TYPE_TIMESTAMP:
            transformNum = CXO_TRANSFORM_DATETIME;
            break;
        case DPI_ORACLE_TYPE_BOOLEAN:
            transformNum = CXO_TRANSFORM_BOOLEAN;
            break;
        case DPI_ORACLE_TYPE_INTERVAL_DS:
            transformNum = CXO_TRANSFORM_TIMEDELTA;
            break;
        case DPI_ORACLE_TYPE_JSON_OBJECT:
            obj = &node->value->asJsonObject;
            result = PyDict_New();
            for (i = 0; i < obj->numFields; i++) {
                name = PyUnicode_DecodeUTF8(obj->fieldNames[i],
                        obj->fieldNameLengths[i], NULL);
                if (!name)
                    return NULL;
                temp = cxoTransform_toPythonFromJson(connection,
                        &obj->fields[i], encodingErrors);
                if (!temp)
                    return NULL;
                if (PyDict_SetItem(result, name, temp) < 0) {
                    Py_DECREF(name);
                    Py_DECREF(temp);
                    return NULL;
                }
                Py_DECREF(name);
                Py_DECREF(temp);
            }
            return result;
        case DPI_ORACLE_TYPE_JSON_ARRAY:
            array = &node->value->asJsonArray;
            result = PyList_New(array->numElements);
            for (i = 0; i < array->numElements; i++) {
                temp = cxoTransform_toPythonFromJson(connection,
                        &array->elements[i], encodingErrors);
                if (!temp) {
                    Py_DECREF(result);
                    return NULL;
                }
                PyList_SET_ITEM(result, i, temp);
            }
            return result;
        default:
            transformNum = CXO_TRANSFORM_UNSUPPORTED;
    }

    return cxoTransform_toPython(transformNum, connection, NULL,
            node->value, encodingErrors);
}
