#include <jni.h>
#include <stdio.h>
#include "JNIBridge.h"
#include <Python.h>

const char* jNP_ARRAY_NAME = "np/NPArray";
const char* jNP_ARRAY_BUFFER_FIELD = "buffer";
const char* jBYTE_BUFFER_NAME = "java/nio/ByteBuffer";
const char* jBYTE_BUFFER_TYPE = "Ljava/nio/ByteBuffer;";

PyObject *npModule, *dtypeFunc, *fromBufferFunc;
const char* NP_MODULE = "numpy";
const char* NP_DTYPE_CONSTRUCTOR = "dtype";
const char* NP_FROM_BUFFER = "frombuffer";

PyObject *NP_INT32;
PyObject *npMaxFunc;

jint JNI_OnLoad(JavaVM *vm, void *reserved) {
  //Setup python environment, acquire required functions
  Py_SetProgramName("numpy4J (Bridge)");  /* optional but recommended */
  Py_Initialize();
  npModule = PyImport_Import(PyString_FromString(NP_MODULE));
  dtypeFunc = PyObject_GetAttrString(npModule, NP_DTYPE_CONSTRUCTOR);
  fromBufferFunc = PyObject_GetAttrString(npModule, NP_FROM_BUFFER);

  NP_INT32 = PyObject_GetAttrString(npModule, "int32");
  npMaxFunc = PyObject_GetAttrString(npModule, "max");

  return JNI_VERSION_1_6; //TODO: 1.2 is "any JNI version"...might be able to use that instead
}

void JNI_OnUnload(JavaVM *vm, void *reserved) {Py_Finalize();}




JNIEXPORT jint JNICALL Java_np_JNIBridge_max
(JNIEnv *env, jobject this, jobject array) {
  //Acquire buffere address
  jclass classNPArray = (*env)->FindClass(env, jNP_ARRAY_NAME);
  jfieldID bufferFID = (*env)->GetFieldID(env, classNPArray, jNP_ARRAY_BUFFER_FIELD, jBYTE_BUFFER_TYPE);
  jclass classByteBuffer = (*env)->FindClass(env, jBYTE_BUFFER_NAME);
  jobject bufferRef = (*env)->GetObjectField(env, array, bufferFID);
  jbyte* jbuffer = (*env)->GetDirectBufferAddress(env, bufferRef);
  jlong jsize = (*env)->GetDirectBufferCapacity(env, bufferRef);
  
  //Create a python buffer from the java buffer
  Py_ssize_t size = (Py_ssize_t) jsize;  //TODO: Verify this is the right way to get to Py_ssize_t...Maybe use PyInt_AsSize_t
  PyObject *pybuffer = PyBuffer_FromReadWriteMemory(jbuffer, size);

  //Create np dtype object, based on the passed array's dtype information
  PyObject *dtypeArgs = PyTuple_New(1);
  PyTuple_SetItem(dtypeArgs, 0, NP_INT32);
  
  PyObject *dtype = PyObject_CallObject(dtypeFunc, dtypeArgs);
  Py_DECREF(dtypeArgs);

  //Create numpy array
  PyObject *arrayArgs = PyTuple_New(2);
  PyTuple_SetItem(arrayArgs, 0, pybuffer);
  PyTuple_SetItem(arrayArgs, 1, dtype);
  PyObject *nparray = PyObject_CallObject(fromBufferFunc, arrayArgs);
  Py_DECREF(dtype);
  Py_DECREF(arrayArgs);

  //Invoke max
  PyObject *maxArgs = PyTuple_New(1);
  PyTuple_SetItem(maxArgs, 0, nparray);
  PyObject *max = PyObject_CallObject(npMaxFunc, maxArgs);
  Py_DECREF(nparray);
  Py_DECREF(maxArgs);
  long rv = PyInt_AsLong(max);
  printf("Result of call: %ld\n", rv);
  //Py_DECREF(max);

  return (jint) rv;
}

