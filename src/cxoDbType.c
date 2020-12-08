//-----------------------------------------------------------------------------
// Copyright (c) 2020, Oracle and/or its affiliates. All rights reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// cxoDbType.c
//   Defines the objects used for identifying all types used by the database.
//-----------------------------------------------------------------------------

#include "cxoModule.h"

//-----------------------------------------------------------------------------
// cxoDbType_free()
//   Free the database type object.
//-----------------------------------------------------------------------------
static void cxoDbType_free(cxoDbType *dbType)
{
    Py_TYPE(dbType)->tp_free((PyObject*) dbType);
}


//-----------------------------------------------------------------------------
// cxoDbType_repr()
//   Return a string representation of a queue.
//-----------------------------------------------------------------------------
static PyObject *cxoDbType_repr(cxoDbType *dbType)
{
    PyObject *module, *name, *dbTypeName, *result;

    dbTypeName = PyUnicode_DecodeASCII(dbType->name, strlen(dbType->name),
            NULL);
    if (!dbTypeName)
        return NULL;
    if (cxoUtils_getModuleAndName(Py_TYPE(dbType), &module, &name) < 0) {
        Py_DECREF(dbTypeName);
        return NULL;
    }
    result = cxoUtils_formatString("<%s.%s %s>",
            PyTuple_Pack(3, module, name, dbTypeName));
    Py_DECREF(module);
    Py_DECREF(name);
    Py_DECREF(dbTypeName);
    return result;
}


//-----------------------------------------------------------------------------
// cxoDbType_richCompare()
//   Peforms a comparison between the database type and another Python object.
// Equality (and inequality) are used to match database API types with their
// associated database types.
//-----------------------------------------------------------------------------
static PyObject *cxoDbType_richCompare(cxoDbType* dbType, PyObject* obj,
        int op)
{
    cxoApiType *apiType;
    int status, equal;

    // only equality and inequality can be checked
    if (op != Py_EQ && op != Py_NE) {
        Py_INCREF(Py_NotImplemented);
        return Py_NotImplemented;
    }

    // check for exact object
    equal = 0;
    if (obj == (PyObject*) dbType) {
        equal = 1;

    // check for API type
    } else {
        status = PyObject_IsInstance(obj, (PyObject*) &cxoPyTypeApiType);
        if (status < 0)
            return NULL;
        if (status == 1) {
            apiType = (cxoApiType*) obj;
            status = PySequence_Contains(apiType->dbTypes, (PyObject*) dbType);
            if (status < 0)
                return NULL;
            equal = (status == 1) ? 1 : 0;
        }
    }

    // determine return value
    if ((equal && op == Py_EQ) || (!equal && op == Py_NE)) {
        Py_RETURN_TRUE;
    }
    Py_RETURN_FALSE;
}


//-----------------------------------------------------------------------------
// cxoDbType_hash()
//   Return a hash value for the instance.
//-----------------------------------------------------------------------------
static Py_hash_t cxoDbType_hash(cxoDbType *dbType)
{
    return (Py_hash_t) dbType->num;
}


//-----------------------------------------------------------------------------
// cxoDbType_fromDataTypeInfo()
//   Return the database type given the data type info available from ODPI-C.
//-----------------------------------------------------------------------------
cxoDbType *cxoDbType_fromDataTypeInfo(dpiDataTypeInfo *info)
{
    char message[120];

    switch (info->oracleTypeNum) {
        case DPI_ORACLE_TYPE_VARCHAR:
            return cxoDbTypeVarchar;
        case DPI_ORACLE_TYPE_NVARCHAR:
            return cxoDbTypeNvarchar;
        case DPI_ORACLE_TYPE_CHAR:
            return cxoDbTypeChar;
        case DPI_ORACLE_TYPE_NCHAR:
            return cxoDbTypeNchar;
        case DPI_ORACLE_TYPE_ROWID:
            return cxoDbTypeRowid;
        case DPI_ORACLE_TYPE_RAW:
            return cxoDbTypeRaw;
        case DPI_ORACLE_TYPE_NATIVE_DOUBLE:
            return cxoDbTypeBinaryDouble;
        case DPI_ORACLE_TYPE_NATIVE_FLOAT:
            return cxoDbTypeBinaryFloat;
        case DPI_ORACLE_TYPE_NATIVE_INT:
            return cxoDbTypeBinaryInteger;
        case DPI_ORACLE_TYPE_NUMBER:
            return cxoDbTypeNumber;
        case DPI_ORACLE_TYPE_DATE:
            return cxoDbTypeDate;
        case DPI_ORACLE_TYPE_TIMESTAMP:
            return cxoDbTypeTimestamp;
        case DPI_ORACLE_TYPE_TIMESTAMP_TZ:
            return cxoDbTypeTimestampTZ;
        case DPI_ORACLE_TYPE_TIMESTAMP_LTZ:
            return cxoDbTypeTimestampLTZ;
        case DPI_ORACLE_TYPE_INTERVAL_DS:
            return cxoDbTypeIntervalDS;
        case DPI_ORACLE_TYPE_INTERVAL_YM:
            return cxoDbTypeIntervalYM;
        case DPI_ORACLE_TYPE_CLOB:
            return cxoDbTypeClob;
        case DPI_ORACLE_TYPE_NCLOB:
            return cxoDbTypeNclob;
        case DPI_ORACLE_TYPE_BLOB:
            return cxoDbTypeBlob;
        case DPI_ORACLE_TYPE_BFILE:
            return cxoDbTypeBfile;
        case DPI_ORACLE_TYPE_STMT:
            return cxoDbTypeCursor;
        case DPI_ORACLE_TYPE_OBJECT:
            return cxoDbTypeObject;
        case DPI_ORACLE_TYPE_LONG_VARCHAR:
            return cxoDbTypeLong;
        case DPI_ORACLE_TYPE_LONG_RAW:
            return cxoDbTypeLongRaw;
        case DPI_ORACLE_TYPE_BOOLEAN:
            return cxoDbTypeBoolean;
        default:
            break;
    }

    snprintf(message, sizeof(message), "Oracle type %d not supported.",
            info->oracleTypeNum);
    cxoError_raiseFromString(cxoNotSupportedErrorException, message);
    return NULL;
}


