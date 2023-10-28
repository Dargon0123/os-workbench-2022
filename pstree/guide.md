# 实验指导
把系统中的进程按照父亲-孩子的树状结构打印到终端。

* -p, --show-pids: 打印每个进程的进程号。
* -n --numeric-sort: 按照pid的数值从小到大顺序输出一个进程的直接孩子。
* -V --version: 打印版本信息。
你可以在命令行中观察系统的 pstree 的执行行为 (如执行 pstree -V、pstree --show-pids 等)。这些参数可能任意组合。

# 解决思路
将问题进行分解

1. 得到命令行参数，根据要求设置标志变量的数值，如`pstree -p` 和`pstree --show-pids`同样效果。
2. 得到系统中所有进程的编号，将其保存到列表中去。
3. 将列表里的每一个编号，得到它的parent是谁。
4. 在内存中把树建立好，且按命令行参数进行排序。
5. 把树打印到终端上。

# 文件夹手册
* /proc/[pid]/stat
(1) pid  %d
(2) comm  %s
(3) state  %c
(4) ppid  %d

* /proc/[pid]/task (since Linux 2.6.0)
线程
This  is  a  directory  that contains one subdirectory for each thread in the process.  The name of each subdirectory is the numerical thread ID ([tid]) of the thread (see gettid(2)).

such as pid 800's tid is 800 895  896
dargon@dd:/proc/800/task$ ls
800  895  896

* /proc/[pid]/task/[pid]/children
子进程
A  space-separated list of child tasks of this task.  Each child task is represented by its TID.