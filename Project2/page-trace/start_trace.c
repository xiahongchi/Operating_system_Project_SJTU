#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/unistd.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/mm_types.h>
#include <asm/pgtable.h>
#include <linux/pid.h>
#include <linux/mman.h>
#include <linux/mm.h>
#include <linux/kallsyms.h>
#include <linux/mmu_context.h>

#include <linux/hugetlb.h>
#include <linux/shm.h>
#include <linux/fs.h>
#include <linux/highmem.h>
#include <linux/security.h>
#include <linux/mempolicy.h>
#include <linux/personality.h>
#include <linux/syscalls.h>
#include <linux/swap.h>
#include <linux/swapops.h>
#include <linux/mmu_notifier.h>
#include <linux/migrate.h>
#include <linux/perf_event.h>
#include <asm/uaccess.h>
#include <asm/pgtable.h>
#include <asm/cacheflush.h>
#include <asm/tlbflush.h>

#define __NR_sys_start_trace 361
static int (*oldcall_361)(void);

int sys_start_trace(pid_t pid, unsigned long start, size_t size);
int mprotect_s(unsigned long start, size_t len,
		unsigned long prot);

int (*security_file_mprotect_s)(struct vm_area_struct * vma,
            unsigned long reqprot, unsigned long prot)=NULL;

int (*mprotect_fixup_s)(struct vm_area_struct *vma, struct vm_area_struct **pprev, 
            unsigned long start, unsigned long end, unsigned long newflags)=NULL;

static int addsyscall_init(void){
    long *syscall=(long*)0xc000d8c4;

    oldcall_361=(int(*)(void))(syscall[__NR_sys_start_trace]);
    syscall[__NR_sys_start_trace]=(unsigned long)sys_start_trace;

    mprotect_fixup_s=(void *) kallsyms_lookup_name("mprotect_fixup");
    if (!mprotect_fixup_s) {
        printk(KERN_ERR "Could not find mprotect_fixup.\n");
        return -ENOSYS;
    }

    security_file_mprotect_s=(void *) kallsyms_lookup_name("security_file_mprotect");
    if (!security_file_mprotect_s) {
        printk(KERN_ERR "Could not find security_file_mprotect.\n");
        return -ENOSYS;
    }

    printk(KERN_INFO "sys_start_trace module load!\n");
    return 0;
}

static void addsyscall_exit(void){
    long *syscall=(long*)0xc000d8c4;

    syscall[__NR_sys_start_trace]=(unsigned long)oldcall_361;
    
    printk(KERN_INFO "sys_start_trace module exit!\n");
}
module_init(addsyscall_init);
module_exit(addsyscall_exit);

MODULE_LICENSE("GPL");

int sys_start_trace(pid_t pid, unsigned long start, size_t size){
    struct task_struct *tsk;
    tsk = pid_task(find_vpid(pid), PIDTYPE_PID);
    if(tsk==NULL){
        printk(KERN_ERR "invalid pid\n");
        return -1;
    }
    if(tsk->pid != current->pid){
        printk(KERN_ERR "unmatched pid with current process!\n");
        return -1;
    }
    if(tsk->trace_flag==1){
        printk(KERN_ERR "second call to sys_start_trace without sys_stop_trace!\n");
        return -EINVAL;
    }
    tsk->trace_flag=1;
    tsk->wcounts=0;

   


    if(mprotect_s(start,size,PROT_READ)<0){
        printk(KERN_ERR "mprotect_fault in sys_start_trace\n");
        return -1;
    }
    printk(KERN_INFO "start_trace! pid=%d\n",tsk->pid);
    return 0;
}

int mprotect_s(unsigned long start, size_t len,
		unsigned long prot)
{
	unsigned long vm_flags, nstart, end, tmp, reqprot;
	struct vm_area_struct *vma, *prev;
	int error = -EINVAL;
	const int grows = prot & (PROT_GROWSDOWN|PROT_GROWSUP);
	prot &= ~(PROT_GROWSDOWN|PROT_GROWSUP);
	if (grows == (PROT_GROWSDOWN|PROT_GROWSUP)) /* can't be both */
		return -EINVAL;

	if (start & ~PAGE_MASK)
		return -EINVAL;
	if (!len)
		return 0;
	len = PAGE_ALIGN(len);
	end = start + len;
	if (end <= start)
		return -ENOMEM;
	if (!arch_validate_prot(prot))
		return -EINVAL;

	reqprot = prot;
	/*
	 * Does the application expect PROT_READ to imply PROT_EXEC:
	 */
	if ((prot & PROT_READ) && (current->personality & READ_IMPLIES_EXEC))
		prot |= PROT_EXEC;

	vm_flags = calc_vm_prot_bits(prot);

	down_write(&current->mm->mmap_sem);

	vma = find_vma(current->mm, start);
	error = -ENOMEM;
	if (!vma)
		goto out;
	prev = vma->vm_prev;
	if (unlikely(grows & PROT_GROWSDOWN)) {
		if (vma->vm_start >= end)
			goto out;
		start = vma->vm_start;
		error = -EINVAL;
		if (!(vma->vm_flags & VM_GROWSDOWN))
			goto out;
	}
	else {
		if (vma->vm_start > start)
			goto out;
		if (unlikely(grows & PROT_GROWSUP)) {
			end = vma->vm_end;
			error = -EINVAL;
			if (!(vma->vm_flags & VM_GROWSUP))
				goto out;
		}
	}
	if (start > vma->vm_start)
		prev = vma;

	for (nstart = start ; ; ) {
		unsigned long newflags;

		/* Here we know that  vma->vm_start <= nstart < vma->vm_end. */

		newflags = vm_flags | (vma->vm_flags & ~(VM_READ | VM_WRITE | VM_EXEC));

		/* newflags >> 4 shift VM_MAY% in place of VM_% */
		if ((newflags & ~(newflags >> 4)) & (VM_READ | VM_WRITE | VM_EXEC)) {
			error = -EACCES;
			goto out;
		}

		error = security_file_mprotect_s(vma, reqprot, prot);
		if (error)
			goto out;

		tmp = vma->vm_end;
		if (tmp > end)
			tmp = end;
		error = mprotect_fixup_s(vma, &prev, nstart, tmp, newflags);
		if (error)
			goto out;
		nstart = tmp;

		if (nstart < prev->vm_end)
			nstart = prev->vm_end;
		if (nstart >= end)
			goto out;

		vma = prev->vm_next;
		if (!vma || vma->vm_start != nstart) {
			error = -ENOMEM;
			goto out;
		}
	}
out:
	up_write(&current->mm->mmap_sem);
	return error;
}

