#include <iostream>
#include <ostream>
#include <stdio.h>

struct sim_context {
    u16 registers[11] = {0};
};

static u16 registers[11] = {0};


void PrintContext(sim_context simContext) {
    printf("Registers");
    printf("- ax: %u", simContext.registers[0]);
    printf("- bx: %u", simContext.registers[1]);
    printf("- cx: %u", simContext.registers[2]);
    printf("- dx: %u", simContext.registers[3]);
    printf("- sp: %u", simContext.registers[4]);
    printf("- dp: %u", simContext.registers[5]);
    printf("- si: %u", simContext.registers[6]);
    printf("- di: %u", simContext.registers[7]);
    printf("- es: %u", simContext.registers[8]);
    printf("- ss: %u", simContext.registers[9]);
    printf("- ds: %u", simContext.registers[10]);
}
