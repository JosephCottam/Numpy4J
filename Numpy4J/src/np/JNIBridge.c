#include <jni.h>
#include <stdio.h>
#include "JNIBridge.h"
#include <Python.h>
#include <stdarg.h>

const char* jBYTE_BUFFER_TYPE = "Ljava/nio/ByteBuffer;";
const char* jNP_RAWTYPE = "Lnp/NPType$RAWTYPE;";
const char* jNP_BYTEORDER = "Lnp/NPType$BYTE_ORDER;";
const char* jNP_ORDER = "Lnp/NPType$ORDER;";

//Cached java identifiers.  Often the ID is used in conjunction with a class,
//but since classes are objects the class can't (always) be retained safely.
//HOWEVER, the IDs can!  So the IDs are stored to save a lot of lookup code later
jfieldID BUFFER_FID, PYADDR_FID;
jmethodID NPARRAY_CID, NPTYPE_CID;


//Cached python objects, cached to save lookup complexity at each use location
PyObject *npModule, *dtypeFunc, *fromBufferFunc;
PyObject *npMaxFunc, *npMinFunc, *npLogFunc, *npMultFunc;

//Java/Python correspondence cache
//TODO: Verify that it is safe to store these java objects.  It may be because they are all static singletons...but maybe not.
PyObject *NP_INT8, *NP_INT16, *NP_INT32, *NP_INT64, *NP_FLOAT32, *NP_FLOAT64;
jobject JNP_INT8, JNP_INT16, JNP_INT32, JNP_INT64, JNP_FLOAT32, JNP_FLOAT64;

jobject JNP_BIGENDIAN, JNP_LITTLEENDIAN, JNP_NATIVE;
jobject JNP_CORDER, JNP_FORDER;

//Verify that the passed python objects are all not null.
//Print a warning if any of them are null
void verifyPythons(char* kind, int count, PyObject *first, ...) {
  PyObject *func=first;
  va_list vals;
  va_start(vals, first);
  while(count != -1) {
    if (func == NULL) {
      printf("NULL %s %d...\n", kind, count);
    }
    count--;
    func = va_arg(vals, PyObject*);
  }
  va_end(vals);
}

//Lookup a Java class's static member by name and type
jobject getStaticField(JNIEnv *env, jclass fromClass, const char *name, const char *typeName) {
  jfieldID fid = (*env)->GetStaticFieldID(env, fromClass, name, typeName);
  if((*env)->ExceptionOccurred(env)) {return NULL;}
  jobject obj = (*env)->GetStaticObjectField(env, fromClass, fid);
  if((*env)->ExceptionOccurred(env)) {return NULL;}
  return obj;
}

jint JNI_OnLoad(JavaVM *vm, void *reserved) {
  return JNI_VERSION_1_6; 
}

