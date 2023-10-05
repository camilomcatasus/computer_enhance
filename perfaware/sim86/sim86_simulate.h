#include <iostream>
#include <ostream>
#include <stdio.h>
struct SimContext {
    u8 registers[8];
};



void PrintContext(SimContext simContext) {
    printf("Registers");
    printf("- ax: %u", simContext.registers[0]);
    printf("- bx: %u", simContext.registers[1]);
    printf("- cx: %u", simContext.registers[2]);
    printf("- dx: %u", simContext.registers[3]);
    printf("- sp: %u", simContext.registers[4]);
    printf("- dp: %u", simContext.registers[5]);
    printf("- si: %u", simContext.registers[6]);
    printf("- di: %u", simContext.registers[7]);
}
