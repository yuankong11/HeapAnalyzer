### Analyze heap usage

`vmtool -a heapAnalyze --classNum 3 --objectNum 3 --backtraceNum 5`{{execute T2}}

> Use the `--classNum` parameter to specify classes that will be shown, use the `--objectNum` parameter to specify objects that will be shown, use the `--backtraceNum` parameter to specify how many times of backtrace by references among objects will be done, and set `--backtraceNum` as -1 to make backtrace do not finish until root is reached.

> If the root reference(the first reference from root, such as stack) of objects is stack of java threads, then the method name will be printed, otherwise only `root` will be printed.
