//-----------------------------------------------------------------------------
// AQ.c
//   Implements the enqueue and dequeue options and message properties objects
// used in Advanced Queuing.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// structures used for handling AQ options and message properties
//-----------------------------------------------------------------------------
typedef struct {
    PyObject_HEAD
    udt_Environment *environment;
    OCIAQEnqOptions *handle;
} udt_EnqOptions;

typedef struct {
    PyObject_HEAD
    udt_Environment *environment;
    OCIAQDeqOptions *handle;
} udt_DeqOptions;

typedef struct {
    PyObject_HEAD
    udt_Environment *environment;
    OCIAQMsgProperties *handle;
} udt_MessageProperties;


//-----------------------------------------------------------------------------
// Declaration of methods used for enqueue options
//-----------------------------------------------------------------------------
static udt_EnqOptions *EnqOptions_New(udt_Environment*);
static void EnqOptions_Free(udt_EnqOptions*);
static PyObject *EnqOptions_GetOCIAttr(udt_EnqOptions*, ub4*);
static int EnqOptions_SetOCIAttr(udt_EnqOptions*, PyObject*, ub4*);


//-----------------------------------------------------------------------------
// Declaration of methods used for dequeue options
//-----------------------------------------------------------------------------
static udt_DeqOptions *DeqOptions_New(udt_Environment*);
static void DeqOptions_Free(udt_DeqOptions*);
static PyObject *DeqOptions_GetOCIAttr(udt_DeqOptions*, ub4*);
static int DeqOptions_SetOCIAttr(udt_DeqOptions*, PyObject*, ub4*);


//-----------------------------------------------------------------------------
// Declaration of methods used for message properties
//-----------------------------------------------------------------------------
static udt_MessageProperties *MessageProperties_New(udt_Environment*);
static void MessageProperties_Free(udt_MessageProperties*);
static PyObject *MessageProperties_GetOCIAttr(udt_MessageProperties*, ub4*);
static int MessageProperties_SetOCIAttr(udt_MessageProperties*, PyObject*,
        ub4*);


//-----------------------------------------------------------------------------
// constants for OCI attributes
//-----------------------------------------------------------------------------
static ub4 gc_AQAttempts = OCI_ATTR_ATTEMPTS;
static ub4 gc_AQConsumerName = OCI_ATTR_CONSUMER_NAME;
static ub4 gc_AQCorrelation = OCI_ATTR_CORRELATION;
static ub4 gc_AQDelay = OCI_ATTR_DELAY;
static ub4 gc_AQDeliveryMode = OCI_ATTR_MSG_DELIVERY_MODE;
static ub4 gc_AQDeqCondition = OCI_ATTR_DEQCOND;
static ub4 gc_AQDeqMode = OCI_ATTR_DEQ_MODE;
static ub4 gc_AQDeqMsgId = OCI_ATTR_DEQ_MSGID;
static ub4 gc_AQEnqTime = OCI_ATTR_ENQ_TIME;
static ub4 gc_AQExceptionQ = OCI_ATTR_EXCEPTION_QUEUE;
static ub4 gc_AQExpiration = OCI_ATTR_EXPIRATION;
static ub4 gc_AQNavigation = OCI_ATTR_NAVIGATION;
static ub4 gc_AQOriginalMsgId = OCI_ATTR_ORIGINAL_MSGID;
static ub4 gc_AQPriority = OCI_ATTR_PRIORITY;
static ub4 gc_AQState = OCI_ATTR_MSG_STATE;
static ub4 gc_AQTransformation = OCI_ATTR_TRANSFORMATION;
static ub4 gc_AQVisibility = OCI_ATTR_VISIBILITY;
static ub4 gc_AQWait = OCI_ATTR_WAIT;


//-----------------------------------------------------------------------------
// declaration of calculated members for Python type "EnqOptions"
//-----------------------------------------------------------------------------
static PyGetSetDef g_EnqOptionsCalcMembers[] = {
    { "deliverymode", 0, (setter) EnqOptions_SetOCIAttr, 0,
            &gc_AQDeliveryMode },
    { "transformation", (getter) EnqOptions_GetOCIAttr,
            (setter) EnqOptions_SetOCIAttr, 0, &gc_AQTransformation },
    { "visibility", (getter) EnqOptions_GetOCIAttr,
            (setter) EnqOptions_SetOCIAttr, 0, &gc_AQVisibility },
    { NULL }
};


