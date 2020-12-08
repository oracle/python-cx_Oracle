//-----------------------------------------------------------------------------
// Copyright (c) 2016, 2020, Oracle and/or its affiliates. All rights reserved.
//
// Portions Copyright 2007-2015, Anthony Tuininga. All rights reserved.
//
// Portions Copyright 2001-2007, Computronix (Canada) Ltd., Edmonton, Alberta,
// Canada. All rights reserved.
//
// Licensed under BSD license (see LICENSE.txt).
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// cxoModule.c
//   Implementation of cx_Oracle module.
//-----------------------------------------------------------------------------

#include "cxoModule.h"

// define macro for adding integer constants
#define CXO_ADD_INT_CONSTANT(name, value) \
    if (PyModule_AddIntConstant(module, name, value) < 0) \
        return NULL;

// define macro for adding Python Database API types
#define CXO_ADD_API_TYPE(name, transformNum, typeObj) \
    if (cxoModule_addApiType(module, name, transformNum, typeObj) < 0) \
        return NULL;

// define macro for adding database types
#define CXO_ADD_DB_TYPE(num, name, transformNum, typeObj) \
    if (cxoModule_addDbType(module, num, name, transformNum, typeObj) < 0) \
        return NULL;

// define macro for associating database types with Database API types
#define CXO_ASSOCIATE_DB_TYPE(apiType, dbType) \
    if (PyList_Append(apiType->dbTypes, (PyObject*) dbType) < 0) \
        return NULL;

// define macro for adding type objects
#define CXO_ADD_TYPE_OBJECT(name, type) \
    Py_INCREF(type); \
    if (PyModule_AddObject(module, name, (PyObject*) type) < 0) \
        return NULL;

// define macro for and making types ready
#define CXO_MAKE_TYPE_READY(type) \
    if (PyType_Ready(type) < 0) \
        return NULL;


//-----------------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------------
PyObject *cxoWarningException = NULL;
PyObject *cxoErrorException = NULL;
PyObject *cxoInterfaceErrorException = NULL;
PyObject *cxoDatabaseErrorException = NULL;
PyObject *cxoDataErrorException = NULL;
PyObject *cxoOperationalErrorException = NULL;
PyObject *cxoIntegrityErrorException = NULL;
PyObject *cxoInternalErrorException = NULL;
PyObject *cxoProgrammingErrorException = NULL;
PyObject *cxoNotSupportedErrorException = NULL;
PyObject *cxoJsonDumpFunction = NULL;
PyObject *cxoJsonLoadFunction = NULL;

cxoDbType *cxoDbTypeBfile = NULL;
cxoDbType *cxoDbTypeBinaryDouble = NULL;
cxoDbType *cxoDbTypeBinaryFloat = NULL;
cxoDbType *cxoDbTypeBinaryInteger = NULL;
cxoDbType *cxoDbTypeBlob = NULL;
cxoDbType *cxoDbTypeBoolean = NULL;
cxoDbType *cxoDbTypeChar = NULL;
cxoDbType *cxoDbTypeClob = NULL;
cxoDbType *cxoDbTypeCursor = NULL;
cxoDbType *cxoDbTypeDate = NULL;
cxoDbType *cxoDbTypeIntervalDS = NULL;
cxoDbType *cxoDbTypeIntervalYM = NULL;
cxoDbType *cxoDbTypeJson = NULL;
cxoDbType *cxoDbTypeLong = NULL;
cxoDbType *cxoDbTypeLongRaw = NULL;
cxoDbType *cxoDbTypeNchar = NULL;
cxoDbType *cxoDbTypeNclob = NULL;
cxoDbType *cxoDbTypeNumber = NULL;
cxoDbType *cxoDbTypeNvarchar = NULL;
cxoDbType *cxoDbTypeObject = NULL;
cxoDbType *cxoDbTypeRaw = NULL;
cxoDbType *cxoDbTypeRowid = NULL;
cxoDbType *cxoDbTypeTimestamp = NULL;
cxoDbType *cxoDbTypeTimestampLTZ = NULL;
cxoDbType *cxoDbTypeTimestampTZ = NULL;
cxoDbType *cxoDbTypeVarchar = NULL;

cxoApiType *cxoApiTypeBinary = NULL;
cxoApiType *cxoApiTypeDatetime = NULL;
cxoApiType *cxoApiTypeNumber = NULL;
cxoApiType *cxoApiTypeRowid = NULL;
cxoApiType *cxoApiTypeString = NULL;

cxoFuture *cxoFutureObj = NULL;
dpiContext *cxoDpiContext = NULL;
dpiVersionInfo cxoClientVersionInfo;


//-----------------------------------------------------------------------------
// cxoModule_addApiType()
//   Create a Python Database API type and add it to the module.
//-----------------------------------------------------------------------------
static int cxoModule_addApiType(PyObject *module, const char *name,
        cxoTransformNum defaultTransformNum, cxoApiType **apiType)
{
    cxoApiType *tempApiType;

    tempApiType =
            (cxoApiType*) cxoPyTypeApiType.tp_alloc(&cxoPyTypeApiType, 0);
    if (!tempApiType)
        return -1;
    tempApiType->name = name;
    tempApiType->defaultTransformNum = defaultTransformNum;
    tempApiType->dbTypes = PyList_New(0);
    if (!tempApiType->dbTypes) {
        Py_DECREF(tempApiType);
        return -1;
    }
    if (PyModule_AddObject(module, name, (PyObject*) tempApiType) < 0) {
        Py_DECREF(tempApiType);
        return -1;
    }
    *apiType = tempApiType;
    return 0;
}


