#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <jni_md.h>
#include <jvmti.h>

typedef struct ClassInfo {
  int id;
  char *name;
  int instance_count;
  long total_size;
} ClassInfo;

typedef struct TagInfo {
  int class_tag;
  int class_object_tag;
  struct TagInfo *referrer;
} TagInfo;

typedef struct ObjectInfo {
  int size;
  TagInfo *object_tag;
} ObjectInfo;

jvmtiEnv *jvmti;
ClassInfo **class_info_array;
ObjectInfo **object_info_array;

int object_number = 0;
int object_record_number = 20;
int class_show_number = 20;
int backtrace_number = 2;

char *getClassName(jclass cls) {
  char *sig, *name;
  jvmti->GetClassSignature(cls, &sig, NULL);
  if (sig) {
    int index = 0;
    int l = strlen(sig);
    if (l > 1 && sig[l - 1] == ';') {
      l--;
    }
    while (sig[index] == '[') {
      index++;
    }
    int type = 0;
    char buf[10];
    switch (sig[index]) {
    case 'Z':
      strcpy(buf, "boolean");
      l = 7;
      break;
    case 'B':
      strcpy(buf, "byte");
      l = 4;
      break;
    case 'C':
      strcpy(buf, "char");
      l = 4;
      break;
    case 'S':
      strcpy(buf, "short");
      l = 5;
      break;
    case 'I':
      strcpy(buf, "int");
      l = 3;
      break;
    case 'J':
      strcpy(buf, "long");
      l = 4;
      break;
    case 'F':
      strcpy(buf, "float");
      l = 5;
      break;
    case 'D':
      strcpy(buf, "double");
      l = 6;
      break;
    case 'L': {
      type = 1;
      l = l - index - 1;
      break;
    }
    default: {
      type = 2;
      l = l - index;
      break;
    }
    }
    name = (char *)malloc(l + index * 2 + 1);
    if (type == 0) {
      strncpy(name, buf, l);
    } else if (type == 1) {
      strncpy(name, sig + index + 1, l);
    } else if (type == 2) {
      strncpy(name, sig + index, l);
    }
    while (index--) {
      name[l++] = '[';
      name[l++] = ']';
    }
    name[l] = 0;
    jvmti->Deallocate((unsigned char *)sig);
    char *t = name;
    while (*t) {
      if (*t == '/') {
        *t = '.';
      }
      t++;
    }
  }
  return name;
}

bool object_info_compare(ObjectInfo *oi1, ObjectInfo *oi2) {
  return oi1->size < oi2->size;
}

bool class_info_compare(ClassInfo *ci1, ClassInfo *ci2) {
  return ci1->total_size > ci2->total_size;
}

void initial_object_info_array(int n) {
  object_info_array = (ObjectInfo **)malloc(sizeof(ObjectInfo *) * n);
  for (int i = 0; i < n; i++) {
    object_info_array[i] = (ObjectInfo *)malloc(sizeof(ObjectInfo));
    object_info_array[i]->size = 0;
    object_info_array[i]->object_tag = 0;
  }
}

void add_object_info(int n, int size, TagInfo *tag) {
  object_number++;
  if (size > object_info_array[0]->size) {
    object_info_array[0]->size = size;
    object_info_array[0]->object_tag = tag;
    std::sort(object_info_array, object_info_array + n, &object_info_compare);
  }
}

jint JNICALL count_HFR(jvmtiHeapReferenceKind reference_kind,
                       const jvmtiHeapReferenceInfo *reference_info,
                       jlong class_tag, jlong referrer_class_tag, jlong size,
                       jlong *tag_ptr, jlong *referrer_tag_ptr, jint length,
                       void *user_data) {
  // 当对象是java.lang.Class对象时，其tag_ptr指向所表示的类的class_tag
  TagInfo *ti = 0;
  if (*tag_ptr == 0) {
    ti = (TagInfo *)malloc(sizeof(TagInfo));
    memset(ti, 0, sizeof(TagInfo));
    *tag_ptr = (jlong)ti;
  } else {
    ti = (TagInfo *)*tag_ptr;
  }
  if (ti != 0 && ti->class_tag == 0) {
    TagInfo *ctti = (TagInfo *)class_tag;
    ti->class_tag = ctti->class_object_tag;
    if (ti->referrer == 0) {
      if (referrer_tag_ptr == 0) {
        ti->referrer = 0;
      } else {
        ti->referrer = (TagInfo *)*referrer_tag_ptr;
      }
    }
    ClassInfo *ci = class_info_array[ti->class_tag];
    ci->instance_count++;
    ci->total_size += size;
    add_object_info(object_record_number, size, ti);
  }
  return JVMTI_VISIT_OBJECTS;
}

// jint JNICALL count_HI(jlong class_tag, jlong size, jlong *tag_ptr, jint
// length,
//                       void *user_data) {
//   // 当对象是java.lang.Class对象时，其tag_ptr指向所表示的类的class_tag
//   if ((*tag_ptr & 0xf) == 0) {
//     *tag_ptr |= 0x1;
//     ClassInfo *ci = class_info_array[class_tag >> 4];
//     ci->instance_count++;
//     ci->total_size += size;
//     add_object_info(object_record_number, size, ci->name);
//   }
//   return JVMTI_VISIT_OBJECTS;
// }

