//-----------------------------------------------------------------------------
// Copyright 2016, 2017, Oracle and/or its affiliates. All rights reserved.
//
// Portions Copyright 2007-2015, Anthony Tuininga. All rights reserved.
//
// Portions Copyright 2001-2007, Computronix (Canada) Ltd., Edmonton, Alberta,
// Canada. All rights reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Variable.c
//   Defines Python types for Oracle variables.
//-----------------------------------------------------------------------------

#include "ObjectType.c"

//-----------------------------------------------------------------------------
// define structure common to all variables
//-----------------------------------------------------------------------------
typedef struct {
    PyObject_HEAD
    dpiVar *handle;
    dpiData *data;
    udt_Connection *connection;
    PyObject *inConverter;
    PyObject *outConverter;
    udt_ObjectType *objectType;
    uint32_t allocatedElements;
    uint32_t size;
    uint32_t bufferSize;
    int isArray;
    struct _udt_VariableType *type;
} udt_Variable;


//-----------------------------------------------------------------------------
// define function types for the common actions that take place on a variable
//-----------------------------------------------------------------------------
typedef int (*SetValueProc)(udt_Variable*, uint32_t, dpiData*, PyObject*);
typedef PyObject * (*GetValueProc)(udt_Variable*, dpiData*);


//-----------------------------------------------------------------------------
// define structure for the common actions that take place on a variable
//-----------------------------------------------------------------------------
typedef struct _udt_VariableType {
    SetValueProc setValueProc;
    GetValueProc getValueProc;
    PyTypeObject *pythonType;
    dpiOracleTypeNum oracleTypeNum;
    dpiNativeTypeNum nativeTypeNum;
    uint32_t size;
} udt_VariableType;


//-----------------------------------------------------------------------------
// Declaration of common variable functions.
//-----------------------------------------------------------------------------
static void Variable_Free(udt_Variable *);
static int Variable_SetValueBytes(udt_Variable *, uint32_t, dpiData *,
        PyObject *);
static PyObject *Variable_Repr(udt_Variable *);
static PyObject *Variable_ExternalCopy(udt_Variable *, PyObject *);
static PyObject *Variable_ExternalSetValue(udt_Variable *, PyObject *);
static PyObject *Variable_ExternalGetValue(udt_Variable *, PyObject *,
        PyObject *);
static PyObject *Variable_ExternalGetActualElements(udt_Variable*, void*);
static PyObject *Variable_ExternalGetValues(udt_Variable*, void*);


//-----------------------------------------------------------------------------
// declaration of members for variables
//-----------------------------------------------------------------------------
static PyMemberDef g_VariableMembers[] = {
    { "bufferSize", T_INT, offsetof(udt_Variable, bufferSize), READONLY },
    { "inconverter", T_OBJECT, offsetof(udt_Variable, inConverter), 0 },
    { "numElements", T_INT, offsetof(udt_Variable, allocatedElements),
            READONLY },
    { "outconverter", T_OBJECT, offsetof(udt_Variable, outConverter), 0 },
    { "size", T_INT, offsetof(udt_Variable, size), READONLY },
    { "type", T_OBJECT, offsetof(udt_Variable, objectType), READONLY },
    { NULL }
};


//-----------------------------------------------------------------------------
// declaration of calculated members for variables
//-----------------------------------------------------------------------------
static PyGetSetDef g_VariableCalcMembers[] = {
    { "actualElements", (getter) Variable_ExternalGetActualElements, 0, 0, 0 },
    { "values", (getter) Variable_ExternalGetValues, 0, 0, 0 },
    { NULL }
};


//-----------------------------------------------------------------------------
// declaration of methods for variables
//-----------------------------------------------------------------------------
static PyMethodDef g_VariableMethods[] = {
    { "copy", (PyCFunction) Variable_ExternalCopy, METH_VARARGS },
    { "setvalue", (PyCFunction) Variable_ExternalSetValue, METH_VARARGS },
    { "getvalue", (PyCFunction) Variable_ExternalGetValue,
              METH_VARARGS  | METH_KEYWORDS },
    { NULL }
};