//-----------------------------------------------------------------------------
// cxoModule_addDbType()
//   Create a database type and add it to the module.
//-----------------------------------------------------------------------------
static int cxoModule_addDbType(PyObject *module, uint32_t num,
        const char *name, cxoTransformNum defaultTransformNum,
        cxoDbType **dbType)
{
    cxoDbType *tempDbType;

    tempDbType = (cxoDbType*) cxoPyTypeDbType.tp_alloc(&cxoPyTypeDbType, 0);
    if (!tempDbType)
        return -1;
    tempDbType->num = num;
    tempDbType->name = name;
    tempDbType->defaultTransformNum = defaultTransformNum;
    if (PyModule_AddObject(module, name, (PyObject*) tempDbType) < 0) {
        Py_DECREF(tempDbType);
        return -1;
    }
    *dbType = tempDbType;
    return 0;
}


//-----------------------------------------------------------------------------
// cxoModule_setException()
//   Create an exception and set it in the provided dictionary.
//-----------------------------------------------------------------------------
static int cxoModule_setException(PyObject *module, PyObject **exception,
        char *name, PyObject *baseException)
{
    char buffer[100];

    sprintf(buffer, "cx_Oracle.%s", name);
    *exception = PyErr_NewException(buffer, baseException, NULL);
    if (!*exception)
        return -1;
    return PyModule_AddObject(module, name, *exception);
}


//-----------------------------------------------------------------------------
// cxoModule_makeDSN()
//   Make a data source name given the host port and SID.
//-----------------------------------------------------------------------------
static PyObject* cxoModule_makeDSN(PyObject* self, PyObject* args,
        PyObject* keywordArgs)
{
    static const unsigned int numConnectDataArgs = 5;
    static char *keywordList[] = { "host", "port", "sid", "service_name",
            "region", "sharding_key", "super_sharding_key", NULL };
    PyObject *result, *connectData, *hostObj, *portObj;
    char connectDataFormat[72], *sourcePtr, *targetPtr;
    PyObject *connectDataArgs[5], *formatArgsArray;
    unsigned int i;

    // parse arguments
    for (i = 0; i < numConnectDataArgs; i++)
        connectDataArgs[i] = NULL;
    if (!PyArg_ParseTupleAndKeywords(args, keywordArgs, "OO|OOOOO",
            keywordList, &hostObj, &portObj, &connectDataArgs[0],
            &connectDataArgs[1], &connectDataArgs[2], &connectDataArgs[3],
            &connectDataArgs[4]))
        return NULL;

    // create list for connect data format arguments
    formatArgsArray = PyList_New(0);
    if (!formatArgsArray)
        return NULL;

    // process each of the connect data arguments
    // build up a format string and a list of format arguments
    targetPtr = connectDataFormat;
    *targetPtr = '\0';
    for (i = 0; i < numConnectDataArgs; i++) {
        if (connectDataArgs[i]) {
            if (PyList_Append(formatArgsArray, connectDataArgs[i]) < 0) {
                Py_DECREF(formatArgsArray);
                return NULL;
            }
            sourcePtr = keywordList[i + 2];
            *targetPtr++ = '(';
            while (*sourcePtr)
                *targetPtr++ = toupper(*sourcePtr++);
            *targetPtr++ = '=';
            *targetPtr++ = '%';
            *targetPtr++ = 's';
            *targetPtr++ = ')';
            *targetPtr = '\0';
        }
    }

    // determine connect data
    connectData = cxoUtils_formatString(connectDataFormat,
            PyList_AsTuple(formatArgsArray));
    Py_DECREF(formatArgsArray);
    if (!connectData)
        return NULL;

    // perform overall format
    result = cxoUtils_formatString("(DESCRIPTION=(ADDRESS="
            "(PROTOCOL=TCP)(HOST=%s)(PORT=%s))(CONNECT_DATA=%s))",
            PyTuple_Pack(3, hostObj, portObj, connectData));
    Py_DECREF(connectData);
    return result;
}


//-----------------------------------------------------------------------------
// cxoModule_clientVersion()
//   Return the version of the Oracle client being used as a 5-tuple.
//-----------------------------------------------------------------------------
static PyObject* cxoModule_clientVersion(PyObject* self, PyObject* args)
{
    if (cxoUtils_initializeDPI(NULL) < 0)
        return NULL;
    return Py_BuildValue("(iiiii)", cxoClientVersionInfo.versionNum,
            cxoClientVersionInfo.releaseNum, cxoClientVersionInfo.updateNum,
            cxoClientVersionInfo.portReleaseNum,
            cxoClientVersionInfo.portUpdateNum);
}


