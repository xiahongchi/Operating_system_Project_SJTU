/*
 * RAS Scheduling Class
 */

#include "sched.h"

#include <linux/slab.h>

extern char *task_group_path(struct task_group *tg);

static inline int on_ras_rq(struct sched_ras_entity *ras_se)
{
	return !list_empty(&ras_se->run_list);
}

static inline struct ras_rq *ras_rq_of_se(struct sched_ras_entity *ras_se)
{
	return ras_se->ras_rq;
}

void init_ras_rq(struct ras_rq *ras_rq, struct rq *rq)
{
	INIT_LIST_HEAD(&ras_rq->run_list);
	ras_rq->ras_time = 0;
	ras_rq->ras_nr_running = 0;
    raw_spin_lock_init(&ras_rq->ras_runtime_lock);
    #ifdef CONFIG_RAS_GROUP_SCHED
    ras_rq->rq = rq;
    #endif
    printk(KERN_INFO "initing ras run queue...\n");
}

static int calc_ras_weight(struct task_struct *p){
    
    int weight;

    if(p->wcounts < (1 << 4)) weight = 10;
    else if(p->wcounts < (1 << 5)) weight = 9;
    else if(p->wcounts < (1 << 6)) weight = 8;
    else if(p->wcounts < (1 << 7)) weight = 7;
    else if(p->wcounts < (1 << 8)) weight = 6;
    else if(p->wcounts < (1 << 9)) weight = 5;
    else if(p->wcounts < (1 << 10)) weight = 4;
    else if(p->wcounts < (1 << 11)) weight = 3;
    else if(p->wcounts < (1 << 12)) weight = 2;
    else if(p->wcounts < (1 << 13)) weight = 1;
    else weight = 0;

    printk(KERN_INFO "calculating weight: pid=%d wcounts=%d weight=%d\n",p->pid,p->wcounts,weight);
    return weight;

}

static void update_curr_ras(struct rq *rq){

    struct task_struct *curr = rq->curr;
	
	u64 delta_exec;

    printk(KERN_INFO "update_curr_ras: process running now: pid=%d\n",curr->pid);
    if (curr->sched_class != &ras_sched_class)
		return;
    

	delta_exec = rq->clock_task - curr->se.exec_start;
	if (unlikely((s64)delta_exec < 0))
		delta_exec = 0;

    printk(KERN_INFO "running time of pid=%d since last update: %llu\n",curr->pid,delta_exec);
    schedstat_set(curr->se.statistics.exec_max,
		      max(curr->se.statistics.exec_max, delta_exec));

	curr->se.sum_exec_runtime += delta_exec;
	account_group_exec_runtime(curr, delta_exec);

	curr->se.exec_start = rq->clock_task;
	cpuacct_charge(curr, delta_exec);
    
    printk(KERN_INFO "update_curr_ras: finished!\n");
}

static void enqueue_task_ras(struct rq *rq, struct task_struct *p, int flags){
    
    struct sched_ras_entity *ras_se = &p->ras;
    struct ras_rq *ras_rq = &rq->ras;
    char *gp;

    printk(KERN_INFO "enqueue_task_ras: pid=%d is now enqueuing...\n",p->pid);

    gp = task_group_path(task_group(p));
    ras_se->weight = calc_ras_weight(p);
    
	if(strcmp(gp,"/") == 0){//foreground
		p->ras.time_slice = RAS_FG_TIMESLICE + ras_se->weight * RAS_INC_TIME;
        printk(KERN_INFO "FG task! pid=%d and get time slice=%d\n",p->pid,p->ras.time_slice);
	}
    else if(strcmp(gp,"/bg_non_interactive") == 0){//background
		p->ras.time_slice = RAS_BG_TIMESLICE + ras_se->weight * RAS_INC_TIME;
        printk(KERN_INFO "BG task! pid=%d and get time slice=%d\n",p->pid,p->ras.time_slice);
	}
    else{
		p->ras.time_slice = RAS_BG_TIMESLICE + ras_se->weight * RAS_INC_TIME;
        printk(KERN_INFO "Unknown type task! pid=%d and get time slice=%d\n",p->pid,p->ras.time_slice);
	}

    printk(KERN_INFO "the time slice of the process is %u",p->ras.time_slice);

	if (flags & ENQUEUE_WAKEUP)
		ras_se->timeout = 0;

	if (flags & ENQUEUE_HEAD)
		list_add(&ras_se->run_list, &ras_rq->run_list);
	else
		list_add_tail(&ras_se->run_list, &ras_rq->run_list);

    rq->ras.ras_nr_running++;

    inc_nr_running(rq);

    printk(KERN_INFO "enqueue_task_ras: finished!\n");
}

