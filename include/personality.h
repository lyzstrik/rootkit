#ifndef PERSONALITY_H
#define PERSONALITY_H

#ifndef ADDR_NO_RANDOMIZE
#define ADDR_NO_RANDOMIZE 0x0040000
#endif

#ifndef READ_IMPLIES_EXEC
#define READ_IMPLIES_EXEC 0x0004000
#endif

void show_personality(void);
void toggle_option(unsigned long option, const char *name);
unsigned long save_personality(void);
void restore_personality(unsigned long saved);
void test_memory_behavior(void);
void run_with_personality(const char *command, unsigned long new_personality);
void become_god(void);

#endif
