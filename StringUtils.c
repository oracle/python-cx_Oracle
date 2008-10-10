//-----------------------------------------------------------------------------
// StringUtils.c
//   Defines constants and routines specific to handling strings.
//-----------------------------------------------------------------------------

typedef struct {
    char *ptr;
    Py_ssize_t size;
#if defined(WITH_UNICODE) && defined(Py_UNICODE_WIDE)
    PyObject *encodedString;
#endif
} udt_StringBuffer;


//-----------------------------------------------------------------------------
// StringBuffer_Fill()
//   Fill the string buffer with the UTF-16 data that Oracle expects.
//-----------------------------------------------------------------------------
static int StringBuffer_Fill(
    udt_StringBuffer *buf,              // buffer to fill
    PyObject *obj)                      // object to fill buffer with
{
    if (!obj) {
        buf->ptr = NULL;
        buf->size = 0;
#if defined(WITH_UNICODE) && defined(Py_UNICODE_WIDE)
        buf->encodedString = NULL;
#endif
        return 0;
    }
#ifdef WITH_UNICODE
    #ifdef Py_UNICODE_WIDE
        int one = 1;
        int byteOrder = (IS_LITTLE_ENDIAN) ? -1 : 1;
        buf->encodedString = PyUnicode_EncodeUTF16(PyUnicode_AS_UNICODE(obj),
                PyUnicode_GET_SIZE(obj), NULL, byteOrder);
        if (!buf->encodedString)
            return -1;
        buf->ptr = PyString_AS_STRING(buf->encodedString);
        buf->size = PyString_GET_SIZE(buf->encodedString);
    #else
        buf->ptr = (char*) PyUnicode_AS_UNICODE(obj);
        buf->size = PyUnicode_GET_DATA_SIZE(obj);
    #endif
#else
    buf->ptr = PyString_AS_STRING(obj);
    buf->size = PyString_GET_SIZE(obj);
#endif
    return 0;
}


#ifdef WITH_UNICODE
    #define CXORA_CHARSETID             OCI_UTF16ID
    #define CXORA_ERROR_TEXT_LENGTH     2048
    #define CXORA_STRING_TYPE           &PyUnicode_Type
    #ifdef Py_UNICODE_WIDE
        #define StringBuffer_CLEAR(buffer) \
            Py_XDECREF((buffer)->encodedString)
        #define CXORA_TO_STRING_OBJ(buffer, chars) \
            PyUnicode_DecodeUTF16(buffer, (chars) * 2, NULL, NULL)
    #else
        #define StringBuffer_CLEAR(buffer)
        #define CXORA_TO_STRING_OBJ(buffer, chars) \
            PyUnicode_FromUnicode((Py_UNICODE*) (buffer), chars)
    #endif
#else
    #define CXORA_CHARSETID             0
    #define CXORA_ERROR_TEXT_LENGTH     1024
    #define CXORA_STRING_TYPE           &PyString_Type
    #define StringBuffer_CLEAR(buffer)
    #define CXORA_TO_STRING_OBJ(buffer, chars) \
        PyString_FromStringAndSize(buffer, chars)
#endif

