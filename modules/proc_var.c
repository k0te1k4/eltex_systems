#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>

#define PROC_NAME "myvar"
#define BUF_SZ 64

static int myvar = 0;
static struct proc_dir_entry *proc_entry;

static ssize_t myvar_read(struct file *file, char __user *ubuf, size_t count, loff_t *ppos)
{
	char buf[BUF_SZ];
	int len = scnprintf(buf, sizeof(buf), "%d\n", myvar);

	return simple_read_from_buffer(ubuf, count, ppos, buf, len);
}

static ssize_t myvar_write(struct file *file, const char __user *ubuf, size_t count, loff_t *ppos)
{
	char buf[BUF_SZ];
	size_t n;
	int val;

	n = min(count, sizeof(buf) - 1);
	if (copy_from_user(buf, ubuf, n))
		return -EFAULT;
	buf[n] = '\0';

	if (kstrtoint(strim(buf), 10, &val))
		return -EINVAL;

	myvar = val;
	return count;
}

static const struct proc_ops myvar_ops = {
	.proc_read  = myvar_read,
	.proc_write = myvar_write,
};

static int __init mymod_init(void)
{
	proc_entry = proc_create(PROC_NAME, 0666, NULL, &myvar_ops);
	if (!proc_entry)
		return -ENOMEM;

	pr_info("procfs module loaded: /proc/%s, myvar=%d\n", PROC_NAME, myvar);
	return 0;
}

static void __exit mymod_exit(void)
{
	if (proc_entry)
		proc_remove(proc_entry);

	pr_info("procfs module unloaded\n");
}

module_init(mymod_init);
module_exit(mymod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("egor");
MODULE_DESCRIPTION("procfs read/write variable example");
