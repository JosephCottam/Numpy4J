#include <jni.h>
#include <stdio.h>
#include "./bin/HelloJNI.h"
#include <Python.h>
 
JNIEXPORT void JNICALL Java_HelloJNI_sayHello(JNIEnv *env, jobject thisObj) {
  Py_SetProgramName("blended");  /* optional but recommended */
  Py_Initialize();
  PyRun_SimpleString("from time import time,ctime\n"
      "print 'Today is',ctime(time())\n");
  Py_Finalize();
   return;
}
