//-----------------------------------------------------------------------------
// Copyright 2016, 2017, Oracle and/or its affiliates. All rights reserved.
//
// Portions Copyright 2007-2015, Anthony Tuininga. All rights reserved.
//
// Portions Copyright 2001-2007, Computronix (Canada) Ltd., Edmonton, Alberta,
// Canada. All rights reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// LobVar.c
//   Defines the routines for handling LOB variables.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Declaration of LOB variable functions.
//-----------------------------------------------------------------------------
static int LobVar_SetValue(udt_Variable*, uint32_t, dpiData*, PyObject*);
static PyObject *LobVar_GetValue(udt_Variable*, dpiData*);

//-----------------------------------------------------------------------------
// Python type declarations
//-----------------------------------------------------------------------------
DECLARE_VARIABLE_TYPE(g_CLOBVarType, CLOB)
DECLARE_VARIABLE_TYPE(g_NCLOBVarType, NCLOB)
DECLARE_VARIABLE_TYPE(g_BLOBVarType, BLOB)
DECLARE_VARIABLE_TYPE(g_BFILEVarType, BFILE)


//-----------------------------------------------------------------------------
// variable type declarations
//-----------------------------------------------------------------------------
static udt_VariableType vt_CLOB = {
    (SetValueProc) LobVar_SetValue,
    (GetValueProc) LobVar_GetValue,
    &g_CLOBVarType,                     // Python type
    DPI_ORACLE_TYPE_CLOB,               // Oracle type
    DPI_NATIVE_TYPE_LOB,                // native type
    0                                   // element length
};


static udt_VariableType vt_NCLOB = {
    (SetValueProc) LobVar_SetValue,
    (GetValueProc) LobVar_GetValue,
    &g_NCLOBVarType,                    // Python type
    DPI_ORACLE_TYPE_NCLOB,              // Oracle type
    DPI_NATIVE_TYPE_LOB,                // native type
    0                                   // element length
};


static udt_VariableType vt_BLOB = {
    (SetValueProc) LobVar_SetValue,
    (GetValueProc) LobVar_GetValue,
    &g_BLOBVarType,                     // Python type
    DPI_ORACLE_TYPE_BLOB,               // Oracle type
    DPI_NATIVE_TYPE_LOB,                // native type
    0                                   // element length
};


static udt_VariableType vt_BFILE = {
    (SetValueProc) LobVar_SetValue,
    (GetValueProc) LobVar_GetValue,
    &g_BFILEVarType,                    // Python type
    DPI_ORACLE_TYPE_BFILE,              // Oracle type
    DPI_NATIVE_TYPE_LOB,                // native type
    0                                   // element length
};


//-----------------------------------------------------------------------------
// LobVar_SetValue()
//   Set the value of the variable.
//-----------------------------------------------------------------------------
static int LobVar_SetValue(udt_Variable *var, uint32_t pos, dpiData *data,
        PyObject *value)
{
    const char *encoding;
    udt_Buffer buffer;
    int status;

    if (var->type == &vt_NCLOB)
        encoding = var->connection->encodingInfo.nencoding;
    else encoding = var->connection->encodingInfo.encoding;
    if (cxBuffer_FromObject(&buffer, value, encoding) < 0)
        return -1;
    Py_BEGIN_ALLOW_THREADS
    status = dpiLob_setFromBytes(data->value.asLOB, buffer.ptr, buffer.size);
    Py_END_ALLOW_THREADS
    cxBuffer_Clear(&buffer);
    if (status < 0)
        return Error_RaiseAndReturnInt();

    return 0;
}


//-----------------------------------------------------------------------------
// LobVar_GetValue()
//   Returns the value stored at the given array position.
//-----------------------------------------------------------------------------
static PyObject *LobVar_GetValue(udt_Variable *var, dpiData *data)
{
    return LOB_New(var->connection, var->type->oracleTypeNum,
            data->value.asLOB);
}

