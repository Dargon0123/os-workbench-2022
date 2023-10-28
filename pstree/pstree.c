#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>


#define FALSE 0
#define TRUE 1


bool showpid = FALSE, showthr = FALSE;
bool issort =FALSE;

/* 构建多叉树节点 */
typedef struct Process_t {
    int pid, ppid, n_son, n_thr;
    char name[512];
    struct Process_t* son[128];
    struct Process_t* thr[128];
}Process;


/* 判定数字 */
bool isnumber(const char* str) {
    int len = strlen(str);
    for (int i = 0; i <len; ++i) {
        if (str[i] > '9' || str[i] < '0') {
            return false;
        }
    }
    return true;
}

/* 比较字符串大小 */
bool my_strcmp(char* str1, char* str2) {
    int len1 = strlen(str1);
    int len2 = strlen(str2);
    int len = len1 < len2 ? len1 : len2;

    for (int i = 0; i < len; ++i) {
        if (str1[i] > str2[i]) {
            return true;
        }
        else if (str1[i] < str2[i]) {
            return false;
        }
    }
    return len1 > len2;
}


char tmp[2048];
void getinfo(Process* ret, int pid) {
    char childfile[256], statname[256], taskdirname[256];
    sprintf(statname, "/proc/%d/stat", pid);
    sprintf(taskdirname, "/proc/%d/task/", pid);
    sprintf(childfile, "/proc/%d/task/%d/children", pid, pid);

    /* 提取该 pid 信息 */
    FILE* fp = fopen(statname, "r");
    if (fp == NULL) {
        printf("Error opening %s directory!\n", statname);
        return;
    }
    fscanf(fp, "%d", &ret->pid);  /* get pid */
    fscanf(fp, "%s", tmp); /* get name (systemd) */
    tmp[strlen(tmp) -1] = '\0'; /* delete "()" */
    strcpy(ret->name, tmp + 1);
    fscanf(fp, "%c", tmp); /* get state */
    fscanf(fp, "%d", &ret->ppid); /* get ppid */
    fclose(fp);

    /* /proc/pid/task/pid/children */
    /* 递归提取子进程信息 */
    fp = fopen(childfile, "r");
    if (fp == NULL) {
        printf("Error opening %s directory!\n", childfile);
        return;
    }
    ret->n_son = 0;
    int childpid;
    while ((fscanf(fp, "%d", &childpid)) != EOF) {
        ret->son[ret->n_son] = (Process*)malloc(sizeof(Process));
        getinfo(ret->son[ret->n_son], childpid);
        ret->n_son++;
    }

    /* /proc/pid/task/ */
    /* 提取进程的线程信息 */
    DIR* dir;
    struct dirent* entry;
    ret->n_thr = 0;
    dir = opendir(taskdirname);
    if (dir == NULL) {
        printf("Error opendir %s directory!\n", taskdirname);
        return;
    }
    while ((entry = readdir(dir)) != NULL) {
        /* 判断是否entry->d_name 数字 */
        if (isnumber(entry->d_name)) {
            int tid = atoi(entry->d_name);
            if (tid != pid) {
                if (showpid) {
                    ret->thr[ret->n_thr] = (Process*) malloc(sizeof(Process));
                    /* 初始化子进程 name 和 pid */
                    sprintf(tmp, "{%s}", ret->name);
                    strcpy(ret->thr[ret->n_thr]->name, tmp);
                    ret->thr[ret->n_thr]->pid = tid;
                    ret->thr[ret->n_thr]->n_son = 0;
                    ret->thr[ret->n_thr]->n_thr = 0;
                }
                ret->n_thr++;
            }
            if (!showpid && ret->n_thr > 0) {
                ret->thr[0] = (Process*) malloc(sizeof(Process));
                sprintf(tmp, "{%s}", ret->name);
                strcpy(ret->thr[0]->name, tmp);
                ret->thr[0]->pid = ret->n_thr; /* 2*[{ModemManager}] */
                ret->n_thr = 1;
                ret->thr[0]->n_thr = ret->thr[0]->n_son = 0;
            }
        }
    }
    closedir(dir);

}


char pre[512] = "";
int stack[512];
int head = 0;
bool isroot = TRUE;

