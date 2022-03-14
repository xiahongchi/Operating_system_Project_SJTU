/*
 *-------program sys_ptree.c--------
 *-------Done by Xia Hongchi--------
 *-----For Project 1, Problem 1-----
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/unistd.h>
#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/list.h>
#include <asm/uaccess.h>

#define __NR_origincall 356

#define Put_user(x, ptr)                                               \
    ({                                                                 \
        if (put_user(x, ptr) != 0)                                     \
            printk(KERN_ERR "put_user() in pstree syscall failed!\n"); \
    })

#define Copy_to_user(to, from, n)                                          \
    ({                                                                     \
        if (copy_to_user(to, from, n) != 0)                                \
            printk(KERN_ERR "copy_to_user() in pstree syscall failed!\n"); \
    })

struct prinfo
{
    pid_t parent_pid;       /* process id of parent */
    pid_t pid;              /* process id */
    pid_t first_child_pid;  /* pid of youngest child */
    pid_t next_sibling_pid; /* pid of older sibling */
    long state;             /* current state of process */
    long uid;               /* user id of process owner */
    char comm[16];          /* name of program executed */
    int layer;              /* layer of process */
};
static void pstree_dfs(struct prinfo *buf, int *nr, int *idx,
                       int *layer, struct task_struct *task);
static int ptree(struct prinfo *buf, int *nr);

static int (*oldcall)(void);

static void pstree_dfs(struct prinfo *buf, int *nr, int *idx,
                       int *layer, struct task_struct *task)
{
    struct list_head *list;
    struct task_struct *t_child;
    list_for_each(list, &(task->children))
    {
        read_lock(&tasklist_lock);
        t_child = list_entry(list, struct task_struct, sibling);
        read_unlock(&tasklist_lock);
        if (*idx < *nr)
        {
            struct list_head *p;
            struct task_struct *q;
            int zero = 0;
            read_lock(&tasklist_lock);
            // ppid
            q = t_child->real_parent;
            if (q)
            {
                Put_user(q->pid, &(buf[*idx].parent_pid));
            }
            else
            {
                Put_user(zero, &(buf[*idx].parent_pid));
            }
            read_unlock(&tasklist_lock);

            // pid
            read_lock(&tasklist_lock);
            Put_user(t_child->pid, &(buf[*idx].pid));
            read_unlock(&tasklist_lock);

            // children
            read_lock(&tasklist_lock);
            p = &(t_child->children);
            if (list_empty(p))
            {
                Put_user(zero, &(buf[*idx].first_child_pid));
            }
            else
            {
                q = list_first_entry(p, struct task_struct, sibling);
                Put_user(q->pid, &(buf[*idx].first_child_pid));
            }
            read_unlock(&tasklist_lock);

            // sibling
            read_lock(&tasklist_lock);
            p = &(t_child->sibling);
            if (list_empty(p))
            {
                Put_user(zero, &(buf[*idx].next_sibling_pid));
            }
            else
            {
                if (p->next == &(t_child->real_parent->children))
                {
                    Put_user(zero, &(buf[*idx].next_sibling_pid));
                }
                else
                {
                    q = list_first_entry(p, struct task_struct, sibling);
                    Put_user(q->pid, &(buf[*idx].next_sibling_pid));
                }
            }
            read_unlock(&tasklist_lock);

            // state
            read_lock(&tasklist_lock);
            Put_user(t_child->state, &(buf[*idx].state));
            read_unlock(&tasklist_lock);

            // uid
            read_lock(&tasklist_lock);
            Put_user(t_child->cred->uid, &(buf[*idx].uid));
            read_unlock(&tasklist_lock);

            // comm
            read_lock(&tasklist_lock);
            Copy_to_user((void *)buf[*idx].comm, (void *)t_child->comm, 16);
            read_unlock(&tasklist_lock);

            // layer
            read_lock(&tasklist_lock);
            Put_user(*layer, &(buf[*idx].layer));
            read_unlock(&tasklist_lock);

            *idx = *idx + 1;
        }
        *layer = *layer + 1;
        pstree_dfs(buf, nr, idx, layer, t_child);
        *layer = *layer - 1;
    }
}
static int ptree(struct prinfo *buf, int *nr)
{
    int idx = 0;
    int layer = 0;
    if (buf == 0 || nr == 0)
        return 1;
    read_lock(&tasklist_lock);
    printk(KERN_INFO "Start ptree syscall!\n");
    if (idx < *nr)
    {
        struct list_head *p;
        struct task_struct *q;
        int zero = 0;
        // ppid
        Put_user(zero, &(buf[idx].parent_pid));

        //  pid
        Put_user(init_task.pid, &(buf[idx].pid));

        //  children:
        p = &init_task.children;
        if (list_empty(p))
        {
            Put_user(zero, &(buf[idx].first_child_pid));
        }
        else
        {
            q = list_first_entry(p, struct task_struct, sibling);
            Put_user(q->pid, &(buf[idx].first_child_pid));
        }

        // sibling
        Put_user(zero, &(buf[idx].next_sibling_pid));

        // state
        Put_user(init_task.state, &(buf[idx].state));

        // uid
        Put_user((init_task.cred)->uid, &(buf[idx].uid));

        //  comm
        Copy_to_user(buf[idx].comm, init_task.comm, 16);

        // layer
        Put_user(layer, &(buf[idx].layer));
        idx++;
    }
    read_unlock(&tasklist_lock);
    layer++;
    pstree_dfs(buf, nr, &idx, &layer, &init_task);
    layer--;
    if (idx < *nr)
    {
        char str[] = "Finish here!";
        Copy_to_user(buf[idx].comm, str, 13);
    }
    printk(KERN_INFO "Finish ptree syscall!\n");
    return 0;
}
static int addsyscall_init(void)
{
    long *syscall = (long *)0xc000d8c4;
    oldcall = (int (*)(void))(syscall[__NR_origincall]);
    syscall[__NR_origincall] = (unsigned long)ptree;
    printk(KERN_INFO "module ptree syscall load!\n");
    return 0;
}

static void addsyscall_exit(void)
{
    long *syscall = (long *)0xc000d8c4;
    syscall[__NR_origincall] = (unsigned long)oldcall;
    printk(KERN_INFO "module ptree syscall exit!\n");
}

module_init(addsyscall_init);
module_exit(addsyscall_exit);