#ifndef HEAP_ANALYZER_H
#define HEAP_ANALYZER_H

#include <cstdlib>
#include <jni_md.h>
#include <jvmti.h>

class ClassInfo {
public:
  int id = 0;
  char *name = 0;
  int instance_count = 0;
  long total_size = 0;
  ClassInfo(int id, char *name) : id(id), name(name) {}
  ~ClassInfo() { free(name); }
  static bool compare(ClassInfo *ci1, ClassInfo *ci2) {
    return ci1->total_size > ci2->total_size;
  }
};

class TagInfo {
public:
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

class ObjectInfo {
public:
  int size = 0;
  TagInfo *object_tag = 0;
  ObjectInfo(int size, TagInfo *object_tag)
      : size(size), object_tag(object_tag) {}
  ObjectInfo() {}
  bool less(ObjectInfo *oi) { return this->size < oi->size; }
};

class ObjectInfoHeap {
private:
  int record_number = 0;
  ObjectInfo **array = 0;

  void swap(int i, int j);
  void adjust(int cur, int limit);
  void sort();

public:
  ObjectInfoHeap(int record_number);
  ~ObjectInfoHeap();
  void add(int size, TagInfo *tag);
  void print();
};

void initial_agent(jvmtiEnv *env);
void destory_agent();
void heap_analyze();

#endif
