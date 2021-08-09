#include <jni_md.h>
#include <jvmti.h>

JNIEXPORT jint JNICALL Agent_OnLoad(JavaVM *vm, char *options, void *reserved);
JNIEXPORT jint JNICALL Agent_OnAttach(JavaVM *jvm, char *options,
                                      void *reserved);
JNIEXPORT void JNICALL Agent_OnUnload(JavaVM *vm);