//-----------------------------------------------------------------------------
// declaration of calculated members for Python type "DeqOptions"
//-----------------------------------------------------------------------------
static PyGetSetDef g_DeqOptionsCalcMembers[] = {
    { "condition", (getter) DeqOptions_GetOCIAttr,
            (setter) DeqOptions_SetOCIAttr, 0, &gc_AQDeqCondition },
    { "consumername", (getter) DeqOptions_GetOCIAttr,
            (setter) DeqOptions_SetOCIAttr, 0, &gc_AQConsumerName },
    { "correlation", (getter) DeqOptions_GetOCIAttr,
            (setter) DeqOptions_SetOCIAttr, 0, &gc_AQCorrelation },
    { "deliverymode", 0, (setter) EnqOptions_SetOCIAttr, 0,
            &gc_AQDeliveryMode },
    { "mode", (getter) DeqOptions_GetOCIAttr,
            (setter) DeqOptions_SetOCIAttr, 0, &gc_AQDeqMode },
    { "msgid", (getter) DeqOptions_GetOCIAttr,
            (setter) DeqOptions_SetOCIAttr, 0, &gc_AQDeqMsgId },
    { "navigation", (getter) DeqOptions_GetOCIAttr,
            (setter) DeqOptions_SetOCIAttr, 0, &gc_AQNavigation },
    { "transformation", (getter) EnqOptions_GetOCIAttr,
            (setter) EnqOptions_SetOCIAttr, 0, &gc_AQTransformation },
    { "visibility", (getter) DeqOptions_GetOCIAttr,
            (setter) DeqOptions_SetOCIAttr, 0, &gc_AQVisibility },
    { "wait", (getter) DeqOptions_GetOCIAttr,
            (setter) DeqOptions_SetOCIAttr, 0, &gc_AQWait },
    { NULL }
};


//-----------------------------------------------------------------------------
// declaration of calculated members for Python type "MessageProperties"
//-----------------------------------------------------------------------------
static PyGetSetDef g_MessagePropertiesCalcMembers[] = {
    { "attempts", (getter) MessageProperties_GetOCIAttr, 0, 0,
            &gc_AQAttempts },
    { "correlation", (getter) MessageProperties_GetOCIAttr,
            (setter) MessageProperties_SetOCIAttr, 0, &gc_AQCorrelation },
    { "delay", (getter) MessageProperties_GetOCIAttr,
            (setter) MessageProperties_SetOCIAttr, 0, &gc_AQDelay },
    { "deliverymode", (getter) MessageProperties_GetOCIAttr, 0, 0,
            &gc_AQDeliveryMode },
    { "enqtime", (getter) MessageProperties_GetOCIAttr, 0, 0, &gc_AQEnqTime },
    { "exceptionq", (getter) MessageProperties_GetOCIAttr,
            (setter) MessageProperties_SetOCIAttr, 0, &gc_AQExceptionQ },
    { "expiration", (getter) MessageProperties_GetOCIAttr,
            (setter) MessageProperties_SetOCIAttr, 0, &gc_AQExpiration },
    { "msgid", (getter) MessageProperties_GetOCIAttr,
            (setter) MessageProperties_SetOCIAttr, 0, &gc_AQOriginalMsgId },
    { "priority", (getter) MessageProperties_GetOCIAttr,
            (setter) MessageProperties_SetOCIAttr, 0, &gc_AQPriority },
    { "state", (getter) MessageProperties_GetOCIAttr, 0, 0, &gc_AQState },
    { NULL }
};


