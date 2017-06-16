//-----------------------------------------------------------------------------
// Copyright 2016, 2017, Oracle and/or its affiliates. All rights reserved.
//
// Portions Copyright 2007-2015, Anthony Tuininga. All rights reserved.
//
// Portions Copyright 2001-2007, Computronix (Canada) Ltd., Edmonton, Alberta,
// Canada. All rights reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Cursor.c
//   Definition of the Python type OracleCursor.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// structure for the Python type "Cursor"
//-----------------------------------------------------------------------------
typedef struct {
    PyObject_HEAD
    dpiStmt *handle;
    dpiStmtInfo stmtInfo;
    udt_Connection *connection;
    PyObject *statement;
    PyObject *statementTag;
    PyObject *bindVariables;
    PyObject *fetchVariables;
    PyObject *rowFactory;
    PyObject *inputTypeHandler;
    PyObject *outputTypeHandler;
    uint32_t arraySize;
    uint32_t bindArraySize;
    uint32_t fetchArraySize;
    int setInputSizes;
    uint64_t rowCount;
    uint32_t fetchBufferRowIndex;
    uint32_t numRowsInFetchBuffer;
    int moreRowsToFetch;
    int isScrollable;
    int fixupRefCursor;
    int isOpen;
} udt_Cursor;


//-----------------------------------------------------------------------------
// dependent function defintions
//-----------------------------------------------------------------------------
static void Cursor_Free(udt_Cursor*);


//-----------------------------------------------------------------------------
// functions for the Python type "Cursor"
//-----------------------------------------------------------------------------
static PyObject *Cursor_GetIter(udt_Cursor*);
static PyObject *Cursor_GetNext(udt_Cursor*);
static PyObject *Cursor_Close(udt_Cursor*, PyObject*);
static PyObject *Cursor_CallFunc(udt_Cursor*, PyObject*, PyObject*);
static PyObject *Cursor_CallProc(udt_Cursor*, PyObject*, PyObject*);
static PyObject *Cursor_Execute(udt_Cursor*, PyObject*, PyObject*);
static PyObject *Cursor_ExecuteMany(udt_Cursor*, PyObject*, PyObject*);
static PyObject *Cursor_ExecuteManyPrepared(udt_Cursor*, PyObject*);
static PyObject *Cursor_FetchOne(udt_Cursor*, PyObject*);
static PyObject *Cursor_FetchMany(udt_Cursor*, PyObject*, PyObject*);
static PyObject *Cursor_FetchAll(udt_Cursor*, PyObject*);
static PyObject *Cursor_FetchRaw(udt_Cursor*, PyObject*, PyObject*);
static PyObject *Cursor_Parse(udt_Cursor*, PyObject*);
static PyObject *Cursor_Prepare(udt_Cursor*, PyObject*);
static PyObject *Cursor_Scroll(udt_Cursor*, PyObject*, PyObject*);
static PyObject *Cursor_SetInputSizes(udt_Cursor*, PyObject*, PyObject*);
static PyObject *Cursor_SetOutputSize(udt_Cursor*, PyObject*);
static PyObject *Cursor_Var(udt_Cursor*, PyObject*, PyObject*);
static PyObject *Cursor_ArrayVar(udt_Cursor*, PyObject*);
static PyObject *Cursor_BindNames(udt_Cursor*, PyObject*);
static PyObject *Cursor_GetDescription(udt_Cursor*, void*);
static PyObject *Cursor_New(PyTypeObject*, PyObject*, PyObject*);
static int Cursor_Init(udt_Cursor*, PyObject*, PyObject*);
static PyObject *Cursor_Repr(udt_Cursor*);
static PyObject* Cursor_GetBatchErrors(udt_Cursor*);
static PyObject *Cursor_GetArrayDMLRowCounts(udt_Cursor*);
static PyObject *Cursor_GetImplicitResults(udt_Cursor*);
static int Cursor_PerformDefine(udt_Cursor*, uint32_t);
static int Cursor_GetVarData(udt_Cursor*);


//-----------------------------------------------------------------------------
// declaration of methods for Python type "Cursor"
//-----------------------------------------------------------------------------
static PyMethodDef g_CursorMethods[] = {
    { "execute", (PyCFunction) Cursor_Execute, METH_VARARGS | METH_KEYWORDS },
    { "fetchall", (PyCFunction) Cursor_FetchAll, METH_NOARGS },
    { "fetchone", (PyCFunction) Cursor_FetchOne, METH_NOARGS },
    { "fetchmany", (PyCFunction) Cursor_FetchMany,
              METH_VARARGS | METH_KEYWORDS },
    { "fetchraw", (PyCFunction) Cursor_FetchRaw,
              METH_VARARGS | METH_KEYWORDS },
    { "prepare", (PyCFunction) Cursor_Prepare, METH_VARARGS },
    { "parse", (PyCFunction) Cursor_Parse, METH_VARARGS },
    { "setinputsizes", (PyCFunction) Cursor_SetInputSizes,
              METH_VARARGS | METH_KEYWORDS },
    { "executemany", (PyCFunction) Cursor_ExecuteMany,
              METH_VARARGS | METH_KEYWORDS },
    { "callproc", (PyCFunction) Cursor_CallProc,
              METH_VARARGS  | METH_KEYWORDS },
    { "callfunc", (PyCFunction) Cursor_CallFunc,
              METH_VARARGS  | METH_KEYWORDS },
    { "executemanyprepared", (PyCFunction) Cursor_ExecuteManyPrepared,
              METH_VARARGS },
    { "setoutputsize", (PyCFunction) Cursor_SetOutputSize, METH_VARARGS },
    { "scroll", (PyCFunction) Cursor_Scroll, METH_VARARGS | METH_KEYWORDS },
    { "var", (PyCFunction) Cursor_Var, METH_VARARGS | METH_KEYWORDS },
    { "arrayvar", (PyCFunction) Cursor_ArrayVar, METH_VARARGS },
    { "bindnames", (PyCFunction) Cursor_BindNames, METH_NOARGS },
    { "close", (PyCFunction) Cursor_Close, METH_NOARGS },
    { "getbatcherrors", (PyCFunction) Cursor_GetBatchErrors, METH_NOARGS },
    { "getarraydmlrowcounts", (PyCFunction) Cursor_GetArrayDMLRowCounts,
              METH_NOARGS },
    { "getimplicitresults", (PyCFunction) Cursor_GetImplicitResults,
              METH_NOARGS },
    { NULL, NULL }
};


//-----------------------------------------------------------------------------
// declaration of members for Python type "Cursor"
//-----------------------------------------------------------------------------
static PyMemberDef g_CursorMembers[] = {
    { "arraysize", T_UINT, offsetof(udt_Cursor, arraySize), 0 },
    { "bindarraysize", T_UINT, offsetof(udt_Cursor, bindArraySize), 0 },
    { "rowcount", T_ULONGLONG, offsetof(udt_Cursor, rowCount), READONLY },
    { "statement", T_OBJECT, offsetof(udt_Cursor, statement), READONLY },
    { "connection", T_OBJECT_EX, offsetof(udt_Cursor, connection), READONLY },
    { "rowfactory", T_OBJECT, offsetof(udt_Cursor, rowFactory), 0 },
    { "bindvars", T_OBJECT, offsetof(udt_Cursor, bindVariables), READONLY },
    { "fetchvars", T_OBJECT, offsetof(udt_Cursor, fetchVariables), READONLY },
    { "inputtypehandler", T_OBJECT, offsetof(udt_Cursor, inputTypeHandler),
            0 },
    { "outputtypehandler", T_OBJECT, offsetof(udt_Cursor, outputTypeHandler),
            0 },
    { "scrollable", T_BOOL, offsetof(udt_Cursor, isScrollable), 0 },
    { NULL }
};


//-----------------------------------------------------------------------------
// declaration of calculated members for Python type "Connection"
//-----------------------------------------------------------------------------
static PyGetSetDef g_CursorCalcMembers[] = {
    { "description", (getter) Cursor_GetDescription, 0, 0, 0 },
    { NULL }
};


//-----------------------------------------------------------------------------
// declaration of Python type "Cursor"
//-----------------------------------------------------------------------------
static PyTypeObject g_CursorType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "cx_Oracle.Cursor",                 // tp_name
    sizeof(udt_Cursor),                 // tp_basicsize
    0,                                  // tp_itemsize
    (destructor) Cursor_Free,           // tp_dealloc
    0,                                  // tp_print
    0,                                  // tp_getattr
    0,                                  // tp_setattr
    0,                                  // tp_compare
    (reprfunc) Cursor_Repr,             // tp_repr
    0,                                  // tp_as_number
    0,                                  // tp_as_sequence
    0,                                  // tp_as_mapping
    0,                                  // tp_hash
    0,                                  // tp_call
    0,                                  // tp_str
    0,                                  // tp_getattro
    0,                                  // tp_setattro
    0,                                  // tp_as_buffer
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
                                        // tp_flags
    0,                                  // tp_doc
    0,                                  // tp_traverse
    0,                                  // tp_clear
    0,                                  // tp_richcompare
    0,                                  // tp_weaklistoffset
    (getiterfunc) Cursor_GetIter,       // tp_iter
    (iternextfunc) Cursor_GetNext,      // tp_iternext
    g_CursorMethods,                    // tp_methods
    g_CursorMembers,                    // tp_members
    g_CursorCalcMembers,                // tp_getset
    0,                                  // tp_base
    0,                                  // tp_dict
    0,                                  // tp_descr_get
    0,                                  // tp_descr_set
    0,                                  // tp_dictoffset
    (initproc) Cursor_Init,             // tp_init
    0,                                  // tp_alloc
    Cursor_New,                         // tp_new
    0,                                  // tp_free
    0,                                  // tp_is_gc
    0                                   // tp_bases
};


