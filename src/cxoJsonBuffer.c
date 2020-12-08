//-----------------------------------------------------------------------------
// Copyright (c) 2020, Oracle and/or its affiliates. All rights reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// cxoJsonBuffer.c
//   Defines buffer structure and routines for populating JSON values. These
// are used to translate Python objects (scalars, dictionaries and lists) into
// JSON values stored in the database.
//-----------------------------------------------------------------------------

#include "cxoModule.h"

#define CXO_JSON_ENCODING               "UTF-8"

//-----------------------------------------------------------------------------
// cxoJsonBuffer_getBuffer()
//   Acquire a new buffer from the array of buffers. If one is not available,
// more space is allocated in chunks.
//-----------------------------------------------------------------------------
static int cxoJsonBuffer_getBuffer(cxoJsonBuffer *buf, cxoBuffer **buffer)
{
    cxoBuffer *tempBuffers;

    if (buf->numBuffers == buf->allocatedBuffers) {
        buf->allocatedBuffers += 16;
        tempBuffers = PyMem_Realloc(buf->buffers,
                buf->allocatedBuffers * sizeof(cxoBuffer));
        if (!tempBuffers) {
            PyErr_NoMemory();
            return -1;
        }
        buf->buffers = tempBuffers;
    }
    *buffer = &buf->buffers[buf->numBuffers++];

    return 0;
}


//-----------------------------------------------------------------------------
// cxoJsonBuffer_populateNode()
//   Populate a particular node with the value of the Python object.
//-----------------------------------------------------------------------------
static int cxoJsonBuffer_populateNode(cxoJsonBuffer *buf, dpiJsonNode *node,
        PyObject *value)
{
    cxoTransformNum transformNum;
    PyObject *childValue, *key;
    cxoBuffer *tempBuffer;
    Py_ssize_t pos, size;
    dpiJsonArray *array;
    dpiJsonObject *obj;
    char message[250];
    uint32_t i;

    // handle NULL values
    if (value == Py_None) {
        node->oracleTypeNum = DPI_ORACLE_TYPE_NONE;
        node->nativeTypeNum = DPI_NATIVE_TYPE_NULL;
        return 0;
    }

    // handle arrays
    if (PyList_Check(value)) {

        // initialize array
        node->oracleTypeNum = DPI_ORACLE_TYPE_JSON_ARRAY;
        node->nativeTypeNum = DPI_NATIVE_TYPE_JSON_ARRAY;
        array = &node->value->asJsonArray;
        array->numElements = (uint32_t) PyList_GET_SIZE(value);
        array->elements = PyMem_Calloc(array->numElements,
                sizeof(dpiJsonNode));
        array->elementValues = PyMem_Calloc(array->numElements,
                sizeof(dpiDataBuffer));
        if (!array->elements || !array->elementValues) {
            PyErr_NoMemory();
            return -1;
        }

        // process each element of the array
        for (i = 0; i < array->numElements; i++) {
            childValue = PyList_GET_ITEM(value, i);
            array->elements[i].value = &array->elementValues[i];
            if (cxoJsonBuffer_populateNode(buf, &array->elements[i],
                    childValue) < 0)
                return -1;
        }

        return 0;
    }

    // handle dictionaries
    if (PyDict_Check(value)) {

        // initialize object
        node->oracleTypeNum = DPI_ORACLE_TYPE_JSON_OBJECT;
        node->nativeTypeNum = DPI_NATIVE_TYPE_JSON_OBJECT;
        obj = &node->value->asJsonObject;
        size = PyDict_Size(value);
        if (size < 0)
            return -1;
        obj->numFields = (uint32_t) size;
        obj->fieldNames = PyMem_Calloc(obj->numFields, sizeof(char*));
        obj->fieldNameLengths = PyMem_Calloc(obj->numFields, sizeof(uint32_t));
        obj->fields = PyMem_Calloc(obj->numFields, sizeof(dpiJsonNode));
        obj->fieldValues = PyMem_Calloc(obj->numFields,
                sizeof(dpiDataBuffer));
        if (!obj->fieldNames || !obj->fieldNameLengths || !obj->fields ||
                !obj->fieldValues) {
            PyErr_NoMemory();
            return -1;
        }

        // process each entry in the dictionary
        i = 0;
        pos = 0;
        while (PyDict_Next(value, &pos, &key, &childValue)) {
            if (cxoJsonBuffer_getBuffer(buf, &tempBuffer) < 0)
                return -1;
            if (cxoBuffer_fromObject(tempBuffer, key, CXO_JSON_ENCODING) < 0)
                return -1;
            obj->fields[i].value = &obj->fieldValues[i];
            obj->fieldNames[i] = (char*) tempBuffer->ptr;
            obj->fieldNameLengths[i] = tempBuffer->size;
            if (cxoJsonBuffer_populateNode(buf, &obj->fields[i],
                    childValue) < 0)
                return -1;
            i++;
        }

        return 0;
    }

    // handle scalar values
    tempBuffer = NULL;
    transformNum = cxoTransform_getNumFromPythonValue(value, 1);
    switch (transformNum) {

        // strings and bytes must have a buffer made available for them to
        // store a reference to the object and the actual pointer and length;
        // numbers are converted to a string in order to prevent precision loss
        case CXO_TRANSFORM_STRING:
        case CXO_TRANSFORM_BINARY:
        case CXO_TRANSFORM_INT:
        case CXO_TRANSFORM_FLOAT:
        case CXO_TRANSFORM_DECIMAL:
            if (cxoJsonBuffer_getBuffer(buf, &tempBuffer) < 0)
                return -1;
            break;

        // swap CXO_TRANSFORM_DATETIME to CXO_TRANSFORM_TIMESTAMP to preserve
        // fractional seconds
        case CXO_TRANSFORM_DATETIME:
            transformNum = CXO_TRANSFORM_TIMESTAMP;
            break;

        // all other types do not need any special processing
        case CXO_TRANSFORM_BOOLEAN:
        case CXO_TRANSFORM_DATE:
        case CXO_TRANSFORM_TIMEDELTA:
            break;

        // any other type is not currently supported
        default:
            snprintf(message, sizeof(message), "Python type %s not supported.",
                    Py_TYPE(value)->tp_name);
            cxoError_raiseFromString(cxoNotSupportedErrorException, message);
            return -1;
    }

    // transform the Python value into the Oracle value
    cxoTransform_getTypeInfo(transformNum, &node->oracleTypeNum,
            &node->nativeTypeNum);
    if (cxoTransform_fromPython(transformNum, &node->nativeTypeNum, value,
            node->value, tempBuffer, CXO_JSON_ENCODING, CXO_JSON_ENCODING,
            NULL, 0) < 0)
        return -1;

    return 0;
}