#include "BooleanVar.c"
#include "StringVar.c"
#include "LongVar.c"
#include "NumberVar.c"
#include "DateTimeVar.c"
#include "LobVar.c"
#include "CursorVar.c"
#include "ObjectVar.c"
#include "IntervalVar.c"


//-----------------------------------------------------------------------------
// VarType_FromQueryInfo()
//   Return a variable type given query metadata, or NULL indicating that the
// data indicated by the query metadata is not supported.
//-----------------------------------------------------------------------------
static udt_VariableType *VarType_FromQueryInfo(dpiQueryInfo *info)
{
    char message[120];

    switch (info->oracleTypeNum) {
        case DPI_ORACLE_TYPE_VARCHAR:
            return &vt_String;
        case DPI_ORACLE_TYPE_NVARCHAR:
            return &vt_NationalCharString;
        case DPI_ORACLE_TYPE_CHAR:
            return &vt_FixedChar;
        case DPI_ORACLE_TYPE_NCHAR:
            return &vt_FixedNationalChar;
        case DPI_ORACLE_TYPE_ROWID:
            return &vt_Rowid;
        case DPI_ORACLE_TYPE_RAW:
            return &vt_Binary;
        case DPI_ORACLE_TYPE_NATIVE_FLOAT:
        case DPI_ORACLE_TYPE_NATIVE_DOUBLE:
            return &vt_NativeFloat;
        case DPI_ORACLE_TYPE_NUMBER:
            if (info->scale == 0 ||
                    (info->scale == -127 && info->precision == 0)) {
                if (info->precision > 0 &&
                        info->precision <= DPI_MAX_INT64_PRECISION)
                    return &vt_NumberAsInteger;
                return &vt_NumberAsLongInteger;
            }
            return &vt_NumberAsFloat;
        case DPI_ORACLE_TYPE_NATIVE_INT:
            return &vt_NativeInteger;
        case DPI_ORACLE_TYPE_DATE:
            return &vt_DateTime;
        case DPI_ORACLE_TYPE_TIMESTAMP:
            return &vt_Timestamp;
        case DPI_ORACLE_TYPE_TIMESTAMP_TZ:
        case DPI_ORACLE_TYPE_TIMESTAMP_LTZ:
            return &vt_TimestampLTZ;
        case DPI_ORACLE_TYPE_INTERVAL_DS:
            return &vt_Interval;
        case DPI_ORACLE_TYPE_CLOB:
            return &vt_CLOB;
        case DPI_ORACLE_TYPE_NCLOB:
            return &vt_NCLOB;
        case DPI_ORACLE_TYPE_BLOB:
            return &vt_BLOB;
        case DPI_ORACLE_TYPE_BFILE:
            return &vt_BFILE;
        case DPI_ORACLE_TYPE_STMT:
            return &vt_Cursor;
        case DPI_ORACLE_TYPE_OBJECT:
            return &vt_Object;
        case DPI_ORACLE_TYPE_LONG_VARCHAR:
            return &vt_LongString;
        case DPI_ORACLE_TYPE_LONG_RAW:
            return &vt_LongBinary;
        default:
            sprintf(message, "Oracle type %d not supported.",
                    info->oracleTypeNum);
            PyErr_SetString(g_NotSupportedErrorException, message);
            return NULL;
    }
}


//-----------------------------------------------------------------------------
// Variable_New()
//   Allocate a new variable.
//-----------------------------------------------------------------------------
static udt_Variable *Variable_New(udt_Cursor *cursor, uint32_t numElements,
        udt_VariableType *type, uint32_t size, int isArray,
        udt_ObjectType *objType)
{
    dpiObjectType *typeHandle = NULL;
    udt_Variable *self;

    // attempt to allocate the object
    self = (udt_Variable*) type->pythonType->tp_alloc(type->pythonType, 0);
    if (!self)
        return NULL;

    // perform basic initialization
    Py_INCREF(cursor->connection);
    self->connection = cursor->connection;
    if (objType) {
        Py_INCREF(objType);
        self->objectType = objType;
        typeHandle = objType->handle;
    }
    if (numElements == 0)
        numElements = 1;
    self->allocatedElements = numElements;
    self->type = type;
    self->size = (size == 0) ? type->size : size;
    self->isArray = isArray;

    // acquire and initialize DPI variable
    if (dpiConn_newVar(cursor->connection->handle, type->oracleTypeNum,
            type->nativeTypeNum, numElements, self->size, 0, isArray,
            typeHandle, &self->handle, &self->data) < 0) {
        Error_RaiseAndReturnNull();
        Py_DECREF(self);
        return NULL;
    }

    // get buffer size for information
    if (dpiVar_getSizeInBytes(self->handle, &self->bufferSize) < 0) {
        Error_RaiseAndReturnNull();
        Py_DECREF(self);
        return NULL;
    }

    return self;
}


