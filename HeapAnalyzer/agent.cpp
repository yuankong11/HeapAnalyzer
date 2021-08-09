#include <cstdio>

#include "agent.hpp"

JNIEXPORT jint JNICALL Agent_OnLoad(JavaVM *vm, char *options, void *reserved) {
  printf("JVMTI OnLoad.\n");
  return 0;
}

JNIEXPORT jint JNICALL Agent_OnAttach(JavaVM *jvm, char *options,
                                      void *reserved) {
  jvmtiEnv *jvmti;
  jint result = jvm->GetEnv((void **)&jvmti, JVMTI_VERSION_1_1);
  if (result != JNI_OK) {
    printf("ERROR: Unable to access JVMTI!\n");
  }
  jvmtiError err = (jvmtiError)0;
  jclass *classes;
  jint count;

  err = jvmti->GetLoadedClasses(&count, &classes);
  if (err) {
    printf("ERROR: JVMTI GetLoadedClasses failed!\n");
  }
  for (int i = 0; i < count; i++) {
    char *s;
    jvmti->GetClassSignature(classes[i], &s, NULL);
    printf("%s\n", s);
  }
  return err;
}

JNIEXPORT void JNICALL Agent_OnUnload(JavaVM *vm) {}
