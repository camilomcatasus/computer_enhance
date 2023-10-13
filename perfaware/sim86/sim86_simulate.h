#include <iostream>
#include <ostream>
#include <stdio.h>



struct sim_context {
    u16 registers[16] = {0};
};



void PrintContext(sim_context simContext) {
    const char* RegNames[16]  = { "none", "a", "b", "c", "d", "sp", "bp", "si", "di", "es", "cs", "ss", "ds", "ip", "flags", "count" };
    printf("Registers\n");
    for(int i = 1; i < 16; i++) {
        printf("- %s: %#06x\n", RegNames[i], simContext.registers[i]);
    }
}