//-----------------------------------------------------------------------------
// Variable_Free()
//   Free an existing variable.
//-----------------------------------------------------------------------------
static void Variable_Free(udt_Variable *self)
{
    if (self->handle) {
        Py_BEGIN_ALLOW_THREADS
        dpiVar_release(self->handle);
        Py_END_ALLOW_THREADS
        self->handle = NULL;
    }
    Py_CLEAR(self->connection);
    Py_CLEAR(self->inConverter);
    Py_CLEAR(self->outConverter);
    Py_CLEAR(self->objectType);
    Py_TYPE(self)->tp_free((PyObject*) self);
}


//-----------------------------------------------------------------------------
// Variable_SetValueBytes()
//   Transfer the value to DPI as bytes. The Python value is transformed into
// an object from which bytes can be extracted.
//-----------------------------------------------------------------------------
static int Variable_SetValueBytes(udt_Variable *var, uint32_t pos,
        dpiData *data, PyObject *value)
{
    dpiData *tempVarData, *sourceData;
    dpiVar *tempVarHandle;
    udt_Buffer buffer;
    uint32_t i;
    int status;

    if (cxBuffer_FromObject(&buffer, value,
            var->connection->encodingInfo.encoding) < 0)
        return -1;
    if (var->type->size > 0 && buffer.size > var->bufferSize) {
        if (dpiConn_newVar(var->connection->handle, var->type->oracleTypeNum,
                var->type->nativeTypeNum, var->allocatedElements,
                buffer.size, 0, var->isArray, NULL, &tempVarHandle,
                &tempVarData) < 0) {
            cxBuffer_Clear(&buffer);
            return Error_RaiseAndReturnInt();
        }
        for (i = 0; i < var->allocatedElements; i++) {
            sourceData = &var->data[i];
            if (i == pos || sourceData->isNull)
                continue;
            if (dpiVar_setFromBytes(tempVarHandle, i,
                    sourceData->value.asBytes.ptr,
                    sourceData->value.asBytes.length) < 0) {
                cxBuffer_Clear(&buffer);
                dpiVar_release(tempVarHandle);
                return Error_RaiseAndReturnInt();
            }
        }
        dpiVar_release(var->handle);
        var->handle = tempVarHandle;
        var->data = tempVarData;
        var->size = buffer.numCharacters;
        var->bufferSize = buffer.size;
    }
    status = dpiVar_setFromBytes(var->handle, pos, buffer.ptr, buffer.size);
    cxBuffer_Clear(&buffer);
    if (status < 0)
        return Error_RaiseAndReturnInt();
    return 0;
}


//-----------------------------------------------------------------------------
// Variable_Check()
//   Returns a boolean indicating if the object is a variable.
//-----------------------------------------------------------------------------
static int Variable_Check(PyObject *object)
{
    PyTypeObject *objectType = Py_TYPE(object);

    return (objectType == &g_CursorVarType ||
            objectType == &g_DateTimeVarType ||
            objectType == &g_BFILEVarType ||
            objectType == &g_BLOBVarType ||
            objectType == &g_CLOBVarType ||
            objectType == &g_LongStringVarType ||
            objectType == &g_LongBinaryVarType ||
            objectType == &g_NumberVarType ||
            objectType == &g_StringVarType ||
            objectType == &g_FixedCharVarType ||
            objectType == &g_NCLOBVarType ||
            objectType == &g_NCharVarType ||
            objectType == &g_FixedNCharVarType ||
            objectType == &g_RowidVarType ||
            objectType == &g_BinaryVarType ||
            objectType == &g_TimestampVarType ||
            objectType == &g_IntervalVarType ||
            objectType == &g_ObjectVarType ||
            objectType == &g_BooleanVarType ||
            objectType == &g_NativeFloatVarType ||
            objectType == &g_NativeIntVarType);
}


