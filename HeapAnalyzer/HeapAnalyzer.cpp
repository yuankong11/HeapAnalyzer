#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <jni_md.h>
#include <jvmti.h>

struct ClassInfo {
  int id = 0;
  char *name = 0;
  int instance_count = 0;
  long total_size = 0;
  ClassInfo(int id, char *name) : id(id), name(name) {}
  ~ClassInfo() { free(name); }
};

struct TagInfo {
  int class_tag = 0;        // 该对象所属类的tag
  int class_object_tag = 0; // 用于java.lang.Class对象标注其表示的类的tag
  // 如果A是一个java.lang.Class的对象，其表示类A_class，则其class_tag指向java.lang.Class，而class_object_tag指向A_class
  TagInfo *referrer = 0; // 引用者，为0时表示由root(JNI、stack等)引用
  jvmtiHeapReferenceInfoStackLocal *stack_info =
      0; // 当引用来自JVMTI_HEAP_REFERENCE_STACK_LOCAL时，记录其reference_info
  TagInfo(int class_object_tag) : class_object_tag(class_object_tag) {}
  TagInfo() {}
  ~TagInfo() { free(stack_info); }
};

struct ObjectInfo {
  int size = 0;
  TagInfo *object_tag = 0;
  ObjectInfo(int size, TagInfo *object_tag)
      : size(size), object_tag(object_tag) {}
  ObjectInfo() {}
  bool less(ObjectInfo *oi) { return this->size < oi->size; }
};

int object_number = 0;
int class_show_number = 20; // 展示占用空间大小最大的类数
int backtrace_number = 2;   // 大对象回溯引用的层数

jvmtiEnv *jvmti;
ClassInfo **class_info_array;

void check_error(jvmtiError err, char const *name) {
  if (err) {
    printf("ERROR: JVMTI %s failed!\n", name);
    exit(-1);
  }
}

struct ObjectInfoHeap {
private:
  int record_number = 0;
  ObjectInfo **array = 0;

  void swap(int i, int j) {
    ObjectInfo *t = array[i];
    array[i] = array[j];
    array[j] = t;
  }

  void adjust(int cur, int limit) {
    int l = 2 * cur + 1, r = 2 * cur + 2, min;
    if (l < limit && array[l]->less(array[cur])) {
      min = l;
    } else {
      min = cur;
    }
    if (r < limit && array[r]->less(array[min])) {
      min = r;
    }
    if (min != cur) {
      swap(min, cur);
      adjust(min, limit);
    }
  }

  void sort() {
    for (int i = record_number - 1; i >= 0; i--) {
      swap(0, i);
      adjust(0, i);
    }
  }

public:
  ObjectInfoHeap(int record_number) : record_number(record_number) {
    array = (ObjectInfo **)malloc(sizeof(ObjectInfo *) * record_number);
    for (int i = 0; i < record_number; i++) {
      array[i] = new ObjectInfo();
    }
  }

  ~ObjectInfoHeap() {
    for (int i = 0; i < record_number; i++) {
      delete array[i];
    }
    free(array);
  }

  void add(int size, TagInfo *tag) {
    if (array[0]->size < size) {
      array[0]->size = size;
      array[0]->object_tag = tag;
      adjust(0, record_number);
    }
  }

  void print() {
    sort();
    printf("\n%-4s\t%-10s\t%s\n", "id", "#bytes", "class_name");
    printf("----------------------------------------------------\n");
    for (int i = 0; i < record_number && array[i]->object_tag != 0; i++) {
      ObjectInfo *oi = array[i];
      ClassInfo *ci = class_info_array[oi->object_tag->class_tag];
      printf("%-4d\t%-10d\t%s", i + 1, oi->size, ci->name);
      TagInfo *ref = oi->object_tag->referrer;
      TagInfo *ref_pre = oi->object_tag;
      for (int j = 0; j < backtrace_number && ref != 0; j++) {
        ci = class_info_array[ref->class_tag];
        printf(" <-- %s", ci->name);
        ref_pre = ref;
        ref = ref->referrer;
      }
      if (ref == 0) {
        printf(" <-- root");
        if (ref_pre->stack_info != 0) {
          char *name;
          check_error(
              jvmti->GetMethodName(ref_pre->stack_info->method, &name, 0, 0),
              "GetMethodName");
          printf("(local variable in method: %s)\n", name);
          check_error(jvmti->Deallocate((unsigned char *)name), "Deallocate");
        } else {
          printf("\n");
        }
      } else {
        printf(" <-- ...\n");
      }
    }
    printf("\n");
  }
};

ObjectInfoHeap *object_info_heap;