/* 进行遍历打印进程 */
void search(Process* cur, int type, bool isproc) {
    /* 打印root进程信息 */
    if (isroot) {
        if (showpid) {
            sprintf(tmp, "%s(%d)", cur->name, cur->pid);
        }
        else {
            sprintf(tmp, "%s", cur->name);
        }
        isroot = false;
        stack[++head] = strlen(tmp) + 1; /* 记录长度 */
    }
    else {
        /* 打印第一个子树 */
        if (type == 0) {
            if (showpid) {
                sprintf(tmp, "-%s(%d)", cur->name, cur->pid);
            }
            else {
                sprintf(tmp, "-%s", cur->name);
            }
            stack[head + 1] = stack[head] + strlen(tmp) + 1;
            head++;
        }
        else {
            if (type == -1) {
                pre[stack[head] - 1] = '\\';
            }
            if (showpid) {
                sprintf(tmp, "%s-%s(%d)", pre, cur->name, cur->pid);
            }
            else {
                sprintf(tmp, "%s-%s", pre, cur->name);
            }
            stack[++head] = strlen(tmp) + 1;
        }        
    }

    printf("%s", tmp);

    if (type == -1) {
        pre[stack[head - 1] - 1] = ' ';
    }
    for (int i = stack[head - 1]; i < stack[head]; ++i) {
        /* 为子进程做准备，将之前父进程的空出来 */
        pre[i] = ' ';
    }
    pre[stack[head]] = '|';

    if ((cur->n_son + cur->n_thr) == 0) { /* 无子进程，线程，结束本行 */
        printf("\n");
    }
    else if ((cur->n_son + cur->n_thr) == 1) {
        printf("--");
        pre[stack[head]] = ' ';
    }
    else {
        printf("-+");
    }

    pre[++stack[head]] = '\0';

    if (isproc) {
        /* 子进程按照pid 排序 */
        if (issort) {
            for (int i = 0; i < cur->n_son; ++i) {
                for (int j = i + 1; j < cur->n_son; ++j) {
                    if (cur->son[i]->pid > cur->son[j]->pid) {
                        /* swap */
                        Process* tmp = cur->son[i];
                        cur->son[i] = cur->son[j];
                        cur->son[j] = tmp;
                    }
                }
            }
        }
        /* 按照 pid‘s name 字符串排序 */
        else {
            for (int i = 0; i < cur->n_son; ++i) {
                for (int j = i + 1; j < cur->n_son; ++j) {
                    if (my_strcmp(cur->son[i]->name, cur->son[j]->name)) {
                        /* swap */
                        Process* tmp = cur->son[i];
                        cur->son[i] = cur->son[j];
                        cur->son[j] = tmp;
                    }
                }
            }
        }
        /* 递归遍历 */
        for (int i = 0; i < cur->n_son; ++i) {
            int ith = i;
            if (cur->n_son > 1 && cur->n_thr == 0 && cur->n_son - 1 == i) {
                ith = -1;
            }
            search(cur->son[i], ith, true);
        }

        if (showpid) {

        }
        else if (cur->n_thr > 0) {
            if (cur->thr[0]->pid > 1) {
                sprintf(tmp, "%d*[%s]", cur->thr[0]->pid, cur->thr[0]->name);
                strcpy(cur->thr[0]->name, tmp);
            }
            int ith;
            if (cur->n_son == 0) ith = 0;
            else ith = -1;
            search(cur->thr[0], ith, false);
        }
    }
    /* 回退 */
    head--;
    pre[stack[head]] = '\0';
}



int main(int argc, char *argv[]) {

    /* 1 获取命令行参数 */
    for (int i = 0; i < argc; i++) {
        assert(argv[i]);
        printf("argv[%d] = %s\n", i, argv[i]);
    }
    assert(!argv[argc]);

    int opt;
    while ((opt = getopt(argc, argv, "Vnp?")) != -1) {
        switch (opt) {
            case 'V':
                printf("pstree ver 0.1\n");
                break;
            case 'n':
                issort = TRUE;
                break;
            case 'p':
                showpid = TRUE;
                break;  
            default:
                printf("Only use -V, -p, -n\n");
                return -1;
        }
    }

    Process* root = malloc(sizeof(Process));

    /* 2 得到系统中所有进程的编号，保存到多叉树中 */
    getinfo(root, 1);

    /* 3 遍历该树，进行打印 */
    search(root, 0, true);

    return 0;
}
