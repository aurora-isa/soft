#ifndef __AURORA_VM_INC
#define __AURORA_VM_INC
#include <stdlib.h>

#define MEMORY_SIZE 16777216
#define valid_register(x) ((x >= 0) && (x < 16) && !((x >= 10) && (x <= 13)))
#define valid_memory(x) ((x >= 0) && (x < 16777216))

typedef struct _machine_state {
    unsigned int regs[16];
    unsigned char memory[MEMORY_SIZE];
} machine_state;

#endif /* __AURORA_MACHINE_STATE_INC */