//-----------------------------------------------------------------------------
// cxoJsonBuffer_freeNode()
//   Frees any arrays allocated earlier for the specified node.
//-----------------------------------------------------------------------------
static void cxoJsonBuffer_freeNode(dpiJsonNode *node)
{
    dpiJsonArray *array;
    dpiJsonObject *obj;
    uint32_t i;

    switch (node->nativeTypeNum) {
        case DPI_NATIVE_TYPE_JSON_ARRAY:
            array = &node->value->asJsonArray;
            if (array->elements) {
                for (i = 0; i < array->numElements; i++) {
                    if (array->elements[i].value)
                        cxoJsonBuffer_freeNode(&array->elements[i]);
                }
                PyMem_Free(array->elements);
                array->elements = NULL;
            }
            if (array->elementValues) {
                PyMem_Free(array->elementValues);
                array->elementValues = NULL;
            }
            break;
        case DPI_NATIVE_TYPE_JSON_OBJECT:
            obj = &node->value->asJsonObject;
            if (obj->fields) {
                for (i = 0; i < obj->numFields; i++) {
                    if (obj->fields[i].value)
                        cxoJsonBuffer_freeNode(&obj->fields[i]);
                }
                PyMem_Free(obj->fields);
                obj->fields = NULL;
            }
            if (obj->fieldNames) {
                PyMem_Free(obj->fieldNames);
                obj->fieldNames = NULL;
            }
            if (obj->fieldNameLengths) {
                PyMem_Free(obj->fieldNameLengths);
                obj->fieldNameLengths = NULL;
            }
            if (obj->fieldValues) {
                PyMem_Free(obj->fieldValues);
                obj->fieldValues = NULL;
            }
            break;
    }
}


//-----------------------------------------------------------------------------
// cxoJsonBuffer_free()
//   Frees any memory allocated for the JSON buffer.
//-----------------------------------------------------------------------------
void cxoJsonBuffer_free(cxoJsonBuffer *buf)
{
    uint32_t i;

    if (buf->buffers) {
        for (i = 0; i < buf->numBuffers; i++)
            cxoBuffer_clear(&buf->buffers[i]);
        PyMem_Free(buf->buffers);
        buf->buffers = NULL;
    }
    cxoJsonBuffer_freeNode(&buf->topNode);
}


//-----------------------------------------------------------------------------
// cxoJsonBuffer_fromObject()
//   Populate the JSON buffer from a Python object.
//-----------------------------------------------------------------------------
int cxoJsonBuffer_fromObject(cxoJsonBuffer *buf, PyObject *obj)
{
    // initialize JSON buffer structure
    buf->topNode.value = &buf->topNodeBuffer;
    buf->allocatedBuffers = 0;
    buf->numBuffers = 0;
    buf->buffers = NULL;

    // populate the top level node
    return cxoJsonBuffer_populateNode(buf, &buf->topNode, obj);
}
