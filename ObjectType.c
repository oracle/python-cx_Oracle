//-----------------------------------------------------------------------------
// ObjectType.c
//   Defines the routines for handling Oracle type information.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// structures used for handling object types
//-----------------------------------------------------------------------------
typedef struct {
    PyObject_HEAD
    udt_Connection *connection;
    OCIType *tdo;
    PyObject *schema;
    PyObject *name;
    PyObject *attributes;
    PyObject *attributesByName;
    OCITypeCode typeCode;
    OCITypeCode collectionTypeCode;
    OCITypeCode elementTypeCode;
    PyObject *elementType;
    boolean isCollection;
} udt_ObjectType;

typedef struct {
    PyObject_HEAD
    PyObject *name;
    OCITypeCode typeCode;
    udt_ObjectType *subType;
} udt_ObjectAttribute;

//-----------------------------------------------------------------------------
// Declaration of type variable functions.
//-----------------------------------------------------------------------------
static udt_ObjectType *ObjectType_New(udt_Connection*, OCIParam*, ub4);
static void ObjectType_Free(udt_ObjectType*);
static PyObject *ObjectType_Repr(udt_ObjectType*);
static PyObject *ObjectType_NewObject(udt_ObjectType*, PyObject*, PyObject*);
static udt_ObjectAttribute *ObjectAttribute_New(udt_Connection*, OCIParam*);
static void ObjectAttribute_Free(udt_ObjectAttribute*);
static PyObject *ObjectAttribute_Repr(udt_ObjectAttribute*);

#include "Object.c"

//-----------------------------------------------------------------------------
// declaration of methods for Python type "ObjectType"
//-----------------------------------------------------------------------------
static PyMethodDef g_ObjectTypeMethods[] = {
    { "newobject", (PyCFunction) ObjectType_NewObject,
            METH_VARARGS | METH_KEYWORDS },
    { NULL, NULL }
};


//-----------------------------------------------------------------------------
// declaration of members for Python type "ObjectType"
//-----------------------------------------------------------------------------
static PyMemberDef g_ObjectTypeMembers[] = {
    { "schema", T_OBJECT, offsetof(udt_ObjectType, schema), READONLY },
    { "name", T_OBJECT, offsetof(udt_ObjectType, name), READONLY },
    { "attributes", T_OBJECT, offsetof(udt_ObjectType, attributes), READONLY },
    { "elementType", T_OBJECT, offsetof(udt_ObjectType, elementType),
            READONLY },
    { "iscollection", T_BOOL, offsetof(udt_ObjectType, isCollection),
            READONLY },
    { NULL }
};


//-----------------------------------------------------------------------------
// declaration of members for Python type "ObjectAttribute"
//-----------------------------------------------------------------------------
static PyMemberDef g_ObjectAttributeMembers[] = {
    { "name", T_OBJECT, offsetof(udt_ObjectAttribute, name), READONLY },
    { NULL }
};


//-----------------------------------------------------------------------------
// Python type declarations
//-----------------------------------------------------------------------------
static PyTypeObject g_ObjectTypeType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "cx_Oracle.ObjectType",             // tp_name
    sizeof(udt_ObjectType),             // tp_basicsize
    0,                                  // tp_itemsize
    (destructor) ObjectType_Free,       // tp_dealloc
    0,                                  // tp_print
    0,                                  // tp_getattr
    0,                                  // tp_setattr
    0,                                  // tp_compare
    (reprfunc) ObjectType_Repr,         // tp_repr
    0,                                  // tp_as_number
    0,                                  // tp_as_sequence
    0,                                  // tp_as_mapping
    0,                                  // tp_hash
    (ternaryfunc) ObjectType_NewObject, // tp_call
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
    g_ObjectTypeMethods,                // tp_methods
    g_ObjectTypeMembers,                // tp_members
    0,                                  // tp_getset
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


