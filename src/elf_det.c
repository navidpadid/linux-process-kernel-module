// SPDX-License-Identifier: (GPL-2.0 OR BSD-2-Clause)
#include <linux/init.h>
#include <linux/module.h> //for module programming
#include <linux/sched.h> //for task_struct
#include <linux/jiffies.h> //for file_operations write and read
#include <linux/kernel.h> //for kernel programming
#include <linux/cred.h>
#include <linux/proc_fs.h> //for using proc
#include <linux/seq_file.h> // for using seq operations
#include <linux/fs.h> //for using file_operations
#include <linux/mm_types.h> //for using vm_area struct
#include <linux/mm.h> //for mm_struct and VMA access
#include <linux/uaccess.h> //for user to kernel and vice versa access
#include <linux/string.h> //for string libs
#include <linux/sched/signal.h> //for task iteration
#include <linux/sched/cputime.h> //for task_cputime
#include "elf_helpers.h"

MODULE_LICENSE("Dual BSD/GPL"); // module license

static char buff[20] =
    "1"; // the common(global) buffer between kernel and user space
static int user_pid; // the desired pid that we get from user
static int number_opens; // number of opens(writes) to the pid file

// skip these instances (will be described bellow)
static struct proc_dir_entry *elfdet_dir, *elfdet_det_entry, *elfdet_pid_entry;

static int procfile_open(struct inode *inode, struct file *file);
static ssize_t procfile_read(struct file *, char __user *, size_t, loff_t *);
static ssize_t procfile_write(struct file *, const char __user *, size_t,
			      loff_t *);

// det proc file_operations starts

// this function is the base function to gather information from kernel
static int elfdet_show(struct seq_file *m, void *v)
{
	struct task_struct *task;
	unsigned long bss_start = 0, bss_end = 0;
	unsigned long heap_start = 0, heap_end = 0;
	unsigned long stack_start = 0, stack_end = 0;
	unsigned long elf_base = 0;
	u64 delta_ns, total_ns;
	u64 usage_permyriad; // CPU usage in hundredths of a percent (X.XX%)
	struct vm_area_struct *vma;
	struct ma_state mas;
	int ret;

	ret = kstrtoint(buff, 10, &user_pid);
	if (ret != 0) {
		seq_puts(m, "Failed to parse PID\n");
		return 0;
	}

	task = pid_task(find_vpid(user_pid), PIDTYPE_PID);

	if (!task || !task->mm) {
		seq_puts(m, "Invalid PID or process has no memory context\n");
		return 0;
	}

	/* CPU usage: total CPU time of task since start divided by elapsed wall
	 * time
	 */
	total_ns = (u64)task->utime + (u64)task->stime;
	delta_ns = ktime_get_ns() - task->start_time;
	usage_permyriad = compute_usage_permyriad(total_ns, delta_ns);

	// Access VMA using VMA iterator for kernel 6.8+
	if (mmap_read_lock_killable(task->mm)) {
		seq_puts(m, "Failed to lock mm\n");
		return 0;
	}

	/* Use mm fields directly for ELF, BSS, heap, and stack
	 * Note: Modern ELF binaries may have end_data == start_brk (no BSS)
	 * rodata is typically merged with code section (start_code to end_code)
	 * Heap shown is brk-based; mmap-allocated heap is not tracked here
	 */

	/* ELF base: First VMA is typically the ELF binary base (for PIE) */
	mas_init(&mas, &task->mm->mm_mt, 0);
	vma = mas_find(&mas, ULONG_MAX);
	if (vma)
		elf_base = vma->vm_start;

	/* Stack: Find the [stack] VMA for actual stack boundaries */
	stack_start = task->mm->start_stack;
	mas_set(&mas, 0);
	mas_for_each(&mas, vma, ULONG_MAX)
	{
		if (vma->vm_start <= task->mm->start_stack &&
		    vma->vm_end >= task->mm->start_stack) {
			/* Found the stack VMA */
			stack_end = vma->vm_start; /* Stack grows down */
			break;
		}
	}

	/* BSS: uninitialized data between end_data and start_brk
	 * May be zero-length in modern binaries
	 */
	compute_bss_range(task->mm->end_data, task->mm->start_brk, &bss_start,
			  &bss_end);

	/* Heap: brk-based heap from start_brk to current brk
	 * Note: Does not include mmap-based allocations (arena heap)
	 */
	compute_heap_range(task->mm->start_brk, task->mm->brk, &heap_start,
			   &heap_end);

	mmap_read_unlock(task->mm);

	// now print the information we want to the det file
	seq_puts(
	    m, "PID \tNAME \tCPU(%) \tSTART_CODE \tEND_CODE "
	       "\tSTART_DATA\tEND_DATA \tBSS_START\tBSS_END\tHEAP_START\tHEAP_"
	       "END\tSTACK_START\tSTACK_END\tELF_BASE\n");
	seq_printf(
	    m,
	    "%.5d\t%.7s\t%llu.%02llu\t0x%.13lx\t0x%.13lx\t0x%.13lx\t0x%."
	    "13lx\t0x%.13lx\t0x%.13lx\t0x%.13lx\t0x%.13lx\t0x%.13lx\t0x%."
	    "13lx\t0x%.13lx\n",
	    task->pid, task->comm, (usage_permyriad / 100),
	    (usage_permyriad % 100), task->mm->start_code, task->mm->end_code,
	    task->mm->start_data, task->mm->end_data, bss_start, bss_end,
	    heap_start, heap_end, stack_start, stack_end, elf_base);

	return 0;
}

