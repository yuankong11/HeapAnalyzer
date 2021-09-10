vmtool
===

> @since 3.5.1

[`vmtool`在线教程](https://arthas.aliyun.com/doc/arthas-tutorials.html?language=cn&id=command-vmtool)

`vmtool` 利用`JVMTI`接口，实现查询内存对象，强制GC等功能。

* [JVM Tool Interface](https://docs.oracle.com/javase/8/docs/platform/jvmti/jvmti.html)

### 获取对象

```bash
$ vmtool --action getInstances --className java.lang.String --limit 10
@String[][
    @String[com/taobao/arthas/core/shell/session/Session],
    @String[com.taobao.arthas.core.shell.session.Session],
    @String[com/taobao/arthas/core/shell/session/Session],
    @String[com/taobao/arthas/core/shell/session/Session],
    @String[com/taobao/arthas/core/shell/session/Session.class],
    @String[com/taobao/arthas/core/shell/session/Session.class],
    @String[com/taobao/arthas/core/shell/session/Session.class],
    @String[com/],
    @String[java/util/concurrent/ConcurrentHashMap$ValueIterator],
    @String[java/util/concurrent/locks/LockSupport],
]
```

> 通过 `--limit`参数，可以限制返回值数量，避免获取超大数据时对JVM造成压力。默认值是10。

### 指定 classloader name

```bash
vmtool --action getInstances --classLoaderClass org.springframework.boot.loader.LaunchedURLClassLoader --className org.springframework.context.ApplicationContext
```


### 指定 classloader hash

可以通过`sc`命令查找到加载class的 classloader。

```bash
$ sc -d org.springframework.context.ApplicationContext
 class-info        org.springframework.boot.context.embedded.AnnotationConfigEmbeddedWebApplicationContext
 code-source       file:/private/tmp/demo-arthas-spring-boot.jar!/BOOT-INF/lib/spring-boot-1.5.13.RELEASE.jar!/
 name              org.springframework.boot.context.embedded.AnnotationConfigEmbeddedWebApplicationContext
...
 class-loader      +-org.springframework.boot.loader.LaunchedURLClassLoader@19469ea2
                     +-sun.misc.Launcher$AppClassLoader@75b84c92
                       +-sun.misc.Launcher$ExtClassLoader@4f023edb
 classLoaderHash   19469ea2
```

然后用`-c`/`--classloader` 参数指定：

```bash
vmtool --action getInstances -c 19469ea2 --className org.springframework.context.ApplicationContext
```

### 指定返回结果展开层数

> `getInstances` action返回结果绑定到`instances`变量上，它是数组。

> 通过 `-x`/`--expand` 参数可以指定结果的展开层次，默认值是1。

```bash
vmtool --action getInstances -c 19469ea2 --className org.springframework.context.ApplicationContext -x 2
```

### 执行表达式

> `getInstances` action返回结果绑定到`instances`变量上，它是数组。可以通过`--express`参数执行指定的表达式。

```bash
vmtool --action getInstances --classLoaderClass org.springframework.boot.loader.LaunchedURLClassLoader --className org.springframework.context.ApplicationContext --express 'instances[0].getBeanDefinitionNames()'
```

### 强制GC

```bash
vmtool --action forceGc
```

* 可以结合 [`vmoption`](vmoption.md) 命令动态打开`PrintGC`开关。

### 分析占用最大堆内存的类、对象及对象间引用关系

```bash
$ vmtool -a heapAnalyze --classNum 3 --objectNum 3 --backtraceNum 5
class_number: 4096
object_number: 107597

id      #bytes          class_name & references
----------------------------------------------------
1       209715216       byte[] <-- ByteHolder <-- root(local variable in method: sleep)
2       104857616       byte[] <-- ByteHolder <-- root(local variable in method: main)
3       524304          char[] <-- java.lang.Object[] <-- java.util.concurrent.ArrayBlockingQueue <-- com.taobao.arthas.core.shell.term.impl.http.api.HttpApiHandler <-- com.taobao.arthas.core.server.ArthasBootstrap <-- java.lang.Class <-- root


id      #instances      #bytes          class_name
----------------------------------------------------
1       7034            327112192       byte[]
2       20523           5704616         char[]
3       2937            631096          java.lang.Object[]
```

> 通过 `--classNum` 参数指定输出的类数量，通过 `--objectNum` 参数指定输出的对象数量，通过 `--backtraceNum` 参数指定回溯对象引用关系的层级，如果 `--backtraceNum` 被设置为-1，则表示不断回溯，直到找到根引用。

> 如果对象的根引用是线程栈，那么在输出的 `root(local variable in method: sleep)` 中会显式输出该引用所处的栈帧方法名，而当对象的根引用来自其他位置，例如JNI栈帧时，无法获得其方法名，只会输出 `root` 。