//-----------------------------------------------------------------------------
// cxoDbType_fromTransformNum()
//   Return the database type given the transformation number.
//-----------------------------------------------------------------------------
cxoDbType *cxoDbType_fromTransformNum(cxoTransformNum transformNum)
{
    char message[120];

    switch (transformNum) {
        case CXO_TRANSFORM_BINARY:
            return cxoDbTypeRaw;
        case CXO_TRANSFORM_BFILE:
            return cxoDbTypeBfile;
        case CXO_TRANSFORM_BLOB:
            return cxoDbTypeBlob;
        case CXO_TRANSFORM_BOOLEAN:
            return cxoDbTypeBoolean;
        case CXO_TRANSFORM_CLOB:
            return cxoDbTypeClob;
        case CXO_TRANSFORM_CURSOR:
            return cxoDbTypeCursor;
        case CXO_TRANSFORM_DATE:
        case CXO_TRANSFORM_DATETIME:
            return cxoDbTypeDate;
        case CXO_TRANSFORM_DECIMAL:
        case CXO_TRANSFORM_FLOAT:
        case CXO_TRANSFORM_INT:
            return cxoDbTypeNumber;
        case CXO_TRANSFORM_FIXED_CHAR:
            return cxoDbTypeChar;
        case CXO_TRANSFORM_FIXED_NCHAR:
            return cxoDbTypeNchar;
        case CXO_TRANSFORM_LONG_BINARY:
            return cxoDbTypeLongRaw;
        case CXO_TRANSFORM_LONG_STRING:
            return cxoDbTypeLong;
        case CXO_TRANSFORM_NATIVE_DOUBLE:
            return cxoDbTypeBinaryDouble;
        case CXO_TRANSFORM_NATIVE_FLOAT:
            return cxoDbTypeBinaryFloat;
        case CXO_TRANSFORM_NATIVE_INT:
            return cxoDbTypeBinaryInteger;
        case CXO_TRANSFORM_NCLOB:
            return cxoDbTypeNclob;
        case CXO_TRANSFORM_NSTRING:
            return cxoDbTypeNvarchar;
        case CXO_TRANSFORM_OBJECT:
            return cxoDbTypeObject;
        case CXO_TRANSFORM_ROWID:
            return cxoDbTypeRowid;
        case CXO_TRANSFORM_NONE:
        case CXO_TRANSFORM_STRING:
            return cxoDbTypeVarchar;
        case CXO_TRANSFORM_TIMEDELTA:
            return cxoDbTypeIntervalDS;
        case CXO_TRANSFORM_TIMESTAMP:
            return cxoDbTypeTimestamp;
        case CXO_TRANSFORM_TIMESTAMP_LTZ:
            return cxoDbTypeTimestampLTZ;
        case CXO_TRANSFORM_TIMESTAMP_TZ:
            return cxoDbTypeTimestampTZ;
        case CXO_TRANSFORM_JSON:
            return cxoDbTypeJson;
        default:
            break;
    }

    snprintf(message, sizeof(message), "transform %d not supported.",
            transformNum);
    cxoError_raiseFromString(cxoNotSupportedErrorException, message);
    return NULL;
}


//-----------------------------------------------------------------------------
// cxoDBType_reduce()
//   Method provided for pickling/unpickling of DB types.
//-----------------------------------------------------------------------------
static PyObject *cxoDBType_reduce(cxoDbType *dbType)
{
    return PyUnicode_DecodeASCII(dbType->name, strlen(dbType->name), NULL);
}


//-----------------------------------------------------------------------------
// declaration of methods
//-----------------------------------------------------------------------------
static PyMethodDef cxoMethods[] = {
    { "__reduce__", (PyCFunction) cxoDBType_reduce, METH_NOARGS },
    { NULL, NULL}
};


//-----------------------------------------------------------------------------
// declaration of members
//-----------------------------------------------------------------------------
static PyMemberDef cxoMembers[] = {
    { "name", T_STRING, offsetof(cxoDbType, name), READONLY },
    { NULL }
};


//-----------------------------------------------------------------------------
// Python type declaration
//-----------------------------------------------------------------------------
PyTypeObject cxoPyTypeDbType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "cx_Oracle.DbType",
    .tp_basicsize = sizeof(cxoDbType),
    .tp_dealloc = (destructor) cxoDbType_free,
    .tp_repr = (reprfunc) cxoDbType_repr,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_members = cxoMembers,
    .tp_methods = cxoMethods,
    .tp_richcompare = (richcmpfunc) cxoDbType_richCompare,
    .tp_hash = (hashfunc) cxoDbType_hash
};