static PyTypeObject g_ObjectAttributeType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "cx_Oracle.ObjectAttribute",        // tp_name
    sizeof(udt_ObjectAttribute),        // tp_basicsize
    0,                                  // tp_itemsize
    (destructor) ObjectAttribute_Free,  // tp_dealloc
    0,                                  // tp_print
    0,                                  // tp_getattr
    0,                                  // tp_setattr
    0,                                  // tp_compare
    (reprfunc) ObjectAttribute_Repr,    // tp_repr
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
    g_ObjectAttributeMembers,           // tp_members
    0,                                  // tp_getset
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
// ObjectType_Describe()
//   Describe the type and store information about it as needed.
//-----------------------------------------------------------------------------
static int ObjectType_Describe(
    udt_ObjectType *self,               // type to populate
    OCIDescribe *describeHandle)        // describe handle
{
    OCIParam *topLevelParam, *attributeListParam, *attributeParam;
    udt_ObjectAttribute *attribute;
    OCIParam *collectionParam;
    ub2 numAttributes;
    sword status;
    int i;

    // describe the type
    status = OCIDescribeAny(self->connection->handle,
            self->connection->environment->errorHandle, (dvoid*) self->tdo, 0,
            OCI_OTYPE_PTR, OCI_DEFAULT, OCI_PTYPE_TYPE, describeHandle);
    if (Environment_CheckForError(self->connection->environment, status,
            "ObjectType_Describe(): describe type") < 0)
        return -1;

    // get top level parameter descriptor
    status = OCIAttrGet(describeHandle, OCI_HTYPE_DESCRIBE, &topLevelParam, 0,
            OCI_ATTR_PARAM, self->connection->environment->errorHandle);
    if (Environment_CheckForError(self->connection->environment, status,
            "ObjectType_Describe(): get top level parameter descriptor") < 0)
        return -1;

    // determine type of type
    status = OCIAttrGet(topLevelParam, OCI_DTYPE_PARAM, &self->typeCode, 0,
            OCI_ATTR_TYPECODE, self->connection->environment->errorHandle);
    if (Environment_CheckForError(self->connection->environment, status,
            "ObjectType_Describe(): get type code") < 0)
        return -1;

    // if a collection, need to determine the sub type
    if (self->typeCode == OCI_TYPECODE_NAMEDCOLLECTION) {
        self->isCollection = 1;

        // determine type of collection
        status = OCIAttrGet(topLevelParam, OCI_DTYPE_PARAM,
                &self->collectionTypeCode, 0, OCI_ATTR_COLLECTION_TYPECODE,
                self->connection->environment->errorHandle);
        if (Environment_CheckForError(self->connection->environment, status,
                "ObjectType_Describe(): get collection type code") < 0)
            return -1;

        // acquire collection parameter descriptor
        status = OCIAttrGet(topLevelParam, OCI_DTYPE_PARAM, &collectionParam,
                0, OCI_ATTR_COLLECTION_ELEMENT,
                self->connection->environment->errorHandle);
        if (Environment_CheckForError(self->connection->environment, status,
                "ObjectType_Describe(): get collection descriptor") < 0)
            return -1;

        // determine type of element
        status = OCIAttrGet(collectionParam, OCI_DTYPE_PARAM,
                &self->elementTypeCode, 0, OCI_ATTR_TYPECODE,
                self->connection->environment->errorHandle);
        if (Environment_CheckForError(self->connection->environment, status,
                "ObjectType_Describe(): get element type code") < 0)
            return -1;

        // if element type is an object type get its type
        if (self->elementTypeCode == OCI_TYPECODE_OBJECT) {
            self->elementType = (PyObject*)
                    ObjectType_New(self->connection, collectionParam,
                            OCI_ATTR_TYPE_NAME);
            if (!self->elementType)
                return -1;
        }

    }

    // determine the number of attributes
    status = OCIAttrGet(topLevelParam, OCI_DTYPE_PARAM,
            (dvoid*) &numAttributes, 0, OCI_ATTR_NUM_TYPE_ATTRS,
            self->connection->environment->errorHandle);
    if (Environment_CheckForError(self->connection->environment, status,
            "ObjectType_Describe(): get number of attributes") < 0)
        return -1;

    // allocate the attribute list and dictionary
    self->attributes = PyList_New(numAttributes);
    if (!self->attributes)
        return -1;
    self->attributesByName = PyDict_New();
    if (!self->attributesByName)
        return -1;

    // acquire the list parameter descriptor
    status = OCIAttrGet(topLevelParam, OCI_DTYPE_PARAM,
            (dvoid*) &attributeListParam, 0, OCI_ATTR_LIST_TYPE_ATTRS,
            self->connection->environment->errorHandle);
    if (Environment_CheckForError(self->connection->environment, status,
            "ObjectType_Describe(): get list parameter descriptor") < 0)
        return -1;

    // create attribute information for each attribute
    for (i = 0; i < numAttributes; i++) {
        status = OCIParamGet(attributeListParam, OCI_DTYPE_PARAM,
                self->connection->environment->errorHandle,
                (dvoid**) &attributeParam, (ub4) i + 1);
        if (Environment_CheckForError(self->connection->environment, status,
                "ObjectType_Describe(): get attribute param descriptor") < 0)
            return -1;
        attribute = ObjectAttribute_New(self->connection, attributeParam);
        if (!attribute)
            return -1;
        PyList_SET_ITEM(self->attributes, i, (PyObject*) attribute);
        if (PyDict_SetItem(self->attributesByName, attribute->name,
                (PyObject*) attribute) < 0)
            return -1;
    }

    return 0;
}


