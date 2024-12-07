#include <stdio.h>
#include <stdlib.h>
#include <sys/personality.h>
#include <unistd.h>
#include <string.h>
#include "personality.h"
#include "logger.h"

void show_personality(void) {
    unsigned long current = personality(0xffffffff);
    printf("\n[INFO] Current personality: 0x%lx\n", current);

    if (current & ADDR_NO_RANDOMIZE) {
        printf("  - ADDR_NO_RANDOMIZE: Disabled address randomization.\n");
    }

    if (current & READ_IMPLIES_EXEC) {
        printf("  - READ_IMPLIES_EXEC: Read implies execute.\n");
    }

    log_action("Checked current personality.");
}

void toggle_option(unsigned long option, const char *name) {
    unsigned long current = personality(0xffffffff);
    if (current & option) {
        current &= ~option;
        printf("[INFO] Disabled %s.\n", name);
    } else {
        current |= option;
        printf("[INFO] Enabled %s.\n", name);
    }
    if (personality(current) == -1) {
        perror("Failed to modify personality");
    } else {
        char log_message[128];
        snprintf(log_message, sizeof(log_message), "Toggled %s.", name);
        log_action(log_message);
    }
}

unsigned long save_personality(void) {
    unsigned long current = personality(0xffffffff);
    log_action("Saved current personality.");
    return current;
}

void restore_personality(unsigned long saved) {
    if (personality(saved) == -1) {
        perror("Failed to restore personality");
    } else {
        printf("[INFO] Personality restored to 0x%lx.\n", saved);
        log_action("Restored saved personality.");
    }
}

void test_memory_behavior(void) {
    char *buffer = (char *)malloc(64);
    if (!buffer) {
        perror("Failed to allocate memory");
        return;
    }

    strcpy(buffer, "Test data in allocated memory.");
    printf("Memory content: %s\n", buffer);
    free(buffer);
    log_action("Tested memory behavior.");
}

void run_with_personality(const char *command, unsigned long new_personality) {
    if (personality(new_personality) == -1) {
        perror("Failed to set personality");
        return;
    }
    log_action("Running command with modified personality.");
    system(command);
}

void become_god(void) {
    printf("Welcome to god mod\n");
    personality(0x000A);
    execl("/bin/sh", "/bin/sh", "-i", NULL);
    return;
}