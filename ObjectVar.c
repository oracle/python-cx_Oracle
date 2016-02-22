//-----------------------------------------------------------------------------
// ObjectVar.c
//   Defines the routines for handling Oracle object variables.
//-----------------------------------------------------------------------------

#include "ObjectType.c"

//-----------------------------------------------------------------------------
// Object type
//-----------------------------------------------------------------------------
typedef struct {
    Variable_HEAD
    dvoid **data;
    dvoid **objectIndicator;
    PyObject **objects;
    udt_Connection *connection;
    udt_ObjectType *objectType;
} udt_ObjectVar;

//-----------------------------------------------------------------------------
// Declaration of object variable functions.
//-----------------------------------------------------------------------------
static int ObjectVar_Initialize(udt_ObjectVar*, udt_Cursor*);
static void ObjectVar_Finalize(udt_ObjectVar*);
static int ObjectVar_SetValue(udt_ObjectVar*, unsigned, PyObject*);
static PyObject *ObjectVar_GetValue(udt_ObjectVar*, unsigned);
static int ObjectVar_PreDefine(udt_ObjectVar*, OCIParam*);
static int ObjectVar_PostDefine(udt_ObjectVar*);
static int ObjectVar_PostBind(udt_ObjectVar*);
static int ObjectVar_PreFetch(udt_ObjectVar*);
static int ObjectVar_IsNull(udt_ObjectVar*, unsigned);
static int ObjectVar_SetType(udt_ObjectVar*, PyObject*);

//-----------------------------------------------------------------------------
// declaration of members for Oracle objects
//-----------------------------------------------------------------------------
static PyMemberDef g_ObjectVarMembers[] = {
    { "type", T_OBJECT, offsetof(udt_ObjectVar, objectType), READONLY },
    { NULL }
};


//-----------------------------------------------------------------------------
// Python type declarations
//-----------------------------------------------------------------------------
static PyTypeObject g_ObjectVarType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "cx_Oracle.OBJECT",                 // tp_name
    sizeof(udt_ObjectVar),              // tp_basicsize
    0,                                  // tp_itemsize
    0,                                  // tp_dealloc
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
    g_ObjectVarMembers                  // tp_members
};


//-----------------------------------------------------------------------------
// variable type declarations
//-----------------------------------------------------------------------------
static udt_VariableType vt_Object = {
    (InitializeProc) ObjectVar_Initialize,
    (FinalizeProc) ObjectVar_Finalize,
    (PreDefineProc) ObjectVar_PreDefine,
    (PostDefineProc) ObjectVar_PostDefine,
    (PostBindProc) ObjectVar_PostBind,
    (PreFetchProc) ObjectVar_PreFetch,
    (IsNullProc) ObjectVar_IsNull,
    (SetValueProc) ObjectVar_SetValue,
    (GetValueProc) ObjectVar_GetValue,
    (GetBufferSizeProc) NULL,
    &g_ObjectVarType,                   // Python type
    SQLT_NTY,                           // Oracle type
    SQLCS_IMPLICIT,                     // charset form
    sizeof(dvoid*),                     // element length
    0,                                  // is character data
    0,                                  // is variable length
    0,                                  // can be copied
    0                                   // can be in array
};


//-----------------------------------------------------------------------------
// ObjectVar_Initialize()
//   Initialize the variable.
//-----------------------------------------------------------------------------
static int ObjectVar_Initialize(
    udt_ObjectVar *self,                // variable to initialize
    udt_Cursor *cursor)                 // cursor to use
{
    ub4 i;

    Py_INCREF(cursor->connection);
    self->connection = cursor->connection;
    self->objectType = NULL;
    self->objectIndicator =
            PyMem_Malloc(self->allocatedElements * sizeof(dvoid*));
    if (!self->objectIndicator) {
        PyErr_NoMemory();
        return -1;
    }
    self->objects = PyMem_Malloc(self->allocatedElements * sizeof(PyObject*));
    if (!self->objects) {
        PyErr_NoMemory();
        return -1;
    }

    for (i = 0; i < self->allocatedElements; i++) {
        self->data[i] = NULL;
        self->objectIndicator[i] = NULL;
        self->objects[i] = NULL;
    }
    return 0;
}


//-----------------------------------------------------------------------------
// ObjectVar_Finalize()
//   Prepare for variable destruction.
//-----------------------------------------------------------------------------
static void ObjectVar_Finalize(
    udt_ObjectVar *self)                // variable to free
{
    ub4 i;

    for (i = 0; i < self->allocatedElements; i++) {
        if (self->objects[i])
            Py_CLEAR(self->objects[i]);
        else if (self->data[i])
            OCIObjectFree(self->environment->handle,
                    self->environment->errorHandle, self->data[i],
                    OCI_DEFAULT);
    }
    Py_CLEAR(self->connection);
    Py_CLEAR(self->objectType);
    if (self->objectIndicator)
        PyMem_Free(self->objectIndicator);
    if (self->objects)
        PyMem_Free(self->objects);
}


//-----------------------------------------------------------------------------
// ObjectVar_PreDefine()
//   Performs additional steps required for defining objects.
//-----------------------------------------------------------------------------
static int ObjectVar_PreDefine(
    udt_ObjectVar *self,                // variable to set up
    OCIParam *param)                    // parameter being defined
{
    self->objectType = ObjectType_New(self->connection, param,
            OCI_ATTR_TYPE_NAME);
    if (!self->objectType)
        return -1;
    return 0;
}