//Intiailize the environment.  Seperated from OnLoad to faciliate error reporting.
JNIEXPORT jint JNICALL Java_np_JNIBridge_init
  (JNIEnv *env, jclass jnparray) {
  //Safely cache relevant java class information
  //jclass classByteBuffer = (*env)->FindClass(env, jBYTE_BUFFER_TYPE);
  jclass class_rawtype = (*env)->FindClass(env, "np/NPType$RAWTYPE");
  jclass class_byteorder = (*env)->FindClass(env, "np/NPType$BYTE_ORDER");
  jclass class_order = (*env)->FindClass(env, "np/NPType$ORDER");
  jclass class_nptype = (*env)->FindClass(env, "np/NPType");
  jclass class_nparray = (*env)->FindClass(env, "np/NPArray");
  if((*env)->ExceptionOccurred(env)) {return 2;}
  
  BUFFER_FID = (*env)->GetFieldID(env, class_nparray, "buffer", jBYTE_BUFFER_TYPE);
  PYADDR_FID = (*env)->GetFieldID(env, class_nparray, "pyaddr", "J");
  NPARRAY_CID = (*env)->GetMethodID(env, class_nparray, "<init>", "(Ljava/nio/ByteBuffer;Lnp/NPType;)V");
  NPTYPE_CID = (*env)->GetMethodID(env, class_nptype, "<init>", "(Lnp/NPType$RAWTYPE;Lnp/NPType$ORDER;Lnp/NPType$BYTE_ORDER;)V");
  if((*env)->ExceptionOccurred(env)) {return 3;}

  JNP_INT8 = getStaticField(env, class_rawtype, "int32", jNP_RAWTYPE);
  JNP_INT16 = getStaticField(env, class_rawtype, "int16", jNP_RAWTYPE);
  JNP_INT32 = getStaticField(env, class_rawtype, "int32", jNP_RAWTYPE);
  JNP_INT64 = getStaticField(env, class_rawtype, "int64", jNP_RAWTYPE);
  JNP_FLOAT32 = getStaticField(env, class_rawtype, "float32", jNP_RAWTYPE);
  JNP_FLOAT64 = getStaticField(env, class_rawtype, "float64", jNP_RAWTYPE);
  if((*env)->ExceptionOccurred(env)) {return 4;}

  JNP_BIGENDIAN = getStaticField(env, class_byteorder, "big", jNP_BYTEORDER);
  JNP_LITTLEENDIAN = getStaticField(env, class_byteorder, "little", jNP_BYTEORDER);
  JNP_NATIVE = getStaticField(env, class_byteorder, "NATIVE", jNP_BYTEORDER);
  if((*env)->ExceptionOccurred(env)) {return 5;}

  JNP_CORDER = getStaticField(env, class_order, "c", jNP_ORDER);
  JNP_FORDER = getStaticField(env, class_order, "fortran", jNP_ORDER);
  if((*env)->ExceptionOccurred(env)) {return 6;}

  //Setup python environment, acquire required functions
  Py_SetProgramName("numpy4J (Bridge)");  /* optional but recommended */
  Py_Initialize();
  npModule = PyImport_Import(PyString_FromString("numpy"));
  NP_INT8 = PyObject_GetAttrString(npModule, "int8");
  NP_INT16 = PyObject_GetAttrString(npModule, "int16");
  NP_INT32 = PyObject_GetAttrString(npModule, "int32");
  NP_INT64 = PyObject_GetAttrString(npModule, "int64");
  NP_FLOAT32 = PyObject_GetAttrString(npModule, "float32");
  NP_FLOAT64 = PyObject_GetAttrString(npModule, "float64");
  
  fromBufferFunc = PyObject_GetAttrString(npModule, "frombuffer");
  dtypeFunc = PyObject_GetAttrString(npModule, "dtype");
  npMinFunc = PyObject_GetAttrString(npModule, "min");
  npMaxFunc = PyObject_GetAttrString(npModule, "max");
  npLogFunc = PyObject_GetAttrString(npModule, "log");
  npMultFunc = PyObject_GetAttrString(npModule, "multiply");
  
  verifyPythons("module", 1, npModule);
  verifyPythons("types", 2, NP_INT32, NP_FLOAT64);
  verifyPythons("function", 6, fromBufferFunc, dtypeFunc, npMinFunc, 
                               npMaxFunc, npLogFunc, npMultFunc);
  
  return 0;
}

void JNI_OnUnload(JavaVM *vm, void *reserved) {Py_Finalize();}

int throwIllegalArg(JNIEnv *env, char *message ) {
  jclass exClass = (*env)->FindClass( env, "java/lang/IllegalArgumentException");
  return (*env)->ThrowNew( env, exClass, message );
}

//Store a python object's reference inside of a java object
//
//This mechanism enables Java objects to be construted out of python allocated
//memory without causing memory leaks (hopefully).  Python resource
//references are stored and the DECREF is called in the Java object's finailze.
//
//This also allows some optimization by re-using python objects (no need to re-wrap)
void save_addr(JNIEnv *env, jobject jnparray, PyObject *nparray) {
  (*env)->SetLongField(env, jnparray, PYADDR_FID, (long) nparray);
  if((*env)->ExceptionOccurred(env)) {return;}
}

