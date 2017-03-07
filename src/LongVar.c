//-----------------------------------------------------------------------------
// Copyright 2016, 2017, Oracle and/or its affiliates. All rights reserved.
//
// Portions Copyright 2007-2015, Anthony Tuininga. All rights reserved.
//
// Portions Copyright 2001-2007, Computronix (Canada) Ltd., Edmonton, Alberta,
// Canada. All rights reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// LongVar.c
//   Defines the routines specific to the long type.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// declaration of long variable functions.
//-----------------------------------------------------------------------------
static PyObject *LongVar_GetValue(udt_Variable*, dpiData*);


//-----------------------------------------------------------------------------
// Python type declarations
//-----------------------------------------------------------------------------
DECLARE_VARIABLE_TYPE(g_LongStringVarType, LONG_STRING)
DECLARE_VARIABLE_TYPE(g_LongBinaryVarType, LONG_BINARY)


//-----------------------------------------------------------------------------
// variable type declarations
//-----------------------------------------------------------------------------
static udt_VariableType vt_LongString = {
    (SetValueProc) Variable_SetValueBytes,
    (GetValueProc) LongVar_GetValue,
    &g_LongStringVarType,               // Python type
    DPI_ORACLE_TYPE_LONG_VARCHAR,       // Oracle type
    DPI_NATIVE_TYPE_BYTES,              // native type
    128 * 1024                          // element length (default)
};


static udt_VariableType vt_LongBinary = {
    (SetValueProc) Variable_SetValueBytes,
    (GetValueProc) LongVar_GetValue,
    &g_LongBinaryVarType,               // Python type
    DPI_ORACLE_TYPE_LONG_RAW,           // Oracle type
    DPI_NATIVE_TYPE_BYTES,              // native type
    128 * 1024                          // element length (default)
};


//-----------------------------------------------------------------------------
// LongVar_GetValue()
//   Returns the value stored at the given array position.
//-----------------------------------------------------------------------------
static PyObject *LongVar_GetValue(udt_Variable *var, dpiData *data)
{
    dpiBytes *bytes;

    bytes = &data->value.asBytes;
    if (var->type == &vt_LongBinary)
        return PyBytes_FromStringAndSize(bytes->ptr, bytes->length);
    return cxString_FromEncodedString(bytes->ptr, bytes->length,
            bytes->encoding);
}

