/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

#include "sim86.h"

#include "sim86_memory.h"
#include "sim86_text.h"
#include "sim86_decode.h"
#include "sim86_simulate.h"

#include "sim86_memory.cpp"
#include "sim86_text.cpp"
#include "sim86_decode.cpp"

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
            if (Operand.ImmediateS32 != NULL)
                return_val = Operand.ImmediateS32;
            else
                return_val = Operand.ImmediateU32;
            
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
    
    printf(" ; 0x%x->0x%x", old_val, SimContext->registers[RegisterAccess.Index]);
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
static void DisAsm8086(memory *Memory, u32 DisAsmByteCount, segmented_access DisAsmStart)
{
    segmented_access At = DisAsmStart;
    
    disasm_context Context = DefaultDisAsmContext();
    sim_context SimContext = sim_context {}; 
    u32 Count = DisAsmByteCount;

    while(Count)
    {
        instruction Instruction = DecodeInstruction(&Context, Memory, &At);
        if(Instruction.Op)
        {

            if(Count >= Instruction.Size)
            {
                Count -= Instruction.Size;
            }
            else
            {
                fprintf(stderr, "ERROR: Instruction extends outside disassembly region\n");
                break;
            }
            
            UpdateContext(&Context, Instruction);

            if(IsPrintable(Instruction))
            {
                PrintInstruction(Instruction, stdout);
            }
            
            SimulateInstruction(&SimContext, Instruction);
            printf("\n");
        }
        else
        {
            fprintf(stderr, "ERROR: Unrecognized binary in instruction stream.\n");
            break;
        }
    }

    PrintContext(SimContext);
}




int main(int ArgCount, char **Args)
{
    memory *Memory = (memory *)malloc(sizeof(memory));
    
    if(ArgCount > 1)
    {
        for(int ArgIndex = 1; ArgIndex < ArgCount; ++ArgIndex)
        {
            char *FileName = Args[ArgIndex];
            u32 BytesRead = LoadMemoryFromFile(FileName, Memory, 0);
            
            printf("; %s disassembly:\n", FileName);
            printf("bits 16\n");
            DisAsm8086(Memory, BytesRead, {});
        }
    }
    else
    {
        fprintf(stderr, "USAGE: %s [8086 machine code file] ...\n", Args[0]);
    }
    
    return 0;
}