#include "Variable.c"


//-----------------------------------------------------------------------------
// Cursor_New()
//   Create a new cursor object.
//-----------------------------------------------------------------------------
static PyObject *Cursor_New(PyTypeObject *type, PyObject *args,
        PyObject *keywordArgs)
{
    return type->tp_alloc(type, 0);
}


//-----------------------------------------------------------------------------
// Cursor_Init()
//   Create a new cursor object.
//-----------------------------------------------------------------------------
static int Cursor_Init(udt_Cursor *self, PyObject *args, PyObject *keywordArgs)
{
    static char *keywordList[] = { "connection", "scrollable", NULL };
    udt_Connection *connection;
    PyObject *scrollableObj;

    // parse arguments
    scrollableObj = NULL;
    if (!PyArg_ParseTupleAndKeywords(args, keywordArgs, "O!|O", keywordList,
            &g_ConnectionType, &connection, &scrollableObj))
        return -1;
    if (GetBooleanValue(scrollableObj, 0, &self->isScrollable) < 0)
        return -1;

    // initialize members
    Py_INCREF(connection);
    self->connection = connection;
    self->arraySize = 100;
    self->fetchArraySize = 100;
    self->bindArraySize = 1;
    self->isOpen = 1;

    return 0;
}


//-----------------------------------------------------------------------------
// Cursor_Repr()
//   Return a string representation of the cursor.
//-----------------------------------------------------------------------------
static PyObject *Cursor_Repr(udt_Cursor *cursor)
{
    PyObject *connectionRepr, *module, *name, *result, *format, *formatArgs;

    format = cxString_FromAscii("<%s.%s on %s>");
    if (!format)
        return NULL;
    connectionRepr = PyObject_Repr((PyObject*) cursor->connection);
    if (!connectionRepr) {
        Py_DECREF(format);
        return NULL;
    }
    if (GetModuleAndName(Py_TYPE(cursor), &module, &name) < 0) {
        Py_DECREF(format);
        Py_DECREF(connectionRepr);
        return NULL;
    }
    formatArgs = PyTuple_Pack(3, module, name, connectionRepr);
    Py_DECREF(module);
    Py_DECREF(name);
    Py_DECREF(connectionRepr);
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
// Cursor_Free()
//   Deallocate the cursor.
//-----------------------------------------------------------------------------
static void Cursor_Free(udt_Cursor *self)
{
    Py_CLEAR(self->statement);
    Py_CLEAR(self->statementTag);
    Py_CLEAR(self->bindVariables);
    Py_CLEAR(self->fetchVariables);
    if (self->handle) {
        dpiStmt_release(self->handle);
        self->handle = NULL;
    }
    Py_CLEAR(self->connection);
    Py_CLEAR(self->rowFactory);
    Py_CLEAR(self->inputTypeHandler);
    Py_CLEAR(self->outputTypeHandler);
    Py_TYPE(self)->tp_free((PyObject*) self);
}


//-----------------------------------------------------------------------------
// Cursor_IsOpen()
//   Determines if the cursor object is open. Since the same cursor can be
// used to execute multiple statements, simply checking for the DPI statement
// handle is insufficient.
//-----------------------------------------------------------------------------
static int Cursor_IsOpen(udt_Cursor *self)
{
    if (!self->isOpen) {
        PyErr_SetString(g_InterfaceErrorException, "not open");
        return -1;
    }
    return Connection_IsConnected(self->connection);
}


//-----------------------------------------------------------------------------
// Cursor_VerifyFetch()
//   Verify that fetching may happen from this cursor.
//-----------------------------------------------------------------------------
static int Cursor_VerifyFetch(udt_Cursor *self)
{
    uint32_t numQueryColumns;

    // make sure the cursor is open
    if (Cursor_IsOpen(self) < 0)
        return -1;

    // fixup REF cursor, if applicable
    if (self->fixupRefCursor) {
        self->fetchArraySize = self->arraySize;
        if (dpiStmt_setFetchArraySize(self->handle, self->fetchArraySize) < 0)
            return Error_RaiseAndReturnInt();
        if (dpiStmt_getNumQueryColumns(self->handle, &numQueryColumns) < 0)
            return Error_RaiseAndReturnInt();
        if (Cursor_PerformDefine(self, numQueryColumns) < 0)
            return Error_RaiseAndReturnInt();
        self->fixupRefCursor = 0;
    }

    // make sure the cursor is for a query
    if (!self->fetchVariables) {
        PyErr_SetString(g_InterfaceErrorException, "not a query");
        return -1;
    }

    return 0;
}


//-----------------------------------------------------------------------------
// Cursor_FetchRow()
//   Fetch a single row from the cursor. Internally the number of rows left in
// the buffer is managed in order to minimize calls to Py_BEGIN_ALLOW_THREADS
// and Py_END_ALLOW_THREADS which have a significant overhead.
//-----------------------------------------------------------------------------
static int Cursor_FetchRow(udt_Cursor *self, int *found,
        uint32_t *bufferRowIndex)
{
    int status;

    // if the number of rows in the fetch buffer is zero and there are more
    // rows to fetch, call DPI with threading enabled in order to perform any
    // fetch requiring a network round trip
    if (self->numRowsInFetchBuffer == 0 && self->moreRowsToFetch) {
        Py_BEGIN_ALLOW_THREADS
        status = dpiStmt_fetchRows(self->handle, self->fetchArraySize,
                &self->fetchBufferRowIndex, &self->numRowsInFetchBuffer,
                &self->moreRowsToFetch);
        Py_END_ALLOW_THREADS
        if (status < 0)
            return Error_RaiseAndReturnInt();
    }

    // keep track of where we are in the fetch buffer
    if (self->numRowsInFetchBuffer == 0)
        *found = 0;
    else {
        *found = 1;
        *bufferRowIndex = self->fetchBufferRowIndex++;
        self->numRowsInFetchBuffer--;
    }

    return 0;
}


//-----------------------------------------------------------------------------
// Cursor_PerformDefine()
//   Perform the defines for the cursor. At this point it is assumed that the
// statement being executed is in fact a query.
//-----------------------------------------------------------------------------
static int Cursor_PerformDefine(udt_Cursor *self, uint32_t numQueryColumns)
{
    PyObject *outputTypeHandler, *result;
    udt_ObjectType *objectType;
    udt_VariableType *varType;
    dpiQueryInfo queryInfo;
    uint32_t pos, size;
    udt_Variable *var;

    // create a list corresponding to the number of items
    self->fetchVariables = PyList_New(numQueryColumns);
    if (!self->fetchVariables)
        return -1;

    // initialize fetching variables; these are used to reduce the number of
    // times that Py_BEGIN_ALLOW_THREADS/Py_END_ALLOW_THREADS is called as
    // there is a significant amount of overhead in making these calls
    self->numRowsInFetchBuffer = 0;
    self->moreRowsToFetch = 1;

    // create a variable for each of the query columns
    self->fetchArraySize = self->arraySize;
    for (pos = 1; pos <= numQueryColumns; pos++) {

        // get query information for the column position
        if (dpiStmt_getQueryInfo(self->handle, pos, &queryInfo) < 0)
            return Error_RaiseAndReturnInt();
        if (queryInfo.sizeInChars)
            size = queryInfo.sizeInChars;
        else size = queryInfo.clientSizeInBytes;

        // determine object type, if applicable
        objectType = NULL;
        if (queryInfo.objectType) {
            objectType = ObjectType_New(self->connection,
                    queryInfo.objectType);
            if (!objectType)
                return -1;
        }

        // determine the default type 
        varType = VarType_FromQueryInfo(&queryInfo);
        if (!varType)
            return -1;

        // see if an output type handler should be used
        var = NULL;
        outputTypeHandler = NULL;
        if (self->outputTypeHandler && self->outputTypeHandler != Py_None)
            outputTypeHandler = self->outputTypeHandler;
        else if (self->connection->outputTypeHandler &&
                self->connection->outputTypeHandler != Py_None)
            outputTypeHandler = self->connection->outputTypeHandler;

        // if using an output type handler, None implies default behavior
        if (outputTypeHandler) {
            result = PyObject_CallFunction(outputTypeHandler, "Os#Oiii",
                    self, queryInfo.name, queryInfo.nameLength,
                    varType->pythonType, size, queryInfo.precision,
                    queryInfo.scale);
            if (!result) {
                Py_XDECREF(objectType);
                return -1;
            } else if (result == Py_None)
                Py_DECREF(result);
            else if (!Variable_Check(result)) {
                Py_DECREF(result);
                Py_XDECREF(objectType);
                PyErr_SetString(PyExc_TypeError,
                        "expecting variable from output type handler");
                return -1;
            } else {
                var = (udt_Variable*) result;
                if (var->allocatedElements < self->fetchArraySize) {
                    Py_DECREF(result);
                    Py_XDECREF(objectType);
                    PyErr_SetString(PyExc_TypeError,
                            "expecting variable with array size large "
                            "enough for fetch");
                    return -1;
                }
            }
        }

        // if no variable created yet, use the database metadata
        if (!var) {
            var = Variable_New(self, self->fetchArraySize, varType, size, 0,
                    objectType);
            if (!var) {
                Py_XDECREF(objectType);
                return -1;
            }
        }

        // add the variable to the fetch variables and perform define
        Py_XDECREF(objectType);
        PyList_SET_ITEM(self->fetchVariables, pos - 1, (PyObject *) var);
        if (dpiStmt_define(self->handle, pos, var->handle) < 0)
            return Error_RaiseAndReturnInt();

    }

    return 0;
}


//-----------------------------------------------------------------------------
// Cursor_ItemDescription()
//   Return a tuple describing the item at the given position.
//-----------------------------------------------------------------------------
static PyObject *Cursor_ItemDescription(udt_Cursor *self, uint32_t pos)
{
    udt_VariableType *varType;
    int displaySize, index;
    dpiQueryInfo queryInfo;
    PyObject *tuple, *temp;

    // get information about the column position
    if (dpiStmt_getQueryInfo(self->handle, pos, &queryInfo) < 0)
        return NULL;
    varType = VarType_FromQueryInfo(&queryInfo);
    if (!varType)
        return NULL;

    // set display size based on data type
    switch (queryInfo.oracleTypeNum) {
        case DPI_ORACLE_TYPE_VARCHAR:
        case DPI_ORACLE_TYPE_NVARCHAR:
        case DPI_ORACLE_TYPE_CHAR:
        case DPI_ORACLE_TYPE_NCHAR:
        case DPI_ORACLE_TYPE_ROWID:
            displaySize = (int) queryInfo.sizeInChars;
            break;
        case DPI_ORACLE_TYPE_RAW:
            displaySize = (int) queryInfo.clientSizeInBytes;
            break;
        case DPI_ORACLE_TYPE_NATIVE_FLOAT:
        case DPI_ORACLE_TYPE_NATIVE_DOUBLE:
        case DPI_ORACLE_TYPE_NATIVE_INT:
        case DPI_ORACLE_TYPE_NUMBER:
            if (queryInfo.precision) {
                displaySize = queryInfo.precision + 1;
                if (queryInfo.scale > 0)
                    displaySize += queryInfo.scale + 1;
            }
            else displaySize = 127;
            break;
        case DPI_ORACLE_TYPE_DATE:
        case DPI_ORACLE_TYPE_TIMESTAMP:
            displaySize = 23;
            break;
        default:
            displaySize = 0;
    }

    // create the tuple and populate it
    tuple = PyTuple_New(7);
    if (!tuple)
        return NULL;

    // set each of the items in the tuple
    PyTuple_SET_ITEM(tuple, 0, cxString_FromEncodedString(queryInfo.name,
            queryInfo.nameLength, self->connection->encodingInfo.encoding));
    Py_INCREF(varType->pythonType);
    PyTuple_SET_ITEM(tuple, 1, (PyObject*) varType->pythonType);
    if (displaySize)
        PyTuple_SET_ITEM(tuple, 2, PyInt_FromLong(displaySize));
    else {
        Py_INCREF(Py_None);
        PyTuple_SET_ITEM(tuple, 2, Py_None);
    }
    if (queryInfo.clientSizeInBytes)
        PyTuple_SET_ITEM(tuple, 3,
                PyInt_FromLong(queryInfo.clientSizeInBytes));
    else {
        Py_INCREF(Py_None);
        PyTuple_SET_ITEM(tuple, 3, Py_None);
    }
    if (queryInfo.precision || queryInfo.scale) {
        PyTuple_SET_ITEM(tuple, 4, PyInt_FromLong(queryInfo.precision));
        PyTuple_SET_ITEM(tuple, 5, PyInt_FromLong(queryInfo.scale));
    } else {
        Py_INCREF(Py_None);
        PyTuple_SET_ITEM(tuple, 4, Py_None);
        Py_INCREF(Py_None);
        PyTuple_SET_ITEM(tuple, 5, Py_None);
    }
    PyTuple_SET_ITEM(tuple, 6, PyInt_FromLong(queryInfo.nullOk != 0));

    // make sure the tuple is ok
    for (index = 0; index < 7; index++) {
        temp = PyTuple_GET_ITEM(tuple, index);
        if (!temp) {
            Py_DECREF(tuple);
            return NULL;
        } else if (temp == Py_None)
            Py_INCREF(temp);
    }

    return tuple;
}


//-----------------------------------------------------------------------------
// Cursor_GetDescription()
//   Return a list of 7-tuples consisting of the description of the define
// variables.
//-----------------------------------------------------------------------------
static PyObject *Cursor_GetDescription(udt_Cursor *self, void *unused)
{
    uint32_t numQueryColumns, i;
    PyObject *results, *tuple;

    // make sure the cursor is open
    if (Cursor_IsOpen(self) < 0)
        return NULL;

    // determine the number of query columns; if not a query return None
    if (!self->handle)
        Py_RETURN_NONE;
    if (dpiStmt_getNumQueryColumns(self->handle, &numQueryColumns) < 0)
        return Error_RaiseAndReturnNull();
    if (numQueryColumns == 0)
        Py_RETURN_NONE;

    // create a list of the required length
    results = PyList_New(numQueryColumns);
    if (!results)
        return NULL;

    // create tuples corresponding to the select-items
    for (i = 0; i < numQueryColumns; i++) {
        tuple = Cursor_ItemDescription(self, i + 1);
        if (!tuple) {
            Py_DECREF(results);
            return NULL;
        }
        PyList_SET_ITEM(results, i, tuple);
    }

    return results;
}


//-----------------------------------------------------------------------------
// Cursor_Close()
//   Close the cursor. Any action taken on this cursor from this point forward
// results in an exception being raised.
//-----------------------------------------------------------------------------
static PyObject *Cursor_Close(udt_Cursor *self, PyObject *args)
{
    if (Cursor_IsOpen(self) < 0)
        return NULL;
    Py_CLEAR(self->bindVariables);
    Py_CLEAR(self->fetchVariables);
    if (self->handle) {
        if (dpiStmt_close(self->handle, NULL, 0) < 0)
            return Error_RaiseAndReturnNull();
        dpiStmt_release(self->handle);
        self->handle = NULL;
    }
    self->isOpen = 0;

    Py_RETURN_NONE;
}


//-----------------------------------------------------------------------------
// Cursor_SetBindVariableHelper()
//   Helper for setting a bind variable.
//-----------------------------------------------------------------------------
static int Cursor_SetBindVariableHelper(udt_Cursor *self, unsigned numElements,
        unsigned arrayPos, PyObject *value, udt_Variable *origVar,
        udt_Variable **newVar, int deferTypeAssignment)
{
    int isValueVar;

    // initialization
    *newVar = NULL;
    isValueVar = Variable_Check(value);

    // handle case where variable is already bound
    if (origVar) {

        // if the value is a variable object, rebind it if necessary
        if (isValueVar) {
            if ( (PyObject*) origVar != value) {
                Py_INCREF(value);
                *newVar = (udt_Variable*) value;
            }

        // if the number of elements has changed, create a new variable
        // this is only necessary for executemany() since execute() always
        // passes a value of 1 for the number of elements
        } else if (numElements > origVar->allocatedElements) {
            *newVar = Variable_New(self, numElements, origVar->type,
                    origVar->size, origVar->isArray, origVar->objectType);
            if (!*newVar)
                return -1;
            if (Variable_SetValue(*newVar, arrayPos, value) < 0) {
                Py_CLEAR(*newVar);
                return -1;
            }

        // otherwise, attempt to set the value
        } else if (Variable_SetValue(origVar, arrayPos, value) < 0) {

            // executemany() should simply fail after the first element
            if (arrayPos > 0)
                return -1;

            // clear the exception and try to create a new variable
            PyErr_Clear();
            origVar = NULL;
        }

    }

    // if no original variable used, create a new one
    if (!origVar) {

        // if the value is a variable object, bind it directly
        if (isValueVar) {
            Py_INCREF(value);
            *newVar = (udt_Variable*) value;

        // otherwise, create a new variable, unless the value is None and
        // we wish to defer type assignment
        } else if (value != Py_None || !deferTypeAssignment) {
            *newVar = Variable_NewByValue(self, value, numElements);
            if (!*newVar)
                return -1;
            if (Variable_SetValue(*newVar, arrayPos, value) < 0) {
                Py_CLEAR(*newVar);
                return -1;
            }
        }

    }

    return 0;
}


//-----------------------------------------------------------------------------
// Cursor_SetBindVariables()
//   Create or set bind variables.
//-----------------------------------------------------------------------------
static int Cursor_SetBindVariables(udt_Cursor *self, PyObject *parameters,
        unsigned numElements, unsigned arrayPos, int deferTypeAssignment)
{
    uint32_t i, origBoundByPos, origNumParams, boundByPos, numParams;
    PyObject *key, *value, *origVar;
    udt_Variable *newVar;
    Py_ssize_t pos, temp;

    // make sure positional and named binds are not being intermixed
    origNumParams = numParams = 0;
    boundByPos = PySequence_Check(parameters);
    if (boundByPos) {
        temp = PySequence_Size(parameters);
        if (temp < 0)
            return -1;
        numParams = (uint32_t) temp;
    }
    if (self->bindVariables) {
        origBoundByPos = PyList_Check(self->bindVariables);
        if (boundByPos != origBoundByPos) {
            PyErr_SetString(g_ProgrammingErrorException,
                    "positional and named binds cannot be intermixed");
            return -1;
        }
        if (origBoundByPos)
            origNumParams = (uint32_t) PyList_GET_SIZE(self->bindVariables);

    // otherwise, create the list or dictionary if needed
    } else {
        if (boundByPos)
            self->bindVariables = PyList_New(numParams);
        else self->bindVariables = PyDict_New();
        if (!self->bindVariables)
            return -1;
        origNumParams = 0;
    }

    // handle positional binds
    if (boundByPos) {
        for (i = 0; i < numParams; i++) {
            value = PySequence_GetItem(parameters, i);
            if (!value)
                return -1;
            Py_DECREF(value);
            if (i < origNumParams) {
                origVar = PyList_GET_ITEM(self->bindVariables, i);
                if (origVar == Py_None)
                    origVar = NULL;
            } else origVar = NULL;
            if (Cursor_SetBindVariableHelper(self, numElements, arrayPos,
                    value, (udt_Variable*) origVar, &newVar,
                    deferTypeAssignment) < 0)
                return -1;
            if (newVar) {
                if (i < PyList_GET_SIZE(self->bindVariables)) {
                    if (PyList_SetItem(self->bindVariables, i,
                            (PyObject*) newVar) < 0) {
                        Py_DECREF(newVar);
                        return -1;
                    }
                } else {
                    if (PyList_Append(self->bindVariables,
                            (PyObject*) newVar) < 0) {
                        Py_DECREF(newVar);
                        return -1;
                    }
                    Py_DECREF(newVar);
                }
            }
        }

    // handle named binds
    } else {
        pos = 0;
        while (PyDict_Next(parameters, &pos, &key, &value)) {
            origVar = PyDict_GetItem(self->bindVariables, key);
            if (Cursor_SetBindVariableHelper(self, numElements, arrayPos,
                    value, (udt_Variable*) origVar, &newVar,
                    deferTypeAssignment) < 0)
                return -1;
            if (newVar) {
                if (PyDict_SetItem(self->bindVariables, key,
                        (PyObject*) newVar) < 0) {
                    Py_DECREF(newVar);
                    return -1;
                }
                Py_DECREF(newVar);
            }
        }
    }

    return 0;
}


//-----------------------------------------------------------------------------
// Cursor_PerformBind()
//   Perform the binds on the cursor.
//-----------------------------------------------------------------------------
static int Cursor_PerformBind(udt_Cursor *self)
{
    PyObject *key, *var;
    Py_ssize_t pos;
    int i;

    // ensure that input sizes are reset
    // this is done before binding is attempted so that if binding fails and
    // a new statement is prepared, the bind variables will be reset and
    // spurious errors will not occur
    self->setInputSizes = 0;

    // set values and perform binds for all bind variables
    if (self->bindVariables) {
        if (PyDict_Check(self->bindVariables)) {
            pos = 0;
            while (PyDict_Next(self->bindVariables, &pos, &key, &var)) {
                if (Variable_Bind((udt_Variable*) var, self, key, 0) < 0)
                    return -1;
            }
        } else {
            for (i = 0; i < PyList_GET_SIZE(self->bindVariables); i++) {
                var = PyList_GET_ITEM(self->bindVariables, i);
                if (var != Py_None) {
                    if (Variable_Bind((udt_Variable*) var, self, NULL,
                            i + 1) < 0)
                        return -1;
                }
            }
        }
    }

    return 0;
}


//-----------------------------------------------------------------------------
// Cursor_CreateRow()
//   Create an object for the row. The object created is a tuple unless a row
// factory function has been defined in which case it is the result of the
// row factory function called with the argument tuple that would otherwise be
// returned.
//-----------------------------------------------------------------------------
static PyObject *Cursor_CreateRow(udt_Cursor *self, uint32_t pos)
{
    PyObject *tuple, *item, *result;
    Py_ssize_t numItems, i;
    udt_Variable *var;

    // bump row count as a new row has been found
    self->rowCount++;

    // create a new tuple
    numItems = PyList_GET_SIZE(self->fetchVariables);
    tuple = PyTuple_New(numItems);
    if (!tuple)
        return NULL;

    // acquire the value for each item
    for (i = 0; i < numItems; i++) {
        var = (udt_Variable*) PyList_GET_ITEM(self->fetchVariables, i);
        item = Variable_GetSingleValue(var, pos);
        if (!item) {
            Py_DECREF(tuple);
            return NULL;
        }
        PyTuple_SET_ITEM(tuple, i, item);
    }

    // if a row factory is defined, call it
    if (self->rowFactory && self->rowFactory != Py_None) {
        result = PyObject_CallObject(self->rowFactory, tuple);
        Py_DECREF(tuple);
        return result;
    }

    return tuple;
}


//-----------------------------------------------------------------------------
// Cursor_InternalPrepare()
//   Internal method for preparing a statement for execution.
//-----------------------------------------------------------------------------
static int Cursor_InternalPrepare(udt_Cursor *self, PyObject *statement,
        PyObject *statementTag)
{
    udt_Buffer statementBuffer, tagBuffer;
    int status;

    // make sure we don't get a situation where nothing is to be executed
    if (statement == Py_None && !self->statement) {
        PyErr_SetString(g_ProgrammingErrorException,
                "no statement specified and no prior statement prepared");
        return -1;
    }

    // nothing to do if the statement is identical to the one already stored
    // but go ahead and prepare anyway for create, alter and drop statments
    if (statement == Py_None || statement == self->statement) {
        if (self->handle && !self->stmtInfo.isDDL)
            return 0;
        statement = self->statement;
    }

    // keep track of the statement
    Py_XDECREF(self->statement);
    Py_INCREF(statement);
    self->statement = statement;

    // keep track of the tag
    Py_XDECREF(self->statementTag);
    Py_XINCREF(statementTag);
    self->statementTag = statementTag;

    // clear fetch and bind variables if applicable
    Py_CLEAR(self->fetchVariables);
    if (!self->setInputSizes)
        Py_CLEAR(self->bindVariables);

    // prepare statement
    if (cxBuffer_FromObject(&statementBuffer, statement,
            self->connection->encodingInfo.encoding) < 0)
        return -1;
    if (cxBuffer_FromObject(&tagBuffer, statementTag,
            self->connection->encodingInfo.encoding) < 0) {
        cxBuffer_Clear(&statementBuffer);
        return -1;
    }
    Py_BEGIN_ALLOW_THREADS
    if (self->handle)
        dpiStmt_release(self->handle);
    status = dpiConn_prepareStmt(self->connection->handle, self->isScrollable,
            (const char*) statementBuffer.ptr, statementBuffer.size,
            (const char*) tagBuffer.ptr, tagBuffer.size, &self->handle);
    Py_END_ALLOW_THREADS
    cxBuffer_Clear(&statementBuffer);
    cxBuffer_Clear(&tagBuffer);
    if (status < 0)
        return Error_RaiseAndReturnInt();

    // get statement information
    if (dpiStmt_getInfo(self->handle, &self->stmtInfo) < 0)
        return Error_RaiseAndReturnInt();

    // set the fetch array size
    if (dpiStmt_setFetchArraySize(self->handle, self->arraySize) < 0)
        return Error_RaiseAndReturnInt();

    // clear row factory, if applicable
    Py_CLEAR(self->rowFactory);

    return 0;
}


//-----------------------------------------------------------------------------
// Cursor_Parse()
//   Parse the statement without executing it. This also retrieves information
// about the select list for select statements.
//-----------------------------------------------------------------------------
static PyObject *Cursor_Parse(udt_Cursor *self, PyObject *args)
{
    uint32_t mode, numQueryColumns;
    dpiStmtInfo stmtInfo;
    PyObject *statement;
    int status;

    // statement text is expected
    if (!PyArg_ParseTuple(args, "S", &statement))
        return NULL;

    // make sure the cursor is open
    if (Cursor_IsOpen(self) < 0)
        return NULL;

    // prepare the statement and get statement information
    if (Cursor_InternalPrepare(self, statement, NULL) < 0)
        return NULL;
    if (dpiStmt_getInfo(self->handle, &stmtInfo) < 0)
        return Error_RaiseAndReturnNull();

    // parse the statement
    if (stmtInfo.isQuery)
        mode = DPI_MODE_EXEC_DESCRIBE_ONLY;
    else mode = DPI_MODE_EXEC_PARSE_ONLY;
    Py_BEGIN_ALLOW_THREADS
    status = dpiStmt_execute(self->handle, mode, &numQueryColumns);
    Py_END_ALLOW_THREADS
    if (status < 0)
        return Error_RaiseAndReturnNull();

    Py_RETURN_NONE;
}


//-----------------------------------------------------------------------------
// Cursor_Prepare()
//   Prepare the statement for execution.
//-----------------------------------------------------------------------------
static PyObject *Cursor_Prepare(udt_Cursor *self, PyObject *args)
{
    PyObject *statement, *statementTag;

    // statement text and optional tag is expected
    statementTag = NULL;
    if (!PyArg_ParseTuple(args, "O|O", &statement, &statementTag))
        return NULL;

    // make sure the cursor is open
    if (Cursor_IsOpen(self) < 0)
        return NULL;

    // prepare the statement
    if (Cursor_InternalPrepare(self, statement, statementTag) < 0)
        return NULL;

    Py_RETURN_NONE;
}


//-----------------------------------------------------------------------------
// Cursor_CallCalculateSize()
//   Calculate the size of the statement that is to be executed.
//-----------------------------------------------------------------------------
static int Cursor_CallCalculateSize(PyObject *name, udt_Variable *returnValue,
        PyObject *listOfArguments, PyObject *keywordArguments, int *size)
{
    Py_ssize_t numPositionalArgs, numKeywordArgs;

    // set base size without any arguments
    *size = 17;

    // add any additional space required to handle the return value
    if (returnValue)
        *size += 6;

    // assume up to 9 characters for each positional argument
    // this allows up to four digits for the placeholder if the bind variale
    // is a boolean value (prior to Oracle 12.1)
    numPositionalArgs = 0;
    if (listOfArguments) {
        numPositionalArgs = PySequence_Size(listOfArguments);
        if (numPositionalArgs < 0)
            return -1;
        *size += (int) (numPositionalArgs * 9);
    }

    // assume up to 15 characters for each keyword argument
    // this allows up to four digits for the placeholder if the bind variable
    // is a boolean value (prior to Oracle 12.1)
    numKeywordArgs = 0;
    if (keywordArguments) {
        numKeywordArgs = PyDict_Size(keywordArguments);
        if (numKeywordArgs < 0)
            return -1;
        *size += (int) (numKeywordArgs * 15);
    }

    // the above assume a maximum of 10,000 arguments; check and raise an
    // error if the number of arguments exceeds this value; more than this
    // number would probably be unusable in any case!
    if (numPositionalArgs + numKeywordArgs > 10000) {
        PyErr_SetString(g_InterfaceErrorException, "too many arguments");
        return -1;
    }

    return 0;
}


//-----------------------------------------------------------------------------
// Cursor_CallBuildStatement()
//   Determine the statement and the bind variables to bind to the statement
// that is created for calling a stored procedure or function.
//-----------------------------------------------------------------------------
static int Cursor_CallBuildStatement(PyObject *name, udt_Variable *returnValue,
        PyObject *listOfArguments, PyObject *keywordArguments, char *statement,
        PyObject **statementObj, PyObject **bindVariables)
{
    PyObject *key, *value, *format, *formatArgs, *positionalArgs, *temp;
    uint32_t i, argNum, numPositionalArgs;
    dpiVersionInfo versionInfo;
    Py_ssize_t pos;
    char *ptr;

    // initialize the bind variables to the list of positional arguments
    if (listOfArguments)
        *bindVariables = PySequence_List(listOfArguments);
    else *bindVariables = PyList_New(0);
    if (!*bindVariables)
        return -1;

    // insert the return variable, if applicable
    if (returnValue) {
        if (PyList_Insert(*bindVariables, 0, (PyObject*) returnValue) < 0)
            return -1;
    }

    // initialize format arguments
    formatArgs = PyList_New(0);
    if (!formatArgs)
        return -1;
    if (PyList_Append(formatArgs, name) < 0) {
        Py_DECREF(formatArgs);
        return -1;
    }

    // begin building the statement
    argNum = 1;
    strcpy(statement, "begin ");
    if (returnValue) {
        strcat(statement, ":1 := ");
        argNum++;
    }
    strcat(statement, "%s");
    ptr = statement + strlen(statement);
    *ptr++ = '(';

    // determine the client version in use
    // booleans are not supported until Oracle 12.1
    if (dpiContext_getClientVersion(g_DpiContext, &versionInfo) < 0)
        return Error_RaiseAndReturnInt();

    // include any positional arguments first
    if (listOfArguments) {
        positionalArgs = PySequence_Fast(listOfArguments,
                "expecting sequence of arguments");
        if (!positionalArgs) {
            Py_DECREF(formatArgs);
            return -1;
        }
        numPositionalArgs = (uint32_t) PySequence_Size(listOfArguments);
        for (i = 0; i < numPositionalArgs; i++) {
            if (i > 0)
                *ptr++ = ',';
            ptr += sprintf(ptr, ":%d", argNum++);
            if (versionInfo.versionNum < 12 &&
                    PyBool_Check(PySequence_Fast_GET_ITEM(positionalArgs, i)))
                ptr += sprintf(ptr, " = 1");
        }
        Py_DECREF(positionalArgs);
    }

    // next append any keyword arguments
    if (keywordArguments) {
        pos = 0;
        while (PyDict_Next(keywordArguments, &pos, &key, &value)) {
            if (PyList_Append(*bindVariables, value) < 0) {
                Py_DECREF(formatArgs);
                return -1;
            }
            if (PyList_Append(formatArgs, key) < 0) {
                Py_DECREF(formatArgs);
                return -1;
            }
            if ((argNum > 1 && !returnValue) || (argNum > 2 && returnValue))
                *ptr++ = ',';
            ptr += sprintf(ptr, "%%s => :%d", argNum++);
            if (versionInfo.versionNum < 12 && PyBool_Check(value))
                ptr += sprintf(ptr, " = 1");
        }
    }

    // create statement object
    strcpy(ptr, "); end;");
    format = cxString_FromAscii(statement);
    if (!format) {
        Py_DECREF(formatArgs);
        return -1;
    }
    temp = PyList_AsTuple(formatArgs);
    Py_DECREF(formatArgs);
    if (!temp) {
        Py_DECREF(format);
        return -1;
    }
    *statementObj = cxString_Format(format, temp);
    Py_DECREF(format);
    Py_DECREF(temp);
    if (!*statementObj)
        return -1;

    return 0;
}


//-----------------------------------------------------------------------------
// Cursor_Call()
//   Call a stored procedure or function.
//-----------------------------------------------------------------------------
static int Cursor_Call(udt_Cursor *self, udt_Variable *returnValue,
        PyObject *name, PyObject *listOfArguments, PyObject *keywordArguments)
{
    PyObject *bindVariables, *statementObj, *results;
    int statementSize;
    char *statement;

    // verify that the arguments are passed correctly
    if (listOfArguments) {
        if (!PySequence_Check(listOfArguments)) {
            PyErr_SetString(PyExc_TypeError, "arguments must be a sequence");
            return -1;
        }
    }
    if (keywordArguments) {
        if (!PyDict_Check(keywordArguments)) {
            PyErr_SetString(PyExc_TypeError,
                    "keyword arguments must be a dictionary");
            return -1;
        }
    }

    // make sure the cursor is open
    if (Cursor_IsOpen(self) < 0)
        return -1;

    // determine the statement size
    if (Cursor_CallCalculateSize(name, returnValue, listOfArguments,
            keywordArguments, &statementSize) < 0)
        return -1;

    // allocate a string for the statement
    statement = (char*) PyMem_Malloc(statementSize);
    if (!statement) {
        PyErr_NoMemory();
        return -1;
    }

    // determine the statement to execute and the argument to pass
    bindVariables = statementObj = NULL;
    if (Cursor_CallBuildStatement(name, returnValue, listOfArguments,
            keywordArguments, statement, &statementObj, &bindVariables) < 0) {
        PyMem_Free(statement);
        Py_XDECREF(statementObj);
        Py_XDECREF(bindVariables);
        return -1;
    }
    PyMem_Free(statement);

    // execute the statement on the cursor
    results = PyObject_CallMethod( (PyObject*) self, "execute", "OO",
            statementObj, bindVariables);
    Py_DECREF(statementObj);
    Py_DECREF(bindVariables);
    if (!results)
        return -1;
    Py_DECREF(results);

    return 0;
}


//-----------------------------------------------------------------------------
// Cursor_CallFunc()
//   Call a stored function and return the return value of the function.
//-----------------------------------------------------------------------------
static PyObject *Cursor_CallFunc(udt_Cursor *self, PyObject *args,
        PyObject *keywordArgs)
{
    static char *keywordList[] = { "name", "returnType", "parameters",
            "keywordParameters", NULL };
    PyObject *listOfArguments, *keywordArguments, *returnType, *results, *name;
    udt_Variable *var;

    // parse arguments
    listOfArguments = keywordArguments = NULL;
    if (!PyArg_ParseTupleAndKeywords(args, keywordArgs, "OO|OO", keywordList,
            &name, &returnType, &listOfArguments, &keywordArguments))
        return NULL;

    // create the return variable
    var = Variable_NewByType(self, returnType, 1);
    if (!var)
        return NULL;

    // call the function
    if (Cursor_Call(self, var, name, listOfArguments, keywordArguments) < 0)
        return NULL;

    // determine the results
    results = Variable_GetValue(var, 0);
    Py_DECREF(var);
    return results;
}


//-----------------------------------------------------------------------------
// Cursor_CallProc()
//   Call a stored procedure and return the (possibly modified) arguments.
//-----------------------------------------------------------------------------
static PyObject *Cursor_CallProc(udt_Cursor *self, PyObject *args,
        PyObject *keywordArgs)
{
    static char *keywordList[] = { "name", "parameters", "keywordParameters",
            NULL };
    PyObject *listOfArguments, *keywordArguments, *results, *var, *temp, *name;
    Py_ssize_t numArgs, i;

    // parse arguments
    listOfArguments = keywordArguments = NULL;
    if (!PyArg_ParseTupleAndKeywords(args, keywordArgs, "O|OO", keywordList,
            &name, &listOfArguments, &keywordArguments))
        return NULL;

    // call the stored procedure
    if (Cursor_Call(self, NULL, name, listOfArguments, keywordArguments) < 0)
        return NULL;

    // create the return value
    numArgs = PyList_GET_SIZE(self->bindVariables);
    results = PyList_New(numArgs);
    if (!results)
        return NULL;
    for (i = 0; i < numArgs; i++) {
        var = PyList_GET_ITEM(self->bindVariables, i);
        temp = Variable_GetValue((udt_Variable*) var, 0);
        if (!temp) {
            Py_DECREF(results);
            return NULL;
        }
        PyList_SET_ITEM(results, i, temp);
    }

    return results;
}


//-----------------------------------------------------------------------------
// Cursor_Execute()
//   Execute the statement.
//-----------------------------------------------------------------------------
static PyObject *Cursor_Execute(udt_Cursor *self, PyObject *args,
        PyObject *keywordArgs)
{
    PyObject *statement, *executeArgs;
    uint32_t numQueryColumns, mode;
    int status;

    executeArgs = NULL;
    if (!PyArg_ParseTuple(args, "O|O", &statement, &executeArgs))
        return NULL;
    if (executeArgs && keywordArgs) {
        if (PyDict_Size(keywordArgs) == 0)
            keywordArgs = NULL;
        else {
            PyErr_SetString(g_InterfaceErrorException,
                    "expecting argument or keyword arguments, not both");
            return NULL;
        }
    }
    if (keywordArgs)
        executeArgs = keywordArgs;
    if (executeArgs) {
        if (!PyDict_Check(executeArgs) && !PySequence_Check(executeArgs)) {
            PyErr_SetString(PyExc_TypeError,
                    "expecting a dictionary, sequence or keyword args");
            return NULL;
        }
    }

    // make sure the cursor is open
    if (Cursor_IsOpen(self) < 0)
        return NULL;

    // prepare the statement, if applicable
    if (Cursor_InternalPrepare(self, statement, NULL) < 0)
        return NULL;

    // perform binds
    if (executeArgs && Cursor_SetBindVariables(self, executeArgs, 1, 0,
            0) < 0)
        return NULL;
    if (Cursor_PerformBind(self) < 0)
        return NULL;

    // execute the statement
    Py_BEGIN_ALLOW_THREADS
    mode = (self->connection->autocommit) ? DPI_MODE_EXEC_COMMIT_ON_SUCCESS :
            DPI_MODE_EXEC_DEFAULT;
    status = dpiStmt_execute(self->handle, mode, &numQueryColumns);
    Py_END_ALLOW_THREADS
    if (status < 0)
        return Error_RaiseAndReturnNull();

    // get the count of the rows affected
    if (dpiStmt_getRowCount(self->handle, &self->rowCount) < 0)
        return Error_RaiseAndReturnNull();

    // perform defines, if necessary
    if (numQueryColumns > 0) {
        if (Cursor_PerformDefine(self, numQueryColumns) < 0) {
            Py_CLEAR(self->fetchVariables);
            return NULL;
        }
    }

    // for queries, return the cursor for convenience
    if (numQueryColumns > 0) {
        Py_INCREF(self);
        return (PyObject*) self;
    }

    // for returning statements, get the variable data for each bound variable
    if (self->stmtInfo.isReturning && Cursor_GetVarData(self) < 0)
        return NULL;

    // for statements other than queries, simply return None
    Py_RETURN_NONE;
}


//-----------------------------------------------------------------------------
// Cursor_ExecuteMany()
//   Execute the statement many times. The number of times is equivalent to the
// number of elements in the array of dictionaries.
//-----------------------------------------------------------------------------
static PyObject *Cursor_ExecuteMany(udt_Cursor *self, PyObject *args,
        PyObject *keywordArgs)
{
    static char *keywordList[] = { "statement", "parameters", "batcherrors",
            "arraydmlrowcounts", NULL };
    int arrayDMLRowCountsEnabled = 0, batchErrorsEnabled = 0;
    PyObject *arguments, *listOfArguments, *statement;
    uint32_t mode, i, numRows;
    int status;

    // expect statement text (optional) plus list of sequences/mappings
    if (!PyArg_ParseTupleAndKeywords(args, keywordArgs, "OO!|ii", keywordList,
            &statement, &PyList_Type, &listOfArguments,
            &batchErrorsEnabled, &arrayDMLRowCountsEnabled))
        return NULL;

    // make sure the cursor is open
    if (Cursor_IsOpen(self) < 0)
        return NULL;

    // determine execution mode
    mode = (self->connection->autocommit) ? DPI_MODE_EXEC_COMMIT_ON_SUCCESS :
            DPI_MODE_EXEC_DEFAULT;
    if (batchErrorsEnabled)
        mode |= DPI_MODE_EXEC_BATCH_ERRORS;
    if (arrayDMLRowCountsEnabled)
        mode |= DPI_MODE_EXEC_ARRAY_DML_ROWCOUNTS;

    // prepare the statement
    if (Cursor_InternalPrepare(self, statement, NULL) < 0)
        return NULL;

    // perform binds
    numRows = (uint32_t) PyList_GET_SIZE(listOfArguments);
    for (i = 0; i < numRows; i++) {
        arguments = PyList_GET_ITEM(listOfArguments, i);
        if (!PyDict_Check(arguments) && !PySequence_Check(arguments)) {
            PyErr_SetString(g_InterfaceErrorException,
                    "expecting a list of dictionaries or sequences");
            return NULL;
        }
        if (Cursor_SetBindVariables(self, arguments, numRows, i,
                (i < numRows - 1)) < 0)
            return NULL;
    }
    if (Cursor_PerformBind(self) < 0)
        return NULL;

    // execute the statement, but only if the number of rows is greater than
    // zero since Oracle raises an error otherwise
    if (numRows > 0) {
        Py_BEGIN_ALLOW_THREADS
        status = dpiStmt_executeMany(self->handle, mode, numRows);
        Py_END_ALLOW_THREADS
        if (status < 0) {
            Error_RaiseAndReturnNull();
            dpiStmt_getRowCount(self->handle, &self->rowCount);
            return NULL;
        }
        if (dpiStmt_getRowCount(self->handle, &self->rowCount) < 0)
            return Error_RaiseAndReturnNull();
    }

    Py_RETURN_NONE;
}


//-----------------------------------------------------------------------------
// Cursor_ExecuteManyPrepared()
//   Execute the prepared statement the number of times requested. At this
// point, the statement must have been already prepared and the bind variables
// must have their values set.
//-----------------------------------------------------------------------------
static PyObject *Cursor_ExecuteManyPrepared(udt_Cursor *self, PyObject *args)
{
    int numIters, status;

    // expect number of times to execute the statement
    if (!PyArg_ParseTuple(args, "i", &numIters))
        return NULL;

    // make sure the cursor is open
    if (Cursor_IsOpen(self) < 0)
        return NULL;

    // perform binds
    if (Cursor_PerformBind(self) < 0)
        return NULL;

    // execute the statement
    Py_BEGIN_ALLOW_THREADS
    status = dpiStmt_executeMany(self->handle, DPI_MODE_EXEC_DEFAULT,
            numIters);
    Py_END_ALLOW_THREADS
    if (status < 0 || dpiStmt_getRowCount(self->handle, &self->rowCount) < 0)
        return Error_RaiseAndReturnNull();

    Py_RETURN_NONE;
}


//-----------------------------------------------------------------------------
// Cursor_MultiFetch()
//   Return a list consisting of the remaining rows up to the given row limit
// (if specified).
//-----------------------------------------------------------------------------
static PyObject *Cursor_MultiFetch(udt_Cursor *self, int rowLimit)
{
    uint32_t bufferRowIndex = 0;
    PyObject *results, *row;
    int found, rowNum;

    // verify fetch can be performed
    if (Cursor_VerifyFetch(self) < 0)
        return NULL;

    // create an empty list
    results = PyList_New(0);
    if (!results)
        return NULL;

    // fetch as many rows as possible
    for (rowNum = 0; rowLimit == 0 || rowNum < rowLimit; rowNum++) {
        if (Cursor_FetchRow(self, &found, &bufferRowIndex) < 0) {
            Py_DECREF(results);
            return NULL;
        }
        if (!found)
            break;
        row = Cursor_CreateRow(self, bufferRowIndex);
        if (!row) {
            Py_DECREF(results);
            return NULL;
        }
        if (PyList_Append(results, row) < 0) {
            Py_DECREF(row);
            Py_DECREF(results);
            return NULL;
        }
        Py_DECREF(row);
    }

    return results;
}


//-----------------------------------------------------------------------------
// Cursor_FetchOne()
//   Fetch a single row from the cursor.
//-----------------------------------------------------------------------------
static PyObject *Cursor_FetchOne(udt_Cursor *self, PyObject *args)
{
    uint32_t bufferRowIndex = 0;
    int found = 0;

    if (Cursor_VerifyFetch(self) < 0)
        return NULL;
    if (Cursor_FetchRow(self, &found, &bufferRowIndex) < 0)
        return NULL;
    if (found)
        return Cursor_CreateRow(self, bufferRowIndex);

    Py_RETURN_NONE;
}


//-----------------------------------------------------------------------------
// Cursor_FetchMany()
//   Fetch multiple rows from the cursor based on the arraysize.
//-----------------------------------------------------------------------------
static PyObject *Cursor_FetchMany(udt_Cursor *self, PyObject *args,
        PyObject *keywordArgs)
{
    static char *keywordList[] = { "numRows", NULL };
    int rowLimit;

    // parse arguments -- optional rowlimit expected
    rowLimit = self->arraySize;
    if (!PyArg_ParseTupleAndKeywords(args, keywordArgs, "|i", keywordList,
            &rowLimit))
        return NULL;

    return Cursor_MultiFetch(self, rowLimit);
}


//-----------------------------------------------------------------------------
// Cursor_FetchAll()
//   Fetch all remaining rows from the cursor.
//-----------------------------------------------------------------------------
static PyObject *Cursor_FetchAll(udt_Cursor *self, PyObject *args)
{
    return Cursor_MultiFetch(self, 0);
}


//-----------------------------------------------------------------------------
// Cursor_FetchRaw()
//   Perform raw fetch on the cursor; return the actual number of rows fetched.
//-----------------------------------------------------------------------------
static PyObject *Cursor_FetchRaw(udt_Cursor *self, PyObject *args,
        PyObject *keywordArgs)
{
    static char *keywordList[] = { "numRows", NULL };
    uint32_t numRowsToFetch, numRowsFetched, bufferRowIndex;
    int moreRows;

    // expect an optional number of rows to retrieve
    numRowsToFetch = self->fetchArraySize;
    if (!PyArg_ParseTupleAndKeywords(args, keywordArgs, "|i", keywordList,
            &numRowsToFetch))
        return NULL;
    if (numRowsToFetch > self->fetchArraySize) {
        PyErr_SetString(g_InterfaceErrorException,
                "rows to fetch exceeds array size");
        return NULL;
    }

    // perform the fetch
    if (dpiStmt_fetchRows(self->handle, numRowsToFetch, &bufferRowIndex,
            &numRowsFetched, &moreRows) < 0)
        return Error_RaiseAndReturnNull();
    self->rowCount += numRowsFetched;
    self->numRowsInFetchBuffer = 0;
    return PyInt_FromLong(numRowsFetched);
}


//-----------------------------------------------------------------------------
// Cursor_Scroll()
//   Scroll the cursor using the value and mode specified.
//-----------------------------------------------------------------------------
static PyObject *Cursor_Scroll(udt_Cursor *self, PyObject *args,
        PyObject *keywordArgs)
{
    static char *keywordList[] = { "value", "mode", NULL };
    dpiFetchMode mode;
    int32_t offset;
    char *strMode;
    int status;

    // parse arguments
    offset = 0;
    strMode = NULL;
    if (!PyArg_ParseTupleAndKeywords(args, keywordArgs, "|is", keywordList,
            &offset, &strMode))
        return NULL;

    // validate mode
    if (!strMode)
        mode = DPI_MODE_FETCH_RELATIVE;
    else if (strcmp(strMode, "relative") == 0)
        mode = DPI_MODE_FETCH_RELATIVE;
    else if (strcmp(strMode, "absolute") == 0)
        mode = DPI_MODE_FETCH_ABSOLUTE;
    else if (strcmp(strMode, "first") == 0)
        mode = DPI_MODE_FETCH_FIRST;
    else if (strcmp(strMode, "last") == 0)
        mode = DPI_MODE_FETCH_LAST;
    else {
        PyErr_SetString(g_InterfaceErrorException,
                "mode must be one of relative, absolute, first or last");
        return NULL;
    }

    // make sure the cursor is open
    if (Cursor_IsOpen(self) < 0)
        return NULL;

    // perform scroll and get new row count and number of rows in buffer
    Py_BEGIN_ALLOW_THREADS
    status = dpiStmt_scroll(self->handle, mode, offset,
            0 - self->numRowsInFetchBuffer);
    if (status == 0)
        status = dpiStmt_fetchRows(self->handle, self->fetchArraySize,
                &self->fetchBufferRowIndex, &self->numRowsInFetchBuffer,
                &self->moreRowsToFetch);
    if (status == 0)
        status = dpiStmt_getRowCount(self->handle, &self->rowCount);
    Py_END_ALLOW_THREADS
    if (status < 0)
        return Error_RaiseAndReturnNull();
    self->rowCount -= self->numRowsInFetchBuffer;

    Py_RETURN_NONE;
}


//-----------------------------------------------------------------------------
// Cursor_SetInputSizes()
//   Set the sizes of the bind variables.
//-----------------------------------------------------------------------------
static PyObject *Cursor_SetInputSizes(udt_Cursor *self, PyObject *args,
        PyObject *keywordArgs)
{
    Py_ssize_t numPositionalArgs, i;
    PyObject *key, *value;
    udt_Variable *var;

    // only expect keyword arguments or positional arguments, not both
    numPositionalArgs = PyTuple_Size(args);
    if (keywordArgs && numPositionalArgs > 0) {
        PyErr_SetString(g_InterfaceErrorException,
                "expecting arguments or keyword arguments, not both");
        return NULL;
    }

    // make sure the cursor is open
    if (Cursor_IsOpen(self) < 0)
        return NULL;

    // eliminate existing bind variables
    Py_CLEAR(self->bindVariables);
    if (keywordArgs)
        self->bindVariables = PyDict_New();
    else self->bindVariables = PyList_New(numPositionalArgs);
    if (!self->bindVariables)
        return NULL;
    self->setInputSizes = 1;

    // process each input
    if (keywordArgs) {
        i = 0;
        while (PyDict_Next(keywordArgs, &i, &key, &value)) {
            var = Variable_NewByType(self, value, self->bindArraySize);
            if (!var)
                return NULL;
            if (PyDict_SetItem(self->bindVariables, key,
                    (PyObject*) var) < 0) {
                Py_DECREF(var);
                return NULL;
            }
            Py_DECREF(var);
        }
    } else {
        for (i = 0; i < numPositionalArgs; i++) {
            value = PyTuple_GET_ITEM(args, i);
            if (value == Py_None) {
                Py_INCREF(Py_None);
                PyList_SET_ITEM(self->bindVariables, i, Py_None);
            } else {
                var = Variable_NewByType(self, value, self->bindArraySize);
                if (!var)
                    return NULL;
                PyList_SET_ITEM(self->bindVariables, i, (PyObject*) var);
            }
        }
    }

    Py_INCREF(self->bindVariables);
    return self->bindVariables;
}


//-----------------------------------------------------------------------------
// Cursor_SetOutputSize()
//   Does nothing as ODPI-C handles long columns dynamically without the need
// to specify a maximum length.
//-----------------------------------------------------------------------------
static PyObject *Cursor_SetOutputSize(udt_Cursor *self, PyObject *args)
{
    int outputSize, outputSizeColumn;

    if (!PyArg_ParseTuple(args, "i|i", &outputSize, &outputSizeColumn))
        return NULL;
    Py_RETURN_NONE;
}


//-----------------------------------------------------------------------------
// Cursor_Var()
//   Create a bind variable and return it.
//-----------------------------------------------------------------------------
static PyObject *Cursor_Var(udt_Cursor *self, PyObject *args,
        PyObject *keywordArgs)
{
    static char *keywordList[] = { "type", "size", "arraysize",
            "inconverter", "outconverter", "typename", NULL };
    PyObject *inConverter, *outConverter, *typeNameObj;
    udt_ObjectType *objType = NULL;
    udt_VariableType *varType;
    int size, arraySize;
    udt_Variable *var;
    PyObject *type;

    // parse arguments
    size = 0;
    arraySize = self->bindArraySize;
    inConverter = outConverter = typeNameObj = NULL;
    if (!PyArg_ParseTupleAndKeywords(args, keywordArgs, "O|iiOOO", keywordList,
            &type, &size, &arraySize, &inConverter, &outConverter,
            &typeNameObj))
        return NULL;

    // determine the type of variable
    varType = Variable_TypeByPythonType(self, type);
    if (!varType)
        return NULL;
    if (size == 0)
        size = varType->size;
    if (typeNameObj) {
        objType = ObjectType_NewByName(self->connection, typeNameObj);
        if (!objType)
            return NULL;
    }

    // create the variable
    var = Variable_New(self, arraySize, varType, size, 0, objType);
    Py_XDECREF(objType);
    if (!var)
        return NULL;
    Py_XINCREF(inConverter);
    var->inConverter = inConverter;
    Py_XINCREF(outConverter);
    var->outConverter = outConverter;

    return (PyObject*) var;
}


//-----------------------------------------------------------------------------
// Cursor_ArrayVar()
//   Create an array bind variable and return it.
//-----------------------------------------------------------------------------
static PyObject *Cursor_ArrayVar(udt_Cursor *self, PyObject *args)
{
    uint32_t size, numElements;
    udt_VariableType *varType;
    PyObject *type, *value;
    udt_Variable *var;

    // parse arguments
    size = 0;
    if (!PyArg_ParseTuple(args, "OO|i", &type, &value, &size))
        return NULL;

    // determine the type of variable
    varType = Variable_TypeByPythonType(self, type);
    if (!varType)
        return NULL;
    if (size == 0)
        size = varType->size;

    // determine the number of elements to create
    if (PyList_Check(value))
        numElements = (uint32_t) PyList_GET_SIZE(value);
    else if (PyInt_Check(value)) {
        numElements = (uint32_t) PyInt_AsLong(value);
        if (PyErr_Occurred())
            return NULL;
    } else {
        PyErr_SetString(PyExc_TypeError,
                "expecting integer or list of values");
        return NULL;
    }

    // create the variable
    var = Variable_New(self, numElements, varType, size, 1, NULL);
    if (!var)
        return NULL;

    // set the value, if applicable
    if (PyList_Check(value)) {
        if (Variable_SetArrayValue(var, value) < 0)
            return NULL;
    }

    return (PyObject*) var;
}


//-----------------------------------------------------------------------------
// Cursor_BindNames()
//   Return a list of bind variable names.
//-----------------------------------------------------------------------------
static PyObject *Cursor_BindNames(udt_Cursor *self, PyObject *args)
{
    uint32_t numBinds, *nameLengths, i;
    PyObject *namesList, *temp;
    const char **names;

    // make sure the cursor is open
    if (Cursor_IsOpen(self) < 0)
        return NULL;

    // ensure that a statement has already been prepared
    if (!self->statement) {
        PyErr_SetString(g_ProgrammingErrorException,
                "statement must be prepared first");
        return NULL;
    }

    // determine the number of binds
    if (dpiStmt_getBindCount(self->handle, &numBinds) < 0)
        return Error_RaiseAndReturnNull();

    // if the number of binds is zero, nothing to do
    if (numBinds == 0)
        return PyList_New(0);

    // allocate memory for the bind names and their lengths
    names = (const char**) PyMem_Malloc(numBinds * sizeof(char*));
    if (!names)
        return PyErr_NoMemory();
    nameLengths = (uint32_t*) PyMem_Malloc(numBinds * sizeof(uint32_t));
    if (!nameLengths) {
        PyMem_Free((void*) names);
        return PyErr_NoMemory();
    }

    // get the bind names
    if (dpiStmt_getBindNames(self->handle, &numBinds, names,
            nameLengths) < 0) {
        PyMem_Free((void*) names);
        PyMem_Free(nameLengths);
        return Error_RaiseAndReturnNull();
    }

    // populate list with the results
    namesList = PyList_New(numBinds);
    if (namesList) {
        for (i = 0; i < numBinds; i++) {
            temp = cxString_FromEncodedString(names[i], nameLengths[i],
                    self->connection->encodingInfo.encoding);
            if (!temp) {
                Py_CLEAR(namesList);
                break;
            }
            PyList_SET_ITEM(namesList, i, temp);
        }
    }
    PyMem_Free((void*) names);
    PyMem_Free(nameLengths);
    return namesList;
}


//-----------------------------------------------------------------------------
// Cursor_GetIter()
//   Return a reference to the cursor which supports the iterator protocol.
//-----------------------------------------------------------------------------
static PyObject *Cursor_GetIter(udt_Cursor *self)
{
    if (Cursor_VerifyFetch(self) < 0)
        return NULL;
    Py_INCREF(self);
    return (PyObject*) self;
}


//-----------------------------------------------------------------------------
// Cursor_GetNext()
//   Return a reference to the cursor which supports the iterator protocol.
//-----------------------------------------------------------------------------
static PyObject *Cursor_GetNext(udt_Cursor *self)
{
    uint32_t bufferRowIndex = 0;
    int found = 0;

    if (Cursor_VerifyFetch(self) < 0)
        return NULL;
    if (Cursor_FetchRow(self, &found, &bufferRowIndex) < 0)
        return NULL;
    if (found)
        return Cursor_CreateRow(self, bufferRowIndex);

    // no more rows, return NULL without setting an exception
    return NULL;
}


//-----------------------------------------------------------------------------
// Cursor_GetVarData()
//   Get the data for all variables bound to the cursor. This is needed for a
// returning statement which may have changed the number of elements in the
// variable and the location of the variable data.
//-----------------------------------------------------------------------------
static int Cursor_GetVarData(udt_Cursor *self)
{
    Py_ssize_t i, size, pos;
    PyObject *key, *value;
    udt_Variable *var;

    // if there are no bind variables, nothing to do
    if (!self->bindVariables)
        return 0;

    // handle bind by position
    if (PyList_Check(self->bindVariables)) {
        size = PyList_GET_SIZE(self->bindVariables);
        for (i = 0; i < size; i++) {
            var = (udt_Variable*) PyList_GET_ITEM(self->bindVariables, i);
            if (dpiVar_getData(var->handle, &var->allocatedElements,
                    &var->data) < 0)
                return Error_RaiseAndReturnInt();
        }

    // handle bind by name
    } else {
        pos = 0;
        while (PyDict_Next(self->bindVariables, &pos, &key, &value)) {
            var = (udt_Variable*) value;
            if (dpiVar_getData(var->handle, &var->allocatedElements,
                    &var->data) < 0)
                return Error_RaiseAndReturnInt();
        }
    }

    return 0;
}




//-----------------------------------------------------------------------------
// Cursor_GetBatchErrors()
//    Returns a list of batch error objects.
//-----------------------------------------------------------------------------
static PyObject* Cursor_GetBatchErrors(udt_Cursor *self)
{
    uint32_t numErrors, i;
    dpiErrorInfo *errors;
    PyObject *result;
    udt_Error *error;

    // determine the number of errors
    if (dpiStmt_getBatchErrorCount(self->handle, &numErrors) < 0)
        return Error_RaiseAndReturnNull();
    if (numErrors == 0)
        return PyList_New(0);

    // allocate memory for the errors
    errors = PyMem_Malloc(numErrors * sizeof(dpiErrorInfo));
    if (!errors)
        return PyErr_NoMemory();

    // get error information
    if (dpiStmt_getBatchErrors(self->handle, numErrors, errors) < 0) {
        PyMem_Free(errors);
        return Error_RaiseAndReturnNull();
    }

    // create result
    result = PyList_New(numErrors);
    if (result) {
        for (i = 0; i < numErrors; i++) {
            error = Error_InternalNew(&errors[i]);
            if (!error) {
                Py_CLEAR(result);
                break;
            }
            PyList_SET_ITEM(result, i, (PyObject*) error);
        }
    }
    PyMem_Free(errors);
    return result;
}


//-----------------------------------------------------------------------------
// Cursor_GetArrayDMLRowCounts
//    Populates the array dml row count list.
//-----------------------------------------------------------------------------
static PyObject* Cursor_GetArrayDMLRowCounts(udt_Cursor *self)
{
    PyObject *result, *element;
    uint32_t numRowCounts, i;
    uint64_t *rowCounts;

    // get row counts from DPI
    if (dpiStmt_getRowCounts(self->handle, &numRowCounts, &rowCounts) < 0)
        return Error_RaiseAndReturnNull();

    // return array
    result = PyList_New(numRowCounts);
    if (!result)
        return NULL;
    for (i = 0; i < numRowCounts; i++) {
        element = PyLong_FromUnsignedLong((unsigned long) rowCounts[i]);
        if (!element) {
            Py_DECREF(result);
            return NULL;
        }
        PyList_SET_ITEM(result, i, element);
    }

    return result;
}


//-----------------------------------------------------------------------------
// Cursor_GetImplicitResults
//   Return a list of cursors available implicitly after execution of a PL/SQL
// block or stored procedure. If none are available, an empty list is returned.
//-----------------------------------------------------------------------------
static PyObject * Cursor_GetImplicitResults(udt_Cursor *self)
{
    udt_Cursor *childCursor;
    dpiStmt *childStmt;
    PyObject *result;

    // make sure the cursor is open
    if (Cursor_IsOpen(self) < 0)
        return NULL;

    // make sure we have a statement executed (handle defined)
    if (!self->handle) {
        PyErr_SetString(g_InterfaceErrorException, "no statement executed");
        return NULL;
    }

    // create result
    result = PyList_New(0);
    if (!result)
        return NULL;
    while (1) {
        if (dpiStmt_getImplicitResult(self->handle, &childStmt) < 0)
            return Error_RaiseAndReturnNull();
        if (!childStmt)
            break;
        childCursor = (udt_Cursor*) PyObject_CallMethod(
                (PyObject*) self->connection, "cursor", NULL);
        if (!childCursor) {
            dpiStmt_release(childStmt);
            Py_DECREF(result);
            return NULL;
        }
        childCursor->handle = childStmt;
        childCursor->fixupRefCursor = 1;
        if (PyList_Append(result, (PyObject*) childCursor) < 0) {
            Py_DECREF(result);
            Py_DECREF(childCursor);
            return NULL;
        }
        Py_DECREF(childCursor);
    }

    return result;
}

