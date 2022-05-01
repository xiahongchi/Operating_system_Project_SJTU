#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/unistd.h>
#include <linux/module.h>
#include <linux/errno.h>

#define __NR_sys_rr_fifo_trace 364
static int (*oldcall_364)(void);

int sys_rr_fifo_trace(pid_t pid);

static int addsyscall_init(void){
    long *syscall=(long*)0xc000d8c4;


    oldcall_364=(int(*)(void))(syscall[__NR_sys_rr_fifo_trace]);
    syscall[__NR_sys_rr_fifo_trace]=(unsigned long)sys_rr_fifo_trace;

    printk(KERN_INFO "sys_rr_fifo_trace module load!\n");
    return 0;
}

static void addsyscall_exit(void){
    long *syscall=(long*)0xc000d8c4;

    syscall[__NR_sys_rr_fifo_trace]=(unsigned long)oldcall_364;

    printk(KERN_INFO "sys_rr_fifo_trace module exit!\n");
}
module_init(addsyscall_init);
module_exit(addsyscall_exit);

MODULE_LICENSE("GPL");

int sys_rr_fifo_trace(pid_t pid){
    struct task_struct *tsk;
    tsk = pid_task(find_vpid(pid), PIDTYPE_PID);
    if(tsk==NULL){
        printk(KERN_ERR "invalid pid\n");
        return -1;
    }
    tsk->rr_fifo_trace=1;
    printk(KERN_INFO "rr_fifo_trace! pid=%d\n",tsk->pid);
    return 0;
}