//-----------------------------------------------------------------------------
// ObjectType_Initialize()
//   Initialize the object type with the information that is required.
//-----------------------------------------------------------------------------
static int ObjectType_Initialize(
    udt_ObjectType *self,               // type to initialize
    OCIParam *param,                    // parameter descriptor
    ub4 nameAttribute)                  // value for the name attribute
{
    OCIDescribe *describeHandle;
    OCIRef *tdoReference;
    sword status;
    char *name;
    ub4 size;

    // determine the schema of the type
    status = OCIAttrGet(param, OCI_HTYPE_DESCRIBE, (dvoid*) &name, &size,
            OCI_ATTR_SCHEMA_NAME, self->connection->environment->errorHandle);
    if (Environment_CheckForError(self->connection->environment, status,
            "ObjectType_Initialize(): get schema name") < 0)
        return -1;
    self->schema = cxString_FromEncodedString(name, size,
            self->connection->environment->encoding);
    if (!self->schema)
        return -1;

    // determine the name of the type
    status = OCIAttrGet(param, OCI_HTYPE_DESCRIBE, (dvoid*) &name, &size,
            nameAttribute, self->connection->environment->errorHandle);
    if (Environment_CheckForError(self->connection->environment, status,
            "ObjectType_Initialize(): get name") < 0)
        return -1;
    self->name = cxString_FromEncodedString(name, size,
            self->connection->environment->encoding);
    if (!self->name)
        return -1;

    // retrieve TDO of the parameter
    status = OCIAttrGet(param, OCI_HTYPE_DESCRIBE, (dvoid*) &tdoReference, 0,
            OCI_ATTR_REF_TDO, self->connection->environment->errorHandle);
    if (Environment_CheckForError(self->connection->environment, status,
            "ObjectType_Initialize(): get TDO reference") < 0)
        return -1;
    status = OCIObjectPin(self->connection->environment->handle,
            self->connection->environment->errorHandle, tdoReference, NULL,
            OCI_PIN_ANY, OCI_DURATION_SESSION, OCI_LOCK_NONE,
            (dvoid**) &self->tdo);
    if (Environment_CheckForError(self->connection->environment, status,
            "ObjectType_Initialize(): pin TDO reference") < 0)
        return -1;

    // acquire a describe handle
    status = OCIHandleAlloc(self->connection->environment->handle,
            (dvoid**) &describeHandle, OCI_HTYPE_DESCRIBE, 0, 0);
    if (Environment_CheckForError(self->connection->environment, status,
            "ObjectType_Initialize(): allocate describe handle") < 0)
        return -1;

    // describe the type
    if (ObjectType_Describe(self, describeHandle) < 0)
        return -1;

    // free the describe handle
    status = OCIHandleFree(describeHandle, OCI_HTYPE_DESCRIBE);
    if (Environment_CheckForError(self->connection->environment, status,
            "ObjectType_Initialize(): free describe handle") < 0)
        return -1;

    return 0;
}


//-----------------------------------------------------------------------------
// ObjectType_New()
//   Allocate a new object type.
//-----------------------------------------------------------------------------
static udt_ObjectType *ObjectType_New(
    udt_Connection *connection,         // connection for type information
    OCIParam *param,                    // parameter descriptor
    ub4 nameAttribute)                  // value for the name attribute
{
    udt_ObjectType *self;

    self = (udt_ObjectType*) g_ObjectTypeType.tp_alloc(&g_ObjectTypeType, 0);
    if (!self)
        return NULL;
    Py_INCREF(connection);
    self->connection = connection;
    if (ObjectType_Initialize(self, param, nameAttribute) < 0) {
        Py_DECREF(self);
        return NULL;
    }

    return self;
}


