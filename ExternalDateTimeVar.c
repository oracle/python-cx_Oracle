//-----------------------------------------------------------------------------
// ExternalDateTimeVar.c
//   Defines the routines for handling date variables external to this module.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// external date/time type
//-----------------------------------------------------------------------------
typedef struct {
    PyObject_HEAD
    unsigned year;
    unsigned month;
    unsigned day;
    unsigned hour;
    unsigned minute;
    unsigned second;
    unsigned fsecond;
} udt_ExternalDateTimeVar;


//-----------------------------------------------------------------------------
// Declaration of external date variable functions.
//-----------------------------------------------------------------------------
static void ExternalDateTimeVar_Free(udt_ExternalDateTimeVar*);
static PyObject *ExternalDateTimeVar_Str(udt_ExternalDateTimeVar*);
static int ExternalDateTimeVar_Cmp(udt_ExternalDateTimeVar*,
        udt_ExternalDateTimeVar*);
static PyObject *ExternalDateTimeVar_New(PyTypeObject*, PyObject*, PyObject*);
static PyObject *ExternalDateTimeVar_Reduce(udt_ExternalDateTimeVar*);


//-----------------------------------------------------------------------------
// declaration of members for Python type
//-----------------------------------------------------------------------------
static PyMemberDef g_ExternalDateTimeVarMembers[] = {
    { "year", T_INT, offsetof(udt_ExternalDateTimeVar, year), READONLY },
    { "month", T_INT, offsetof(udt_ExternalDateTimeVar, month), READONLY },
    { "day", T_INT, offsetof(udt_ExternalDateTimeVar, day), READONLY },
    { "hour", T_INT, offsetof(udt_ExternalDateTimeVar, hour), READONLY },
    { "minute", T_INT, offsetof(udt_ExternalDateTimeVar, minute), READONLY },
    { "second", T_INT, offsetof(udt_ExternalDateTimeVar, second), READONLY },
    { "fsecond", T_INT, offsetof(udt_ExternalDateTimeVar, fsecond), READONLY },
    { NULL }
};


//-----------------------------------------------------------------------------
// declaration of methods for Python type
//-----------------------------------------------------------------------------
static PyMethodDef g_ExternalDateTimeVarMethods[] = {
    {"__reduce__", (PyCFunction) ExternalDateTimeVar_Reduce, METH_NOARGS },
    { NULL, NULL }
};


//-----------------------------------------------------------------------------
// Python type declaration
//-----------------------------------------------------------------------------
static PyTypeObject g_ExternalDateTimeVarType = {
    PyObject_HEAD_INIT(NULL)
    0,					// ob_size
    "cx_Oracle.Timestamp",		// tp_name
    sizeof(udt_ExternalDateTimeVar),	// tp_basicsize
    0,					// tp_itemsize
    (destructor) ExternalDateTimeVar_Free,
                                        // tp_dealloc
    0,					// tp_print
    0,					// tp_getattr
    0,					// tp_setattr
    (cmpfunc) ExternalDateTimeVar_Cmp,	// tp_compare
    (reprfunc) ExternalDateTimeVar_Str,	// tp_repr
    0,					// tp_as_number
    0,					// tp_as_sequence
    0,					// tp_as_mapping
    0,					// tp_hash
    0,					// tp_call
    (reprfunc) ExternalDateTimeVar_Str,	// tp_str
                                        // tp_getattro
    0,					// tp_getattro
    0,					// tp_setattro
    0,					// tp_as_buffer
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
                                        // tp_flags
    0,					// tp_doc
    0,					// tp_traverse
    0,					// tp_clear
    0,					// tp_richcompare
    0,					// tp_weaklistoffset
    0,					// tp_iter
    0,					// tp_iternext
    g_ExternalDateTimeVarMethods,	// tp_methods
    g_ExternalDateTimeVarMembers,	// tp_members
    0,					// tp_getset
    0,					// tp_base
    0,					// tp_dict
    0,					// tp_descr_get
    0,					// tp_descr_set
    0,					// tp_dictoffset
    0,					// tp_init
    0,                                  // tp_alloc
    ExternalDateTimeVar_New,		// tp_new
    0,                                  // tp_free
    0,                                  // tp_is_gc
    0                                   // tp_bases
};