static void dequeue_task_ras(struct rq *rq, struct task_struct *p, int flags)
{
    struct sched_ras_entity *ras_se = &p->ras;
    
    printk(KERN_INFO "dequeue_task_ras: pid=%d is now dequeuing...\n",p->pid);

	update_curr_ras(rq);
    
    if (on_ras_rq(ras_se)){
		list_del_init(&ras_se->run_list);
        rq->ras.ras_nr_running--;
    }

    dec_nr_running(rq);

    printk(KERN_INFO "dequeue_task_ras: finished!\n");
}

static struct task_struct *pick_next_task_ras(struct rq *rq){

    struct sched_ras_entity *next = NULL;
	struct task_struct *p;
	struct ras_rq *ras_rq;
   

    ras_rq = &rq->ras;

    if (!ras_rq->ras_nr_running)
		return NULL;

    next = list_entry((ras_rq->run_list).next, struct sched_ras_entity, run_list);
    p = container_of(next, struct task_struct, ras);
    p->se.exec_start = rq->clock_task;
 
    printk(KERN_INFO "pick_next_task_ras: picking pid=%d\n",p->pid);

	return p;
    
}

static void put_prev_task_ras(struct rq *rq, struct task_struct *p){
    
    struct ras_rq *ras_rq;
    struct sched_ras_entity *ras_se;
    
    update_curr_ras(rq);
    
    ras_rq = &rq->ras;
    ras_se = &p->ras;

    if (on_ras_rq(ras_se)){
		struct list_head *queue = &ras_rq->run_list;
        list_move_tail(&ras_se->run_list, queue);
        printk(KERN_INFO "put_prev_task_ras: putting pid=%d back to the queue\n",p->pid);
	}

}   

static void yield_task_ras(struct rq *rq)
{
	struct ras_rq *ras_rq;
    struct sched_ras_entity *ras_se;

    printk(KERN_INFO "yield_task_ras: pid=%d yield itself\n",rq->curr->pid);

    ras_rq = &rq->ras;
    ras_se = &rq->curr->ras;

    if (on_ras_rq(ras_se)){
		struct list_head *queue = &ras_rq->run_list;
        list_move_tail(&ras_se->run_list, queue);
	}
}

/*
 * Preempt the current task with a newly woken task if needed:
 */
static void check_preempt_curr_ras(struct rq *rq, struct task_struct *p, int flags)
{
	struct sched_ras_entity *ras_se;

    printk(KERN_INFO "check_preempt_curr_ras: woken task: pid=%d\n",p->pid);

    ras_se = &p->ras;

    //update the weight and then compare:
    ras_se->weight = calc_ras_weight(p);
    (rq->curr->ras).weight = calc_ras_weight(rq->curr);

    if (ras_se->weight > (rq->curr->ras).weight) {
		resched_task(rq->curr);
        printk(KERN_INFO "preempt! woken task pid=%d's weight %d is higher than current pid=%d's weight %d",
            p->pid,ras_se->weight,rq->curr->pid,(rq->curr->ras).weight);
	}
}

static void set_curr_task_ras(struct rq *rq){
	
    struct task_struct *p = rq->curr;

    printk(KERN_INFO "set_curr_task_ras: pid=%d\n",p->pid);
	p->se.exec_start = rq->clock_task;
	
}

static void watchdog(struct rq *rq, struct task_struct *p)
{
	unsigned long soft, hard;

	/* max may change after cur was read, this will be fixed next tick */
	soft = task_rlimit(p, RLIMIT_RTTIME);
	hard = task_rlimit_max(p, RLIMIT_RTTIME);

	if (soft != RLIM_INFINITY) {
		unsigned long next;

		p->ras.timeout++;
		next = DIV_ROUND_UP(min(soft, hard), USEC_PER_SEC/HZ);
		if (p->ras.timeout > next)
			p->cputime_expires.sched_exp = p->se.sum_exec_runtime;
	}
}