//-----------------------------------------------------------------------------
// cxoModule_initClientLib()
//   Initialize the client library now, rather than when the first call to
// get the Oracle Client library version, create a standalone connection or
// session pool is performed.
//-----------------------------------------------------------------------------
static PyObject* cxoModule_initClientLib(PyObject* self, PyObject* args,
        PyObject* keywordArgs)
{
    static char *keywordList[] = { "lib_dir", "config_dir", "error_url",
            "driver_name", NULL };
    Py_ssize_t libDirSize, configDirSize, errorUrlSize, driverNameSize;
    dpiContextCreateParams params;

    memset(&params, 0, sizeof(dpiContextCreateParams));
    libDirSize = configDirSize = errorUrlSize = driverNameSize = 0;
    if (!PyArg_ParseTupleAndKeywords(args, keywordArgs, "|z#z#z#z#",
            keywordList, &params.oracleClientLibDir, &libDirSize,
            &params.oracleClientConfigDir, &configDirSize,
            &params.loadErrorUrl, &errorUrlSize, &params.defaultDriverName,
            &driverNameSize))
        return NULL;
    if (libDirSize == 0)
        params.oracleClientLibDir = NULL;
    if (configDirSize == 0)
        params.oracleClientConfigDir = NULL;
    if (errorUrlSize == 0)
        params.loadErrorUrl = NULL;
    if (driverNameSize == 0)
        params.defaultDriverName = NULL;
    if (cxoUtils_initializeDPI(&params) < 0)
        return NULL;

    Py_RETURN_NONE;
}


//-----------------------------------------------------------------------------
// cxoModule_time()
//   Returns a time value suitable for binding.
//-----------------------------------------------------------------------------
static PyObject* cxoModule_time(PyObject* self, PyObject* args)
{
    return cxoError_raiseFromString(cxoNotSupportedErrorException,
            "Oracle does not support time only variables");
}


//-----------------------------------------------------------------------------
// cxoModule_timeFromTicks()
//   Returns a time value suitable for binding.
//-----------------------------------------------------------------------------
static PyObject* cxoModule_timeFromTicks(PyObject* self, PyObject* args)
{
    return cxoError_raiseFromString(cxoNotSupportedErrorException,
            "Oracle does not support time only variables");
}


//-----------------------------------------------------------------------------
// cxoModule_dateFromTicks()
//   Returns a date value suitable for binding.
//-----------------------------------------------------------------------------
static PyObject* cxoModule_dateFromTicks(PyObject* self, PyObject* args)
{
    return cxoTransform_dateFromTicks(args);
}


//-----------------------------------------------------------------------------
// cxoModule_timestampFromTicks()
//   Returns a date value suitable for binding.
//-----------------------------------------------------------------------------
static PyObject* cxoModule_timestampFromTicks(PyObject* self, PyObject* args)
{
    return cxoTransform_timestampFromTicks(args);
}


//-----------------------------------------------------------------------------
// Declaration of methods supported by this module
//-----------------------------------------------------------------------------
static PyMethodDef cxoModuleMethods[] = {
    { "makedsn", (PyCFunction) cxoModule_makeDSN,
            METH_VARARGS | METH_KEYWORDS },
    { "Time", (PyCFunction) cxoModule_time, METH_VARARGS },
    { "DateFromTicks", (PyCFunction) cxoModule_dateFromTicks, METH_VARARGS },
    { "TimeFromTicks", (PyCFunction) cxoModule_timeFromTicks, METH_VARARGS },
    { "TimestampFromTicks", (PyCFunction) cxoModule_timestampFromTicks,
            METH_VARARGS },
    { "clientversion", (PyCFunction) cxoModule_clientVersion, METH_NOARGS },
    { "init_oracle_client", (PyCFunction) cxoModule_initClientLib,
            METH_VARARGS | METH_KEYWORDS },
    { NULL }
};


//-----------------------------------------------------------------------------
// Declaration of module definition
//-----------------------------------------------------------------------------
static struct PyModuleDef cxoModuleDef = {
    PyModuleDef_HEAD_INIT,
    "cx_Oracle",
    NULL,
    -1,
    cxoModuleMethods,                      // methods
    NULL,                                  // m_reload
    NULL,                                  // traverse
    NULL,                                  // clear
    NULL                                   // free
};