//-----------------------------------------------------------------------------
// Python type declarations
//-----------------------------------------------------------------------------
static PyTypeObject g_EnqOptionsType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "cx_Oracle.EnqOptions",             // tp_name
    sizeof(udt_EnqOptions),             // tp_basicsize
    0,                                  // tp_itemsize
    (destructor) EnqOptions_Free,       // tp_dealloc
    0,                                  // tp_print
    0,                                  // tp_getattr
    0,                                  // tp_setattr
    0,                                  // tp_compare
    0,                                  // tp_repr
    0,                                  // tp_as_number
    0,                                  // tp_as_sequence
    0,                                  // tp_as_mapping
    0,                                  // tp_hash
    0,                                  // tp_call
    0,                                  // tp_str
    0,                                  // tp_getattro
    0,                                  // tp_setattro
    0,                                  // tp_as_buffer
    Py_TPFLAGS_DEFAULT,                 // tp_flags
    0,                                  // tp_doc
    0,                                  // tp_traverse
    0,                                  // tp_clear
    0,                                  // tp_richcompare
    0,                                  // tp_weaklistoffset
    0,                                  // tp_iter
    0,                                  // tp_iternext
    0,                                  // tp_methods
    0,                                  // tp_members
    g_EnqOptionsCalcMembers,            // tp_getset
    0,                                  // tp_base
    0,                                  // tp_dict
    0,                                  // tp_descr_get
    0,                                  // tp_descr_set
    0,                                  // tp_dictoffset
    0,                                  // tp_init
    0,                                  // tp_alloc
    0,                                  // tp_new
    0,                                  // tp_free
    0,                                  // tp_is_gc
    0                                   // tp_bases
};


static PyTypeObject g_DeqOptionsType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "cx_Oracle.DeqOptions",             // tp_name
    sizeof(udt_DeqOptions),             // tp_basicsize
    0,                                  // tp_itemsize
    (destructor) DeqOptions_Free,       // tp_dealloc
    0,                                  // tp_print
    0,                                  // tp_getattr
    0,                                  // tp_setattr
    0,                                  // tp_compare
    0,                                  // tp_repr
    0,                                  // tp_as_number
    0,                                  // tp_as_sequence
    0,                                  // tp_as_mapping
    0,                                  // tp_hash
    0,                                  // tp_call
    0,                                  // tp_str
    0,                                  // tp_getattro
    0,                                  // tp_setattro
    0,                                  // tp_as_buffer
    Py_TPFLAGS_DEFAULT,                 // tp_flags
    0,                                  // tp_doc
    0,                                  // tp_traverse
    0,                                  // tp_clear
    0,                                  // tp_richcompare
    0,                                  // tp_weaklistoffset
    0,                                  // tp_iter
    0,                                  // tp_iternext
    0,                                  // tp_methods
    0,                                  // tp_members
    g_DeqOptionsCalcMembers,            // tp_getset
    0,                                  // tp_base
    0,                                  // tp_dict
    0,                                  // tp_descr_get
    0,                                  // tp_descr_set
    0,                                  // tp_dictoffset
    0,                                  // tp_init
    0,                                  // tp_alloc
    0,                                  // tp_new
    0,                                  // tp_free
    0,                                  // tp_is_gc
    0                                   // tp_bases
};


static PyTypeObject g_MessagePropertiesType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "cx_Oracle.MessageProperties",      // tp_name
    sizeof(udt_MessageProperties),      // tp_basicsize
    0,                                  // tp_itemsize
    (destructor) MessageProperties_Free,// tp_dealloc
    0,                                  // tp_print
    0,                                  // tp_getattr
    0,                                  // tp_setattr
    0,                                  // tp_compare
    0,                                  // tp_repr
    0,                                  // tp_as_number
    0,                                  // tp_as_sequence
    0,                                  // tp_as_mapping
    0,                                  // tp_hash
    0,                                  // tp_call
    0,                                  // tp_str
    0,                                  // tp_getattro
    0,                                  // tp_setattro
    0,                                  // tp_as_buffer
    Py_TPFLAGS_DEFAULT,                 // tp_flags
    0,                                  // tp_doc
    0,                                  // tp_traverse
    0,                                  // tp_clear
    0,                                  // tp_richcompare
    0,                                  // tp_weaklistoffset
    0,                                  // tp_iter
    0,                                  // tp_iternext
    0,                                  // tp_methods
    0,                                  // tp_members
    g_MessagePropertiesCalcMembers,     // tp_getset
    0,                                  // tp_base
    0,                                  // tp_dict
    0,                                  // tp_descr_get
    0,                                  // tp_descr_set
    0,                                  // tp_dictoffset
    0,                                  // tp_init
    0,                                  // tp_alloc
    0,                                  // tp_new
    0,                                  // tp_free
    0,                                  // tp_is_gc
    0                                   // tp_bases
};