jint JNICALL untag(jlong class_tag, jlong size, jlong *tag_ptr, jint length,
                   void *user_data) {
  *tag_ptr = 0;
  return JVMTI_VISIT_OBJECTS;
}

// JAVA程序将会进入STW状态
JNIEXPORT jint JNICALL Agent_OnAttach(JavaVM *jvm, char *options,
                                      void *reserved) {
  printf("INFO: Agent OnAttach.\n");

  jint result = jvm->GetEnv((void **)&jvmti, JVMTI_VERSION_1_1);
  if (result != JNI_OK) {
    printf("ERROR: Unable to access JVMTI!\n");
    return result;
  }
  jvmtiCapabilities capa;
  memset(&capa, 0, sizeof(capa));
  capa.can_tag_objects = 1;
  jvmti->AddCapabilities(&capa);
  jvmtiError err = (jvmtiError)0;

  // err = jvmti->ForceGarbageCollection();
  // if (err) {
  //   printf("Error: JVMTI ForceGarbageCollection error code %d.\n", err);
  //   return err;
  // }

  jclass *classes;
  jint class_number;
  err = jvmti->GetLoadedClasses(&class_number, &classes);
  if (err) {
    printf("ERROR: JVMTI GetLoadedClasses failed!\n");
    return err;
  }
  printf("class_number: %d\n", class_number);

  static_assert(sizeof(jlong) == sizeof(TagInfo *));

  class_number++;
  class_info_array = (ClassInfo **)malloc(sizeof(ClassInfo *) * class_number);
  for (int i = 1; i < class_number; i++) {
    ClassInfo *ci = (ClassInfo *)malloc(sizeof(ClassInfo));
    memset(ci, 0, sizeof(ClassInfo));
    ci->id = i;
    ci->name = getClassName(classes[i - 1]);
    class_info_array[i] = ci;
    TagInfo *ti = (TagInfo *)malloc(sizeof(TagInfo));
    ti->class_object_tag = i;
    ti->class_tag = 0;
    ti->referrer = 0;
    jvmti->SetTag(classes[i - 1], (jlong)ti);
  }
  initial_object_info_array(object_record_number);

  jvmtiHeapCallbacks heapCallbacks;
  memset(&heapCallbacks, 0, sizeof(heapCallbacks));
  heapCallbacks.heap_reference_callback = &count_HFR;
  err = jvmti->FollowReferences(0, NULL, NULL, &heapCallbacks, NULL);
  if (err) {
    printf("Error: JVMTI FollowReferences error code %d.\n", err);
    return err;
  }
  // memset(&heapCallbacks, 0, sizeof(heapCallbacks));
  // heapCallbacks.heap_iteration_callback = &count_HI;
  // err = jvmti->IterateThroughHeap(0, NULL, &heapCallbacks, NULL);
  // if (err) {
  //   printf("Error: JVMTI IterateThroughHeap error code %d.\n", err);
  //   return err;
  // }

  printf("object_number: %d\n", object_number);
  printf("\n%-4s\t%-10s\t%s\n", "id", "#bytes", "class_name");
  printf("----------------------------------------------------\n");
  for (int i = object_record_number - 1;
       i >= 0 && object_info_array[i]->object_tag != 0; i--) {
    ObjectInfo *oi = object_info_array[i];
    ClassInfo *ci = class_info_array[oi->object_tag->class_tag];
    printf("%-4d\t%-10d\t%s", object_record_number - i, oi->size, ci->name);
    TagInfo *ref = oi->object_tag->referrer;
    for (int j = 0; j < backtrace_number && ref != 0; j++) {
      ci = class_info_array[ref->class_tag];
      printf(" <-- %s", ci->name);
      ref = ref->referrer;
    }
    if (ref == 0) {
      printf(" <-- root\n");
    } else {
      printf(" <-- ...\n");
    }
  }
  printf("\n");

  std::sort(class_info_array + 1, class_info_array + class_number,
            &class_info_compare);
  printf("\n%-4s\t%-12s\t%-15s\t%s\n", "id", "#instances", "#bytes",
         "class_name");
  printf("----------------------------------------------------\n");
  for (int i = 1; i - 1 < class_show_number && i < class_number; i++) {
    ClassInfo *ci = class_info_array[i];
    printf("%-4d\t%-12d\t%-15ld\t%s\n", i, ci->instance_count, ci->total_size,
           ci->name);
  }
  printf("\n");

  memset(&heapCallbacks, 0, sizeof(heapCallbacks));
  heapCallbacks.heap_iteration_callback = &untag;
  err = jvmti->IterateThroughHeap(0, NULL, &heapCallbacks, NULL);
  if (err) {
    printf("Error: JVMTI IterateThroughHeap error code %d.\n", err);
    return err;
  }

  for (int i = 1; i < class_number; i++) {
    free(class_info_array[i]->name);
    free(class_info_array[i]);
  }
  free(class_info_array);
  for (int i = 0; i < object_record_number; i++) {
    free(object_info_array[i]);
  }
  free(object_info_array);
  jvmti->Deallocate((unsigned char *)classes);
  jvmti->DisposeEnvironment();
  jvmti = NULL;
  fflush(stdout);
  return JNI_OK;
}

JNIEXPORT void JNICALL Agent_OnUnload(JavaVM *vm) {
  printf("INFO: Agent OnUnload.\n");
}
