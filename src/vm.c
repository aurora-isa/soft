#include "vm.h"
#include <stdio.h>
#include <string.h>

void hexDump (char *desc, void *addr, int len) {
    int i;
    unsigned char buff[17];
    unsigned char *pc = (unsigned char*)addr;

    // Output description if given.
    if (desc != NULL)
        printf ("%s:\n", desc);

    if (len == 0) {
        printf("  ZERO LENGTH\n");
        return;
    }
    if (len < 0) {
        printf("  NEGATIVE LENGTH: %i\n",len);
        return;
    }

    // Process every byte in the data.
    for (i = 0; i < len; i++) {
        // Multiple of 16 means new line (with line offset).

        if ((i % 16) == 0) {
            // Just don't print ASCII for the zeroth line.
            if (i != 0)
                printf ("  %s\n", buff);

            // Output the offset.
            printf ("  %04x ", i);
        }

        // Now the hex code for the specific character.
        printf (" %02x", pc[i]);

        // And store a printable ASCII character for later.
        if ((pc[i] < 0x20) || (pc[i] > 0x7e))
            buff[i % 16] = '.';
        else
            buff[i % 16] = pc[i];
        buff[(i % 16) + 1] = '\0';
    }

    // Pad out last line if not exactly 16 characters.
    while ((i % 16) != 0) {
        printf ("   ");
        i++;
    }

    // And print the final ASCII bit.
    printf ("  %s\n", buff);
}

machine_state *machine_state_new() {
    machine_state *ms = calloc(1, sizeof(machine_state));
    return ms;
}

void machine_state_dump(machine_state *ms) {
    hexDump("regs", ms->regs, 16 * sizeof(unsigned int));
}