//-----------------------------------------------------------------------------
// EnqOptions_New()
//   Create a new enqueue options object.
//-----------------------------------------------------------------------------
static udt_EnqOptions *EnqOptions_New(
    udt_Environment *env)               // environment in which to create
{
    udt_EnqOptions *self;
    sword status;

    self = (udt_EnqOptions*) g_EnqOptionsType.tp_alloc(&g_EnqOptionsType, 0);
    if (!self)
        return NULL;
    Py_INCREF(env);
    self->environment = env;
    status = OCIDescriptorAlloc(env->handle, (dvoid**) &self->handle,
            OCI_DTYPE_AQENQ_OPTIONS, 0, 0);
    if (Environment_CheckForError(env, status, "EnqOptions_New()") < 0) {
        Py_DECREF(self);
        return NULL;
    }

    return self;
}


//-----------------------------------------------------------------------------
// EnqOptions_Free()
//   Free the memory associated with the enqueue options object.
//-----------------------------------------------------------------------------
static void EnqOptions_Free(
    udt_EnqOptions *self)               // object to free
{
    if (self->handle)
        OCIDescriptorFree(self->handle, OCI_DTYPE_AQENQ_OPTIONS);
    Py_CLEAR(self->environment);
    Py_TYPE(self)->tp_free((PyObject*) self);
}


//-----------------------------------------------------------------------------
// EnqOptions_GetOCIAttr()
//   Get the value of the OCI attribute.
//-----------------------------------------------------------------------------
static PyObject *EnqOptions_GetOCIAttr(
    udt_EnqOptions *self,               // options object
    ub4 *attribute)                     // OCI attribute type
{
    ub4 valueLength, ub4Value;
    dvoid *ociValue = NULL;
    char *textValue;
    sword status;

    // get the value from the OCI
    switch (*attribute) {
        case OCI_ATTR_VISIBILITY:
            ociValue = &ub4Value;
            break;
        case OCI_ATTR_TRANSFORMATION:
            ociValue = &textValue;
            break;
    };
    status = OCIAttrGet(self->handle, OCI_DTYPE_AQENQ_OPTIONS, ociValue,
            &valueLength, *attribute, self->environment->errorHandle);
    if (Environment_CheckForError(self->environment, status,
            "EnqOptions_GetOCIAttr()") < 0)
        return NULL;
    if (*attribute == gc_AQTransformation) {
        if (!textValue)
            Py_RETURN_NONE;
        return cxString_FromEncodedString(textValue, valueLength,
                self->environment->encoding);
    }
    return PyInt_FromLong(ub4Value);
}


//-----------------------------------------------------------------------------
// EnqOptions_SetOCIAttr()
//   Set the value of the OCI attribute.
//-----------------------------------------------------------------------------
static int EnqOptions_SetOCIAttr(
    udt_EnqOptions *self,               // options object
    PyObject *value,                    // value to set
    ub4 *attribute)                     // OCI attribute type
{
    dvoid *ociValue = NULL;
    ub4 valueLength = 0;
    udt_Buffer buffer;
    ub4 ub4Value;
    ub2 ub2Value;
    sword status;

    switch (*attribute) {
        case OCI_ATTR_MSG_DELIVERY_MODE:
            ub2Value = (ub2) PyInt_AsLong(value);
            if (PyErr_Occurred())
                return -1;
            ociValue = &ub2Value;
            break;
        case OCI_ATTR_VISIBILITY:
            ub4Value = (ub4) PyInt_AsLong(value);
            if (PyErr_Occurred())
                return -1;
            ociValue = &ub4Value;
            break;
        case OCI_ATTR_TRANSFORMATION:
            if (cxBuffer_FromObject(&buffer, value,
                    self->environment->encoding) < 0)
                return -1;
            ociValue = (dvoid*) buffer.ptr;
            valueLength = buffer.size;
            break;
    };
    status = OCIAttrSet(self->handle, OCI_DTYPE_AQENQ_OPTIONS,
            ociValue, valueLength, *attribute, self->environment->errorHandle);
    if (*attribute == gc_AQTransformation)
        cxBuffer_Clear(&buffer);
    if (Environment_CheckForError(self->environment, status,
            "EnqOptions_SetOCIAttr()") < 0)
        return -1;
    return 0;
}


