#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kprobes.h>
#include "hooker.h"

static unsigned long find_symbol_address(const char *name) {
    struct kprobe kp = { .symbol_name = name };
    unsigned long addr = 0;

    if (register_kprobe(&kp) == 0) {
        addr = (unsigned long)kp.addr;
        unregister_kprobe(&kp);
    }

    return addr;
}

static void notrace fh_ftrace_thunk(unsigned long ip, unsigned long parent_ip, struct ftrace_ops *ops, struct pt_regs *regs) {
    struct ftrace_hook *hook = container_of(ops, struct ftrace_hook, ops);

    if (hook && regs && !within_module(parent_ip, THIS_MODULE)) {
        regs->ip = (unsigned long)hook->function;
    }
}

static int fh_resolve_hook_address(struct ftrace_hook *hook) {
    hook->address = find_symbol_address(hook->name);
    *(unsigned long *)(hook->original) = hook->address;
    return 0;
}

int fh_install_hook(struct ftrace_hook *hook) {
    fh_resolve_hook_address(hook);

    hook->ops.func = (ftrace_func_t)fh_ftrace_thunk;
    hook->ops.flags = FTRACE_OPS_FL_SAVE_REGS | FTRACE_OPS_FL_IPMODIFY;

    ftrace_set_filter_ip(&hook->ops, hook->address, 0, 0);
    register_ftrace_function(&hook->ops);
    
    return 0;
}

void fh_remove_hook(struct ftrace_hook *hook) {
    unregister_ftrace_function(&hook->ops);
    ftrace_set_filter_ip(&hook->ops, hook->address, 1, 0);
}
