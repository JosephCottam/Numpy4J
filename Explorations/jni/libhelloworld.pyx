cdef extern from "jnihelloworld.h" :
    ctypedef struct JNIEnv :
        pass
    ctypedef struct JavaVM :
        pass
    ctypedef struct jobject :
        pass
    ctypedef struct jstring :
        pass
    ctypedef struct RET_jstring :
        pass
    ctypedef struct RET_jint :
        pass
    RET_jint JNI_VERSION_1_6
    char* JNI_GetStringUTFChars (JNIEnv*, jstring)
    void JNI_ReleaseStringUTFChars (JNIEnv*, jstring, char*)
    RET_jstring JNI_NewStringUTF (JNIEnv*, char*)

cdef extern from "Python.h" :
    void Py_Initialize()

cdef extern void initlibhelloworld ()

cdef public RET_jint JNI_OnLoad (JavaVM* jvm, void* reserved) :
    return JNI_VERSION_1_6

import helloworld

cdef public RET_jstring Java_HelloWorld_Hello (JNIEnv* env, jobject obj, jstring name) :
    return JNI_NewStringUTF(env, "test")