//-----------------------------------------------------------------------------
// DeqOptions_New()
//   Create a new dequeue options object.
//-----------------------------------------------------------------------------
static udt_DeqOptions *DeqOptions_New(
    udt_Environment *env)               // environment in which to create
{
    udt_DeqOptions *self;
    sword status;

    self = (udt_DeqOptions*) g_DeqOptionsType.tp_alloc(&g_DeqOptionsType, 0);
    if (!self)
        return NULL;
    Py_INCREF(env);
    self->environment = env;
    status = OCIDescriptorAlloc(env->handle, (dvoid**) &self->handle,
            OCI_DTYPE_AQDEQ_OPTIONS, 0, 0);
    if (Environment_CheckForError(env, status, "DeqOptions_New()") < 0) {
        Py_DECREF(self);
        return NULL;
    }

    return self;
}


//-----------------------------------------------------------------------------
// DeqOptions_Free()
//   Free the memory associated with the dequeue options object.
//-----------------------------------------------------------------------------
static void DeqOptions_Free(
    udt_DeqOptions *self)               // object to free
{
    if (self->handle)
        OCIDescriptorFree(self->handle, OCI_DTYPE_AQDEQ_OPTIONS);
    Py_CLEAR(self->environment);
    Py_TYPE(self)->tp_free((PyObject*) self);
}


//-----------------------------------------------------------------------------
// DeqOptions_GetOCIAttr()
//   Get the value of the OCI attribute.
//-----------------------------------------------------------------------------
static PyObject *DeqOptions_GetOCIAttr(
    udt_DeqOptions *self,               // options object
    ub4 *attribute)                     // OCI attribute type
{
    ub4 valueLength, ub4Value;
    dvoid *ociValue = NULL;
    char *rawValuePtr;
    OCIRaw *rawValue;
    char *textValue;
    sword status;

    // get the value from the OCI
    switch (*attribute) {
        case OCI_ATTR_DEQ_MODE:
        case OCI_ATTR_NAVIGATION:
        case OCI_ATTR_VISIBILITY:
        case OCI_ATTR_WAIT:
            ociValue = &ub4Value;
            break;
        case OCI_ATTR_CONSUMER_NAME:
        case OCI_ATTR_CORRELATION:
        case OCI_ATTR_DEQCOND:
        case OCI_ATTR_TRANSFORMATION:
            ociValue = &textValue;
            break;
        case OCI_ATTR_DEQ_MSGID:
            rawValue = NULL;
            ociValue = &rawValue;
            break;
    };
    status = OCIAttrGet(self->handle, OCI_DTYPE_AQDEQ_OPTIONS, ociValue,
            &valueLength, *attribute, self->environment->errorHandle);
    if (Environment_CheckForError(self->environment, status,
            "DeqOptions_GetOCIAttr()") < 0)
        return NULL;
    if (ociValue == &textValue) {
        if (!textValue)
            Py_RETURN_NONE;
        return cxString_FromEncodedString(textValue, valueLength,
                self->environment->encoding);
    } else if (ociValue == &rawValue) {
        if (!rawValue)
            Py_RETURN_NONE;
        rawValuePtr = (char*) OCIRawPtr(self->environment->handle, rawValue);
        valueLength = OCIRawSize(self->environment->handle, rawValue);
        return PyBytes_FromStringAndSize(rawValuePtr, valueLength);
    }
    return PyInt_FromLong(ub4Value);
}