//-----------------------------------------------------------------------------
// Variable_TypeByPythonType()
//   Return a variable type given a Python type object or NULL if the Python
// type does not have a corresponding variable type.
//-----------------------------------------------------------------------------
static udt_VariableType *Variable_TypeByPythonType(udt_Cursor* cursor,
        PyObject* type)
{
    if (type == (PyObject*) &g_StringVarType)
        return &vt_String;
    if (type == (PyObject*) cxString_Type)
        return &vt_String;
    if (type == (PyObject*) &g_FixedCharVarType)
        return &vt_FixedChar;
    if (type == (PyObject*) &g_NCharVarType)
        return &vt_NationalCharString;
#if PY_MAJOR_VERSION < 3
    if (type == (PyObject*) &PyUnicode_Type)
        return &vt_NationalCharString;
#endif
    if (type == (PyObject*) &g_FixedNCharVarType)
        return &vt_FixedNationalChar;
    if (type == (PyObject*) &g_NCLOBVarType)
        return &vt_NCLOB;
    if (type == (PyObject*) &g_RowidVarType)
        return &vt_Rowid;
    if (type == (PyObject*) &g_BinaryVarType)
        return &vt_Binary;
    if (type == (PyObject*) &cxBinary_Type)
        return &vt_Binary;
    if (type == (PyObject*) &g_LongStringVarType)
        return &vt_LongString;
    if (type == (PyObject*) &g_LongBinaryVarType)
        return &vt_LongBinary;
    if (type == (PyObject*) &g_BFILEVarType)
        return &vt_BFILE;
    if (type == (PyObject*) &g_BLOBVarType)
        return &vt_BLOB;
    if (type == (PyObject*) &g_CLOBVarType)
        return &vt_CLOB;
    if (type == (PyObject*) &g_NumberVarType)
        return &vt_NumberAsFloat;
    if (type == (PyObject*) &PyFloat_Type)
        return &vt_NumberAsFloat;
#if PY_MAJOR_VERSION < 3
    if (type == (PyObject*) &PyInt_Type)
        return &vt_NumberAsInteger;
#endif
    if (type == (PyObject*) &PyLong_Type)
        return &vt_NumberAsLongInteger;
    if (type == (PyObject*) g_DecimalType)
        return &vt_NumberAsDecimal;
    if (type == (PyObject*) &g_BooleanVarType)
        return &vt_Boolean;
    if (type == (PyObject*) &PyBool_Type)
        return &vt_Boolean;
    if (type == (PyObject*) &g_DateTimeVarType)
        return &vt_DateTime;
    if (type == (PyObject*) PyDateTimeAPI->DateType)
        return &vt_Date;
    if (type == (PyObject*) PyDateTimeAPI->DateTimeType)
        return &vt_DateTime;
    if (type == (PyObject*) &g_IntervalVarType)
        return &vt_Interval;
    if (type == (PyObject*) PyDateTimeAPI->DeltaType)
        return &vt_Interval;
    if (type == (PyObject*) &g_TimestampVarType)
        return &vt_Timestamp;
    if (type == (PyObject*) &g_CursorVarType)
        return &vt_Cursor;
    if (type == (PyObject*) &g_NativeFloatVarType)
        return &vt_NativeFloat;
    if (type == (PyObject*) &g_NativeIntVarType)
        return &vt_NativeInteger;
    if (type == (PyObject*) &g_ObjectVarType)
        return &vt_Object;
    if (type == (PyObject*) &g_ObjectType)
        return &vt_Object;

    PyErr_SetString(g_NotSupportedErrorException,
            "Variable_TypeByPythonType(): unhandled data type");
    return NULL;
}


