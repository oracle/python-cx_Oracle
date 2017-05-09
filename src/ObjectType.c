//-----------------------------------------------------------------------------
// Copyright 2016, 2017, Oracle and/or its affiliates. All rights reserved.
//
// Portions Copyright 2007-2015, Anthony Tuininga. All rights reserved.
//
// Portions Copyright 2001-2007, Computronix (Canada) Ltd., Edmonton, Alberta,
// Canada. All rights reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ObjectType.c
//   Defines the routines for handling Oracle type information.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// structures used for handling object types
//-----------------------------------------------------------------------------
typedef struct {
    PyObject_HEAD
    dpiObjectType *handle;
    PyObject *schema;
    PyObject *name;
    PyObject *attributes;
    PyObject *attributesByName;
    udt_Connection *connection;
    dpiOracleTypeNum elementOracleTypeNum;
    dpiNativeTypeNum elementNativeTypeNum;
    PyObject *elementType;
    char isCollection;
} udt_ObjectType;

typedef struct {
    PyObject_HEAD
    PyObject *name;
    dpiObjectAttr *handle;
    dpiOracleTypeNum oracleTypeNum;
    dpiNativeTypeNum nativeTypeNum;
    udt_ObjectType *type;
} udt_ObjectAttribute;

//-----------------------------------------------------------------------------
// Declaration of functions
//-----------------------------------------------------------------------------
static udt_ObjectType *ObjectType_New(udt_Connection*, dpiObjectType*);
static void ObjectType_Free(udt_ObjectType*);
static PyObject *ObjectType_Repr(udt_ObjectType*);
static PyObject *ObjectType_NewObject(udt_ObjectType*, PyObject*, PyObject*);
static udt_ObjectAttribute *ObjectAttribute_New(udt_Connection*,
        dpiObjectAttr*);
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
// ObjectType_Initialize()
//   Initialize the object type with the information that is required.
//-----------------------------------------------------------------------------
static int ObjectType_Initialize(udt_ObjectType *self,
        udt_Connection *connection)
{
    dpiObjectAttr **attributes;
    udt_ObjectAttribute *attr;
    dpiObjectTypeInfo info;
    uint16_t i;

    // get object type information
    if (dpiObjectType_getInfo(self->handle, &info) < 0)
        return Error_RaiseAndReturnInt();
    Py_INCREF(connection);
    self->connection = connection;
    self->schema = cxString_FromEncodedString(info.schema, info.schemaLength,
            connection->encodingInfo.encoding);
    if (!self->schema)
        return -1;
    self->name = cxString_FromEncodedString(info.name, info.nameLength,
            connection->encodingInfo.encoding);
    if (!self->name)
        return -1;
    self->isCollection = info.isCollection;
    self->elementOracleTypeNum = info.elementOracleTypeNum;
    self->elementNativeTypeNum = info.elementDefaultNativeTypeNum;
    if (info.elementObjectType) {
        self->elementType = (PyObject*) ObjectType_New(connection,
                info.elementObjectType);
        if (!self->elementType)
            return -1;
    }

    // allocate the attribute list (temporary and permanent) and dictionary
    self->attributes = PyList_New(info.numAttributes);
    if (!self->attributes)
        return -1;
    self->attributesByName = PyDict_New();
    if (!self->attributesByName)
        return -1;

    // get the list of attributes from DPI
    attributes = PyMem_Malloc(sizeof(dpiObjectAttr*) * info.numAttributes);
    if (!attributes) {
        PyErr_NoMemory();
        return -1;
    }
    if (dpiObjectType_getAttributes(self->handle, info.numAttributes,
            attributes) < 0) {
        PyMem_Free(attributes);
        return Error_RaiseAndReturnInt();
    }

    // create attribute information for each attribute
    for (i = 0; i < info.numAttributes; i++) {
        attr = ObjectAttribute_New(connection, attributes[i]);
        if (!attr) {
            PyMem_Free(attributes);
            return -1;
        }
        PyList_SET_ITEM(self->attributes, i, (PyObject*) attr);
        if (PyDict_SetItem(self->attributesByName, attr->name,
                (PyObject*) attr) < 0)
            return -1;
    }
    PyMem_Free(attributes);
    return 0;
}