//Take a java NPArray and make a python np.array
PyObject* make_nparray(JNIEnv *env, jobject jnparray) {
  jlong pyobj = (*env)->GetLongField(env, jnparray, PYADDR_FID);
  if (pyobj != 0) {return (PyObject*) pyobj;}
  
  //Acquire buffer information 
  jobject bufferRef = (*env)->GetObjectField(env, jnparray, BUFFER_FID);
  jbyte* jbuffer = (*env)->GetDirectBufferAddress(env, bufferRef);
  jlong jsize = (*env)->GetDirectBufferCapacity(env, bufferRef);
  if((*env)->ExceptionOccurred(env)) {return NULL;}
  
  //Create a python buffer from the java buffer
  Py_ssize_t size = PyInt_AsSsize_t(PyInt_FromLong(jsize)); 
  PyObject *pybuffer = PyBuffer_FromReadWriteMemory(jbuffer, size);

  //Create np dtype object, based on the passed array's dtype information
  PyObject *dtypeArgs = PyTuple_Pack(1, NP_INT32); //TODO: Acquire and inspect the passed array's dtype information; 
  PyObject *dtype = PyObject_CallObject(dtypeFunc, dtypeArgs);

  //Create numpy array
  PyObject *arrayArgs = PyTuple_Pack(2, pybuffer, dtype);
  PyObject *nparray = PyObject_CallObject(fromBufferFunc, arrayArgs);
  Py_DECREF(dtype);
  Py_DECREF(arrayArgs);
  Py_DECREF(dtypeArgs);
  save_addr(env, jnparray, nparray);

  return nparray;
}


//Convert python numpy base types into java raw types
jobject rawtype_Py2J(JNIEnv *env, PyObject *py_rawtype) {
  if (py_rawtype == NP_INT8) {return JNP_INT8;}
  if (py_rawtype == NP_INT16) {return JNP_INT16;}
  if (py_rawtype == NP_INT32) {return JNP_INT32;}
  if (py_rawtype == NP_INT64) {return JNP_INT64;}
  if (py_rawtype == NP_FLOAT32) {return JNP_FLOAT32;}
  if (py_rawtype == NP_FLOAT64) {return JNP_FLOAT64;}
  throwIllegalArg(env, "Unsupported raw type requested (py->java conversion)");
  if((*env)->ExceptionOccurred(env)) {return NULL;}
  return NULL;  //TODO: Improve error reporting
}

//Convert the python array-order indicator into the java array-order indicator
//TODO: There are a lot of order & contiguous & c & f flags...this is probably more nuanced than I'm getting at here
jobject order_Py2J(JNIEnv *env, PyObject *nparray) {
  PyObject *flags = PyObject_GetAttrString(nparray, "flags");
  PyObject *isC =  PyObject_GetAttrString(flags, "c_contiguous"); 
 
  jobject rv;  
  if (isC == Py_True) {rv = JNP_CORDER;}
  else {rv = JNP_FORDER;}

  Py_DECREF(flags);
  Py_DECREF(isC);

  return rv;
}


//Convert the python byte-order indicator into the java byte-order indicator
jobject byteorder_Py2J(JNIEnv *env, PyObject *dtype) {
  PyObject *py_byteorder =  PyObject_GetAttrString(dtype, "byteorder");
  char orderindicator = PyString_AsString(py_byteorder)[0];
  
  Py_DECREF(py_byteorder);

  switch(orderindicator) {
    case ('='): return JNP_NATIVE; 
    case ('<'): return JNP_LITTLEENDIAN;
    case ('>'): return JNP_BIGENDIAN; 
  }

  throwIllegalArg(env, "Unsupported byte-order requested (py->java conversion)");
  return NULL;
}

//Construct a Java NPType object for a given python numpy array
jobject make_jnptype(JNIEnv *env, PyObject *nparray) {
  PyObject *py_dtype = PyObject_GetAttrString(nparray, "dtype");
  PyObject *py_rawType =  PyObject_GetAttrString(py_dtype, "type");
  jobject jdtype = rawtype_Py2J(env, py_rawType);
  jobject jorder = order_Py2J(env, nparray);
  jobject jbyteorder = byteorder_Py2J(env, py_dtype);
  if((*env)->ExceptionOccurred(env)) {return NULL;}

  jclass cls = (*env)->FindClass(env, "np/NPType");
  jobject nptype = (*env)->NewObject(env, cls, NPTYPE_CID, jdtype, jorder, jbyteorder);
  if((*env)->ExceptionOccurred(env)) {return NULL;}

  Py_DECREF(py_dtype);
  Py_DECREF(py_rawType);

  return nptype;
}