//-----------------------------------------------------------------------------
// ExternalDateTimeVar_NewFromC()
//   Create a new external date variable from C code.
//-----------------------------------------------------------------------------
PyObject *ExternalDateTimeVar_NewFromC(
    PyTypeObject *type,			// type of object
    unsigned year,			// year
    unsigned month,			// month
    unsigned day,			// day
    unsigned hour,			// hour
    unsigned minute,			// minute
    unsigned second,			// second
    unsigned fsecond)			// fractional seconds
{
    udt_ExternalDateTimeVar *var;

    var = (udt_ExternalDateTimeVar*) type->tp_alloc(type, 0);
    if (var) {
        var->year = year;
        var->month = month;
        var->day = day;
        var->hour = hour;
        var->minute = minute;
        var->second = second;
        var->fsecond = fsecond;
    }

    return (PyObject*) var;
}


//-----------------------------------------------------------------------------
// ExternalDateTimeVar_New()
//   Create a new cursor object.
//-----------------------------------------------------------------------------
static PyObject *ExternalDateTimeVar_New(
    PyTypeObject *type,                 // type object
    PyObject *args,                     // arguments
    PyObject *keywordArgs)              // keyword arguments
{
    unsigned year, month, day, hour, minute, second, fsecond;

    hour = minute = second = fsecond = 0;
    if (!PyArg_ParseTuple(args, "iii|iiii", &year, &month, &day, &hour,
            &minute, &second, &fsecond))
        return NULL;
    return ExternalDateTimeVar_NewFromC(type, year, month, day, hour, minute,
            second, fsecond);
}


//-----------------------------------------------------------------------------
// ExternalDateTimeVar_Reduce()
//   Provide information for pickling and unpickling.
//-----------------------------------------------------------------------------
static PyObject* ExternalDateTimeVar_Reduce(
    udt_ExternalDateTimeVar* self)	// object to pickle
{
    return Py_BuildValue("(O, (iiiiiii))", &g_ExternalDateTimeVarType,
            self->year, self->month, self->day, self->hour, self->minute,
            self->second, self->fsecond);
}


//-----------------------------------------------------------------------------
// ExternalDateTimeVar_Free()
//   Free an external date variable.
//-----------------------------------------------------------------------------
static void ExternalDateTimeVar_Free(
    udt_ExternalDateTimeVar *var)	// variable to free
{
    PyObject_DEL(var);
}


//-----------------------------------------------------------------------------
// ExternalDateTimeVar_Str()
//   Return the string representation of the external date variable object.
//-----------------------------------------------------------------------------
static PyObject *ExternalDateTimeVar_Str(
    udt_ExternalDateTimeVar *var)	// external date variable object
{
    char value[100];

    sprintf(value, "%.4d-%.2d-%.2d %.2d:%.2d:%.2d", var->year,
            var->month, var->day, var->hour, var->minute, var->second);
    if (var->fsecond > 0)
        sprintf(value + strlen(value), ".%.6d", var->fsecond);
    return PyString_FromString(value);
}

//-----------------------------------------------------------------------------
// ExternalDateTimeVar_Cmp()
//   Return -1 if the second date is less than the first; 0 if the dates are
// the same and +1 if the second date is greater than the first.
//-----------------------------------------------------------------------------
static int ExternalDateTimeVar_Cmp(
    udt_ExternalDateTimeVar *var1,	// first date
    udt_ExternalDateTimeVar *var2)	// second date
{
    if (var1->year < var2->year)
        return -1;
    if (var1->year > var2->year)
        return 1;
    if (var1->month < var2->month)
        return -1;
    if (var1->month > var2->month)
        return 1;
    if (var1->day < var2->day)
        return -1;
    if (var1->day > var2->day)
        return 1;
    if (var1->hour < var2->hour)
        return -1;
    if (var1->hour > var2->hour)
        return 1;
    if (var1->minute < var2->minute)
        return -1;
    if (var1->minute > var2->minute)
        return 1;
    if (var1->second < var2->second)
        return -1;
    if (var1->second > var2->second)
        return 1;
    if (var1->fsecond < var2->fsecond)
        return -1;
    if (var1->fsecond > var2->fsecond)
        return 1;

    return 0;
}

