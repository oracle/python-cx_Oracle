//-----------------------------------------------------------------------------
// Copyright 2016, 2017, Oracle and/or its affiliates. All rights reserved.
//
// Portions Copyright 2007-2015, Anthony Tuininga. All rights reserved.
//
// Portions Copyright 2001-2007, Computronix (Canada) Ltd., Edmonton, Alberta,
// Canada. All rights reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// CursorVar.c
//   Defines the routines specific to the cursor type.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Declaration of cursor variable functions.
//-----------------------------------------------------------------------------
static int CursorVar_SetValue(udt_Variable*, uint32_t, dpiData*, PyObject*);
static PyObject *CursorVar_GetValue(udt_Variable*, dpiData*);


//-----------------------------------------------------------------------------
// Python type declarations
//-----------------------------------------------------------------------------
DECLARE_VARIABLE_TYPE(g_CursorVarType, CURSOR)


//-----------------------------------------------------------------------------
// variable type declarations
//-----------------------------------------------------------------------------
static udt_VariableType vt_Cursor = {
    (SetValueProc) CursorVar_SetValue,
    (GetValueProc) CursorVar_GetValue,
    &g_CursorVarType,                   // Python type
    DPI_ORACLE_TYPE_STMT,               // Oracle type
    DPI_NATIVE_TYPE_STMT,               // native type
    0                                   // element length
};


//-----------------------------------------------------------------------------
// CursorVar_SetValue()
//   Set the value of the variable.
//-----------------------------------------------------------------------------
static int CursorVar_SetValue(udt_Variable *var, uint32_t pos, dpiData *data,
        PyObject *value)
{
    udt_Cursor *cursor;

    if (!PyObject_IsInstance(value, (PyObject*) &g_CursorType)) {
        PyErr_SetString(PyExc_TypeError, "expecting cursor");
        return -1;
    }
    cursor = (udt_Cursor *) value;
    if (cursor->handle) {
        if (dpiVar_setFromStmt(var->handle, pos, cursor->handle) < 0)
            return Error_RaiseAndReturnInt();
    } else {
        cursor->handle = data->value.asStmt;
        dpiStmt_addRef(cursor->handle);
    }
    cursor->fixupRefCursor = 1;
    return 0;
}


//-----------------------------------------------------------------------------
// CursorVar_GetValue()
//   Set the value of the variable.
//-----------------------------------------------------------------------------
static PyObject *CursorVar_GetValue(udt_Variable *var, dpiData *data)
{
    udt_Cursor *cursor;

    cursor = (udt_Cursor*) PyObject_CallMethod((PyObject*) var->connection,
            "cursor", NULL);
    if (!cursor)
        return NULL;
    cursor->handle = data->value.asStmt;
    dpiStmt_addRef(cursor->handle);
    cursor->fixupRefCursor = 1;
    return (PyObject*) cursor;
}

