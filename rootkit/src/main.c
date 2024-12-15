#include <stdio.h>
#include <stdlib.h>
#include "personality.h"
#include "logger.h"

void show_menu() {
    printf("\n===== Personality Manager =====\n");
    printf("0. Just become god\n");
    printf("1. Show current personality\n");
    printf("2. Toggle ADDR_NO_RANDOMIZE\n");
    printf("3. Toggle READ_IMPLIES_EXEC\n");
    printf("4. Test memory behavior\n");
    printf("5. Save current personality\n");
    printf("6. Restore saved personality\n");
    printf("7. Run a command with custom personality\n");
    printf("8. Exit\n");
    printf("================================\n");
    printf("Choose an option: ");
}

int main() {
    int choice;
    unsigned long saved_personality = 0;
    char command[256];

    while (1) {
        show_menu();
        if (scanf("%d", &choice) != 1) {
            fprintf(stderr, "Invalid input. Exiting...\n");
            break;
        }

        switch (choice) {
            case 0:
                become_god();
                break;
            case 1:
                show_personality();
                break;
            case 2:
                toggle_option(ADDR_NO_RANDOMIZE, "ADDR_NO_RANDOMIZE");
                break;
            case 3:
                toggle_option(READ_IMPLIES_EXEC, "READ_IMPLIES_EXEC");
                break;
            case 4:
                test_memory_behavior();
                break;
            case 5:
                saved_personality = save_personality();
                break;
            case 6:
                restore_personality(saved_personality);
                break;
            case 7:
                printf("\nEnter command to run: ");
                scanf(" %[^\n]", command);
                run_with_personality(command, saved_personality);
                break;
            case 8:
                log_action("Exited program.");
                exit(EXIT_SUCCESS);
            default:
                printf("Invalid option. Try again.\n");
        }
    }

    return 0;
}