// runs when opening file
static int elfdet_open(struct inode *inode, struct file *file)
{
	return single_open(file, elfdet_show, NULL); // calling elfdet_show
}

// file operations of det proc (using proc_ops for kernel 5.6+)
static const struct proc_ops elfdet_det_ops = {
    .proc_open = elfdet_open,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
};

// elf proc file_operations starts

// runs when elf opens
// will be called every time this file is accessed shows number of accessed
// times
static int procfile_open(struct inode *inode, struct file *file)
{
	number_opens++;
	pr_info("procfile opened %d times\n", number_opens);
	return 0;
}

// when we cat elf file this function will be run (this is useless here)
// because our info is in det file not here!
static ssize_t procfile_read(struct file *file, char __user *buffer,
			     size_t length, loff_t *offset)
{
	static int finished;
	char tmp[64];
	int len;

	// normal return value other than '0' will cause loop

	pr_info("procfile read called\n");

	if (finished) {
		pr_info("procfs read: END\n");
		finished = 0;
		return 0;
	}

	finished = 1;
	len = snprintf(tmp, sizeof(tmp), "buff variable : %s\n", buff);
	if (len < 0)
		return -EFAULT;
	if (len > length)
		len = length;
	if (copy_to_user(buffer, tmp, len))
		return -EFAULT;
	return len;
}

// most important function of elf! called when we write some characters into it
static ssize_t procfile_write(struct file *file, const char __user *buffer,
			      size_t length, loff_t *offset)
{
	long ret;

	ret = strncpy_from_user(buff, buffer, sizeof(buff) - 1);
	// copy the characters to buff (global buffer, in order to use it in
	// kernel)
	if (ret < 0)
		return ret;
	buff[ret] = '\0'; // Null terminate
	pr_info("procfs_write called\n");
	return length;
}

static const struct proc_ops write_pops = {
    .proc_open = procfile_open,
    .proc_read = procfile_read,
    .proc_write = procfile_write, // this is the important part
};

static int elfdet_init(void)
{
	elfdet_dir = proc_mkdir("elf_det", NULL);
	// creating the directory: elf_det in proc

	if (!elfdet_dir)
		return -ENOMEM;

	// 0644 means owner read/write, others read-only
	elfdet_det_entry =
	    proc_create("det", 0644, elfdet_dir, &elfdet_det_ops);
	// create proc file det with elfdet_det_ops
	pr_info("det initiated; /proc/elf_det/det created\n");
	elfdet_pid_entry = proc_create("pid", 0644, elfdet_dir, &write_pops);
	// create proc file pid with write_pops
	pr_info("pid initiated; /proc/elf_det/pid created\n");

	if (!elfdet_det_entry)
		return -ENOMEM;

	return 0;
}

// the remove operations done by module(cleaning up)
static void elfdet_exit(void)
{
	proc_remove(elfdet_det_entry);
	pr_info("elf_det exited; /proc/elf_det/det deleted\n");
	proc_remove(elfdet_pid_entry);
	pr_info("elf_det exited; /proc/elf_det/pid deleted\n");
	proc_remove(elfdet_dir);
}

// macros for init and exit
module_init(elfdet_init);
module_exit(elfdet_exit);
