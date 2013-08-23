#include <jni.h>
#include <stdio.h>
#include "JNIBridge.h"
#include <Python.h>

const char* jNP_ARRAY_NAME = "np/NPArray";
const char* jNP_ARRAY_BUFFER_FIELD = "buffer";
const char* jBYTE_BUFFER_NAME = "java/nio/ByteBuffer";
const char* jBYTE_BUFFER_TYPE = "Ljava/nio/ByteBuffer;";
jfieldID BUFFER_FID;


PyObject *npModule, *dtypeFunc, *fromBufferFunc;
const char* NP_MODULE = "numpy";
const char* NP_DTYPE_CONSTRUCTOR = "dtype";
const char* NP_FROM_BUFFER = "frombuffer";

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
  Py_buffer *buffer;
  int err = PyObject_GetBuffer(pynparray, buffer, PyBUF_SIMPLE);
  jobject bytebuffer = (*env)->NewDirectByteBuffer(env, buffer->buf, buffer->len);

  jclass dtype_cls = (*env)->FindClass(env, "np/NPType$DTYPE");
  if((*env)->ExceptionOccurred(env)) {return;}
  jfieldID fid = (*env)->GetStaticFieldID(env, dtype_cls, "float64", "Lnp/NPType$DTYPE;");//TODO: match from the input array
  if((*env)->ExceptionOccurred(env)) {return;}
  jobject jdtype = (*env)->GetStaticObjectField(env, dtype_cls, fid);
  if((*env)->ExceptionOccurred(env)) {return;}
  
  jclass nptype_cls = (*env)->FindClass(env, "np/NPType");
  if((*env)->ExceptionOccurred(env)) {return;}
  jmethodID nptype_const = (*env)->GetMethodID(env, nptype_cls, "<init>", "(Lnp/NPType$DTYPE;)V");
  if((*env)->ExceptionOccurred(env)) {return;}
  jobject nptype = (*env)->NewObject(env, nptype_cls, nptype_const, jdtype);
  if((*env)->ExceptionOccurred(env)) {return;}

  jclass nparray_cls = (*env)->FindClass(env, "np/NPArray");
  if((*env)->ExceptionOccurred(env)) {return;}
  jmethodID nparray_const = (*env)->GetMethodID(env, nparray_cls, "<init>", "(Ljava/nio/ByteBuffer;Lnp/NPType;)V");
  if((*env)->ExceptionOccurred(env)) {return;}
  jobject nparray = (*env)->NewObject(env, nparray_cls, nparray_const, bytebuffer, nptype);
  if((*env)->ExceptionOccurred(env)) {return;}
  
  return nparray;
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
  jobject rv = make_jnparray(env, rslt);
  Py_DECREF(nparray);
  Py_DECREF(rslt);
  return rv;
}
