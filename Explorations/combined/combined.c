#include <jni.h>
#include <stdio.h>
#include "./bin/HelloJNI.h"
#include <Python.h>


jint JNI_OnLoad(JavaVM *vm, void *reserved) {
  Py_SetProgramName("numpy4J (Bridge)");  /* optional but recommended */
  Py_Initialize();
  return JNI_VERSION_1_6; //TODO: 1.2 is "any JNI version"...might be able to use that instead
}

void JNI_OnUnload(JavaVM *vm, void *reserved) {
  Py_Finalize();
}


JNIEXPORT void JNICALL Java_HelloJNI_sayHello(JNIEnv *env, jobject thisObj) {
  PyRun_SimpleString("from time import time,ctime\n"
      "print 'Today is',ctime(time())\n");
   return;
}
