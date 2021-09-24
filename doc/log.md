```bash
[arthas@10243]$ vmtool -a heapAnalyze --classNum 5 --objectNum 10
class_number: 4101
object_number: 107299

id      #bytes          class_name
----------------------------------------------------
1       209715216       byte[]
2       104857616       byte[]
3       524304          char[]
4       524304          char[]
5       524304          char[]
6       524304          char[]
7       524304          char[]
8       524304          char[]
9       524304          char[]
10      524304          char[]


id      #instances      #bytes          class_name
----------------------------------------------------
1       7043            327124360       byte[]
2       20303           5660096         char[]
3       2936            631136          java.lang.Object[]
4       20270           486480          java.lang.String
5       4110            462904          java.lang.Class

[arthas@10243]$ vmtool -a referenceAnalyze --className ByteHolder

id      #bytes          class_name & references
----------------------------------------------------
1       16              ByteHolder <-- root(local variable in method: main)
2       16              ByteHolder <-- root(local variable in method: sleep)
```

