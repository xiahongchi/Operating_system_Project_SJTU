#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/unistd.h>
#include <linux/module.h>
#include <linux/errno.h>

#define __NR_sys_set_wcounts 365
static int (*oldcall_365)(void);

int sys_set_wcounts(pid_t pid,int wcounts);

static int addsyscall_init(void){
    long *syscall=(long*)0xc000d8c4;


    oldcall_365=(int(*)(void))(syscall[__NR_sys_set_wcounts]);
    syscall[__NR_sys_set_wcounts]=(unsigned long)sys_set_wcounts;

    printk(KERN_INFO "sys_set_wcounts module load!\n");
    return 0;
}

static void addsyscall_exit(void){
    long *syscall=(long*)0xc000d8c4;

    syscall[__NR_sys_set_wcounts]=(unsigned long)oldcall_365;

    printk(KERN_INFO "sys_set_wcounts module exit!\n");
}
module_init(addsyscall_init);
module_exit(addsyscall_exit);

MODULE_LICENSE("GPL");

int sys_set_wcounts(pid_t pid,int wcounts){
    struct task_struct *tsk;
    tsk = pid_task(find_vpid(pid), PIDTYPE_PID);
    if(tsk==NULL){
        printk(KERN_ERR "invalid pid\n");
        return -1;
    }
    tsk->wcounts=wcounts;
    printk(KERN_INFO "set_wcounts! pid=%d\n",tsk->pid);
    return 0;
}