//-----------------------------------------------------------------------------
// ObjectType_New()
//   Allocate a new object type.
//-----------------------------------------------------------------------------
static udt_ObjectType *ObjectType_New(udt_Connection *connection,
        dpiObjectType *handle)
{
    udt_ObjectType *self;

    self = (udt_ObjectType*) g_ObjectTypeType.tp_alloc(&g_ObjectTypeType, 0);
    if (!self)
        return NULL;
    if (dpiObjectType_addRef(handle) < 0) {
        Py_DECREF(self);
        Error_RaiseAndReturnNull();
        return NULL;
    }
    self->handle = handle;
    if (ObjectType_Initialize(self, connection) < 0) {
        Py_DECREF(self);
        return NULL;
    }

    return self;
}


//-----------------------------------------------------------------------------
// ObjectType_NewByName()
//   Create a new object type given its name.
//-----------------------------------------------------------------------------
static udt_ObjectType *ObjectType_NewByName(udt_Connection *connection,
        PyObject *name)
{
    udt_ObjectType *objType;
    dpiObjectType *handle;
    udt_Buffer buffer;
    int status;

    if (cxBuffer_FromObject(&buffer, name,
            connection->encodingInfo.encoding) < 0)
        return NULL;
    status = dpiConn_getObjectType(connection->handle, buffer.ptr,
            buffer.size, &handle);
    cxBuffer_Clear(&buffer);
    if (status < 0)
        return (udt_ObjectType*) Error_RaiseAndReturnNull();
    objType = ObjectType_New(connection, handle);
    dpiObjectType_release(handle);
    return objType;
}


//-----------------------------------------------------------------------------
// ObjectType_Free()
//   Free the memory associated with an object type.
//-----------------------------------------------------------------------------
static void ObjectType_Free(udt_ObjectType *self)
{
    if (self->handle) {
        dpiObjectType_release(self->handle);
        self->handle = NULL;
    }
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
static PyObject *ObjectType_Repr(udt_ObjectType *self)
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
static PyObject *ObjectType_NewObject(udt_ObjectType *self, PyObject *args,
        PyObject *keywordArgs)
{
    static char *keywordList[] = { "value", NULL };
    PyObject *initialValue;
    dpiObject *handle;
    udt_Object *obj;

    // parse arguments
    initialValue = NULL;
    if (!PyArg_ParseTupleAndKeywords(args, keywordArgs, "|O", keywordList,
            &initialValue))
        return NULL;

    // get handle to newly created object
    if (dpiObjectType_createObject(self->handle, &handle) < 0)
        return Error_RaiseAndReturnNull();

    // create the object
    obj = (udt_Object*) Object_New(self, handle, 0);
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
static int ObjectAttribute_Initialize(udt_ObjectAttribute *self,
        udt_Connection *connection)
{
    dpiObjectAttrInfo info;

    if (dpiObjectAttr_getInfo(self->handle, &info) < 0)
        return Error_RaiseAndReturnInt();
    self->oracleTypeNum = info.oracleTypeNum;
    self->nativeTypeNum = info.defaultNativeTypeNum;
    self->name = cxString_FromEncodedString(info.name, info.nameLength,
            connection->encodingInfo.encoding);
    if (!self->name)
        return -1;
    if (info.objectType) {
        self->type = ObjectType_New(connection, info.objectType);
        if (!self->type)
            return -1;
    }

    return 0;
}


//-----------------------------------------------------------------------------
// ObjectAttribute_New()
//   Allocate a new object attribute.
//-----------------------------------------------------------------------------
static udt_ObjectAttribute *ObjectAttribute_New(udt_Connection *connection,
        dpiObjectAttr *handle)
{
    udt_ObjectAttribute *self;

    self = (udt_ObjectAttribute*)
            g_ObjectAttributeType.tp_alloc(&g_ObjectAttributeType, 0);
    if (!self) {
        dpiObjectAttr_release(handle);
        return NULL;
    }
    self->handle = handle;
    if (ObjectAttribute_Initialize(self, connection) < 0) {
        Py_DECREF(self);
        return NULL;
    }

    return self;
}


//-----------------------------------------------------------------------------
// ObjectAttribute_Free()
//   Free the memory associated with an object attribute.
//-----------------------------------------------------------------------------
static void ObjectAttribute_Free(udt_ObjectAttribute *self)
{
    if (self->handle) {
        dpiObjectAttr_release(self->handle);
        self->handle = NULL;
    }
    Py_CLEAR(self->name);
    Py_CLEAR(self->type);
    Py_TYPE(self)->tp_free((PyObject*) self);
}


//-----------------------------------------------------------------------------
// ObjectAttribute_Repr()
//   Return a string representation of the object attribute.
//-----------------------------------------------------------------------------
static PyObject *ObjectAttribute_Repr(udt_ObjectAttribute *self)
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

