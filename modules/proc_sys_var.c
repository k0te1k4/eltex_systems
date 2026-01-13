#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/mutex.h>

#define PROC_NAME "myvar"

static int myvar;
static DEFINE_MUTEX(myvar_lock);

// procfs

static ssize_t myvar_proc_read(struct file *file, char __user *buf,
                               size_t count, loff_t *ppos)
{
    char tmp[32];
    int len;

    mutex_lock(&myvar_lock);
    len = scnprintf(tmp, sizeof(tmp), "%d\n", myvar);
    mutex_unlock(&myvar_lock);

    return simple_read_from_buffer(buf, count, ppos, tmp, len);
}

static ssize_t myvar_proc_write(struct file *file, const char __user *buf,
                                size_t count, loff_t *ppos)
{
    char tmp[32];
    int val;

    if (count >= sizeof(tmp))
        return -EINVAL;

    if (copy_from_user(tmp, buf, count))
        return -EFAULT;

    tmp[count] = '\0';

    if (kstrtoint(strim(tmp), 10, &val))
        return -EINVAL;

    mutex_lock(&myvar_lock);
    myvar = val;
    mutex_unlock(&myvar_lock);

    return count;
}


static const struct proc_ops myvar_proc_ops = {
    .proc_read  = myvar_proc_read,
    .proc_write = myvar_proc_write,
};

static struct proc_dir_entry *proc_ent;

// sysfs

static struct kobject *myvar_kobj;

static ssize_t myvar_sysfs_show(struct kobject *kobj,
                                struct kobj_attribute *attr, char *buf)
{
    int v;

    mutex_lock(&myvar_lock);
    v = myvar;
    mutex_unlock(&myvar_lock);

    return scnprintf(buf, PAGE_SIZE, "%d\n", v);
}

static ssize_t myvar_sysfs_store(struct kobject *kobj,
                                 struct kobj_attribute *attr,
                                 const char *buf, size_t count)
{
    int val;

    if (kstrtoint(strim((char *)buf), 10, &val))
        return -EINVAL;

    mutex_lock(&myvar_lock);
    myvar = val;
    mutex_unlock(&myvar_lock);

    return count;
}

static struct kobj_attribute myvar_attr =
    __ATTR(myvar, 0664, myvar_sysfs_show, myvar_sysfs_store);



static int __init proc_sys_init(void)
{
    proc_ent = proc_create(PROC_NAME, 0666, NULL, &myvar_proc_ops);
    if (!proc_ent)
        return -ENOMEM;

    myvar_kobj = kobject_create_and_add("myvar", kernel_kobj);
    if (!myvar_kobj) {
        proc_remove(proc_ent);
        return -ENOMEM;
    }

    if (sysfs_create_file(myvar_kobj, &myvar_attr.attr)) {
        kobject_put(myvar_kobj);
        proc_remove(proc_ent);
        return -ENOMEM;
    }

    pr_info("loaded: /proc/%s and /sys/kernel/myvar/myvar (value=%d)\n",
            PROC_NAME, myvar);
    return 0;
}

static void __exit proc_sys_exit(void)
{
    if (myvar_kobj) {
        sysfs_remove_file(myvar_kobj, &myvar_attr.attr);
        kobject_put(myvar_kobj);
    }
    if (proc_ent)
        proc_remove(proc_ent);

    pr_info("unloaded\n");
}

module_init(proc_sys_init);
module_exit(proc_sys_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("egor");
MODULE_DESCRIPTION("procfs + sysfs variable");
