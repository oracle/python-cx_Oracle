//-----------------------------------------------------------------------------
// Copyright (c) 2019, 2020, Oracle and/or its affiliates. All rights reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// cxoQueue.c
//   Defines the routines for handling queues (advanced queuing). These queues
// permit sending and receiving messages defined by the database.
//-----------------------------------------------------------------------------

#include "cxoModule.h"

//-----------------------------------------------------------------------------
// cxoQueue_new()
//   Create a new queue (advanced queuing).
//-----------------------------------------------------------------------------
cxoQueue *cxoQueue_new(cxoConnection *conn, dpiQueue *handle)
{
    dpiDeqOptions *deqOptions;
    dpiEnqOptions *enqOptions;
    cxoQueue *queue;

    // create queue and populate basic attributes
    queue = (cxoQueue*) cxoPyTypeQueue.tp_alloc(&cxoPyTypeQueue, 0);
    if (!queue) {
        dpiQueue_release(handle);
        return NULL;
    }
    Py_INCREF(conn);
    queue->conn = conn;
    queue->handle = handle;

    // get dequeue options
    if (dpiQueue_getDeqOptions(queue->handle, &deqOptions) < 0) {
        cxoError_raiseAndReturnNull();
        Py_DECREF(queue);
        return NULL;
    }
    queue->deqOptions = (PyObject*) cxoDeqOptions_new(conn, deqOptions);
    if (!queue->deqOptions) {
        Py_DECREF(queue);
        return NULL;
    }

    // get enqueue options
    if (dpiQueue_getEnqOptions(queue->handle, &enqOptions) < 0) {
        cxoError_raiseAndReturnNull();
        Py_DECREF(queue);
        return NULL;
    }
    queue->enqOptions = (PyObject*) cxoEnqOptions_new(conn, enqOptions);
    if (!queue->enqOptions) {
        Py_DECREF(queue);
        return NULL;
    }

    return queue;
}


//-----------------------------------------------------------------------------
// cxoQueue_free()
//   Free the memory associated with a queue.
//-----------------------------------------------------------------------------
static void cxoQueue_free(cxoQueue *queue)
{
    if (queue->handle) {
        dpiQueue_release(queue->handle);
        queue->handle = NULL;
    }
    Py_CLEAR(queue->conn);
    Py_CLEAR(queue->name);
    Py_CLEAR(queue->payloadType);
    Py_CLEAR(queue->deqOptions);
    Py_CLEAR(queue->enqOptions);
    Py_TYPE(queue)->tp_free((PyObject*) queue);
}


//-----------------------------------------------------------------------------
// cxoQueue_repr()
//   Return a string representation of a queue.
//-----------------------------------------------------------------------------
static PyObject *cxoQueue_repr(cxoQueue *queue)
{
    PyObject *module, *name, *result;

    if (cxoUtils_getModuleAndName(Py_TYPE(queue), &module, &name) < 0)
        return NULL;
    result = cxoUtils_formatString("<%s.%s %r>",
            PyTuple_Pack(3, module, name, queue->name));
    Py_DECREF(module);
    Py_DECREF(name);
    return result;
}


//-----------------------------------------------------------------------------
// cxoQueue_deqHelper()
//   Helper for dequeuing messages from a queue.
//-----------------------------------------------------------------------------
int cxoQueue_deqHelper(cxoQueue *queue, uint32_t *numProps,
        cxoMsgProps **props)
{
    uint32_t bufferLength, i, j;
    dpiMsgProps **handles;
    dpiObject *objHandle;
    const char *buffer;
    cxoMsgProps *temp;
    cxoObject *obj;
    int ok, status;

    // use the same array to store the intermediate values provided by ODPI-C;
    // by doing so there is no need to allocate an additional array and any
    // values created by this helper routine are cleaned up on error
    handles = (dpiMsgProps**) props;

    // perform dequeue
    Py_BEGIN_ALLOW_THREADS
    status = dpiQueue_deqMany(queue->handle, numProps, handles);
    Py_END_ALLOW_THREADS
    if (status < 0)
        return cxoError_raiseAndReturnInt();

    // create objects that are returned to the user
    for (i = 0; i < *numProps; i++) {

        // create message property object
        temp = cxoMsgProps_new(queue->conn, handles[i]);
        ok = (temp) ? 1 : 0;
        props[i] = temp;

        // get payload from ODPI-C message property
        if (ok && dpiMsgProps_getPayload(temp->handle, &objHandle, &buffer,
                &bufferLength) < 0) {
            cxoError_raiseAndReturnInt();
            ok = 0;
        }

        // store payload on cx_Oracle message property
        if (ok && objHandle) {
            obj = (cxoObject*) cxoObject_new(queue->payloadType, objHandle);
            if (obj && dpiObject_addRef(objHandle) < 0) {
                cxoError_raiseAndReturnInt();
                obj->handle = NULL;
                Py_CLEAR(obj);
                ok = 0;
            }
            temp->payload = (PyObject*) obj;
        } else if (ok) {
            temp->payload = PyBytes_FromStringAndSize(buffer, bufferLength);
        }

        // if an error occurred, do some cleanup
        if (!ok || !temp->payload) {
            Py_XDECREF(temp);
            for (j = 0; j < i; j++)
                Py_DECREF(props[j]);
            for (j = i + 1; j < *numProps; j++)
                dpiMsgProps_release(handles[j]);
            return -1;
        }

    }

    return 0;
}