//-----------------------------------------------------------------------------
// ObjectVar_PostDefine()
//   Performs additional steps required for defining objects.
//-----------------------------------------------------------------------------
static int ObjectVar_PostDefine(
    udt_ObjectVar *self)                // variable to set up
{
    sword status;

    status = OCIDefineObject(self->defineHandle,
            self->environment->errorHandle, self->objectType->tdo, self->data,
            0, self->objectIndicator, 0);
    return Environment_CheckForError(self->environment, status,
            "ObjectVar_PostDefine(): define object");
}


//-----------------------------------------------------------------------------
// ObjectVar_PostBind()
//   Performs additional steps required for binding objects.
//-----------------------------------------------------------------------------
static int ObjectVar_PostBind(
    udt_ObjectVar *self)                // variable to set up
{
    sword status;

    if (!self->objectType) {
        PyErr_SetString(g_InterfaceErrorException,
                "object type not associated with bind variable");
        return -1;
    }
    status = OCIBindObject(self->bindHandle, self->environment->errorHandle,
            self->objectType->tdo, self->data, 0, self->objectIndicator, 0);
    return Environment_CheckForError(self->environment, status,
            "ObjectVar_PostBind(): bind object");
}


//-----------------------------------------------------------------------------
// ObjectVar_PreFetch()
//   Free objects prior to next fetch.
//-----------------------------------------------------------------------------
static int ObjectVar_PreFetch(
    udt_ObjectVar *var)                 // variable to free
{
    ub4 i;

    for (i = 0; i < var->allocatedElements; i++) {
        Py_CLEAR(var->objects[i]);
        if (var->data[i])
            OCIObjectFree(var->environment->handle,
                    var->environment->errorHandle, var->data[i], OCI_DEFAULT);
    }

    return 0;
}


//-----------------------------------------------------------------------------
// ObjectVar_IsNull()
//   Returns a boolean indicating if the variable is null or not.
//-----------------------------------------------------------------------------
static int ObjectVar_IsNull(
    udt_ObjectVar *self,                // variable to set up
    unsigned pos)                       // position to check
{
    if (!self->objectIndicator[pos])
        return 1;
    return (*((OCIInd*) self->objectIndicator[pos]) == OCI_IND_NULL);
}


//-----------------------------------------------------------------------------
// ObjectVar_SetValue()
//   Set the value of the variable.
//-----------------------------------------------------------------------------
static int ObjectVar_SetValue(
    udt_ObjectVar *self,                // variable to determine value for
    unsigned pos,                       // array position
    PyObject *value)                    // value to set
{
    udt_Object *object;

    // only cx_Oracle.Object values are permitted and the types must match
    // if the variable doesn't have a type yet, assign it
    if (Py_TYPE(value) != &g_ObjectType) {
        PyErr_SetString(PyExc_TypeError, "expecting cx_Oracle.Object");
        return -1;
    }
    object = (udt_Object*) value;
    if (!self->objectType) {
        Py_INCREF(object->objectType);
        self->objectType = object->objectType;
    } else if (object->objectType->tdo != self->objectType->tdo) {
        PyErr_SetString(PyExc_TypeError,
                "expecting same type as the variable itself");
        return -1;
    }

    // eliminate prior value, if needed
    if (self->objects[pos])
        Py_CLEAR(self->objects[pos]);
    else OCIObjectFree(self->environment->handle,
            self->environment->errorHandle, self->data[pos], OCI_DEFAULT);

    // set new value
    Py_INCREF(value);
    self->objects[pos] = value;
    self->data[pos] = object->instance;
    self->objectIndicator[pos] = object->indicator;
    return 0;
}

//-----------------------------------------------------------------------------
// ObjectVar_GetValue()
//   Returns the value stored at the given array position.
//-----------------------------------------------------------------------------
static PyObject *ObjectVar_GetValue(
    udt_ObjectVar *self,                // variable to determine value for
    unsigned pos)                       // array position
{
    PyObject *obj;

    // create the object, if needed; for collections, return a list, not the
    // object itself
    if (!self->objects[pos]) {
        obj = Object_New(self->objectType, self->data[pos],
                self->objectIndicator[pos], 1);
        if (!obj)
            return NULL;
        self->objects[pos] = obj;
    }

    Py_INCREF(self->objects[pos]);
    return self->objects[pos];
}


//-----------------------------------------------------------------------------
// ObjectVar_SetType()
//   Internal method used to set the type when creating an object variable.
// This will also create the object instances.
//-----------------------------------------------------------------------------
static int ObjectVar_SetType(
    udt_ObjectVar *self,                // variable to initialize the type
    PyObject *typeNameObj)              // value to set
{
    dvoid *instance, *indicator;
    sword status;
    ub4 i;

    // get the object type from the name
    self->objectType = ObjectType_NewByName(self->connection, typeNameObj);
    if (!self->objectType)
        return -1;

    // initialize the object instances
    for (i = 0; i < self->allocatedElements; i++) {

        // create the object instance
        status = OCIObjectNew(self->connection->environment->handle,
                self->connection->environment->errorHandle,
                self->connection->handle, self->objectType->typeCode,
                self->objectType->tdo, NULL, OCI_DURATION_SESSION, TRUE,
                &instance);
        if (Environment_CheckForError(self->connection->environment, status,
                "ObjectVar_SetType(): create object instance") < 0)
            return -1;
        self->data[i] = instance;

        // get the null indicator structure
        status = OCIObjectGetInd(self->connection->environment->handle,
                self->connection->environment->errorHandle, instance,
                &indicator);
        if (Environment_CheckForError(self->connection->environment, status,
                "ObjectVar_SetType(): get indicator structure") < 0)
            return -1;
        *((OCIInd*) indicator) = OCI_IND_NULL;
        self->objectIndicator[i] = indicator;
    }

    return 0;
}