//-----------------------------------------------------------------------------
// Variable_TypeByValue()
//   Return a variable type given a Python object or NULL if the Python
// object does not have a corresponding variable type.
//-----------------------------------------------------------------------------
static udt_VariableType *Variable_TypeByValue(PyObject* value, uint32_t* size,
        uint32_t *numElements)
{
    udt_VariableType *varType;
    PyObject *elementValue;
    char buffer[200];
    int i, result;

    // handle scalars
    if (value == Py_None) {
        *size = 1;
        return &vt_String;
    }
    if (cxString_Check(value)) {
        *size = (uint32_t) cxString_GetSize(value);
        return &vt_String;
    }
    if (PyBool_Check(value))
        return &vt_Boolean;
#if PY_MAJOR_VERSION < 3
    if (PyUnicode_Check(value)) {
        *size = (uint32_t) PyUnicode_GET_SIZE(value);
        return &vt_NationalCharString;
    }
    if (PyInt_Check(value))
        return &vt_NumberAsInteger;
#else
    if (PyBytes_Check(value)) {
        *size = (uint32_t) PyBytes_GET_SIZE(value);
        return &vt_Binary;
    }
#endif
    if (PyLong_Check(value))
        return &vt_NumberAsLongInteger;
    if (PyFloat_Check(value))
        return &vt_NumberAsFloat;
    if (cxBinary_Check(value)) {
        udt_Buffer temp;
        if (cxBuffer_FromObject(&temp, value, NULL) < 0)
            return NULL;
        *size = temp.size;
        cxBuffer_Clear(&temp);
        return &vt_Binary;
    }
    if (PyDateTime_Check(value))
        return &vt_DateTime;
    if (PyDate_Check(value))
        return &vt_DateTime;
    if (PyDelta_Check(value))
        return &vt_Interval;
    result = PyObject_IsInstance(value, (PyObject*) &g_CursorType);
    if (result < 0)
        return NULL;
    if (result)
        return &vt_Cursor;
    if (Py_TYPE(value) == g_DecimalType)
        return &vt_NumberAsDecimal;
    if (Py_TYPE(value) == &g_ObjectType)
        return &vt_Object;

    // handle arrays
    if (PyList_Check(value)) {
        elementValue = Py_None;
        for (i = 0; i < PyList_GET_SIZE(value); i++) {
            elementValue = PyList_GET_ITEM(value, i);
            if (elementValue != Py_None)
                break;
        }
        varType = Variable_TypeByValue(elementValue, size, numElements);
        if (!varType)
            return NULL;
        *numElements = (uint32_t) PyList_GET_SIZE(value);
        *size = varType->size;
        return varType;
    }

    sprintf(buffer, "Variable_TypeByValue(): unhandled data type %.*s", 150,
            Py_TYPE(value)->tp_name);
    PyErr_SetString(g_NotSupportedErrorException, buffer);
    return NULL;
}


//-----------------------------------------------------------------------------
// Variable_DefaultNewByValue()
//   Default method for determining the type of variable to use for the data.
//-----------------------------------------------------------------------------
static udt_Variable *Variable_DefaultNewByValue(udt_Cursor *cursor,
        PyObject *value, uint32_t numElements)
{
    udt_ObjectType *objType = NULL;
    udt_VariableType *varType;
    uint32_t size = 0;
    udt_Object *obj;

    varType = Variable_TypeByValue(value, &size, &numElements);
    if (!varType)
        return NULL;
    if (varType == &vt_Object) {
        obj = (udt_Object*) value;
        objType = obj->objectType;
    }
    return Variable_New(cursor, numElements, varType, size,
            PyList_Check(value), objType);
}


//-----------------------------------------------------------------------------
// Variable_NewByInputTypeHandler()
//   Allocate a new variable by looking at the type of the data.
//-----------------------------------------------------------------------------
static udt_Variable *Variable_NewByInputTypeHandler(udt_Cursor *cursor,
        PyObject *inputTypeHandler, PyObject *value, uint32_t numElements)
{
    PyObject *var;

    var = PyObject_CallFunction(inputTypeHandler, "OOi", cursor, value,
            numElements);
    if (!var)
        return NULL;
    if (var != Py_None) {
        if (!Variable_Check(var)) {
            Py_DECREF(var);
            PyErr_SetString(PyExc_TypeError,
                    "expecting variable from input type handler");
            return NULL;
        }
        return (udt_Variable*) var;
    }
    Py_DECREF(var);
    return Variable_DefaultNewByValue(cursor, value, numElements);
}


