//-----------------------------------------------------------------------------
// Copyright 2016, 2017, Oracle and/or its affiliates. All rights reserved.
//
// Portions Copyright 2007-2015, Anthony Tuininga. All rights reserved.
//
// Portions Copyright 2001-2007, Computronix (Canada) Ltd., Edmonton, Alberta,
// Canada. All rights reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// StringVar.c
//   Defines the routines specific to the string type.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Declaration of string variable functions.
//-----------------------------------------------------------------------------
static PyObject *RowidVar_GetValue(udt_Variable*, dpiData*);
static PyObject *StringVar_GetValue(udt_Variable*, dpiData*);
static int RowidVar_SetValue(udt_Variable*, uint32_t, dpiData*, PyObject*);

//-----------------------------------------------------------------------------
// Python type declarations
//-----------------------------------------------------------------------------
DECLARE_VARIABLE_TYPE(g_StringVarType, STRING)
DECLARE_VARIABLE_TYPE(g_NCharVarType, NCHAR)
DECLARE_VARIABLE_TYPE(g_FixedCharVarType, FIXED_CHAR)
DECLARE_VARIABLE_TYPE(g_FixedNCharVarType, FIXED_NCHAR)
DECLARE_VARIABLE_TYPE(g_RowidVarType, ROWID)
DECLARE_VARIABLE_TYPE(g_BinaryVarType, BINARY)


//-----------------------------------------------------------------------------
// variable type declarations
//-----------------------------------------------------------------------------
static udt_VariableType vt_String = {
    (SetValueProc) Variable_SetValueBytes,
    (GetValueProc) StringVar_GetValue,
    &g_StringVarType,                   // Python type
    DPI_ORACLE_TYPE_VARCHAR,            // Oracle type
    DPI_NATIVE_TYPE_BYTES,              // native type
    4000                                // element length (default)
};


static udt_VariableType vt_NationalCharString = {
    (SetValueProc) Variable_SetValueBytes,
    (GetValueProc) StringVar_GetValue,
    &g_NCharVarType,                    // Python type
    DPI_ORACLE_TYPE_NVARCHAR,           // Oracle type
    DPI_NATIVE_TYPE_BYTES,              // native type
    4000                                // element length (default)
};


static udt_VariableType vt_FixedChar = {
    (SetValueProc) Variable_SetValueBytes,
    (GetValueProc) StringVar_GetValue,
    &g_FixedCharVarType,                // Python type
    DPI_ORACLE_TYPE_CHAR,               // Oracle type
    DPI_NATIVE_TYPE_BYTES,              // native type
    2000                                // element length (default)
};


static udt_VariableType vt_FixedNationalChar = {
    (SetValueProc) Variable_SetValueBytes,
    (GetValueProc) StringVar_GetValue,
    &g_FixedNCharVarType,               // Python type
    DPI_ORACLE_TYPE_NCHAR,              // Oracle type
    DPI_NATIVE_TYPE_BYTES,              // native type
    2000                                // element length (default)
};


static udt_VariableType vt_Rowid = {
    (SetValueProc) RowidVar_SetValue,
    (GetValueProc) RowidVar_GetValue,
    &g_RowidVarType,                    // Python type
    DPI_ORACLE_TYPE_ROWID,              // Oracle type
    DPI_NATIVE_TYPE_ROWID,              // native type
    18                                  // element length (default)
};


static udt_VariableType vt_Binary = {
    (SetValueProc) Variable_SetValueBytes,
    (GetValueProc) StringVar_GetValue,
    &g_BinaryVarType,                   // Python type
    DPI_ORACLE_TYPE_RAW,                // Oracle type
    DPI_NATIVE_TYPE_BYTES,              // native type
    4000                                // element length (default)
};


//-----------------------------------------------------------------------------
// RowidVar_GetValue()
//   Returns the value stored at the given array position.
//-----------------------------------------------------------------------------
static PyObject *RowidVar_GetValue(udt_Variable *var, dpiData *data)
{
    uint32_t rowidLength;
    const char *rowid;

    if (dpiRowid_getStringValue(data->value.asRowid, &rowid, &rowidLength) < 0)
        return Error_RaiseAndReturnNull();
    return cxString_FromEncodedString(rowid, rowidLength,
            var->connection->encodingInfo.encoding);
}


//-----------------------------------------------------------------------------
// RowidVar_SetValue()
//   Rowid values cannot be set.
//-----------------------------------------------------------------------------
static int RowidVar_SetValue(udt_Variable *var, uint32_t pos, dpiData *data,
        PyObject *value)
{
    PyErr_SetString(g_NotSupportedErrorException,
            "setting rowid variables, use a string variable instead");
    return -1;
}


//-----------------------------------------------------------------------------
// StringVar_GetValue()
//   Returns the value stored at the given array position.
//-----------------------------------------------------------------------------
static PyObject *StringVar_GetValue(udt_Variable *var, dpiData *data)
{
    dpiBytes *bytes;

    bytes = &data->value.asBytes;
    if (var->type == &vt_Binary)
        return PyBytes_FromStringAndSize(bytes->ptr, bytes->length);
    else if (var->type == &vt_FixedNationalChar ||
            var->type == &vt_NationalCharString)
        return PyUnicode_Decode(bytes->ptr, bytes->length, bytes->encoding,
                NULL);
    return cxString_FromEncodedString(bytes->ptr, bytes->length,
            bytes->encoding);
}