//-----------------------------------------------------------------------------
// ObjectType_NewByName()
//   Create a new object type given its name.
//-----------------------------------------------------------------------------
static udt_ObjectType *ObjectType_NewByName(
    udt_Connection *connection,         // connection for type information
    PyObject *name)                     // name of object type to describe
{
    OCIDescribe *describeHandle;
    udt_ObjectType *result;
    udt_Buffer buffer;
    OCIParam *param;
#if ORACLE_VERSION_HEX >= ORACLE_VERSION(12, 1)
    OCIType *tdo;
#endif
    sword status;

    // allocate describe handle
    status = OCIHandleAlloc(connection->environment->handle,
            (dvoid**) &describeHandle, OCI_HTYPE_DESCRIBE, 0, 0);
    if (Environment_CheckForError(connection->environment, status,
            "ObjectType_NewByName(): allocate describe handle") < 0)
        return NULL;

    // describe the object
    if (cxBuffer_FromObject(&buffer, name,
            connection->environment->encoding) < 0) {
        OCIHandleFree(describeHandle, OCI_HTYPE_DESCRIBE);
        return NULL;
    }
#if ORACLE_VERSION_HEX >= ORACLE_VERSION(12, 1)
    status = OCITypeByFullName(connection->environment->handle,
            connection->environment->errorHandle, connection->handle,
            buffer.ptr, buffer.size, NULL, 0, OCI_DURATION_SESSION,
            OCI_TYPEGET_ALL, &tdo);
    cxBuffer_Clear(&buffer);
    if (Environment_CheckForError(connection->environment, status,
            "ObjectType_NewByName(): get type by full name") < 0) {
        OCIHandleFree(describeHandle, OCI_HTYPE_DESCRIBE);
        return NULL;
    }
    status = OCIDescribeAny(connection->handle,
            connection->environment->errorHandle, (dvoid*) tdo, 0,
            OCI_OTYPE_PTR, 0, OCI_PTYPE_TYPE, describeHandle);
    if (Environment_CheckForError(connection->environment, status,
            "ObjectType_NewByName(): describe type") < 0) {
        OCIHandleFree(describeHandle, OCI_HTYPE_DESCRIBE);
        return NULL;
    }
#else
    status = OCIDescribeAny(connection->handle,
            connection->environment->errorHandle, (dvoid*) buffer.ptr,
            buffer.size, OCI_OTYPE_NAME, 0, OCI_PTYPE_TYPE, describeHandle);
    cxBuffer_Clear(&buffer);
    if (Environment_CheckForError(connection->environment, status,
            "ObjectType_NewByName(): describe type") < 0) {
        OCIHandleFree(describeHandle, OCI_HTYPE_DESCRIBE);
        return NULL;
    }
#endif

    // get the parameter handle
    status = OCIAttrGet(describeHandle, OCI_HTYPE_DESCRIBE, &param, 0,
            OCI_ATTR_PARAM, connection->environment->errorHandle);
    if (Environment_CheckForError(connection->environment, status,
            "ObjectType_NewByName(): get parameter handle") < 0) {
        OCIHandleFree(describeHandle, OCI_HTYPE_DESCRIBE);
        return NULL;
    }

    // get object type
    result = ObjectType_New(connection, param, OCI_ATTR_NAME);
    if (!result) {
        OCIHandleFree(describeHandle, OCI_HTYPE_DESCRIBE);
        return NULL;
    }

    // free the describe handle
    status = OCIHandleFree(describeHandle, OCI_HTYPE_DESCRIBE);
    if (Environment_CheckForError(connection->environment, status,
            "ObjectType_NewByName(): free describe handle") < 0)
        return NULL;

    return result;
}


//-----------------------------------------------------------------------------
// ObjectType_Free()
//   Free the memory associated with an object type.
//-----------------------------------------------------------------------------
static void ObjectType_Free(
    udt_ObjectType *self)               // object type to free
{
    if (self->tdo)
        OCIObjectUnpin(self->connection->environment->handle,
                self->connection->environment->errorHandle, self->tdo);
    Py_CLEAR(self->connection);
    Py_CLEAR(self->schema);
    Py_CLEAR(self->name);
    Py_CLEAR(self->attributes);
    Py_CLEAR(self->attributesByName);
    Py_CLEAR(self->elementType);
    Py_TYPE(self)->tp_free((PyObject*) self);
}


//-----------------------------------------------------------------------------
// ObjectType_Repr()
//   Return a string representation of the object type.
//-----------------------------------------------------------------------------
static PyObject *ObjectType_Repr(
    udt_ObjectType *self)               // object type to return the string for
{
    PyObject *module, *name, *result, *format, *formatArgs;

    if (GetModuleAndName(Py_TYPE(self), &module, &name) < 0)
        return NULL;
    format = cxString_FromAscii("<%s.%s %s.%s>");
    if (!format) {
        Py_DECREF(module);
        Py_DECREF(name);
        return NULL;
    }
    formatArgs = PyTuple_Pack(4, module, name, self->schema, self->name);
    Py_DECREF(module);
    Py_DECREF(name);
    if (!formatArgs) {
        Py_DECREF(format);
        return NULL;
    }
    result = cxString_Format(format, formatArgs);
    Py_DECREF(format);
    Py_DECREF(formatArgs);
    return result;
}


