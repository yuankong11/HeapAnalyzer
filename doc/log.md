# windows10平台：

```bash
[arthas@14452]$ vmtool -a heapAnalyze --classNum 5 --objectNum 5 --backtraceNum 10
class_number: 4129
object_number: 116045

id      #bytes          class_name & references
----------------------------------------------------
1       209715216       byte[] <-- ByteHolder <-- root(local variable in method: sleep)
2       104857616       byte[] <-- ByteHolder <-- root(local variable in method: main)
3       524304          char[] <-- java.lang.Object[] <-- java.util.concurrent.ArrayBlockingQueue <-- com.taobao.arthas.core.shell.term.impl.http.api.HttpApiHandler <-- com.taobao.arthas.core.server.ArthasBootstrap <-- java.lang.Class <-- root
4       524304          char[] <-- java.lang.Object[] <-- java.util.concurrent.ArrayBlockingQueue <-- com.taobao.arthas.core.shell.term.impl.http.api.HttpApiHandler <-- com.taobao.arthas.core.server.ArthasBootstrap <-- java.lang.Class <-- root
5       524304          char[] <-- java.lang.Object[] <-- java.util.concurrent.ArrayBlockingQueue <-- com.taobao.arthas.core.shell.term.impl.http.api.HttpApiHandler <-- com.taobao.arthas.core.server.ArthasBootstrap <-- java.lang.Class <-- root


id      #instances      #bytes          class_name
----------------------------------------------------
1       6574            323552728       byte[]
2       22582           5945080         char[]
3       1891            822600          int[]
4       2956            637712          java.lang.Object[]
5       22214           533136          java.lang.String
```

# linux平台：

```bash
[arthas@5244]$ vmtool -a heapAnalyze --classNum 3 --objectNum 3 --backtraceNum 5
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

# linux平台(dacapo应用)：

```bash
[arthas@6504]$ vmtool -a heapAnalyze --classNum 5 --objectNum 10 --backtraceNum -1
class_number: 4520
object_number: 5293587

id      #bytes          class_name & references
----------------------------------------------------
1       2097168         java.lang.Object[] <-- org.h2.util.ObjectArray <-- org.h2.index.ScanIndex <-- org.h2.table.TableData <-- org.h2.constraint.ConstraintReferential <-- java.lang.Object[] <-- org.h2.util.ObjectArray <-- org.h2.table.TableData <-- org.h2.constraint.ConstraintReferential <-- java.lang.Object[] <-- org.h2.util.ObjectArray <-- org.h2.table.TableData <-- org.h2.constraint.ConstraintReferential <-- java.lang.Object[] <-- org.h2.util.ObjectArray <-- org.h2.table.TableData <-- org.h2.constraint.ConstraintReferential <-- java.lang.Object[] <-- org.h2.util.ObjectArray <-- org.h2.table.TableData <-- java.util.HashMap$Node <-- java.util.HashMap$Node[] <-- java.util.HashMap <-- org.h2.schema.Schema <-- org.h2.engine.Database <-- root(local variable in method: executeUpdate)
2       524304          char[] <-- java.lang.Object[] <-- java.util.concurrent.ArrayBlockingQueue <-- com.taobao.arthas.core.shell.term.impl.http.api.HttpApiHandler <-- com.taobao.arthas.core.server.ArthasBootstrap <-- java.lang.Class <-- root
3       524304          byte[] <-- java.lang.Class <-- root
4       524304          char[] <-- java.lang.Object[] <-- java.util.concurrent.ArrayBlockingQueue <-- com.taobao.arthas.core.shell.term.impl.http.api.HttpApiHandler <-- com.taobao.arthas.core.server.ArthasBootstrap <-- java.lang.Class <-- root
5       524304          char[] <-- java.lang.Object[] <-- java.util.concurrent.ArrayBlockingQueue <-- com.taobao.arthas.core.shell.term.impl.http.api.HttpApiHandler <-- com.taobao.arthas.core.server.ArthasBootstrap <-- java.lang.Class <-- root
6       524304          char[] <-- java.lang.Object[] <-- java.util.concurrent.ArrayBlockingQueue <-- com.taobao.arthas.core.shell.term.impl.http.api.HttpApiHandler <-- com.taobao.arthas.core.server.ArthasBootstrap <-- java.lang.Class <-- root
7       524304          char[] <-- java.lang.Object[] <-- java.util.concurrent.ArrayBlockingQueue <-- com.taobao.arthas.core.shell.term.impl.http.api.HttpApiHandler <-- com.taobao.arthas.core.server.ArthasBootstrap <-- java.lang.Class <-- root
8       524304          java.lang.Object[] <-- org.h2.util.ObjectArray <-- org.h2.index.ScanIndex <-- org.h2.table.TableData <-- org.h2.constraint.ConstraintReferential <-- java.lang.Object[] <-- org.h2.util.ObjectArray <-- org.h2.table.TableData <-- org.h2.constraint.ConstraintReferential <-- java.lang.Object[] <-- org.h2.util.ObjectArray <-- org.h2.table.TableData <-- org.h2.constraint.ConstraintReferential <-- java.lang.Object[] <-- org.h2.util.ObjectArray <-- org.h2.table.TableData <-- org.h2.constraint.ConstraintReferential <-- java.lang.Object[] <-- org.h2.util.ObjectArray <-- org.h2.table.TableData <-- org.h2.constraint.ConstraintReferential <-- java.lang.Object[] <-- org.h2.util.ObjectArray <-- org.h2.table.TableData <-- java.util.HashMap$Node <-- java.util.HashMap$Node[] <-- java.util.HashMap <-- org.h2.schema.Schema <-- org.h2.engine.Database <-- root(local variable in method: executeUpdate)
9       524304          char[] <-- java.lang.Object[] <-- java.util.concurrent.ArrayBlockingQueue <-- com.taobao.arthas.core.shell.term.impl.http.api.HttpApiHandler <-- com.taobao.arthas.core.server.ArthasBootstrap <-- java.lang.Class <-- root
10      524304          char[] <-- java.lang.Object[] <-- java.util.concurrent.ArrayBlockingQueue <-- com.taobao.arthas.core.shell.term.impl.http.api.HttpApiHandler <-- com.taobao.arthas.core.server.ArthasBootstrap <-- java.lang.Class <-- root


id      #instances      #bytes          class_name
----------------------------------------------------
1       542333          43386640        org.h2.result.Row
2       542729          36854984        org.h2.value.Value[]
3       1062110         33987520        org.h2.index.TreeNode
4       450270          26855120        char[]
5       978333          15653328        org.h2.value.ValueStringFixed
```