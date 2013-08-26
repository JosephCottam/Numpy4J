#include <jni.h>
#include <stdio.h>
#include "JNIBridge.h"
#include <Python.h>
#include <stdarg.h>

const char* jBYTE_BUFFER_TYPE = "Ljava/nio/ByteBuffer;";
jfieldID BUFFER_FID, PYADDR_FID;


PyObject *npModule, *dtypeFunc, *fromBufferFunc;
PyObject *NP_INT32, *NP_FLOAT64;
PyObject *npMaxFunc, *npMinFunc, *npLogFunc, *npMultFunc;

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

jint JNI_OnLoad(JavaVM *vm, void *reserved) {
  //Safely cache relevant java class information
  JNIEnv *env;
  (*vm)->AttachCurrentThread(vm, (void **) &env, NULL);
  if((*env)->ExceptionOccurred(env)) {return;}

  jclass classNPArray = (*env)->FindClass(env, "np/NPArray");
  if((*env)->ExceptionOccurred(env)) {return;}
  jclass classByteBuffer = (*env)->FindClass(env, jBYTE_BUFFER_TYPE);
  if((*env)->ExceptionOccurred(env)) {return;}
  BUFFER_FID = (*env)->GetFieldID(env, classNPArray, "buffer", jBYTE_BUFFER_TYPE);
  if((*env)->ExceptionOccurred(env)) {return;}
  PYADDR_FID = (*env)->GetFieldID(env, classNPArray, "pyaddr", "J");
  if((*env)->ExceptionOccurred(env)) {return;}  (*vm)->DetachCurrentThread(vm);

  //Setup python environment, acquire required functions
  Py_SetProgramName("numpy4J (Bridge)");  /* optional but recommended */
  Py_Initialize();
  npModule = PyImport_Import(PyString_FromString("numpy"));
  NP_INT32 = PyObject_GetAttrString(npModule, "int32");
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
  
  return JNI_VERSION_1_6; //TODO: 1.2 is "any JNI version"...might be able to use that instead
}

void JNI_OnUnload(JavaVM *vm, void *reserved) {Py_Finalize();}

void save_addr(JNIEnv *env, jobject jnparray, PyObject *nparray) {
  (*env)->SetLongField(env, jnparray, PYADDR_FID, (long) nparray);
}

//Take a java NPArray and make a python np.array
//TODO: Investigate caching these dtypes as they will likely be re-used
PyObject* make_nparray(JNIEnv *env, jobject jnparray) {
  jlong pyobj = (*env)->GetLongField(env, jnparray, PYADDR_FID);
  if (pyobj != 0) {return (PyObject*) pyobj;}

  
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
  save_addr(env, jnparray, nparray);

  return nparray;
}



//TODO: Optimize if the python array actually shares a buffer with a java nparray that was an argument
jobject make_jnparray(JNIEnv* env, PyObject *nparray) {
  PyObject *arrayview = PyMemoryView_FromObject(nparray);
  Py_buffer *buffer = PyMemoryView_GET_BUFFER(arrayview);
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
  jobject jnparray = (*env)->NewObject(env, nparray_cls, nparray_const, bytebuffer, nptype);
  if((*env)->ExceptionOccurred(env)) {return;}
  save_addr(env, jnparray, nparray);
  
  return jnparray;
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

PyObject* invoke_obj_func2(PyObject *func, PyObject *npa1, PyObject *npa2) {
  PyObject *args = PyTuple_Pack(2, npa1, npa2);
  PyObject *val = PyObject_CallObject(func, args);
  Py_DECREF(args);
  return val;
}
//--------------- JNI CALL TARGETS ------------

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