static void task_tick_ras(struct rq *rq, struct task_struct *p, int queued){
    
    struct sched_ras_entity *ras_se = &p->ras;
    char *gp;

    printk(KERN_INFO "task_tick_ras: now pid=%d is running...\n",p->pid);
	update_curr_ras(rq);

    watchdog(rq, p);
    
    printk(KERN_INFO "RAS Tracing: pid=%d, remaining time slice=%d\n",p->pid,ras_se->time_slice);

    if (--p->ras.time_slice){
		return;
    }
    
	printk(KERN_INFO "task_tick_ras: now pid=%d is out of its time slice\n",p->pid);
	gp = task_group_path(task_group(p));
    ras_se->weight = calc_ras_weight(p);
    
	if(strcmp(gp,"/") == 0){//foreground
		p->ras.time_slice = RAS_FG_TIMESLICE + ras_se->weight * RAS_INC_TIME;
        printk(KERN_INFO "FG task! pid=%d and get time slice=%d\n",p->pid,p->ras.time_slice);
	}
    else if(strcmp(gp,"/bg_non_interactive") == 0){//background
		p->ras.time_slice = RAS_BG_TIMESLICE + ras_se->weight * RAS_INC_TIME;
        printk(KERN_INFO "BG task! pid=%d and get time slice=%d\n",p->pid,p->ras.time_slice);
	}
    else{
		p->ras.time_slice = RAS_BG_TIMESLICE + ras_se->weight * RAS_INC_TIME;
        printk(KERN_INFO "Unknown type task! pid=%d and get time slice=%d\n",p->pid,p->ras.time_slice);
	}

    if (rq->ras.run_list.prev != rq->ras.run_list.next) {
        
        struct ras_rq *ras_rq = &rq->ras;
        if (on_ras_rq(ras_se)){
            struct list_head *queue = &ras_rq->run_list;
            list_move_tail(&ras_se->run_list, queue);
        }
        
        set_tsk_need_resched(p);
        printk(KERN_INFO "reschedule current process pid=%d in task_tick(), new time slice=%d\n",p->pid,p->ras.time_slice);
        return;
    }
}

static unsigned int get_rr_interval_ras(struct rq *rq, struct task_struct *task){
	
	char *gp;
    unsigned int timeslice;
    struct sched_ras_entity *ras_se = &task->ras;

    printk(KERN_INFO "get_rr_interval_ras() is called\n");
    
	gp = task_group_path(task_group(task));

    //here we can set time slice as well:
    ras_se->weight = calc_ras_weight(task);

	if(strcmp(gp,"/") == 0){//foreground
		task->ras.time_slice = RAS_FG_TIMESLICE + ras_se->weight * RAS_INC_TIME;
	}
    else if(strcmp(gp,"/bg_non_interactive") == 0){//background
		task->ras.time_slice = RAS_BG_TIMESLICE + ras_se->weight * RAS_INC_TIME;
	}
    else{
		task->ras.time_slice = RAS_BG_TIMESLICE + ras_se->weight * RAS_INC_TIME;
	}
	

    return task->ras.time_slice;
}

static void switched_to_ras(struct rq *rq, struct task_struct *p){
    
    printk(KERN_INFO "switched_to_ras() is called\n");
    if (p->on_rq && rq->curr != p) {
        if (/* Don't resched if we changed runqueues */
		    rq == task_rq(p))
			resched_task(rq->curr);
    }
    

}

#ifdef CONFIG_SMP
static int select_task_rq_ras(struct task_struct *p, int sd_flag, int flags){

}

static void set_cpus_allowed_ras(struct task_struct *p, const struct cpumask *new_mask){

}

static void task_woken_ras(struct rq *rq, struct task_struct *p){

}

static void rq_online_ras(struct rq *rq){

}

static void rq_offline_ras(struct rq *rq){

}

static void switched_from_ras(struct rq *rq, struct task_struct *p){

}

static void pre_schedule_ras(struct rq *rq, struct task_struct *prev){

}

static void post_schedule_ras(struct rq *rq){

}

#endif

static void prio_changed_ras(struct rq *rq, struct task_struct *p, int oldprio){

}

const struct sched_class ras_sched_class = {
	.next			= &fair_sched_class,
	.enqueue_task		= enqueue_task_ras,
	.dequeue_task		= dequeue_task_ras,
	.yield_task		= yield_task_ras,

	.check_preempt_curr	= check_preempt_curr_ras,

	.pick_next_task		= pick_next_task_ras,
	.put_prev_task		= put_prev_task_ras,

#ifdef CONFIG_SMP
	.select_task_rq		= select_task_rq_ras,

	.set_cpus_allowed       = set_cpus_allowed_ras,
	.rq_online              = rq_online_ras,
	.rq_offline             = rq_offline_ras,
	.pre_schedule		= pre_schedule_ras,
	.post_schedule		= post_schedule_ras,
	.task_woken		= task_woken_ras,
	.switched_from		= switched_from_ras,
#endif

	.set_curr_task          = set_curr_task_ras,
	.task_tick		= task_tick_ras,

	.get_rr_interval	= get_rr_interval_ras,

	.prio_changed		= prio_changed_ras,
	.switched_to		= switched_to_ras,
};

