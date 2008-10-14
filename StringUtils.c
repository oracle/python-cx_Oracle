//-----------------------------------------------------------------------------
// StringUtils.c
//   Defines constants and routines specific to handling strings.
//-----------------------------------------------------------------------------

// define structure for abstracting string buffers
typedef struct {
    char *ptr;
    Py_ssize_t size;
#if defined(WITH_UNICODE) && defined(Py_UNICODE_WIDE)
    PyObject *encodedString;
#endif
} udt_StringBuffer;


// use the bytes methods in cx_Oracle and define them as the equivalent string
// type methods as is done in Python 2.6
#ifndef PyBytes_Check
    #define PyBytes_Type                PyString_Type
    #define PyBytes_AS_STRING           PyString_AS_STRING
    #define PyBytes_GET_SIZE            PyString_GET_SIZE
    #define PyBytes_Check               PyString_Check
    #define PyBytes_Format              PyString_Format
    #define PyBytes_FromString          PyString_FromString
    #define PyBytes_FromStringAndSize   PyString_FromStringAndSize
#endif


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
        buf->ptr = PyBytes_AS_STRING(buf->encodedString);
        buf->size = PyBytes_GET_SIZE(buf->encodedString);
    #else
        buf->ptr = (char*) PyUnicode_AS_UNICODE(obj);
        buf->size = PyUnicode_GET_DATA_SIZE(obj);
    #endif
#else
    buf->ptr = PyBytes_AS_STRING(obj);
    buf->size = PyBytes_GET_SIZE(obj);
#endif
    return 0;
}


#ifdef WITH_UNICODE
    #define CXORA_CHARSETID             OCI_UTF16ID
    #define CXORA_ERROR_TEXT_LENGTH     2048
    #define cxString_Type               &PyUnicode_Type
    #define cxString_Format             PyUnicode_Format
    #define cxString_Check              PyUnicode_Check
    #define cxString_FromAscii(str) \
        PyUnicode_DecodeASCII(str, strlen(str), NULL)
    #ifdef Py_UNICODE_WIDE
        #define StringBuffer_Clear(buffer) \
            Py_XDECREF((buffer)->encodedString)
        #define cxString_FromEncodedString(buffer, numBytes) \
            PyUnicode_DecodeUTF16(buffer, numBytes, NULL, NULL)
    #else
        #define StringBuffer_Clear(buffer)
        #define cxString_FromEncodedString(buffer, numBytes) \
            PyUnicode_FromUnicode((Py_UNICODE*) (buffer), (numBytes) / 2)
    #endif
#else
    #define CXORA_CHARSETID             0
    #define CXORA_ERROR_TEXT_LENGTH     1024
    #define cxString_Type               &PyBytes_Type
    #define cxString_Format             PyBytes_Format
    #define cxString_Check              PyBytes_Check
    #define StringBuffer_Clear(buffer)
    #define cxString_FromAscii(str) \
        PyBytes_FromString(str)
    #define cxString_FromEncodedString(buffer, numBytes) \
        PyBytes_FromStringAndSize(buffer, numBytes)
#endif

