//-----------------------------------------------------------------------------
// Copyright 2016, 2017, Oracle and/or its affiliates. All rights reserved.
//
// Portions Copyright 2007-2015, Anthony Tuininga. All rights reserved.
//
// Portions Copyright 2001-2007, Computronix (Canada) Ltd., Edmonton, Alberta,
// Canada. All rights reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ObjectVar.c
//   Defines the routines for handling Oracle object variables.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Declaration of object variable functions.
//-----------------------------------------------------------------------------
static int ObjectVar_SetValue(udt_Variable*, uint32_t, dpiData*, PyObject*);
static PyObject *ObjectVar_GetValue(udt_Variable*, dpiData*);


//-----------------------------------------------------------------------------
// Python type declarations
//-----------------------------------------------------------------------------
DECLARE_VARIABLE_TYPE(g_ObjectVarType, OBJECT)


//-----------------------------------------------------------------------------
// variable type declarations
//-----------------------------------------------------------------------------
static udt_VariableType vt_Object = {
    (SetValueProc) ObjectVar_SetValue,
    (GetValueProc) ObjectVar_GetValue,
    &g_ObjectVarType,                   // Python type
    DPI_ORACLE_TYPE_OBJECT,             // Oracle type
    DPI_NATIVE_TYPE_OBJECT,             // native type
    0                                   // element length
};


//-----------------------------------------------------------------------------
// ObjectVar_SetValue()
//   Set the value of the variable. Only cx_Oracle.Object values are permitted.
//-----------------------------------------------------------------------------
static int ObjectVar_SetValue(udt_Variable *self, uint32_t pos, dpiData *data,
        PyObject *value)
{
    udt_Object *obj;

    if (Py_TYPE(value) != &g_ObjectType) {
        PyErr_SetString(PyExc_TypeError, "expecting cx_Oracle.Object");
        return -1;
    }
    obj = (udt_Object*) value;
    if (dpiVar_setFromObject(self->handle, pos, obj->handle) < 0)
        return Error_RaiseAndReturnInt();
    return 0;
}


//-----------------------------------------------------------------------------
// ObjectVar_GetValue()
//   Returns the value stored at the given array position.
//-----------------------------------------------------------------------------
static PyObject *ObjectVar_GetValue(udt_Variable *self, dpiData *data)
{
    return Object_New(self->objectType, data->value.asObject, 1);
}

