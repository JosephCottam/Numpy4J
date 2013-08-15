#include <jni.h>
#include <stdio.h>
#include "./bin/HelloJNI.h"
 
JNIEXPORT void JNICALL Java_HelloJNI_sayHello(JNIEnv *env, jobject thisObj) {
   printf("Hello World!\n");
   return;
}