#define PC ms->regs[15]
int machine_run(machine_state *ms) {
    unsigned char ins[4];
    unsigned char p2, regA, regB;

    if (!valid_memory(PC)) /* skipped past end instruction */
        return -1;

    ins[0] = ms->memory[PC++];
    if (!valid_memory(PC)) /* skipped past end instruction */
        return -1;

    ins[1] = ms->memory[PC++];

    if (!((ins[0] >> 4) ^ 0xf)) {
        short dr;
        /* special instruction */
        ins[2] = ms->memory[PC++];
        ins[3] = ms->memory[PC++];

        /* destination register */
        dr = ins[0] & 0xf;
        if (!valid_register(dr)) /* invalid, but just ignore */ return 0;

        /* copy over bytes */
        ms->regs[dr] = (ins[3] << 16) + (ins[2] << 8) + (ins[1]);

        /* done */
        return 0;
    }

    /* Preliminary setup */
    p2 = ins[0] & 0xf;

    regA = ins[1] >> 4;
    regB = ins[1] & 0xf;

    switch (ins[0] >> 4) {
    case 0x0: /* No-op */
        break;
    case 0x1:
        switch ((ins[0] & 0xc) >> 2) {
        case 0x0:
            /* copy N bytes (N_max = 3) starting from the memory address contained in register "a" to the register "b" */
            ms->regs[regB] = (ms->memory[ms->regs[regA] & 0xffffff] << 16) + (ms->memory[(ms->regs[regA]+1) & 0xffffff] << 8) + (ms->memory[(ms->regs[regA]+2) & 0xffffff]);
            break;
        case 0x1:
            /* copy N byte value (N_max = 3) of register "a" to the area of memory specified by the memory address contained in register "b" */
            ms->memory[ms->regs[regB]] = ((ms->regs[regA] >> 16) | 0xff);
            ms->memory[ms->regs[regB] + 1] = ((ms->regs[regA] >> 8) | 0xff);
            ms->memory[ms->regs[regB] + 2] = ((ms->regs[regA]) | 0xff);
            break;
        case 0x2:
            /* copy N byte value (N_max = 3) of register "a" to register "b" */
            ms->regs[regB] = ms->regs[regA];
            break;
        default:
            break;
        }
        break;
    case 0x2:
        /* clear program status */
        ms->regs[14] = 0;
        /* set appropriate flags */
        if (ms->regs[regA] == ms->regs[regB]) ms->regs[14] |= 1;
        if (ms->regs[regA] > ms->regs[regB]) ms->regs[14] |= 2;
        if (ms->regs[regA] < ms->regs[regB]) ms->regs[14] |= 4;
        break;
    case 0x3:
        switch (p2) {
        case 0x0:
            PC = ms->regs[regA];
            break;
        case 0x1:
            if (!(ms->regs[14] ^ 0x1)) PC = ms->regs[regA];
            break;
        case 0x2:
            if (!(ms->regs[14] ^ 0x2)) PC = ms->regs[regA];
            break;
        case 0x3:
            if (!(ms->regs[14] ^ 0x3)) PC = ms->regs[regA];
            break;
        case 0x4:
            if (!(ms->regs[14] ^ 0x4)) PC = ms->regs[regA];
            break;
        case 0x5:
            if (!(ms->regs[14] ^ 0x5)) PC = ms->regs[regA];
            break;
        case 0xF:
            if (ms->regs[14] ^ 0x1) PC = ms->regs[regA];
            break;
        default:
            /* error, ignore */
            break;
        }
        break;
    case 0x4:
        switch (p2) {
        case 0x0:
            ms->regs[regA] = (ms->regs[regA] + ms->regs[regB]) & 0xffffff;
            break;
        case 0x1:
            ms->regs[regA] = (ms->regs[regA] - ms->regs[regB]) & 0xffffff;
            break;
        case 0x2:
            ms->regs[regA] = (ms->regs[regA] * ms->regs[regB]) & 0xffffff;
            break;
        case 0x3:
            ms->regs[regA] = (ms->regs[regA] / ms->regs[regB]) & 0xffffff;
            break;
        case 0x4:
            ms->regs[regA] = (ms->regs[regA] % ms->regs[regB]) & 0xffffff;
            break;
        case 0x5:
            ms->regs[regA] = (ms->regs[regA] << ms->regs[regB]) & 0xffffff;
            break;
        case 0x6:
            ms->regs[regA] = (ms->regs[regA] >> ms->regs[regB]) & 0xffffff;
            break;
        case 0x7:
            ms->regs[regA] = (ms->regs[regA] | ms->regs[regB]) & 0xffffff;
            break;
        case 0x8:
            ms->regs[regA] = (ms->regs[regA] & ms->regs[regB]) & 0xffffff;
            break;
        case 0x9:
            ms->regs[regA] = (ms->regs[regA] ^ ms->regs[regB]) & 0xffffff;
            break;
        case 0xA:
            ms->regs[regA] = (~ms->regs[regA]) & 0xffffff;
            break;
        case 0xB:
            ms->regs[regA] = (ms->regs[regA] + 1) & 0xffffff;
            break;
        case 0xC:
            ms->regs[regA] = (ms->regs[regA] - 1) & 0xffffff;
            break;
        default:
            /* error, ignore */
            break;
        }
        break;
    case 0xe:
        switch (p2) {
        case 0x0: /* halt */
            return 1;
        case 0xf:
            printf("reg#%d has contents is `%s`\n", regA, &ms->memory[ms->regs[regA]]);
            break;
        default:
            break;
        }
        break;
    }

    return 0;
}
#undef PC

unsigned char initial[] = {
                            0xf0, 0x18, 0x00, 0x00,     /* skip mem addr */
                            0x4b, 0x10,                 /* incr 1 in regA */
                            0x4b, 0x20,                 /* incr 1 in regB */
                            0x20, 0x12,                 /* compare two registers */
                            0x30, 0x00,                 /* skip past text */
                            /* data storage */
                            'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', 0x00,
                            0x00, 0x00,                 /* separator */
                            0xf0, 0x0c, 0x00, 0x00,     /* start addr */
                            0xef, 0x00,                 /* print out something */
                            0x00, 0x00,                 /* done */
                            0xe0, 0x00,                 /* halt */
                            0x00, 0x00                  
};

int main() {
    int i = 0;
    machine_state *ms = machine_state_new();
    memcpy(ms->memory, initial, sizeof(initial) / sizeof(unsigned char));
    do {
        machine_state_dump(ms);
        printf("on byte 0x%08x (%d), ins: 0x%02x%02x\n", ms->regs[15], ms->regs[15], ms->memory[ms->regs[15]], ms->memory[ms->regs[15] + 1]);
        i++;
        if (i > 32) break;
    } while (!machine_run(ms));
    
    hexDump("memory", ms->memory, 32);
    
    free(ms);
    return 0;
}
