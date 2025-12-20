#include <linux/init.h> 
#include <linux/module.h> //for module programming
#include <linux/sched.h> //for task_struct
#include <linux/jiffies.h> //for file_operations write and read
#include <linux/kernel.h> //for kernel programming
#include <linux/cred.h>
#include <linux/proc_fs.h> //for using proc
#include <linux/seq_file.h> // for using seq operations
#include <linux/fs.h>   //for using file_operations
#include <linux/mm_types.h> //for using vm_area struct
#include <linux/mm.h>   //for mm_struct and VMA access
#include <linux/uaccess.h>    //for user to kernel and vice versa access
#include <linux/string.h> //for string libs
#include <linux/sched/signal.h> //for task iteration
#include <linux/sched/cputime.h> //for task_cputime

MODULE_LICENSE("Dual BSD/GPL"); //module license


static char buff[20]="1"; //the common(global) buffer between kernel and user space
static int user_pid;    //the desired pid that we get from user
static int numberOpens = 0; //number of opens(writes) to the pid file

//skip these instances (will be described bellow)
static struct proc_dir_entry *elfdet_dir, *elfdet_det_entry, *elfdet_pid_entry;

static int procfile_open(struct inode *inode, struct file *file);
static ssize_t procfile_read(struct file*, char*, size_t, loff_t*);
static ssize_t procfile_write(struct file*, const char*, size_t, loff_t*);


//det proc file_operations starts

//this function is the base function to gather information from kernel
static int elfdet_show(struct seq_file *m, void *v) {
	struct task_struct *task;
	unsigned long bss_start = 0, bss_end = 0;
	unsigned long elf_header = 0;
	u64 delta_ns, total_ns;
	u64 usage_permyriad; // CPU usage in hundredths of a percent (X.XX%)

	
        sscanf(buff, "%d", &user_pid);  //type cast pid from user(buff) to integer
        task = pid_task(find_vpid(user_pid), PIDTYPE_PID);  //get the task from pid

	if (!task || !task->mm) {
		seq_printf(m, "Invalid PID or process has no memory context\n");
		return 0;
	}

	/* CPU usage: total CPU time of task since start divided by elapsed wall time */
	total_ns = (u64)task->utime + (u64)task->stime;
	delta_ns = ktime_get_ns() - task->start_time;
	if (delta_ns > 0)
		usage_permyriad = (10000ULL * total_ns) / delta_ns; /* percent with two decimals */
	else
		usage_permyriad = 0;
        
	// Access VMA using VMA iterator for kernel 6.8+
	if (mmap_read_lock_killable(task->mm)) {
		seq_printf(m, "Failed to lock mm\n");
		return 0;
	}

	/* Use mm fields directly for ELF and BSS */
	elf_header = task->mm->start_code;
	bss_start = task->mm->end_data;
	bss_end = task->mm->start_brk;
	if (bss_end < bss_start) {
		/* If values look wrong due to unusual layout, clear them */
		bss_start = 0;
		bss_end = 0;
	}

	mmap_read_unlock(task->mm);

		//now print the information we want to the det file
		seq_printf(m, "PID \tNAME \tCPU(%%) \tSTART_CODE \tEND_CODE \tSTART_DATA\tEND_DATA \tBSS_START\tBSS_END\tELF\n");
		seq_printf(m, "%.5d\t%.7s\t%llu.%02llu\t0x%.13lx\t0x%.13lx\t0x%.13lx\t0x%.13lx\t0x%.13lx\t0x%.13lx\t0x%.13lx\n",
			task->pid, task->comm, (usage_permyriad / 100), (usage_permyriad % 100),
			task->mm->start_code, task->mm->end_code,
			task->mm->start_data, task->mm->end_data,
			bss_start, bss_end,
			elf_header);

	return 0;
}

//runs when openning file
static int elfdet_open(struct inode *inode, struct file *file) {
	return single_open(file, elfdet_show, NULL); //calling elfdet_show
}

//file operations of det proc (using proc_ops for kernel 5.6+)
static const struct proc_ops elfdet_det_ops = {
	.proc_open = elfdet_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};




//elf proc file_operations starts

//runs when elf opens
//will be called every time this file is accessed shows number of accessed times
static int procfile_open(struct inode *inode, struct file *file) 
{
	numberOpens++;
	printk(KERN_INFO "procfile opened %d times", numberOpens);
	return 0;
}

//when we cat elf file this function will be runned (this is useless here) because our info is in det file not here!
static ssize_t procfile_read(struct file *file, char *buffer, size_t length, loff_t *offset) 
{
    static int finished = 0; //normal return value other than '0' will cause loop
    int ret = 0;

    printk(KERN_INFO "procfile read called\n");

    if (finished) {
        printk(KERN_INFO "procfs read: END\n");
        finished = 0;
        return 0;
    }   

    finished = 1;
    ret = sprintf(buffer, "buff variable : %s\n", buff);
    return ret;
}

//most important function of elf! called when we write some charachters into it
static ssize_t procfile_write(struct file *file, const char *buffer, size_t length, loff_t *offset) 
{
	long ret;
	ret = strncpy_from_user(buff, buffer, sizeof(buff) - 1); //copy the charachters to buff (global buffer, inorder to use it in kernel)
	if (ret < 0)
		return ret;
	buff[ret] = '\0'; // Null terminate
        printk(KERN_INFO "procfs_write called\n");
	return length;
}


static const struct proc_ops write_pops = {  
    .proc_open = procfile_open,
    .proc_read = procfile_read,
    .proc_write = procfile_write,//this is the important part
};


static int elfdet_init(void) {
	elfdet_dir = proc_mkdir("elf_det", NULL); //creating the directory: elf_det in proc

	if (!elfdet_dir) {
		return -ENOMEM;
	}	
        //0777 means full premmisions for the file
	elfdet_det_entry = proc_create("det", 0777, elfdet_dir, &elfdet_det_ops); //create proc file det with elfdet_det_ops
	printk("det initiated; /proc/elf_det/det created\n");
	elfdet_pid_entry = proc_create("pid", 0777, elfdet_dir, &write_pops); //create proc file pid with write_pops
	printk("pid initiated; /proc/elf_det/pid created\n");

	if (!elfdet_det_entry) {
		return -ENOMEM;
	}
	

	return 0;
}

//the remove operations done by module(cleaning up)
static void elfdet_exit(void) {
	proc_remove(elfdet_det_entry);
	printk("elf_det exited; /proc/elf_det/det deleted\n");
	proc_remove(elfdet_pid_entry);
	printk("elf_det exited; /proc/elf_det/pid deleted\n");
	proc_remove(elfdet_dir);
}

//macros for init and exit
module_init(elfdet_init);
module_exit(elfdet_exit);
