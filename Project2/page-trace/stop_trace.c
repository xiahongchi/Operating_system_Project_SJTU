#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/unistd.h>
#include <linux/module.h>
#include <linux/errno.h>

#define __NR_sys_stop_trace 362
static int (*oldcall_362)(void);

int sys_stop_trace(pid_t pid);


int sys_get_trace(pid_t pid, int *wcounts);

static int addsyscall_init(void){
    long *syscall=(long*)0xc000d8c4;


    oldcall_362=(int(*)(void))(syscall[__NR_sys_stop_trace]);
    syscall[__NR_sys_stop_trace]=(unsigned long)sys_stop_trace;

    printk(KERN_INFO "sys_stop_trace module load!\n");
    return 0;
}

static void addsyscall_exit(void){
    long *syscall=(long*)0xc000d8c4;

    syscall[__NR_sys_stop_trace]=(unsigned long)oldcall_362;

    printk(KERN_INFO "sys_stop_trace mmodule exit!\n");
}
module_init(addsyscall_init);
module_exit(addsyscall_exit);

MODULE_LICENSE("GPL");

int sys_stop_trace(pid_t pid){
    struct task_struct *tsk;
    tsk = pid_task(find_vpid(pid), PIDTYPE_PID);
    if(tsk==NULL){
        printk(KERN_ERR "invalid pid\n");
        return -1;
    }
    if(tsk->trace_flag==0){
        printk(KERN_ERR "stop a process which havn't start page tracing!\n");
        return -EINVAL;
    }
    tsk->trace_flag=0;
    printk(KERN_INFO "stop_trace! pid=%d\n",tsk->pid);
    return 0;
}

