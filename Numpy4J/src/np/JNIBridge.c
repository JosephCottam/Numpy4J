#include <jni.h>
#include <stdio.h>
#include "JNIBridge.h"
#include <Python.h>
#include <stdarg.h>

const char* jBYTE_BUFFER_TYPE = "Ljava/nio/ByteBuffer;";
const char* jNP_RAWTYPE = "Lnp/NPType$RAWTYPE;";
const char* jNP_BYTEORDER = "Lnp/NPType$BYTE_ORDER;";
const char* jNP_ORDER = "Lnp/NPType$ORDER;";
jfieldID BUFFER_FID, PYADDR_FID;
jmethodID NPARRAY_CID, NPTYPE_CID;

PyObject *npModule, *dtypeFunc, *fromBufferFunc;
PyObject *npMaxFunc, *npMinFunc, *npLogFunc, *npMultFunc;

PyObject *NP_INT8, *NP_INT16, *NP_INT32, *NP_INT64, *NP_FLOAT32, *NP_FLOAT64;
jobject JNP_INT8, JNP_INT16, JNP_INT32, JNP_INT64, JNP_FLOAT32, JNP_FLOAT64;

jobject JNP_BIGENDIAN, JNP_LITTLEENDIAN, JNP_NATIVE;
jobject JNP_CORDER, JNP_FORDER;

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

jobject getStaticField(JNIEnv *env, jclass fromClass, const char *name, const char *typeName) {
  jfieldID fid = (*env)->GetStaticFieldID(env, fromClass, name, typeName);
  if((*env)->ExceptionOccurred(env)) {return;}
  jobject obj = (*env)->GetStaticObjectField(env, fromClass, fid);
  if((*env)->ExceptionOccurred(env)) {return;}
  return obj;
}

jint JNI_OnLoad(JavaVM *vm, void *reserved) {
  //Safely cache relevant java class information
  JNIEnv *env;
  (*vm)->AttachCurrentThread(vm, (void **) &env, NULL);
  if((*env)->ExceptionOccurred(env)) {return;}

  jclass classByteBuffer = (*env)->FindClass(env, jBYTE_BUFFER_TYPE);
  jclass class_rawtype = (*env)->FindClass(env, "np/NPType$RAWTYPE");
  jclass class_byteorder = (*env)->FindClass(env, "np/NPType$BYTE_ORDER");
  jclass class_order = (*env)->FindClass(env, "np/NPType$ORDER");
  jclass class_nptype = (*env)->FindClass(env, "np/NPType");
  jclass class_nparray = (*env)->FindClass(env, "np/NPArray");
  if((*env)->ExceptionOccurred(env)) {return;}
  
  BUFFER_FID = (*env)->GetFieldID(env, class_nparray, "buffer", jBYTE_BUFFER_TYPE);
  PYADDR_FID = (*env)->GetFieldID(env, class_nparray, "pyaddr", "J");
  NPARRAY_CID = (*env)->GetMethodID(env, class_nparray, "<init>", "(Ljava/nio/ByteBuffer;Lnp/NPType;)V");
  NPTYPE_CID = (*env)->GetMethodID(env, class_nptype, "<init>", "(Lnp/NPType$RAWTYPE;Lnp/NPType$ORDER;Lnp/NPType$BYTE_ORDER;)V");
  if((*env)->ExceptionOccurred(env)) {return;}

  JNP_INT8 = getStaticField(env, class_rawtype, "int32", jNP_RAWTYPE);
  JNP_INT16 = getStaticField(env, class_rawtype, "int16", jNP_RAWTYPE);
  JNP_INT32 = getStaticField(env, class_rawtype, "int32", jNP_RAWTYPE);
  JNP_INT64 = getStaticField(env, class_rawtype, "int64", jNP_RAWTYPE);
  JNP_FLOAT32 = getStaticField(env, class_rawtype, "float32", jNP_RAWTYPE);
  JNP_FLOAT64 = getStaticField(env, class_rawtype, "float64", jNP_RAWTYPE);
  if((*env)->ExceptionOccurred(env)) {return;}

  JNP_BIGENDIAN = getStaticField(env, class_byteorder, "big", jNP_BYTEORDER);
  JNP_LITTLEENDIAN = getStaticField(env, class_byteorder, "little", jNP_BYTEORDER);
  JNP_NATIVE = getStaticField(env, class_byteorder, "NATIVE", jNP_BYTEORDER);
  if((*env)->ExceptionOccurred(env)) {return;}

  JNP_CORDER = getStaticField(env, class_order, "c", jNP_ORDER);
  JNP_FORDER = getStaticField(env, class_order, "fortran", jNP_ORDER);
  if((*env)->ExceptionOccurred(env)) {return;}

  (*vm)->DetachCurrentThread(vm);

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

  return JNI_VERSION_1_6; //TODO: 1.2 is "any JNI version"...might be able to use that instead
}

int throwIllegalArg(JNIEnv *env, char *message ) {
  jclass exClass = (*env)->FindClass( env, "java/lang/IllegalArgumentException");
  return (*env)->ThrowNew( env, exClass, message );
}


void JNI_OnUnload(JavaVM *vm, void *reserved) {Py_Finalize();}

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
  if((*env)->ExceptionOccurred(env)) {return;}
  
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
  if((*env)->ExceptionOccurred(env)) {return;}
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
}

jobject make_nptype(JNIEnv *env, PyObject *nparray) {
  PyObject *py_dtype = PyObject_GetAttrString(nparray, "dtype");
  PyObject *py_rawType =  PyObject_GetAttrString(py_dtype, "type");
  jobject jdtype = rawtype_Py2J(env, py_rawType);
  jobject jorder = order_Py2J(env, nparray);
  jobject jbyteorder = byteorder_Py2J(env, py_dtype);
  if((*env)->ExceptionOccurred(env)) {return;}

  jclass cls = (*env)->FindClass(env, "np/NPType");
  jobject nptype = (*env)->NewObject(env, cls, NPTYPE_CID, jdtype, jorder, jbyteorder);
  if((*env)->ExceptionOccurred(env)) {return;}

  Py_DECREF(py_dtype);
  Py_DECREF(py_rawType);

  return nptype;
}

jobject make_jnparray(JNIEnv *env, PyObject *nparray) {
  PyObject *arrayview = PyMemoryView_FromObject(nparray);
  Py_buffer *buffer = PyMemoryView_GET_BUFFER(arrayview);
  PyObject *len = PyLong_FromSize_t(buffer->len);
  long length = PyLong_AsLong(len); 


  jobject jbytebuffer = (*env)->NewDirectByteBuffer(env, buffer->buf, (jlong) length);
  //jobject jbytebuffer = (*env)->NewDirectByteBuffer(env, buffer->buf, buffer->len);
  jobject nptype = make_nptype(env, nparray); 
  if((*env)->ExceptionOccurred(env)) {return;}

  jclass class_nparray = (*env)->FindClass(env, "np/NPArray");
  jobject jnparray = (*env)->NewObject(env, class_nparray, NPARRAY_CID, jbytebuffer, nptype);
  if((*env)->ExceptionOccurred(env)) {return;}

  Py_DECREF(arrayview);
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