char *getClassName(jclass cls) {
  char *sig, *name;
  check_error(jvmti->GetClassSignature(cls, &sig, NULL), "GetClassSignature");
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
    check_error(jvmti->Deallocate((unsigned char *)sig), "Deallocate");
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

bool class_info_compare(ClassInfo *ci1, ClassInfo *ci2) {
  return ci1->total_size > ci2->total_size;
}

jint JNICALL count_HFR(jvmtiHeapReferenceKind reference_kind,
                       const jvmtiHeapReferenceInfo *reference_info,
                       jlong class_tag, jlong referrer_class_tag, jlong size,
                       jlong *tag_ptr, jlong *referrer_tag_ptr, jint length,
                       void *user_data) {
  // 当对象是java.lang.Class对象时，其tag_ptr指向所表示的类的class_tag
  TagInfo *ti = 0;
  if (*tag_ptr == 0) {
    ti = new TagInfo();
    *tag_ptr = (jlong)ti;
  } else {
    ti = (TagInfo *)*tag_ptr;
  }
  if (ti != 0 && ti->class_tag == 0) {
    TagInfo *ctti = (TagInfo *)class_tag;
    ti->class_tag = ctti->class_object_tag;
    if (ti->referrer == 0) {
      // 当引用者是root时，referrer_tag_ptr为0
      if (referrer_tag_ptr != 0) {
        ti->referrer = (TagInfo *)*referrer_tag_ptr;
      } else {
        if (reference_kind == JVMTI_HEAP_REFERENCE_STACK_LOCAL) {
          ti->stack_info = (jvmtiHeapReferenceInfoStackLocal *)malloc(
              sizeof(jvmtiHeapReferenceInfoStackLocal));
          memcpy(ti->stack_info, reference_info,
                 sizeof(jvmtiHeapReferenceInfoStackLocal));
        }
      }
    }
    ClassInfo *ci = class_info_array[ti->class_tag];
    ci->instance_count++;
    ci->total_size += size;
    object_number++;
    object_info_heap->add(size, ti);
  }
  return JVMTI_VISIT_OBJECTS;
}

jint JNICALL untag(jlong class_tag, jlong size, jlong *tag_ptr, jint length,
                   void *user_data) {
  if (*tag_ptr != 0) {
    delete (TagInfo *)*tag_ptr;
    *tag_ptr = 0;
  }
  return JVMTI_VISIT_OBJECTS;
}

void initial_agent(JavaVM *jvm) {
  jint result = jvm->GetEnv((void **)&jvmti, JVMTI_VERSION_1_2);
  if (result != JNI_OK) {
    printf("ERROR: Unable to access JVMTI!\n");
    exit(-1);
  }
  jvmtiCapabilities capa;
  memset(&capa, 0, sizeof(capa));
  capa.can_tag_objects = 1;
  check_error(jvmti->AddCapabilities(&capa), "AddCapabilities");
}

void destory_agent() {
  check_error(jvmti->DisposeEnvironment(), "DisposeEnvironment");
  jvmti = NULL;
  fflush(stdout);
}

void heap_analyze() {
  jclass *classes;
  jint class_number;
  check_error(jvmti->GetLoadedClasses(&class_number, &classes),
              "GetLoadedClasses");
  printf("class_number: %d\n", class_number);

  static_assert(sizeof(jlong) == sizeof(TagInfo *));

  class_number++;
  class_info_array = (ClassInfo **)malloc(sizeof(ClassInfo *) * class_number);
  for (int i = 1; i < class_number; i++) {
    ClassInfo *ci = new ClassInfo(i, getClassName(classes[i - 1]));
    class_info_array[i] = ci;
    TagInfo *ti = new TagInfo(i);
    check_error(jvmti->SetTag(classes[i - 1], (jlong)ti), "SetTag");
  }
  object_info_heap = new ObjectInfoHeap(20);

  jvmtiHeapCallbacks heapCallbacks;
  memset(&heapCallbacks, 0, sizeof(heapCallbacks));
  heapCallbacks.heap_reference_callback = &count_HFR;
  check_error(jvmti->FollowReferences(0, NULL, NULL, &heapCallbacks, NULL),
              "FollowReferences");

  printf("object_number: %d\n", object_number);
  object_info_heap->print();

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
  check_error(jvmti->IterateThroughHeap(0, NULL, &heapCallbacks, NULL),
              "IterateThroughHeap");

  for (int i = 1; i < class_number; i++) {
    delete class_info_array[i];
  }
  free(class_info_array);
  delete object_info_heap;
  check_error(jvmti->Deallocate((unsigned char *)classes), "Deallocate");
}

// JAVA程序将会进入STW状态
JNIEXPORT jint JNICALL Agent_OnAttach(JavaVM *jvm, char *options,
                                      void *reserved) {
  printf("INFO: Agent OnAttach.\n");
  initial_agent(jvm);
  heap_analyze();
  destory_agent();
  return JNI_OK;
}

JNIEXPORT void JNICALL Agent_OnUnload(JavaVM *vm) {
  printf("INFO: Agent OnUnload.\n");
}
