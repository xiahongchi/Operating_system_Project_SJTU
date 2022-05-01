#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/unistd.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <asm/uaccess.h>

#define __NR_sys_get_trace 363
static int (*oldcall_363)(void);


int sys_get_trace(pid_t pid, int *wcounts);

static int addsyscall_init(void){
    long *syscall=(long*)0xc000d8c4;


    oldcall_363=(int(*)(void))(syscall[__NR_sys_get_trace]);
    syscall[__NR_sys_get_trace]=(unsigned long)sys_get_trace;

    printk(KERN_INFO "sys_get_trace module load!\n");
    return 0;
}

static void addsyscall_exit(void){
    long *syscall=(long*)0xc000d8c4;

    syscall[__NR_sys_get_trace]=(unsigned long)oldcall_363;

    printk(KERN_INFO "sys_get_trace mmodule exit!\n");
}
module_init(addsyscall_init);
module_exit(addsyscall_exit);

int sys_get_trace(pid_t pid, int *wcounts){
    struct task_struct *tsk;
    tsk = pid_task(find_vpid(pid), PIDTYPE_PID);
    if(tsk==NULL){
        printk(KERN_ERR "invalid pid\n");
        return -1;
    }
    put_user(tsk->wcounts,wcounts);
    return 0;
}

MODULE_LICENSE("GPL");