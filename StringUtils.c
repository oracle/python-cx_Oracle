//-----------------------------------------------------------------------------
// StringUtils.c
//   Defines constants and routines specific to handling strings.
//-----------------------------------------------------------------------------

// define structure for abstracting string buffers
typedef struct {
    const void *ptr;
    Py_ssize_t size;
#ifdef Py_UNICODE_WIDE
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
// StringBuffer_Init()
//   Initialize the string buffer with an empty string. Returns 0 as a
// convenience to the caller.
//-----------------------------------------------------------------------------
static int StringBuffer_Init(
    udt_StringBuffer *buf)              // buffer to fill
{
    buf->ptr = NULL;
    buf->size = 0;
#ifdef Py_UNICODE_WIDE
    buf->encodedString = NULL;
#endif
    return 0;
}


//-----------------------------------------------------------------------------
// StringBuffer_FromUnicode()
//   Populate the string buffer from a unicode object.
//-----------------------------------------------------------------------------
static int StringBuffer_FromUnicode(
    udt_StringBuffer *buf,              // buffer to fill
    PyObject *obj)                      // unicode object expected
{
    if (!obj)
        return StringBuffer_Init(buf);
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
    return 0;
}


//-----------------------------------------------------------------------------
// StringBuffer_FromBytes()
//   Populate the string buffer from a bytes object.
//-----------------------------------------------------------------------------
static int StringBuffer_FromBytes(
    udt_StringBuffer *buf,              // buffer to fill
    PyObject *obj)                      // bytes object expected
{
    if (!obj)
        return StringBuffer_Init(buf);
    buf->ptr = PyBytes_AS_STRING(obj);
    buf->size = PyBytes_GET_SIZE(obj);
#ifdef Py_UNICODE_WIDE
    buf->encodedString = NULL;
#endif
    return 0;
}


#ifdef WITH_UNICODE
    #define CXORA_CHARSETID             OCI_UTF16ID
    #define CXORA_BYTES_PER_CHAR        2
    #define cxString_Type               &PyUnicode_Type
    #define cxString_Format             PyUnicode_Format
    #define cxString_Check              PyUnicode_Check
    #define cxString_GetSize            PyUnicode_GET_SIZE
    #define cxString_FromObject         PyObject_Unicode
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
    #define StringBuffer_Fill           StringBuffer_FromUnicode
#else
    #define CXORA_CHARSETID             0
    #define CXORA_BYTES_PER_CHAR        1
    #define cxString_Type               &PyBytes_Type
    #define cxString_Format             PyBytes_Format
    #define cxString_Check              PyBytes_Check
    #define cxString_GetSize            PyBytes_GET_SIZE
    #define cxString_FromObject         PyObject_Str
    #define StringBuffer_Clear(buffer)
    #define cxString_FromAscii(str) \
        PyBytes_FromString(str)
    #define cxString_FromEncodedString(buffer, numBytes) \
        PyBytes_FromStringAndSize(buffer, numBytes)
    #define StringBuffer_Fill           StringBuffer_FromBytes
#endif

