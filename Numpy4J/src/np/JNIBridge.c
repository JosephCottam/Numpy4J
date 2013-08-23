#include <jni.h>
#include <stdio.h>
#include "JNIBridge.h"
#include <Python.h>

const char* jNP_ARRAY_NAME = "np/NPArray";
const char* jNP_ARRAY_BUFFER_FIELD = "buffer";
const char* jBYTE_BUFFER_NAME = "java/nio/ByteBuffer";
const char* jBYTE_BUFFER_TYPE = "Ljava/nio/ByteBuffer;";
jfieldID BUFFER_FID;


PyObject *npModule, *dtypeFunc, *fromBufferFunc, *getBufferFunc;
const char* NP_MODULE = "numpy";
const char* NP_DTYPE_CONSTRUCTOR = "dtype";
const char* NP_FROM_BUFFER = "frombuffer";
const char* NP_GET_BUFFER = "getbuffer";

PyObject *NP_INT32, *NP_FLOAT64;
PyObject *npMaxFunc, *npMinFunc, *npLogFunc;

jint JNI_OnLoad(JavaVM *vm, void *reserved) {
  //Safely cache relevant java class information
  JNIEnv *env;
  (*vm)->AttachCurrentThread(vm, (void **) &env, NULL);
  if((*env)->ExceptionOccurred(env)) {return;}

  jclass classNPArray = (*env)->FindClass(env, jNP_ARRAY_NAME);
  if((*env)->ExceptionOccurred(env)) {return;}
  jclass classByteBuffer = (*env)->FindClass(env, jBYTE_BUFFER_NAME);
  if((*env)->ExceptionOccurred(env)) {return;}
  BUFFER_FID = (*env)->GetFieldID(env, classNPArray, jNP_ARRAY_BUFFER_FIELD, jBYTE_BUFFER_TYPE);
  if((*env)->ExceptionOccurred(env)) {return;}
  (*vm)->DetachCurrentThread(vm);

  //Setup python environment, acquire required functions
  Py_SetProgramName("numpy4J (Bridge)");  /* optional but recommended */
  Py_Initialize();
  npModule = PyImport_Import(PyString_FromString(NP_MODULE));
  dtypeFunc = PyObject_GetAttrString(npModule, NP_DTYPE_CONSTRUCTOR);
  fromBufferFunc = PyObject_GetAttrString(npModule, NP_FROM_BUFFER);
  getBufferFunc = PyObject_GetAttrString(npModule, NP_GEt_BUFFER);
  
  NP_INT32 = PyObject_GetAttrString(npModule, "int32");
  NP_FLOAT64 = PyObject_GetAttrString(npModule, "float64");
  npMinFunc = PyObject_GetAttrString(npModule, "min");
  npMaxFunc = PyObject_GetAttrString(npModule, "max");
  npLogFunc = PyObject_GetAttrString(npModule, "log");

  return JNI_VERSION_1_6; //TODO: 1.2 is "any JNI version"...might be able to use that instead
}

void JNI_OnUnload(JavaVM *vm, void *reserved) {Py_Finalize();}

//Take a java NPArray and make a python np.array
//TODO: Investigate caching these dtypes as they will likely be re-used
PyObject* make_nparray(JNIEnv *env, jobject jnparray) {
  
  //Acquire buffer information 
  jobject bufferRef = (*env)->GetObjectField(env, jnparray, BUFFER_FID);
  jbyte* jbuffer = (*env)->GetDirectBufferAddress(env, bufferRef);
  jlong jsize = (*env)->GetDirectBufferCapacity(env, bufferRef);
  
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

  return nparray;
}


//--------------- IN/OUT BUSINESS ------------
//Invoke a np function that returns an int
int invoke_int_func(PyObject *func, PyObject *nparray) {
  PyObject *args = PyTuple_Pack(1, nparray);
  PyObject *val = PyObject_CallObject(func, args);
  
  //Cleanup for return
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

//TODO: Optimize if the python array actually shares a buffer with a java nparray that was an argument
jobject make_jnparray(JNIEnv* env, PyObject *pynparray) {
  PyObject *args = PyTuple_Pack(1, pynparray);
  PyObject *buffer = PyObject_CallObject(getBufferFunc, args);
  Py_ssize_t capacity = ((Py_Buffer*) buffer)->len;
  void *buf = ((Py_Buffer*) buffer)->buf;
  jobject bytebuffer = (*env)->NewDirectByteBuffer(env, buf, capacity);

  jclass dtype_class = (*env)->FindClass(env, "np/NPType");
  jmethodID dtype_const = (*env)->GetMethodID(env, cls, "<init>", "()V");
  jobject dtype = (*env)->NewObject(env, cls, dtype_class);

  jclass nparray_class = (*env)->FindClass(env, "np/NPArray");
  jmethodID nparray_const = (*env)->GetMethodID(env, cls, "<init>", "(Ljava/nio/ByteBuffer;,Lnp/NPType/DTYPE;)V");
  if((*env)->ExceptionOccurred(env)) {return;}
  jobject nparray = (*env)->NewObject(env, cls, dtype_class, dtype, bytebuffer);

  if (NULL == midInit) return NULL;
  // Call back constructor to allocate a new instance, with an int argument


  return bytebuffer;
}


//--------------- JNI CALL TARGETS ------------

JNIEXPORT jint JNICALL Java_np_JNIBridge_max
(JNIEnv *env, jobject this, jobject jnparray) {
  PyObject *nparray = make_nparray(env, jnparray);
  jint rv = (jint) invoke_int_func(npMaxFunc, nparray); 
  Py_DECREF(nparray);
  return rv; 
}

JNIEXPORT jint JNICALL Java_np_JNIBridge_min
(JNIEnv *env, jobject this, jobject jnparray) {
  PyObject *nparray = make_nparray(env, jnparray);
  jint rv = (jint) invoke_int_func(npMinFunc, nparray); 
  Py_DECREF(nparray);
  return rv; 
}

JNIEXPORT jobject JNICALL Java_np_JNIBridge_log
(JNIEnv *env, jobject this, jobject jnparray) {
  PyObject *nparray = make_nparray(env, jnparray);
  PyObject *rslt = invoke_obj_func(npLogFunc, nparray);
  jobject rv = make_jnparray(rslt);
  Py_DECREF(nparray);
  py_DECREF(rslt);
  return rv;
}
