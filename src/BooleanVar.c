//-----------------------------------------------------------------------------
// Copyright 2016, 2017, Oracle and/or its affiliates. All rights reserved.
//
// Portions Copyright 2007-2015, Anthony Tuininga. All rights reserved.
//
// Portions Copyright 2001-2007, Computronix (Canada) Ltd., Edmonton, Alberta,
// Canada. All rights reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// BooleanVar.c
//   Defines the routines for handling boolean variables (only available after
// Oracle 12.1).
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Declaration of variable functions.
//-----------------------------------------------------------------------------
static int BooleanVar_SetValue(udt_Variable*, uint32_t, dpiData*, PyObject*);
static PyObject *BooleanVar_GetValue(udt_Variable*, dpiData*);


//-----------------------------------------------------------------------------
// Python type declaration
//-----------------------------------------------------------------------------
DECLARE_VARIABLE_TYPE(g_BooleanVarType, BOOLEAN)


//-----------------------------------------------------------------------------
// variable type declarations
//-----------------------------------------------------------------------------
static udt_VariableType vt_Boolean = {
    (SetValueProc) BooleanVar_SetValue,
    (GetValueProc) BooleanVar_GetValue,
    &g_BooleanVarType,                  // Python type
    DPI_ORACLE_TYPE_BOOLEAN,            // Oracle type
    DPI_NATIVE_TYPE_BOOLEAN,            // native type
    0                                   // element length
};


//-----------------------------------------------------------------------------
// BooleanVar_GetValue()
//   Returns the value stored at the given array position.
//-----------------------------------------------------------------------------
static PyObject *BooleanVar_GetValue(udt_Variable *var, dpiData *data)
{
    if (data->value.asBoolean)
        Py_RETURN_TRUE;
    Py_RETURN_FALSE;
}


//-----------------------------------------------------------------------------
// BooleanVar_SetValue()
//   Set the value of the variable at the given array position.
//-----------------------------------------------------------------------------
static int BooleanVar_SetValue(udt_Variable *var, uint32_t pos, dpiData *data,
        PyObject *value)
{
    data->value.asBoolean = (value == Py_True);
    return 0;
}