//-----------------------------------------------------------------------------
// cxoQueue_enqHelper()
//   Helper for enqueuing messages from a queue.
//-----------------------------------------------------------------------------
int cxoQueue_enqHelper(cxoQueue *queue, uint32_t numProps,
        cxoMsgProps **props)
{
    dpiMsgProps **handles, *tempHandle;
    cxoBuffer buffer;
    cxoObject *obj;
    uint32_t i;
    int status;

    // use the same array to store the intermediate values required by ODPI-C;
    // by doing so there is no need to allocate an additional array
    handles = (dpiMsgProps**) props;

    // process array
    for (i = 0; i < numProps; i++) {

        // verify that the message property object has a payload
        if (!props[i]->payload || props[i]->payload == Py_None) {
            cxoError_raiseFromString(cxoProgrammingErrorException,
                    "message has no payload");
            return -1;
        }

        // transfer payload to message properties object
        tempHandle = props[i]->handle;
        if (PyObject_IsInstance(props[i]->payload,
                (PyObject*) &cxoPyTypeObject)) {
            obj = (cxoObject*) props[i]->payload;
            if (dpiMsgProps_setPayloadObject(props[i]->handle,
                    obj->handle) < 0)
                return cxoError_raiseAndReturnInt();
        } else {
            if (cxoBuffer_fromObject(&buffer, props[i]->payload,
                    props[i]->encoding) < 0)
                return -1;
            status = dpiMsgProps_setPayloadBytes(props[i]->handle, buffer.ptr,
                    buffer.size);
            cxoBuffer_clear(&buffer);
            if (status < 0)
                return cxoError_raiseAndReturnInt();
        }
        handles[i] = tempHandle;

    }

    // perform enqueue
    Py_BEGIN_ALLOW_THREADS
    status = dpiQueue_enqMany(queue->handle, numProps, handles);
    Py_END_ALLOW_THREADS
    if (status < 0)
        return cxoError_raiseAndReturnInt();

    return 0;
}


//-----------------------------------------------------------------------------
// cxoQueue_deqMany()
//   Dequeue a single message to the queue.
//-----------------------------------------------------------------------------
static PyObject *cxoQueue_deqMany(cxoQueue *queue, PyObject *args)
{
    unsigned int numPropsFromPython;
    uint32_t numProps, i;
    cxoMsgProps **props;
    PyObject *result;

    if (!PyArg_ParseTuple(args, "I", &numPropsFromPython))
        return NULL;
    numProps = (uint32_t) numPropsFromPython;
    props = PyMem_Malloc(numProps * sizeof(cxoMsgProps*));
    if (!props)
        return NULL;
    if (cxoQueue_deqHelper(queue, &numProps, props) < 0) {
        PyMem_Free(props);
        return NULL;
    }
    result = PyList_New(numProps);
    if (!result) {
        for (i = 0; i < numProps; i++)
            Py_DECREF(props[i]);
        PyMem_Free(props);
        return NULL;
    }
    for (i = 0; i < numProps; i++)
        PyList_SET_ITEM(result, i, (PyObject*) props[i]);
    PyMem_Free(props);
    return result;
}


//-----------------------------------------------------------------------------
// cxoQueue_deqOne()
//   Dequeue a single message to the queue.
//-----------------------------------------------------------------------------
static PyObject *cxoQueue_deqOne(cxoQueue *queue, PyObject *args)
{
    uint32_t numProps = 1;
    cxoMsgProps *props;

    if (cxoQueue_deqHelper(queue, &numProps, &props) < 0)
        return NULL;
    if (numProps > 0)
        return (PyObject*) props;
    Py_RETURN_NONE;
}