//-----------------------------------------------------------------------------
// Variable_NewByValue()
//   Allocate a new variable by looking at the type of the data.
//-----------------------------------------------------------------------------
static udt_Variable *Variable_NewByValue(udt_Cursor *cursor, PyObject *value,
        uint32_t numElements)
{
    if (cursor->inputTypeHandler && cursor->inputTypeHandler != Py_None)
        return Variable_NewByInputTypeHandler(cursor, cursor->inputTypeHandler,
                value, numElements);
    if (cursor->connection->inputTypeHandler &&
            cursor->connection->inputTypeHandler != Py_None)
        return Variable_NewByInputTypeHandler(cursor,
                cursor->connection->inputTypeHandler, value, numElements);
    return Variable_DefaultNewByValue(cursor, value, numElements);
}


//-----------------------------------------------------------------------------
// Variable_NewArrayByType()
//   Allocate a new PL/SQL array by looking at the Python data type.
//-----------------------------------------------------------------------------
static udt_Variable *Variable_NewArrayByType(udt_Cursor *cursor,
        PyObject *value)
{
    PyObject *typeObj, *numElementsObj;
    udt_VariableType *varType;
    uint32_t numElements;

    if (PyList_GET_SIZE(value) != 2) {
        PyErr_SetString(g_ProgrammingErrorException,
                "expecting an array of two elements [type, numelems]");
        return NULL;
    }

    typeObj = PyList_GET_ITEM(value, 0);
    numElementsObj = PyList_GET_ITEM(value, 1);
    if (!PyInt_Check(numElementsObj)) {
        PyErr_SetString(g_ProgrammingErrorException,
                "number of elements must be an integer");
        return NULL;
    }

    varType = Variable_TypeByPythonType(cursor, typeObj);
    if (!varType)
        return NULL;

    numElements = PyInt_AsLong(numElementsObj);
    if (PyErr_Occurred())
        return NULL;
    return Variable_New(cursor, numElements, varType, varType->size, 1, NULL);
}


//-----------------------------------------------------------------------------
// Variable_NewByType()
//   Allocate a new variable by looking at the Python data type.
//-----------------------------------------------------------------------------
static udt_Variable *Variable_NewByType(udt_Cursor *cursor, PyObject *value,
        uint32_t numElements)
{
    udt_VariableType *varType;
    int size;

    // passing an integer is assumed to be a string
    if (PyInt_Check(value)) {
        size = PyInt_AsLong(value);
        if (PyErr_Occurred())
            return NULL;
        varType = &vt_String;
        return Variable_New(cursor, numElements, varType, size, 0, NULL);
    }

    // passing an array of two elements to define an array
    if (PyList_Check(value))
        return Variable_NewArrayByType(cursor, value);

    // handle directly bound variables
    if (Variable_Check(value)) {
        Py_INCREF(value);
        return (udt_Variable*) value;
    }

    // everything else ought to be a Python type
    varType = Variable_TypeByPythonType(cursor, value);
    if (!varType)
        return NULL;
    return Variable_New(cursor, numElements, varType, varType->size, 0, NULL);
}


//-----------------------------------------------------------------------------
// Variable_Bind()
//   Allocate a variable and bind it to the given statement.
//-----------------------------------------------------------------------------
static int Variable_Bind(udt_Variable *var, udt_Cursor *cursor, PyObject *name,
        uint32_t pos)
{
    udt_Buffer nameBuffer;
    int status;

    // perform the bind
    if (name) {
        if (cxBuffer_FromObject(&nameBuffer, name,
                cursor->connection->encodingInfo.encoding) < 0)
            return -1;
        status = dpiStmt_bindByName(cursor->handle, (char*) nameBuffer.ptr,
                nameBuffer.size, var->handle);
        cxBuffer_Clear(&nameBuffer);
    } else {
        status = dpiStmt_bindByPos(cursor->handle, pos, var->handle);
    }
    if (status < 0)
        return Error_RaiseAndReturnInt();

    return 0;
}