//Construct a java NPArray to share the memory with the given numpy array
jobject make_jnparray(JNIEnv *env, PyObject *nparray) {
  PyObject *arrayview = PyMemoryView_FromObject(nparray);
  Py_buffer *buffer = PyMemoryView_GET_BUFFER(arrayview);
  PyObject *len = PyLong_FromSize_t(buffer->len);
  long length = PyLong_AsLong(len); 


  jobject jbytebuffer = (*env)->NewDirectByteBuffer(env, buffer->buf, (jlong) length);
  jobject nptype = make_jnptype(env, nparray); 
  if((*env)->ExceptionOccurred(env)) {return NULL;}

  jclass class_nparray = (*env)->FindClass(env, "np/NPArray");
  jobject jnparray = (*env)->NewObject(env, class_nparray, NPARRAY_CID, jbytebuffer, nptype);
  if((*env)->ExceptionOccurred(env)) {return NULL;}
  
  save_addr(env, jnparray, nparray);

  //DO NOT dispose of the memory view object; The buffer is a POINTER to a part of the memory view (see issue #9)
  Py_DECREF(len);
  return jnparray;
}




//--------------- IN/OUT BUSINESS ------------
//Invoke a np function that returns an int
int invoke_int_func(PyObject *func, PyObject *nparray) {
  PyObject *args = PyTuple_Pack(1, nparray);
  PyObject *val = PyObject_CallObject(func, args);
  
  long rv = PyInt_AsLong(val);
  Py_DECREF(args);
  Py_DECREF(val);
  return (int) rv;
}

PyObject* invoke_obj_func(PyObject *func, PyObject *nparray) {
  PyObject *args = PyTuple_Pack(1, nparray);
  PyObject *val = PyObject_CallObject(func, args);
  Py_DECREF(args);
  return val;
}

PyObject* invoke_obj_func2(PyObject *func, PyObject *npa1, PyObject *npa2) {
  PyObject *args = PyTuple_Pack(2, npa1, npa2);
  PyObject *val = PyObject_CallObject(func, args);
  Py_DECREF(args);
  return val;
}


//--------------- CORE JNI CALL TARGETS ------------

JNIEXPORT jint JNICALL Java_np_JNIBridge_max
(JNIEnv *env, jobject this, jobject jnparray) {
  PyObject *nparray = make_nparray(env, jnparray);
  jint rv = (jint) invoke_int_func(npMaxFunc, nparray); 
  return rv; 
}

JNIEXPORT jint JNICALL Java_np_JNIBridge_min
(JNIEnv *env, jobject this, jobject jnparray) {
  PyObject *nparray = make_nparray(env, jnparray);
  jint rv = (jint) invoke_int_func(npMinFunc, nparray); 
  return rv; 
}

JNIEXPORT jobject JNICALL Java_np_JNIBridge_log
(JNIEnv *env, jobject this, jobject jnparray) {
  PyObject *nparray = make_nparray(env, jnparray);
  PyObject *rslt = invoke_obj_func(npLogFunc, nparray);
  jobject rv = make_jnparray(env, rslt);
  Py_DECREF(rslt);
  return rv;
}

JNIEXPORT jobject JNICALL Java_np_JNIBridge_mult
(JNIEnv *env, jclass this, jobject a1, jobject a2) {
  PyObject *npa1 = make_nparray(env, a1);
  PyObject *npa2 = make_nparray(env, a2);
  PyObject *rslt = invoke_obj_func2(npMultFunc, npa1, npa2);
  jobject rv = make_jnparray(env, rslt);
  Py_DECREF(rslt);
  return rv;
}


JNIEXPORT void JNICALL Java_np_JNIBridge_freePython
(JNIEnv *env, jclass this, jlong addr) {
  if (addr !=0) {Py_DECREF((PyObject*) addr);}
}



