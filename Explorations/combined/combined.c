#include <jni.h>
#include <stdio.h>
#include "./bin/combinedJNI.h"
#include <Python.h>


jint JNI_OnLoad(JavaVM *vm, void *reserved) {
  Py_SetProgramName("numpy4J (Bridge)");  /* optional but recommended */
  Py_Initialize();
  return JNI_VERSION_1_6; //TODO: 1.2 is "any JNI version"...might be able to use that instead
}

void JNI_OnUnload(JavaVM *vm, void *reserved) {
  Py_Finalize();
}

JNIEXPORT void JNICALL Java_HelloCombined_sayHello
  (JNIEnv *env, jobject thisObj, jstring name) {

  PyObject *pName, *pModule, *pDict, *pFunc, *pArgs, *pValue;


  const char* _name = (*env)->GetStringUTFChars(env, name, NULL);
  pName = PyString_FromString(_name);

  pModule = PyImport_Import(PyString_FromString("numpy"));




  PyRun_SimpleString("from time import time,ctime\n"
      "print 'Today is',ctime(time())\n");

  (*env)->ReleaseStringUTFChars(env, name, _name);
  return;  
}