//-----------------------------------------------------------------------------
// DeqOptions_SetOCIAttr()
//   Set the value of the OCI attribute.
//-----------------------------------------------------------------------------
static int DeqOptions_SetOCIAttr(
    udt_DeqOptions *self,               // options object
    PyObject *value,                    // value to set
    ub4 *attribute)                     // OCI attribute type
{
    Py_ssize_t rawValueLength;
    OCIRaw *rawValue = NULL;
    dvoid *ociValue = NULL;
    ub4 valueLength = 0;
    udt_Buffer buffer;
    char *rawValuePtr;
    ub4 ub4Value;
    ub2 ub2Value;
    sword status;

    cxBuffer_Init(&buffer);
    switch (*attribute) {
        case OCI_ATTR_MSG_DELIVERY_MODE:
            ub2Value = (ub2) PyInt_AsLong(value);
            if (PyErr_Occurred())
                return -1;
            ociValue = &ub2Value;
            break;
        case OCI_ATTR_DEQ_MODE:
        case OCI_ATTR_NAVIGATION:
        case OCI_ATTR_VISIBILITY:
        case OCI_ATTR_WAIT:
            ub4Value = (ub4) PyInt_AsLong(value);
            if (PyErr_Occurred())
                return -1;
            ociValue = &ub4Value;
            break;
        case OCI_ATTR_CONSUMER_NAME:
        case OCI_ATTR_CORRELATION:
        case OCI_ATTR_DEQCOND:
        case OCI_ATTR_TRANSFORMATION:
            if (cxBuffer_FromObject(&buffer, value,
                    self->environment->encoding) < 0)
                return -1;
            ociValue = (dvoid*) buffer.ptr;
            valueLength = buffer.size;
            break;
        case OCI_ATTR_DEQ_MSGID:
            if (PyBytes_AsStringAndSize(value, &rawValuePtr,
                    &rawValueLength) < 0)
                return -1;
            status = OCIRawAssignBytes(self->environment->handle,
                    self->environment->errorHandle, (const ub1*) rawValuePtr,
                    (ub4) rawValueLength, &rawValue);
            if (Environment_CheckForError(self->environment, status,
                    "DeqOptions_SetOCIAttr(): assign raw value") < 0)
                return -1;
            ociValue = (dvoid*) rawValue;
            break;
    };
    status = OCIAttrSet(self->handle, OCI_DTYPE_AQDEQ_OPTIONS,
            ociValue, valueLength, *attribute, self->environment->errorHandle);
    cxBuffer_Clear(&buffer);
    if (rawValue)
        OCIRawResize(self->environment->handle, self->environment->errorHandle,
                0, &rawValue);
    if (Environment_CheckForError(self->environment, status,
            "DeqOptions_SetOCIAttr(): set value") < 0)
        return -1;
    return 0;
}


//-----------------------------------------------------------------------------
// MessageProperties_New()
//   Create a new dequeue options object.
//-----------------------------------------------------------------------------
static udt_MessageProperties *MessageProperties_New(
    udt_Environment *env)               // environment in which to create
{
    udt_MessageProperties *self;
    sword status;

    self = (udt_MessageProperties*)
            g_MessagePropertiesType.tp_alloc(&g_MessagePropertiesType, 0);
    if (!self)
        return NULL;
    Py_INCREF(env);
    self->environment = env;
    status = OCIDescriptorAlloc(env->handle, (dvoid**) &self->handle,
            OCI_DTYPE_AQMSG_PROPERTIES, 0, 0);
    if (Environment_CheckForError(env, status,
            "MessageProperties_New()") < 0) {
        Py_DECREF(self);
        return NULL;
    }

    return self;
}


//-----------------------------------------------------------------------------
// MessageProperties_Free()
//   Free the memory associated with the message properties object.
//-----------------------------------------------------------------------------
static void MessageProperties_Free(
    udt_MessageProperties *self)               // object to free
{
    if (self->handle)
        OCIDescriptorFree(self->handle, OCI_DTYPE_AQMSG_PROPERTIES);
    Py_CLEAR(self->environment);
    Py_TYPE(self)->tp_free((PyObject*) self);
}


