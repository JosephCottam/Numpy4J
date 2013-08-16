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
    RET_jint JNI_VERSION_1_2
    char* JNI_GetStringUTFChars (JNIEnv*, jstring)
    void JNI_ReleaseStringUTFChars (JNIEnv*, jstring, char*)
    RET_jstring JNI_NewStringUTF (JNIEnv*, char*)

cdef extern from "Python.h" :
    void Py_Initialize()

cdef extern void initlibhelloworld ()

cdef public RET_jint JNI_OnLoad (JavaVM* jvm, void* reserved) :
    Py_Initialize()
    initlibhelloworld()
    return JNI_VERSION_1_2

import helloworld

cdef public RET_jstring Java_HelloWorld_Hello (JNIEnv* env, jobject obj, jstring name) :
    cdef char* _name = JNI_GetStringUTFChars(env, name)
    cdef str result = helloworld.hello(_name)
    JNI_ReleaseStringUTFChars(env, name, _name)
    return JNI_NewStringUTF(env, "test")
