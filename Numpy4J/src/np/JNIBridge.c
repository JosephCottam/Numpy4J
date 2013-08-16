#include <jni.h>
#include <stdio.h>
#include "JNIBridge.h"
#include <Python.h>

PyObject *npModule;

jint JNI_OnLoad(JavaVM *vm, void *reserved) {
  Py_SetProgramName("numpy4J (Bridge)");  /* optional but recommended */
  Py_Initialize();
  npModule = PyImport_Import(PyString_FromString("numpy"));

  return JNI_VERSION_1_6; //TODO: 1.2 is "any JNI version"...might be able to use that instead
}

void JNI_OnUnload(JavaVM *vm, void *reserved) {
  Py_Finalize();
}

JNIEXPORT jint JNICALL Java_np_JNIBridge_max
(JNIEnv *env, jobject this, jobject array) {
  return 0;
}