//-----------------------------------------------------------------------------
// cxoQueue_enqMany()
//   Enqueue multiple messages to the queue.
//-----------------------------------------------------------------------------
static PyObject *cxoQueue_enqMany(cxoQueue *queue, PyObject *args)
{
    PyObject *seq, *seqCheck, *temp;
    Py_ssize_t seqLength, i;
    cxoMsgProps **props;
    int status;

    // validate arguments
    if (!PyArg_ParseTuple(args, "O", &seqCheck))
        return NULL;
    seq = PySequence_Fast(seqCheck, "expecting sequence");
    if (!seq)
        return NULL;

    // zero messages means nothing to do
    seqLength = PySequence_Length(seq);
    if (seqLength == 0) {
        Py_DECREF(seq);
        Py_RETURN_NONE;
    }

    // populate array of properties
    props = PyMem_Malloc(seqLength * sizeof(cxoMsgProps*));
    if (!props) {
        PyErr_NoMemory();
        Py_DECREF(seq);
        return NULL;
    }
    for (i = 0; i < seqLength; i++) {
        temp = PySequence_Fast_GET_ITEM(seq, i);
        if (Py_TYPE(temp) != &cxoPyTypeMsgProps) {
            Py_DECREF(seq);
            PyMem_Free(props);
            PyErr_SetString(PyExc_TypeError,
                    "expecting sequence of message property objects");
            return NULL;
        }
        props[i] = (cxoMsgProps*) temp;
    }

    // perform enqueue
    status = cxoQueue_enqHelper(queue, (uint32_t) seqLength, props);
    Py_DECREF(seq);
    PyMem_Free(props);
    if (status < 0)
        return NULL;

    Py_RETURN_NONE;
}


//-----------------------------------------------------------------------------
// cxoQueue_enqOne()
//   Enqueue a single message to the queue.
//-----------------------------------------------------------------------------
static PyObject *cxoQueue_enqOne(cxoQueue *queue, PyObject *args)
{
    cxoMsgProps *props;

    if (!PyArg_ParseTuple(args, "O!", &cxoPyTypeMsgProps, &props))
        return NULL;
    if (cxoQueue_enqHelper(queue, 1, &props) < 0)
        return NULL;

    Py_RETURN_NONE;
}


//-----------------------------------------------------------------------------
// declaration of methods
//-----------------------------------------------------------------------------
static PyMethodDef cxoMethods[] = {
    { "deqmany", (PyCFunction) cxoQueue_deqMany, METH_VARARGS },
    { "deqone", (PyCFunction) cxoQueue_deqOne, METH_NOARGS },
    { "enqmany", (PyCFunction) cxoQueue_enqMany, METH_VARARGS },
    { "enqone", (PyCFunction) cxoQueue_enqOne, METH_VARARGS },
    { "deqMany", (PyCFunction) cxoQueue_deqMany, METH_VARARGS },
    { "deqOne", (PyCFunction) cxoQueue_deqOne, METH_NOARGS },
    { "enqMany", (PyCFunction) cxoQueue_enqMany, METH_VARARGS },
    { "enqOne", (PyCFunction) cxoQueue_enqOne, METH_VARARGS },
    { NULL }
};


//-----------------------------------------------------------------------------
// declaration of members
//-----------------------------------------------------------------------------
static PyMemberDef cxoMembers[] = {
    { "connection", T_OBJECT, offsetof(cxoQueue, conn), READONLY },
    { "deqoptions", T_OBJECT, offsetof(cxoQueue, deqOptions), READONLY },
    { "enqoptions", T_OBJECT, offsetof(cxoQueue, enqOptions), READONLY },
    { "name", T_OBJECT, offsetof(cxoQueue, name), READONLY },
    { "payload_type", T_OBJECT, offsetof(cxoQueue, payloadType), READONLY },
    { "deqOptions", T_OBJECT, offsetof(cxoQueue, deqOptions), READONLY },
    { "enqOptions", T_OBJECT, offsetof(cxoQueue, enqOptions), READONLY },
    { "payloadType", T_OBJECT, offsetof(cxoQueue, payloadType), READONLY },
    { NULL }
};


//-----------------------------------------------------------------------------
// Python type declarations
//-----------------------------------------------------------------------------
PyTypeObject cxoPyTypeQueue = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "cx_Oracle.Queue",
    .tp_basicsize = sizeof(cxoQueue),
    .tp_dealloc = (destructor) cxoQueue_free,
    .tp_repr = (reprfunc) cxoQueue_repr,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_methods = cxoMethods,
    .tp_members = cxoMembers
};
