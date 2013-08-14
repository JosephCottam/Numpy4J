#include <jni.h>
#define RET_jstring JNIEXPORT jstring JNICALL
#define RET_jint JNIEXPORT jint JNICALL
#define JNI_GetStringUTFChars(X,Y) ((*X)->GetStringUTFChars(X,Y,NULL))
#define JNI_NewStringUTF(X,Y) ((*X)->NewStringUTF(X,Y))
#define JNI_ReleaseStringUTFChars(X,Y,Z) ((*X)->ReleaseStringUTFChars(X,Y,Z))
