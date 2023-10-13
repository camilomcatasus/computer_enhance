/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

#include "sim86.h"
#include <string>
#include <vector>

/* NOTE(casey): _CRT_SECURE_NO_WARNINGS is here because otherwise we cannot
   call fopen(). If we replace fopen() with fopen_s() to avoid the warning,
   then the code doesn't compile on Linux anymore, since fopen_s() does not
   exist there.
   
   What exactly the CRT maintainers were thinking when they made this choice,
   I have no idea.
*/
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "sim86_instruction.h"
#include "sim86_instruction_table.h"
#include "sim86_memory.h"
#include "sim86_text.h"
#include "sim86_decode.h"
#include "sim86_simulate.h"
#include "sim86_instruction.cpp"
#include "sim86_instruction_table.cpp"
#include "sim86_memory.cpp"
#include "sim86_text.cpp"
#include "sim86_decode.cpp"

static u32 LoadMemoryFromFile(char *FileName, segmented_access SegMem, u32 AtOffset)
{
    u32 Result = 0;
    
    // NOTE(casey): Because we are simulating a machine, we only attempt to load as
    // much of a file as will fit into that machine's memory. 
    // Any additional bytes are discarded.
    u32 BaseAddress = GetAbsoluteAddressOf(SegMem, AtOffset);
    u32 HighAddress = GetHighestAddress(SegMem);
    u32 MaxBytes = (HighAddress - BaseAddress) + 1;
    
    FILE *File = fopen(FileName, "rb");
    if(File)
    {
        Result = fread(SegMem.Memory + BaseAddress, 1, MaxBytes, File);
        fclose(File);
    }
    else
    {
        fprintf(stderr, "ERROR: Unable to open %s.\n", FileName);
    }
    
    return Result;
}

static segmented_access AllocateMemoryPow2(u32 SizePow2)
{
    static u8 FailedAllocationByte;
    
    u8 *Memory = (u8 *)malloc(1 << SizePow2);
    if(!Memory)
    {
        SizePow2 = 0;
        Memory = &FailedAllocationByte;
    }
    
    segmented_access Result = FixedMemoryPow2(SizePow2, Memory);
    return Result;
}

static std::vector<instruction> DisAsm8086(u32 DisAsmByteCount, segmented_access DisAsmStart, bool verbose)
{
    segmented_access At = DisAsmStart;
    
    instruction_table Table = Get8086InstructionTable();
    std::vector<instruction> Instructions;
    u32 Count = DisAsmByteCount;
    while(Count)
    {
        instruction Instruction = DecodeInstruction(Table, At);
        if(Instruction.Op)
        {
            if(Count >= Instruction.Size)
            {
                At = MoveBaseBy(At, Instruction.Size);
                Count -= Instruction.Size;
            }
            else
            {
                fprintf(stderr, "ERROR: Instruction extends outside disassembly region\n");
                break;
            }
            
            Instructions.push_back(Instruction);
            if(verbose) {
                PrintInstruction(Instruction, stdout);
                printf("\n");
            }
        }
        else
        {
            fprintf(stderr, "ERROR: Unrecognized binary in instruction stream.\n");
            break;
        }
    }

    return Instructions;
}

u16 GetOperandValue(sim_context* SimContext, instruction_operand Operand) {
    u16 return_val = 0x0;

    switch (Operand.Type) {
        case Operand_Register: {
            u16 temp_val = SimContext->registers[Operand.Register.Index] >> (8 * Operand.Register.Offset);
            if(Operand.Register.Count == 1)
                temp_val = temp_val & 0b11111111;
            return_val = temp_val;
            
        } break;
        case Operand_Immediate: {
                return_val = Operand.Immediate.Value;
            
        } break;
        default: break;
    }

    return return_val;
}

void SetRegisterValue(sim_context* SimContext, register_access RegisterAccess, u16 value) {
    u16 temp_val = value;
    u16 old_val = SimContext->registers[RegisterAccess.Index];
    if(RegisterAccess.Count == 1)
    {
        SimContext->registers[RegisterAccess.Index] = SimContext->registers[RegisterAccess.Index] & (0b11111111 << (8 * (1 - RegisterAccess.Offset)));
        SimContext->registers[RegisterAccess.Index] = SimContext->registers[RegisterAccess.Index] | (value << (8 * RegisterAccess.Offset));
    }
    else
    {
        SimContext->registers[RegisterAccess.Index] = value;
    }

    const char* RegNames[16]  = { "none", "a", "b", "c", "d", "sp", "bp", "si", "di", "es", "cs", "ss", "ds", "ip", "flags", "count" };
    
    printf("%s 0x%x->0x%x\n", RegNames[RegisterAccess.Index], old_val, SimContext->registers[RegisterAccess.Index]);
}

void SimulateInstruction(sim_context* SimContext, instruction Instruction) {
    switch (Instruction.Op) {
        case Op_mov: {
            u8 base_register = 0;
            u16 load_value = GetOperandValue(SimContext, Instruction.Operands[1]);
            switch (Instruction.Operands[0].Type) {
                case Operand_Register: {
                    SetRegisterValue(SimContext, Instruction.Operands[0].Register, load_value);
                } break;
                case Operand_Memory: {

                } break;
                default: {
                    fprintf(stderr, "ERROR: Did not expect operand type in instruction");
                } break;
            }
        } break;

        default:
            break;
    }
}

void Simulate(std::vector<instruction> Instructions) {
    sim_context SimContext; 
    const u32 IP_INDEX = 13;
    
    while(SimContext.registers[IP_INDEX] != Instructions.size())
    {
        SimulateInstruction(&SimContext, Instructions[SimContext.registers[IP_INDEX]]);
        SimContext.registers[IP_INDEX] += 1;
    }

    PrintContext(SimContext);
}

int main(int ArgCount, char **Args)
{
    segmented_access MainMemory = AllocateMemoryPow2(20);
    if(IsValid(MainMemory))
    {
        if(ArgCount > 1)
        {
            for(int ArgIndex = 1; ArgIndex < ArgCount; ++ArgIndex)
            {
                char *FileName = Args[ArgIndex];
                u32 BytesRead = LoadMemoryFromFile(FileName, MainMemory, 0);
                
                printf("; %s disassembly:\n", FileName);
                printf("bits 16\n");
                std::vector<instruction> Instructions = DisAsm8086(BytesRead, MainMemory, true);
                Simulate(Instructions);
            }
        }
        else
        {
            fprintf(stderr, "USAGE: %s [8086 machine code file] ...\n", Args[0]);
        }
    }
    else
    {
        fprintf(stderr, "ERROR: Unable to allow main memory for 8086.\n");
    }
    
    return 0;
}