//-----------------------------------------------------------------------------
// MessageProperties_GetOCIAttr()
//   Get the value of the OCI attribute.
//-----------------------------------------------------------------------------
static PyObject *MessageProperties_GetOCIAttr(
    udt_MessageProperties *self,        // options object
    ub4 *attribute)                     // OCI attribute type
{
    ub4 valueLength, ub4Value;
    dvoid *ociValue = NULL;
    char *rawValuePtr;
    OCIDate dateValue;
    OCIRaw *rawValue;
    char *textValue;
    sb4 sb4Value;
    ub2 ub2Value;
    sword status;

    // get the value from the OCI
    switch (*attribute) {
        case OCI_ATTR_MSG_DELIVERY_MODE:
            ociValue = &ub2Value;
            break;
        case OCI_ATTR_ATTEMPTS:
        case OCI_ATTR_DELAY:
        case OCI_ATTR_EXPIRATION:
        case OCI_ATTR_PRIORITY:
            ociValue = &sb4Value;
            break;
        case OCI_ATTR_MSG_STATE:
            ociValue = &ub4Value;
            break;
        case OCI_ATTR_CORRELATION:
        case OCI_ATTR_EXCEPTION_QUEUE:
            ociValue = &textValue;
            break;
        case OCI_ATTR_ENQ_TIME:
            ociValue = &dateValue;
            break;
        case OCI_ATTR_ORIGINAL_MSGID:
            rawValue = NULL;
            ociValue = &rawValue;
            break;
    };
    status = OCIAttrGet(self->handle, OCI_DTYPE_AQMSG_PROPERTIES, ociValue,
            &valueLength, *attribute, self->environment->errorHandle);
    if (Environment_CheckForError(self->environment, status,
            "MessageProperties_GetOCIAttr()") < 0)
        return NULL;
    if (ociValue == &textValue) {
        if (!textValue)
            Py_RETURN_NONE;
        return cxString_FromEncodedString(textValue, valueLength,
                self->environment->encoding);
    } else if (ociValue == &rawValue) {
        if (!rawValue)
            Py_RETURN_NONE;
        rawValuePtr = (char*) OCIRawPtr(self->environment->handle, rawValue);
        valueLength = OCIRawSize(self->environment->handle, rawValue);
        return PyBytes_FromStringAndSize(rawValuePtr, valueLength);
    } else if (ociValue == &dateValue)
        return OracleDateToPythonDate(&vt_DateTime, &dateValue);
    else if (ociValue == &ub2Value)
        return PyInt_FromLong(ub2Value);
    else if (ociValue == &sb4Value)
        return PyInt_FromLong(sb4Value);
    return PyInt_FromLong(ub4Value);
}


//-----------------------------------------------------------------------------
// MessageProperties_SetOCIAttr()
//   Set the value of the OCI attribute.
//-----------------------------------------------------------------------------
static int MessageProperties_SetOCIAttr(
    udt_MessageProperties *self,               // options object
    PyObject *value,                    // value to set
    ub4 *attribute)                     // OCI attribute type
{
    Py_ssize_t rawValueLength;
    OCIRaw *rawValue = NULL;
    dvoid *ociValue = NULL;
    ub4 valueLength = 0;
    udt_Buffer buffer;
    char *rawValuePtr;
    ub4 ub4Value;
    ub2 ub2Value;
    sword status;

    cxBuffer_Init(&buffer);
    switch (*attribute) {
        case OCI_ATTR_MSG_DELIVERY_MODE:
            ub2Value = (ub2) PyInt_AsLong(value);
            if (PyErr_Occurred())
                return -1;
            ociValue = &ub2Value;
            break;
        case OCI_ATTR_DEQ_MODE:
        case OCI_ATTR_NAVIGATION:
        case OCI_ATTR_VISIBILITY:
        case OCI_ATTR_WAIT:
            ub4Value = (ub4) PyInt_AsLong(value);
            if (PyErr_Occurred())
                return -1;
            ociValue = &ub4Value;
            break;
        case OCI_ATTR_CONSUMER_NAME:
        case OCI_ATTR_CORRELATION:
        case OCI_ATTR_DEQCOND:
        case OCI_ATTR_TRANSFORMATION:
            if (cxBuffer_FromObject(&buffer, value,
                    self->environment->encoding) < 0)
                return -1;
            ociValue = (dvoid*) buffer.ptr;
            valueLength = buffer.size;
            break;
        case OCI_ATTR_DEQ_MSGID:
            if (PyBytes_AsStringAndSize(value, &rawValuePtr,
                    &rawValueLength) < 0)
                return -1;
            status = OCIRawAssignBytes(self->environment->handle,
                    self->environment->errorHandle, (const ub1*) rawValuePtr,
                    (ub4) rawValueLength, &rawValue);
            if (Environment_CheckForError(self->environment, status,
                    "MessageProperties_SetOCIAttr(): assign raw value") < 0)
                return -1;
            ociValue = (dvoid*) rawValue;
            break;
    };
    status = OCIAttrSet(self->handle, OCI_DTYPE_AQMSG_PROPERTIES,
            ociValue, valueLength, *attribute, self->environment->errorHandle);
    cxBuffer_Clear(&buffer);
    if (rawValue)
        OCIRawResize(self->environment->handle, self->environment->errorHandle,
                0, &rawValue);
    if (Environment_CheckForError(self->environment, status,
            "MessageProperties_SetOCIAttr(): set value") < 0)
        return -1;
    return 0;
}

