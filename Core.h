#ifndef __CORE_H__
#define __CORE_H__

#include "Instruction_Memory.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#define VariableName(name) #name

#define BOOL bool

typedef uint8_t Byte;
typedef int64_t Signal;
typedef int64_t Register;

// FIXME. Implement the following functions in Core.c
// FIXME (1). Control Unit.



typedef struct ControlSignals
{
    Signal Branch;
    Signal MemRead;
    Signal MemtoReg;
    Signal ALUOp;
    Signal MemWrite;
    Signal ALUSrc;
    Signal RegWrite;
}ControlSignals;
typedef struct Reg_Signals
{
    Signal Branch;
    Signal MemRead;
    Signal MemtoReg;
    Signal ALUOp;
    Signal MemWrite;
    Signal ALUSrc;
    Signal RegWrite;
	Signal PC;
    Signal instruction;
	ControlSignals signals;	
    Signal read_reg_val_1;
	Signal read_reg_val_2;
    Signal imm_sign_extended;	
	Signal write_reg;
	Signal zero_out;
	Signal branch_address;
	Signal reg_read_2_val;
    Signal mem_read_data;
	Signal alu_result;
	Signal reg_write_mux_val;			
}Reg_Signals;
struct Core;
typedef struct Core Core;
typedef struct Core
{
    Tick clk; // Keep track of core clock
    Addr PC; // Keep track of program counter

    // What else you need? Data memory? Register file?
    Instruction_Memory *instr_mem;
   
    Byte data_mem[1024]; // data memory

    Register reg_file[32]; // register file.

    bool (*tick)(Core *core);
	
	Reg_Signals IF_reg;
	
	Reg_Signals ID_reg;	
	
	Reg_Signals E_reg;
	
	Reg_Signals M_reg;
	
	Reg_Signals WB_reg;
	
	int stages_complete;
	
	int stages_after_last_PC;
	
}Core;





Core *initCore(Instruction_Memory *i_mem);
bool tickFunc(Core *core);


void ControlUnit(unsigned instruction, 
				Signal input,
                 ControlSignals *signals);

// FIXME (2). ALU Control Unit.
Signal ALUControlUnit(Signal ALUOp,
                      Signal Funct7,
                      Signal Funct3);

// FIXME (3). Imme. Generator
Signal ImmeGen(Signal input, unsigned instruction);

// FIXME (4). ALU
void ALU(Signal input_0,
         Signal input_1,
         Signal ALU_ctrl_signal,
         Signal *ALU_result,
         Signal *zero);

// (4). MUX
Signal MUX(Signal sel,
           Signal input_0,
           Signal input_1);

// (5). Add
Signal Add(Signal input_0,
           Signal input_1);

// (6). ShiftLeft1
Signal ShiftLeft1(Signal input);

#endif