//-----------------------------------------------------------------------------
// Variable_GetSingleValue()
//   Return the value of the variable at the given position.
//-----------------------------------------------------------------------------
static PyObject *Variable_GetSingleValue(udt_Variable *var, uint32_t arrayPos)
{
    PyObject *value, *result;
    dpiData *data;

    data = &var->data[arrayPos];
    if (data->isNull)
        Py_RETURN_NONE;
    value = (*var->type->getValueProc)(var, data);
    if (value && var->outConverter && var->outConverter != Py_None) {
        result = PyObject_CallFunctionObjArgs(var->outConverter, value, NULL);
        Py_DECREF(value);
        return result;
    }

    return value;
}


//-----------------------------------------------------------------------------
// Variable_GetArrayValue()
//   Return the value of the variable as an array.
//-----------------------------------------------------------------------------
static PyObject *Variable_GetArrayValue(udt_Variable *var,
        uint32_t numElements)
{
    PyObject *value, *singleValue;
    uint32_t i;

    value = PyList_New(numElements);
    if (!value)
        return NULL;

    for (i = 0; i < numElements; i++) {
        singleValue = Variable_GetSingleValue(var, i);
        if (!singleValue) {
            Py_DECREF(value);
            return NULL;
        }
        PyList_SET_ITEM(value, i, singleValue);
    }

    return value;
}


//-----------------------------------------------------------------------------
// Variable_GetValue()
//   Return the value of the variable.
//-----------------------------------------------------------------------------
static PyObject *Variable_GetValue(udt_Variable *var, uint32_t arrayPos)
{
    uint32_t numElements;

    if (var->isArray) {
        if (dpiVar_getNumElementsInArray(var->handle, &numElements) < 0)
            return Error_RaiseAndReturnNull();
        return Variable_GetArrayValue(var, numElements);
    }

    return Variable_GetSingleValue(var, arrayPos);
}


//-----------------------------------------------------------------------------
// Variable_SetSingleValue()
//   Set a single value in the variable.
//-----------------------------------------------------------------------------
static int Variable_SetSingleValue(udt_Variable *var, uint32_t arrayPos,
        PyObject *value)
{
    PyObject *convertedValue = NULL;
    int result = 0;
    dpiData *data;

    // convert value, if necessary
    if (var->inConverter && var->inConverter != Py_None) {
        convertedValue = PyObject_CallFunctionObjArgs(var->inConverter, value,
                NULL);
        if (!convertedValue)
            return -1;
        value = convertedValue;
    }

    // set up for transformation from Python to value expected by DPI
    data = &var->data[arrayPos];
    data->isNull = (value == Py_None);
    if (!data->isNull)
        result = (*var->type->setValueProc)(var, arrayPos, data, value);
    Py_XDECREF(convertedValue);
    return result;
}


//-----------------------------------------------------------------------------
// Variable_SetArrayValue()
//   Set all of the array values for the variable.
//-----------------------------------------------------------------------------
static int Variable_SetArrayValue(udt_Variable *var, PyObject *value)
{
    uint32_t numElements, i;

    // ensure we have an array to set
    if (!PyList_Check(value)) {
        PyErr_SetString(PyExc_TypeError, "expecting array data");
        return -1;
    }

    // set the number of actual elements
    numElements = (uint32_t) PyList_GET_SIZE(value);
    if (dpiVar_setNumElementsInArray(var->handle, numElements) < 0)
        return Error_RaiseAndReturnInt();

    // set all of the values
    for (i = 0; i < numElements; i++) {
        if (Variable_SetSingleValue(var, i, PyList_GET_ITEM(value, i)) < 0)
            return -1;
    }

    return 0;
}


//-----------------------------------------------------------------------------
// Variable_SetValue()
//   Set the value of the variable.
//-----------------------------------------------------------------------------
static int Variable_SetValue(udt_Variable *var, uint32_t arrayPos,
        PyObject *value)
{
    if (var->isArray) {
        if (arrayPos > 0) {
            PyErr_SetString(g_NotSupportedErrorException,
                    "arrays of arrays are not supported by the OCI");
            return -1;
        }
        return Variable_SetArrayValue(var, value);
    }
    return Variable_SetSingleValue(var, arrayPos, value);
}


