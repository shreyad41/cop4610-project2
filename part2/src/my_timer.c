#include <linux/module.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/mutex.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/time64.h>
#include <linux/timekeeping.h>

#define PROC_NAME "timer"

static struct proc_dir_entry *my_timer_entry;
static struct timespec64 prev_time;
static bool prev_time_is_valid = false;
static DEFINE_MUTEX(my_timer_mutex);

static int my_timer_show(struct seq_file *output, void *unused)
{
    struct timespec64 current_time;
    struct timespec64 elapsed_time;

    mutex_lock(&my_timer_mutex);
    ktime_get_real_ts64(&current_time);

    seq_printf(output, "current time: %lld.%09ld\n",
               (long long)current_time.tv_sec, current_time.tv_nsec);

    if (prev_time_is_valid)
    {
        elapsed_time = timespec64_sub(current_time, prev_time);
        seq_printf(output, "elapsed time: %lld.%09ld\n",
                   (long long)elapsed_time.tv_sec, elapsed_time.tv_nsec);
    }

    prev_time = current_time;
    prev_time_is_valid = true;
    mutex_unlock(&my_timer_mutex);

    return 0;
}

static int my_timer_open(struct inode *inode, struct file *file)
{
    return single_open(file, my_timer_show, NULL);
}

static const struct proc_ops my_timer_ops = {
    .proc_open = my_timer_open,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
};

static int __init my_timer_init(void)
{
    prev_time_is_valid = false;
    my_timer_entry = proc_create(PROC_NAME, 0, NULL, &my_timer_ops);
    if (!my_timer_entry)
    {
        pr_err("Failed to create /proc/%s\n", PROC_NAME);
        return -ENOMEM;
    }
    return 0;
}

static void __exit my_timer_exit(void)
{
    proc_remove(my_timer_entry);
}

module_init(my_timer_init);
module_exit(my_timer_exit);

// module uses GPL license. Linux needs this for kernel function access.
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Time module that reports current and elapsed time");