//-----------------------------------------------------------------------------
// ObjectType_NewObject()
//   Factory function for creating objects of the type which can be bound.
//-----------------------------------------------------------------------------
static PyObject *ObjectType_NewObject(
    udt_ObjectType *self,               // object type to return the string for
    PyObject *args,                     // arguments
    PyObject *keywordArgs)              // keyword arguments
{
    static char *keywordList[] = { "value", NULL };
    PyObject *initialValue;
    udt_Object *obj;

    // parse arguments
    initialValue = NULL;
    if (!PyArg_ParseTupleAndKeywords(args, keywordArgs, "|O", keywordList,
            &initialValue))
        return NULL;

    // create the object
    obj = Object_Create(self);
    if (!obj)
        return NULL;

    // populate collection, if applicable
    if (initialValue) {
        if (Object_InternalExtend(obj, initialValue) < 0) {
            Py_DECREF(obj);
            return NULL;
        }
    }

    return (PyObject*) obj;
}


//-----------------------------------------------------------------------------
// ObjectAttribute_Initialize()
//   Initialize the new object attribute.
//-----------------------------------------------------------------------------
static int ObjectAttribute_Initialize(
    udt_ObjectAttribute *self,          // object attribute to initialize
    udt_Connection *connection,         // connection in use
    OCIParam *param)                    // parameter descriptor
{
    sword status;
    char *name;
    ub4 size;

    // determine the name of the attribute
    status = OCIAttrGet(param, OCI_DTYPE_PARAM, (dvoid*) &name, &size,
            OCI_ATTR_NAME, connection->environment->errorHandle);
    if (Environment_CheckForError(connection->environment, status,
            "ObjectAttribute_Initialize(): get name") < 0)
        return -1;
    self->name = cxString_FromEncodedString(name, size,
            connection->environment->encoding);
    if (!self->name)
        return -1;

    // determine the type of the attribute
    status = OCIAttrGet(param, OCI_DTYPE_PARAM, (dvoid*) &self->typeCode, 0,
            OCI_ATTR_TYPECODE, connection->environment->errorHandle);
    if (Environment_CheckForError(connection->environment, status,
            "ObjectAttribute_Initialize(): get type code") < 0)
        return -1;

    // if the type of the attribute is object, recurse
    switch (self->typeCode) {
        case OCI_TYPECODE_NAMEDCOLLECTION:
        case OCI_TYPECODE_OBJECT:
            self->subType = ObjectType_New(connection, param,
                    OCI_ATTR_TYPE_NAME);
            if (!self->subType)
                return -1;
            break;
    };

    return 0;
}


//-----------------------------------------------------------------------------
// ObjectAttribute_New()
//   Allocate a new object attribute.
//-----------------------------------------------------------------------------
static udt_ObjectAttribute *ObjectAttribute_New(
    udt_Connection *connection,         // connection information
    OCIParam *param)                    // parameter descriptor
{
    udt_ObjectAttribute *self;

    self = (udt_ObjectAttribute*)
            g_ObjectAttributeType.tp_alloc(&g_ObjectAttributeType, 0);
    if (!self)
        return NULL;
    if (ObjectAttribute_Initialize(self, connection, param) < 0) {
        Py_DECREF(self);
        return NULL;
    }

    return self;
}


//-----------------------------------------------------------------------------
// ObjectAttribute_Free()
//   Free the memory associated with an object attribute.
//-----------------------------------------------------------------------------
static void ObjectAttribute_Free(
    udt_ObjectAttribute *self)          // object attribute to free
{
    Py_CLEAR(self->name);
    Py_CLEAR(self->subType);
    Py_TYPE(self)->tp_free((PyObject*) self);
}


//-----------------------------------------------------------------------------
// ObjectAttribute_Repr()
//   Return a string representation of the object attribute.
//-----------------------------------------------------------------------------
static PyObject *ObjectAttribute_Repr(
    udt_ObjectAttribute *self)          // attribute to return the string for
{
    PyObject *module, *name, *result, *format, *formatArgs;

    if (GetModuleAndName(Py_TYPE(self), &module, &name) < 0)
        return NULL;
    format = cxString_FromAscii("<%s.%s %s>");
    if (!format) {
        Py_DECREF(module);
        Py_DECREF(name);
        return NULL;
    }
    formatArgs = PyTuple_Pack(3, module, name, self->name);
    Py_DECREF(module);
    Py_DECREF(name);
    if (!formatArgs) {
        Py_DECREF(format);
        return NULL;
    }
    result = cxString_Format(format, formatArgs);
    Py_DECREF(format);
    Py_DECREF(formatArgs);
    return result;
}

