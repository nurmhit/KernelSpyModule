#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/workqueue.h>
#include <linux/interrupt.h>
#include <asm/io.h>
#include <linux/kthread.h>
#include <linux/delay.h>


static struct task_struct *thread_st;

int cnt = 0;

static void got_char(void *scancode)
{
	printk(KERN_INFO "Scan Code %x %s.\n",
	       (int)*((char *)scancode) & 0x7F,
	       *((char *)scancode) & 0x80 ? "Released" : "Pressed");
}

static int thread_fn(void *unused)
{
    while(!kthread_should_stop())
	{
		ssleep(60);
		printk(KERN_INFO "Counter: %d\n",cnt);
		cnt = 0;
	}	
    printk(KERN_INFO "Thread Stopping\n");
    do_exit(0);
    return 0;
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Narek Mkhitaryan");

irqreturn_t irq_handler(int irq, void *dev_id, struct pt_regs *regs)
{

	static unsigned char scancode;
	unsigned char status;

	status = inb(0x64);
	scancode = inb(0x60);
	cnt++;

  	return IRQ_HANDLED;
}


static int __init irq_ex_init(void)
{
    thread_st = kthread_run(thread_fn, NULL, "mythread");
    return request_irq (1, (irq_handler_t) irq_handler,IRQF_SHARED, "test_keyboard_irq_handler",(void *)(irq_handler));
}

static void __exit irq_ex_exit(void)
{
   free_irq(1, (void*)irq_handler);
   printk(KERN_INFO "Cleaning Up\n");
   if (thread_st)
   {
       kthread_stop(thread_st);
       printk(KERN_INFO "Thread stopped");
   }
}

module_init(irq_ex_init);
module_exit(irq_ex_exit);