//-----------------------------------------------------------------------------
// Variable_ExternalCopy()
//   Copy the contents of the source variable to the destination variable.
//-----------------------------------------------------------------------------
static PyObject *Variable_ExternalCopy(udt_Variable *targetVar, PyObject *args)
{
    uint32_t sourcePos, targetPos;
    udt_Variable *sourceVar;

    if (!PyArg_ParseTuple(args, "Oii", &sourceVar, &sourcePos, &targetPos))
        return NULL;
    if (Py_TYPE(targetVar) != Py_TYPE(sourceVar)) {
        PyErr_SetString(g_ProgrammingErrorException,
                "source and target variable type must match");
        return NULL;
    }
    if (dpiVar_copyData(targetVar->handle, targetPos, sourceVar->handle,
            sourcePos) < 0)
        return Error_RaiseAndReturnNull();

    Py_RETURN_NONE;
}


//-----------------------------------------------------------------------------
// Variable_ExternalSetValue()
//   Set the value of the variable at the given position.
//-----------------------------------------------------------------------------
static PyObject *Variable_ExternalSetValue(udt_Variable *var, PyObject *args)
{
    PyObject *value;
    uint32_t pos;

    if (!PyArg_ParseTuple(args, "iO", &pos, &value))
      return NULL;
    if (Variable_SetValue(var, pos, value) < 0)
      return NULL;

    Py_RETURN_NONE;
}


//-----------------------------------------------------------------------------
// Variable_ExternalGetValue()
//   Return the value of the variable at the given position.
//-----------------------------------------------------------------------------
static PyObject *Variable_ExternalGetValue(udt_Variable *var, PyObject *args,
        PyObject *keywordArgs)
{
    static char *keywordList[] = { "pos", NULL };
    uint32_t pos = 0;

    if (!PyArg_ParseTupleAndKeywords(args, keywordArgs, "|i", keywordList,
            &pos))
        return NULL;
    return Variable_GetValue(var, pos);
}


//-----------------------------------------------------------------------------
// Variable_ExternalGetActualElements()
//   Return the values of the variable at all positions as a list.
//-----------------------------------------------------------------------------
static PyObject *Variable_ExternalGetActualElements(udt_Variable *var,
        void *unused)
{
    uint32_t numElements;

    if (dpiVar_getNumElementsInArray(var->handle, &numElements) < 0)
        return Error_RaiseAndReturnNull();
    return PyInt_FromLong(numElements);
}


//-----------------------------------------------------------------------------
// Variable_ExternalGetValues()
//   Return the values of the variable at all positions as a list.
//-----------------------------------------------------------------------------
static PyObject *Variable_ExternalGetValues(udt_Variable *var, void *unused)
{
    uint32_t numElements;

    if (dpiVar_getNumElementsInArray(var->handle, &numElements) < 0)
        return Error_RaiseAndReturnNull();
    return Variable_GetArrayValue(var, numElements);
}


//-----------------------------------------------------------------------------
// Variable_Repr()
//   Return a string representation of the variable.
//-----------------------------------------------------------------------------
static PyObject *Variable_Repr(udt_Variable *var)
{
    PyObject *value, *module, *name, *result, *format, *formatArgs;
    uint32_t numElements;

    if (var->isArray) {
        if (dpiVar_getNumElementsInArray(var->handle, &numElements) < 0)
            return Error_RaiseAndReturnNull();
        value = Variable_GetArrayValue(var, numElements);
    } else if (var->allocatedElements == 1)
        value = Variable_GetSingleValue(var, 0);
    else value = Variable_GetArrayValue(var, var->allocatedElements);
    if (!value)
        return NULL;

    format = cxString_FromAscii("<%s.%s with value %r>");
    if (!format) {
        Py_DECREF(value);
        return NULL;
    }
    if (GetModuleAndName(Py_TYPE(var), &module, &name) < 0) {
        Py_DECREF(value);
        Py_DECREF(format);
        return NULL;
    }
    formatArgs = PyTuple_Pack(3, module, name, value);
    Py_DECREF(module);
    Py_DECREF(name);
    Py_DECREF(value);
    if (!formatArgs) {
        Py_DECREF(format);
        return NULL;
    }
    result = cxString_Format(format, formatArgs);
    Py_DECREF(format);
    Py_DECREF(formatArgs);
    return result;
}