//-----------------------------------------------------------------------------
// cxoModule_initialize()
//   Initialization routine for the module.
//-----------------------------------------------------------------------------
static PyObject *cxoModule_initialize(void)
{
    PyObject *module;

    // initialize transforms
    if (cxoTransform_init() < 0)
        return NULL;

    // prepare the types for use by the module
    CXO_MAKE_TYPE_READY(&cxoPyTypeApiType);
    CXO_MAKE_TYPE_READY(&cxoPyTypeConnection);
    CXO_MAKE_TYPE_READY(&cxoPyTypeCursor);
    CXO_MAKE_TYPE_READY(&cxoPyTypeDbType);
    CXO_MAKE_TYPE_READY(&cxoPyTypeDeqOptions);
    CXO_MAKE_TYPE_READY(&cxoPyTypeEnqOptions);
    CXO_MAKE_TYPE_READY(&cxoPyTypeError);
    CXO_MAKE_TYPE_READY(&cxoPyTypeFuture);
    CXO_MAKE_TYPE_READY(&cxoPyTypeLob);
    CXO_MAKE_TYPE_READY(&cxoPyTypeMsgProps);
    CXO_MAKE_TYPE_READY(&cxoPyTypeMessage);
    CXO_MAKE_TYPE_READY(&cxoPyTypeMessageQuery);
    CXO_MAKE_TYPE_READY(&cxoPyTypeMessageRow);
    CXO_MAKE_TYPE_READY(&cxoPyTypeMessageTable);
    CXO_MAKE_TYPE_READY(&cxoPyTypeObjectAttr);
    CXO_MAKE_TYPE_READY(&cxoPyTypeObject);
    CXO_MAKE_TYPE_READY(&cxoPyTypeObjectType);
    CXO_MAKE_TYPE_READY(&cxoPyTypeQueue);
    CXO_MAKE_TYPE_READY(&cxoPyTypeSessionPool);
    CXO_MAKE_TYPE_READY(&cxoPyTypeSodaCollection);
    CXO_MAKE_TYPE_READY(&cxoPyTypeSodaDatabase);
    CXO_MAKE_TYPE_READY(&cxoPyTypeSodaDoc);
    CXO_MAKE_TYPE_READY(&cxoPyTypeSodaDocCursor);
    CXO_MAKE_TYPE_READY(&cxoPyTypeSodaOperation);
    CXO_MAKE_TYPE_READY(&cxoPyTypeSubscr);
    CXO_MAKE_TYPE_READY(&cxoPyTypeVar);

    // initialize module and retrieve the dictionary
    module = PyModule_Create(&cxoModuleDef);
    if (!module)
        return NULL;

    // create exception object and add it to the dictionary
    if (cxoModule_setException(module, &cxoWarningException,
            "Warning", NULL) < 0)
        return NULL;
    if (cxoModule_setException(module, &cxoErrorException,
            "Error", NULL) < 0)
        return NULL;
    if (cxoModule_setException(module, &cxoInterfaceErrorException,
            "InterfaceError", cxoErrorException) < 0)
        return NULL;
    if (cxoModule_setException(module, &cxoDatabaseErrorException,
            "DatabaseError", cxoErrorException) < 0)
        return NULL;
    if (cxoModule_setException(module, &cxoDataErrorException,
            "DataError", cxoDatabaseErrorException) < 0)
        return NULL;
    if (cxoModule_setException(module, &cxoOperationalErrorException,
            "OperationalError", cxoDatabaseErrorException) < 0)
        return NULL;
    if (cxoModule_setException(module, &cxoIntegrityErrorException,
            "IntegrityError", cxoDatabaseErrorException) < 0)
        return NULL;
    if (cxoModule_setException(module, &cxoInternalErrorException,
            "InternalError", cxoDatabaseErrorException) < 0)
        return NULL;
    if (cxoModule_setException(module, &cxoProgrammingErrorException,
            "ProgrammingError", cxoDatabaseErrorException) < 0)
        return NULL;
    if (cxoModule_setException(module, &cxoNotSupportedErrorException,
            "NotSupportedError", cxoDatabaseErrorException) < 0)
        return NULL;

    // set up the types that are available
    CXO_ADD_TYPE_OBJECT("ApiType", &cxoPyTypeApiType)
    CXO_ADD_TYPE_OBJECT("Binary", &PyBytes_Type)
    CXO_ADD_TYPE_OBJECT("Connection", &cxoPyTypeConnection)
    CXO_ADD_TYPE_OBJECT("Cursor", &cxoPyTypeCursor)
    CXO_ADD_TYPE_OBJECT("Date", cxoPyTypeDate)
    CXO_ADD_TYPE_OBJECT("DbType", &cxoPyTypeDbType)
    CXO_ADD_TYPE_OBJECT("DeqOptions", &cxoPyTypeDeqOptions)
    CXO_ADD_TYPE_OBJECT("EnqOptions", &cxoPyTypeEnqOptions)
    CXO_ADD_TYPE_OBJECT("_Error", &cxoPyTypeError)
    CXO_ADD_TYPE_OBJECT("LOB", &cxoPyTypeLob)
    CXO_ADD_TYPE_OBJECT("MessageProperties", &cxoPyTypeMsgProps)
    CXO_ADD_TYPE_OBJECT("Object", &cxoPyTypeObject)
    CXO_ADD_TYPE_OBJECT("ObjectType", &cxoPyTypeObjectType)
    CXO_ADD_TYPE_OBJECT("SessionPool", &cxoPyTypeSessionPool)
    CXO_ADD_TYPE_OBJECT("SodaCollection", &cxoPyTypeSodaCollection)
    CXO_ADD_TYPE_OBJECT("SodaDatabase", &cxoPyTypeSodaDatabase)
    CXO_ADD_TYPE_OBJECT("SodaDoc", &cxoPyTypeSodaDoc)
    CXO_ADD_TYPE_OBJECT("SodaDocCursor", &cxoPyTypeSodaDocCursor)
    CXO_ADD_TYPE_OBJECT("SodaOperation", &cxoPyTypeSodaOperation)
    CXO_ADD_TYPE_OBJECT("Timestamp", cxoPyTypeDateTime)
    CXO_ADD_TYPE_OBJECT("Var", &cxoPyTypeVar)

    // the name "connect" is required by the DB API
    CXO_ADD_TYPE_OBJECT("connect", &cxoPyTypeConnection)

    // create the database types (preferred names)
    CXO_ADD_DB_TYPE(DPI_ORACLE_TYPE_BFILE, "DB_TYPE_BFILE",
            CXO_TRANSFORM_BFILE, &cxoDbTypeBfile)
    CXO_ADD_DB_TYPE(DPI_ORACLE_TYPE_NATIVE_DOUBLE, "DB_TYPE_BINARY_DOUBLE",
            CXO_TRANSFORM_NATIVE_DOUBLE, &cxoDbTypeBinaryDouble)
    CXO_ADD_DB_TYPE(DPI_ORACLE_TYPE_NATIVE_FLOAT, "DB_TYPE_BINARY_FLOAT",
            CXO_TRANSFORM_NATIVE_FLOAT, &cxoDbTypeBinaryFloat)
    CXO_ADD_DB_TYPE(DPI_ORACLE_TYPE_NATIVE_INT, "DB_TYPE_BINARY_INTEGER",
            CXO_TRANSFORM_NATIVE_INT, &cxoDbTypeBinaryInteger)
    CXO_ADD_DB_TYPE(DPI_ORACLE_TYPE_BLOB, "DB_TYPE_BLOB",
            CXO_TRANSFORM_BLOB, &cxoDbTypeBlob)
    CXO_ADD_DB_TYPE(DPI_ORACLE_TYPE_BOOLEAN, "DB_TYPE_BOOLEAN",
            CXO_TRANSFORM_BOOLEAN, &cxoDbTypeBoolean)
    CXO_ADD_DB_TYPE(DPI_ORACLE_TYPE_CHAR, "DB_TYPE_CHAR",
            CXO_TRANSFORM_FIXED_CHAR, &cxoDbTypeChar)
    CXO_ADD_DB_TYPE(DPI_ORACLE_TYPE_CLOB, "DB_TYPE_CLOB",
            CXO_TRANSFORM_CLOB, &cxoDbTypeClob)
    CXO_ADD_DB_TYPE(DPI_ORACLE_TYPE_STMT, "DB_TYPE_CURSOR",
            CXO_TRANSFORM_CURSOR, &cxoDbTypeCursor)
    CXO_ADD_DB_TYPE(DPI_ORACLE_TYPE_DATE, "DB_TYPE_DATE",
            CXO_TRANSFORM_DATETIME, &cxoDbTypeDate)
    CXO_ADD_DB_TYPE(DPI_ORACLE_TYPE_INTERVAL_DS, "DB_TYPE_INTERVAL_DS",
            CXO_TRANSFORM_TIMEDELTA, &cxoDbTypeIntervalDS)
    CXO_ADD_DB_TYPE(DPI_ORACLE_TYPE_INTERVAL_YM, "DB_TYPE_INTERVAL_YM",
            CXO_TRANSFORM_UNSUPPORTED, &cxoDbTypeIntervalYM)
    CXO_ADD_DB_TYPE(DPI_ORACLE_TYPE_JSON, "DB_TYPE_JSON",
            CXO_TRANSFORM_JSON, &cxoDbTypeJson)
    CXO_ADD_DB_TYPE(DPI_ORACLE_TYPE_LONG_VARCHAR, "DB_TYPE_LONG",
            CXO_TRANSFORM_LONG_STRING, &cxoDbTypeLong)
    CXO_ADD_DB_TYPE(DPI_ORACLE_TYPE_LONG_RAW, "DB_TYPE_LONG_RAW",
            CXO_TRANSFORM_LONG_BINARY, &cxoDbTypeLongRaw)
    CXO_ADD_DB_TYPE(DPI_ORACLE_TYPE_NCHAR, "DB_TYPE_NCHAR",
            CXO_TRANSFORM_FIXED_NCHAR, &cxoDbTypeNchar)
    CXO_ADD_DB_TYPE(DPI_ORACLE_TYPE_NCLOB, "DB_TYPE_NCLOB",
            CXO_TRANSFORM_NCLOB, &cxoDbTypeNclob)
    CXO_ADD_DB_TYPE(DPI_ORACLE_TYPE_NUMBER, "DB_TYPE_NUMBER",
            CXO_TRANSFORM_FLOAT, &cxoDbTypeNumber)
    CXO_ADD_DB_TYPE(DPI_ORACLE_TYPE_NVARCHAR, "DB_TYPE_NVARCHAR",
            CXO_TRANSFORM_NSTRING, &cxoDbTypeNvarchar)
    CXO_ADD_DB_TYPE(DPI_ORACLE_TYPE_OBJECT, "DB_TYPE_OBJECT",
            CXO_TRANSFORM_OBJECT, &cxoDbTypeObject)
    CXO_ADD_DB_TYPE(DPI_ORACLE_TYPE_RAW, "DB_TYPE_RAW",
            CXO_TRANSFORM_BINARY, &cxoDbTypeRaw)
    CXO_ADD_DB_TYPE(DPI_ORACLE_TYPE_ROWID, "DB_TYPE_ROWID",
            CXO_TRANSFORM_ROWID, &cxoDbTypeRowid)
    CXO_ADD_DB_TYPE(DPI_ORACLE_TYPE_TIMESTAMP, "DB_TYPE_TIMESTAMP",
            CXO_TRANSFORM_TIMESTAMP, &cxoDbTypeTimestamp)
    CXO_ADD_DB_TYPE(DPI_ORACLE_TYPE_TIMESTAMP_LTZ, "DB_TYPE_TIMESTAMP_LTZ",
            CXO_TRANSFORM_TIMESTAMP_LTZ, &cxoDbTypeTimestampLTZ)
    CXO_ADD_DB_TYPE(DPI_ORACLE_TYPE_TIMESTAMP_TZ, "DB_TYPE_TIMESTAMP_TZ",
            CXO_TRANSFORM_TIMESTAMP_TZ, &cxoDbTypeTimestampTZ)
    CXO_ADD_DB_TYPE(DPI_ORACLE_TYPE_VARCHAR, "DB_TYPE_VARCHAR",
            CXO_TRANSFORM_STRING, &cxoDbTypeVarchar)

    // create the synonyms for database types (deprecated names)
    CXO_ADD_TYPE_OBJECT("BFILE", cxoDbTypeBfile)
    CXO_ADD_TYPE_OBJECT("BLOB", cxoDbTypeBlob)
    CXO_ADD_TYPE_OBJECT("CLOB", cxoDbTypeClob)
    CXO_ADD_TYPE_OBJECT("CURSOR", cxoDbTypeCursor)
    CXO_ADD_TYPE_OBJECT("OBJECT", cxoDbTypeObject)
    CXO_ADD_TYPE_OBJECT("FIXED_CHAR", cxoDbTypeChar)
    CXO_ADD_TYPE_OBJECT("FIXED_NCHAR", cxoDbTypeNchar)
    CXO_ADD_TYPE_OBJECT("NCHAR", cxoDbTypeNvarchar)
    CXO_ADD_TYPE_OBJECT("INTERVAL", cxoDbTypeIntervalDS)
    CXO_ADD_TYPE_OBJECT("LONG_BINARY", cxoDbTypeLongRaw)
    CXO_ADD_TYPE_OBJECT("LONG_STRING", cxoDbTypeLong)
    CXO_ADD_TYPE_OBJECT("NCLOB", cxoDbTypeNclob)
    CXO_ADD_TYPE_OBJECT("TIMESTAMP", cxoDbTypeTimestamp)
    CXO_ADD_TYPE_OBJECT("NATIVE_INT", cxoDbTypeBinaryInteger)
    CXO_ADD_TYPE_OBJECT("NATIVE_FLOAT", cxoDbTypeBinaryDouble)
    CXO_ADD_TYPE_OBJECT("BOOLEAN", cxoDbTypeBoolean)

    // create the Python Database API types
    CXO_ADD_API_TYPE("BINARY", CXO_TRANSFORM_BINARY, &cxoApiTypeBinary)
    CXO_ADD_API_TYPE("DATETIME", CXO_TRANSFORM_DATETIME, &cxoApiTypeDatetime)
    CXO_ADD_API_TYPE("NUMBER", CXO_TRANSFORM_FLOAT, &cxoApiTypeNumber)
    CXO_ADD_API_TYPE("ROWID", CXO_TRANSFORM_ROWID, &cxoApiTypeRowid)
    CXO_ADD_API_TYPE("STRING", CXO_TRANSFORM_STRING, &cxoApiTypeString)

    // associate the Python Database API types with the database types
    CXO_ASSOCIATE_DB_TYPE(cxoApiTypeBinary, cxoDbTypeLongRaw)
    CXO_ASSOCIATE_DB_TYPE(cxoApiTypeBinary, cxoDbTypeRaw)
    CXO_ASSOCIATE_DB_TYPE(cxoApiTypeDatetime, cxoDbTypeDate)
    CXO_ASSOCIATE_DB_TYPE(cxoApiTypeDatetime, cxoDbTypeTimestamp)
    CXO_ASSOCIATE_DB_TYPE(cxoApiTypeDatetime, cxoDbTypeTimestampLTZ)
    CXO_ASSOCIATE_DB_TYPE(cxoApiTypeDatetime, cxoDbTypeTimestampTZ)
    CXO_ASSOCIATE_DB_TYPE(cxoApiTypeNumber, cxoDbTypeBinaryDouble)
    CXO_ASSOCIATE_DB_TYPE(cxoApiTypeNumber, cxoDbTypeBinaryFloat)
    CXO_ASSOCIATE_DB_TYPE(cxoApiTypeNumber, cxoDbTypeBinaryInteger)
    CXO_ASSOCIATE_DB_TYPE(cxoApiTypeNumber, cxoDbTypeNumber)
    CXO_ASSOCIATE_DB_TYPE(cxoApiTypeRowid, cxoDbTypeRowid)
    CXO_ASSOCIATE_DB_TYPE(cxoApiTypeString, cxoDbTypeChar)
    CXO_ASSOCIATE_DB_TYPE(cxoApiTypeString, cxoDbTypeLong)
    CXO_ASSOCIATE_DB_TYPE(cxoApiTypeString, cxoDbTypeNchar)
    CXO_ASSOCIATE_DB_TYPE(cxoApiTypeString, cxoDbTypeNvarchar)
    CXO_ASSOCIATE_DB_TYPE(cxoApiTypeString, cxoDbTypeVarchar)

    // create constants required by Python DB API 2.0
    if (PyModule_AddStringConstant(module, "apilevel", "2.0") < 0)
        return NULL;
    if (PyModule_AddIntConstant(module, "threadsafety", 2) < 0)
        return NULL;
    if (PyModule_AddStringConstant(module, "paramstyle", "named") < 0)
        return NULL;

    // add version and build time for easier support
    if (PyModule_AddStringConstant(module, "version",
            CXO_BUILD_VERSION_STRING) < 0)
        return NULL;
    if (PyModule_AddStringConstant(module, "__version__",
            CXO_BUILD_VERSION_STRING) < 0)
        return NULL;
    if (PyModule_AddStringConstant(module, "buildtime",
            __DATE__ " " __TIME__) < 0)
        return NULL;

    // create and initialize future object
    cxoFutureObj = (cxoFuture*) cxoPyTypeFuture.tp_alloc(&cxoPyTypeFuture, 0);
    if (!cxoFutureObj)
        return NULL;
    if (PyModule_AddObject(module, "__future__", (PyObject*) cxoFutureObj) < 0)
        return NULL;

    // add constants for authorization modes
    CXO_ADD_INT_CONSTANT("DEFAULT_AUTH", DPI_MODE_AUTH_DEFAULT)
    CXO_ADD_INT_CONSTANT("SYSASM", DPI_MODE_AUTH_SYSASM)
    CXO_ADD_INT_CONSTANT("SYSBKP", DPI_MODE_AUTH_SYSBKP)
    CXO_ADD_INT_CONSTANT("SYSDBA", DPI_MODE_AUTH_SYSDBA)
    CXO_ADD_INT_CONSTANT("SYSDGD", DPI_MODE_AUTH_SYSDGD)
    CXO_ADD_INT_CONSTANT("SYSKMT", DPI_MODE_AUTH_SYSKMT)
    CXO_ADD_INT_CONSTANT("SYSOPER", DPI_MODE_AUTH_SYSOPER)
    CXO_ADD_INT_CONSTANT("SYSRAC", DPI_MODE_AUTH_SYSRAC)
    CXO_ADD_INT_CONSTANT("PRELIM_AUTH", DPI_MODE_AUTH_PRELIM)

    // add constants for session pool get modes
    CXO_ADD_INT_CONSTANT("SPOOL_ATTRVAL_WAIT", DPI_MODE_POOL_GET_WAIT)
    CXO_ADD_INT_CONSTANT("SPOOL_ATTRVAL_NOWAIT", DPI_MODE_POOL_GET_NOWAIT)
    CXO_ADD_INT_CONSTANT("SPOOL_ATTRVAL_FORCEGET", DPI_MODE_POOL_GET_FORCEGET)
    CXO_ADD_INT_CONSTANT("SPOOL_ATTRVAL_TIMEDWAIT",
            DPI_MODE_POOL_GET_TIMEDWAIT)

    // add constants for database shutdown modes
    CXO_ADD_INT_CONSTANT("DBSHUTDOWN_ABORT", DPI_MODE_SHUTDOWN_ABORT)
    CXO_ADD_INT_CONSTANT("DBSHUTDOWN_FINAL", DPI_MODE_SHUTDOWN_FINAL)
    CXO_ADD_INT_CONSTANT("DBSHUTDOWN_IMMEDIATE", DPI_MODE_SHUTDOWN_IMMEDIATE)
    CXO_ADD_INT_CONSTANT("DBSHUTDOWN_TRANSACTIONAL",
            DPI_MODE_SHUTDOWN_TRANSACTIONAL)
    CXO_ADD_INT_CONSTANT("DBSHUTDOWN_TRANSACTIONAL_LOCAL",
            DPI_MODE_SHUTDOWN_TRANSACTIONAL_LOCAL)

    // add constants for purity
    CXO_ADD_INT_CONSTANT("ATTR_PURITY_DEFAULT", DPI_PURITY_DEFAULT)
    CXO_ADD_INT_CONSTANT("ATTR_PURITY_NEW", DPI_PURITY_NEW)
    CXO_ADD_INT_CONSTANT("ATTR_PURITY_SELF", DPI_PURITY_SELF)

    // add constants for subscription protocols
    CXO_ADD_INT_CONSTANT("SUBSCR_PROTO_OCI", DPI_SUBSCR_PROTO_CALLBACK)
    CXO_ADD_INT_CONSTANT("SUBSCR_PROTO_MAIL", DPI_SUBSCR_PROTO_MAIL)
    CXO_ADD_INT_CONSTANT("SUBSCR_PROTO_SERVER", DPI_SUBSCR_PROTO_PLSQL)
    CXO_ADD_INT_CONSTANT("SUBSCR_PROTO_HTTP", DPI_SUBSCR_PROTO_HTTP)

    // add constants for subscription quality of service
    CXO_ADD_INT_CONSTANT("SUBSCR_QOS_RELIABLE", DPI_SUBSCR_QOS_RELIABLE)
    CXO_ADD_INT_CONSTANT("SUBSCR_QOS_DEREG_NFY", DPI_SUBSCR_QOS_DEREG_NFY)
    CXO_ADD_INT_CONSTANT("SUBSCR_QOS_ROWIDS", DPI_SUBSCR_QOS_ROWIDS)
    CXO_ADD_INT_CONSTANT("SUBSCR_QOS_QUERY", DPI_SUBSCR_QOS_QUERY)
    CXO_ADD_INT_CONSTANT("SUBSCR_QOS_BEST_EFFORT", DPI_SUBSCR_QOS_BEST_EFFORT)

    // add constants for subscription namespaces
    CXO_ADD_INT_CONSTANT("SUBSCR_NAMESPACE_AQ", DPI_SUBSCR_NAMESPACE_AQ)
    CXO_ADD_INT_CONSTANT("SUBSCR_NAMESPACE_DBCHANGE",
            DPI_SUBSCR_NAMESPACE_DBCHANGE)

    // add constants for subscription grouping classes
    CXO_ADD_INT_CONSTANT("SUBSCR_GROUPING_CLASS_TIME",
            DPI_SUBSCR_GROUPING_CLASS_TIME)

    // add constants for subscription grouping types
    CXO_ADD_INT_CONSTANT("SUBSCR_GROUPING_TYPE_SUMMARY",
            DPI_SUBSCR_GROUPING_TYPE_SUMMARY)
    CXO_ADD_INT_CONSTANT("SUBSCR_GROUPING_TYPE_LAST",
            DPI_SUBSCR_GROUPING_TYPE_LAST)

    // add constants for event types
    CXO_ADD_INT_CONSTANT("EVENT_NONE", DPI_EVENT_NONE)
    CXO_ADD_INT_CONSTANT("EVENT_STARTUP", DPI_EVENT_STARTUP)
    CXO_ADD_INT_CONSTANT("EVENT_SHUTDOWN", DPI_EVENT_SHUTDOWN)
    CXO_ADD_INT_CONSTANT("EVENT_SHUTDOWN_ANY", DPI_EVENT_SHUTDOWN_ANY)
    CXO_ADD_INT_CONSTANT("EVENT_DEREG", DPI_EVENT_DEREG)
    CXO_ADD_INT_CONSTANT("EVENT_OBJCHANGE", DPI_EVENT_OBJCHANGE)
    CXO_ADD_INT_CONSTANT("EVENT_QUERYCHANGE", DPI_EVENT_QUERYCHANGE)
    CXO_ADD_INT_CONSTANT("EVENT_AQ", DPI_EVENT_AQ)

    // add constants for opcodes
    CXO_ADD_INT_CONSTANT("OPCODE_ALLOPS", DPI_OPCODE_ALL_OPS)
    CXO_ADD_INT_CONSTANT("OPCODE_ALLROWS", DPI_OPCODE_ALL_ROWS)
    CXO_ADD_INT_CONSTANT("OPCODE_INSERT", DPI_OPCODE_INSERT)
    CXO_ADD_INT_CONSTANT("OPCODE_UPDATE", DPI_OPCODE_UPDATE)
    CXO_ADD_INT_CONSTANT("OPCODE_DELETE", DPI_OPCODE_DELETE)
    CXO_ADD_INT_CONSTANT("OPCODE_ALTER", DPI_OPCODE_ALTER)
    CXO_ADD_INT_CONSTANT("OPCODE_DROP", DPI_OPCODE_DROP)

    // add constants for AQ dequeue modes
    CXO_ADD_INT_CONSTANT("DEQ_BROWSE", DPI_MODE_DEQ_BROWSE)
    CXO_ADD_INT_CONSTANT("DEQ_LOCKED", DPI_MODE_DEQ_LOCKED)
    CXO_ADD_INT_CONSTANT("DEQ_REMOVE", DPI_MODE_DEQ_REMOVE)
    CXO_ADD_INT_CONSTANT("DEQ_REMOVE_NODATA", DPI_MODE_DEQ_REMOVE_NO_DATA)

    // add constants for AQ dequeue navigation
    CXO_ADD_INT_CONSTANT("DEQ_FIRST_MSG", DPI_DEQ_NAV_FIRST_MSG)
    CXO_ADD_INT_CONSTANT("DEQ_NEXT_TRANSACTION", DPI_DEQ_NAV_NEXT_TRANSACTION)
    CXO_ADD_INT_CONSTANT("DEQ_NEXT_MSG", DPI_DEQ_NAV_NEXT_MSG)

    // add constants for AQ dequeue visibility
    CXO_ADD_INT_CONSTANT("DEQ_IMMEDIATE", DPI_VISIBILITY_IMMEDIATE)
    CXO_ADD_INT_CONSTANT("DEQ_ON_COMMIT", DPI_VISIBILITY_ON_COMMIT)

    // add constants for AQ dequeue wait
    CXO_ADD_INT_CONSTANT("DEQ_NO_WAIT", DPI_DEQ_WAIT_NO_WAIT)
    CXO_ADD_INT_CONSTANT("DEQ_WAIT_FOREVER", DPI_DEQ_WAIT_FOREVER)

    // add constants for AQ enqueue visibility
    CXO_ADD_INT_CONSTANT("ENQ_IMMEDIATE", DPI_VISIBILITY_IMMEDIATE)
    CXO_ADD_INT_CONSTANT("ENQ_ON_COMMIT", DPI_VISIBILITY_ON_COMMIT)

    // add constants for AQ table purge mode (message)
    CXO_ADD_INT_CONSTANT("MSG_PERSISTENT", DPI_MODE_MSG_PERSISTENT)
    CXO_ADD_INT_CONSTANT("MSG_BUFFERED", DPI_MODE_MSG_BUFFERED)
    CXO_ADD_INT_CONSTANT("MSG_PERSISTENT_OR_BUFFERED",
            DPI_MODE_MSG_PERSISTENT_OR_BUFFERED)

    // add constants for AQ message state
    CXO_ADD_INT_CONSTANT("MSG_EXPIRED", DPI_MSG_STATE_EXPIRED)
    CXO_ADD_INT_CONSTANT("MSG_READY", DPI_MSG_STATE_READY)
    CXO_ADD_INT_CONSTANT("MSG_PROCESSED", DPI_MSG_STATE_PROCESSED)
    CXO_ADD_INT_CONSTANT("MSG_WAITING", DPI_MSG_STATE_WAITING)

    // add special constants for AQ delay/expiration
    CXO_ADD_INT_CONSTANT("MSG_NO_DELAY", 0)
    CXO_ADD_INT_CONSTANT("MSG_NO_EXPIRATION", -1)

    return module;
}


//-----------------------------------------------------------------------------
// Start routine for the module.
//-----------------------------------------------------------------------------
PyMODINIT_FUNC PyInit_cx_Oracle(void)
{
    return cxoModule_initialize();
}
