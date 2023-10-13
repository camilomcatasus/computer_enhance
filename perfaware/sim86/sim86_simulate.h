#include <iostream>
#include <ostream>
#include <stdio.h>

struct sim_context {
    u16 registers[15] = {0};
};



void PrintContext(sim_context simContext) {
    printf("Registers\n");
    printf("- ax: %#06x\n", simContext.registers[1]);
    printf("- bx: %#06x\n", simContext.registers[2]);
    printf("- cx: %#06x\n", simContext.registers[3]);
    printf("- dx: %#06x\n", simContext.registers[4]);
    printf("- sp: %#06x\n", simContext.registers[5]);
    printf("- dp: %#06x\n", simContext.registers[6]);
    printf("- si: %#06x\n", simContext.registers[7]);
    printf("- di: %#06x\n", simContext.registers[8]);
    printf("- es: %#06x\n", simContext.registers[9]);
    printf("- ss: %#06x\n", simContext.registers[10]);
    printf("- ds: %#06x\n", simContext.registers[11]);
}
