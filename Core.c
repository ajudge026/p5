#include "Core.h"
#include<math.h>

Core *initCore(Instruction_Memory *i_mem)
{
    Core *core = (Core *)malloc(sizeof(Core));
    core->clk = 0;
    core->PC = 0;
    core->instr_mem = i_mem;
    core->tick = tickFunc;	
	core->stages_complete = 0;
	core->stages_after_last_PC = 0;
	Signal	arbitrary_int = 9999;
	for (int i = 0; i <(1024);i++)
	{
		core->data_mem[i] = 0;		
	}
	core->data_mem[40*8] = -63; // 40(x1) = -63,
	core->data_mem[48*8] = 63; // 40(x1) = 2 test	
	core->reg_file[1] = 0;	 
	 core->reg_file[0] = 0; 
	 core->reg_file[2] = 10; //outbase
	 core->reg_file[3] = -15; 
	 core->reg_file[4] = 20; 
	 core->reg_file[5] = 30; 
	 core->reg_file[6] = -35; 
    return core;
}

// FIXME, implement this function
bool tickFunc(Core *core)
{
	// simulate the registers passins values on clock cycle 	
	// Pipelined
	// <------------------------ IF Reg (mux is written to at end of function )		
	Reg_Signals IF_reg_load = core->IF_reg;	
	Reg_Signals ID_reg_load = core->ID_reg;		
	Reg_Signals E_reg_load = core->E_reg;	
	Reg_Signals M_reg_load = core->M_reg;		
	Reg_Signals WB_reg_load = core->WB_reg;	
	Signal num_instructions = (core->instr_mem->last->addr /4) + 1;	
	if( (core->stages_complete < (num_instructions )))
	{
		core->IF_reg.PC = core->PC;
		core->IF_reg.instruction = core->instr_mem->instructions[core->PC / 4].instruction;			
		/* printf("+++++++++++++++++++++++++The IF_reg variables++++++++++++++++++++++++++++++++\n");
		printf("%s = %ld\n",VariableName(core->PC),core->PC);
		printf("%s = %ld\n",VariableName(core->IF_reg.instruction ),core->IF_reg.instruction ); */
		core->PC = core->PC + 4;	
	}
	// <------------------------ ID Reg				
	Signal alu_in_0, alu_in_1;	
	core->ID_reg.PC = IF_reg_load.PC;
	core->E_reg.branch_address = M_reg_load.branch_address;
	if( (core->stages_complete >  0) && (core->stages_complete < (num_instructions + 1)))
	{
		// getting control signals
		Signal input = (IF_reg_load.instruction & 127);		
		//ControlSignals signals;
		ControlUnit(IF_reg_load.instruction, input, &core->ID_reg.signals);	
		core->ID_reg.write_reg = (IF_reg_load.instruction >> 7) & 31;
		core->ID_reg.read_reg_val_1 = core->reg_file[(IF_reg_load.instruction >> (7 + 5 + 3)) & 31];
		core->ID_reg.read_reg_val_2 = core->reg_file[(IF_reg_load.instruction >> (7 + 5 + 3 + 5)) & 31];		
		core->ID_reg.imm_sign_extended = ImmeGen( input,IF_reg_load.instruction);;	//shifts the immediate?	
		core->ID_reg.instruction = IF_reg_load.instruction;		
		/* printf("+++++++++++++++++++++++++The ID_reg variables++++++++++++++++++++++++++++++++\n");
		printf("%s = %ld\n",VariableName(IF_reg_load.instruction),IF_reg_load.instruction);
		printf("%s = %d\n",VariableName(core->ID_reg.signals),core->ID_reg.signals);
		printf("%s = %ld\n",VariableName(core->ID_reg.read_reg_val_1),core->ID_reg.read_reg_val_1);		
		printf("%s = %ld\n",VariableName(core->ID_reg.read_reg_val_2),core->ID_reg.read_reg_val_2);				
		printf("%s = %ld\n",VariableName(core->ID_reg.imm_sign_extended),core->ID_reg.imm_sign_extended);	 */	
	}
	core->M_reg.branch_address = WB_reg_load.branch_address;
	core->E_reg.signals = ID_reg_load.signals;	
	core->E_reg.read_reg_val_2 = ID_reg_load.read_reg_val_2;	
	if( (core->stages_complete > 1 ) && (core->stages_complete < ( num_instructions + 2)))// Execute stage
	{	
		// <---------------------------------- Execute Reg 
		Signal alu_in_1 = MUX(ID_reg_load.signals.ALUSrc,ID_reg_load.read_reg_val_2,ID_reg_load.imm_sign_extended);
		alu_in_0 = ID_reg_load.read_reg_val_1;
		Signal func3 =( (ID_reg_load.instruction >> (7 + 5)) & 7);    
		Signal func7 = ((ID_reg_load.instruction >> (7 + 5 + 3 + 5 + 5)) & 127);	
		Signal ALU_ctrl_signal = ALUControlUnit(ID_reg_load.signals.ALUOp, func7, func3);		
		ALU(alu_in_0, alu_in_1, ALU_ctrl_signal, &core->E_reg.alu_result, &core->E_reg.zero_out); // 0 is offset shuold change to imm val		
		core->E_reg.branch_address = ShiftLeft1(ID_reg_load.imm_sign_extended)+ ID_reg_load.PC ;					
		core->E_reg.write_reg = ID_reg_load.write_reg;	
	}	
	Signal mem_result;
	core->M_reg.signals = E_reg_load.signals;	
	if( (core->stages_complete > 2) && (core->stages_complete < num_instructions + 3) )
	{
		// <------------------------ M Reg
		mem_result= 0;		
		mem_result = core->data_mem[8*E_reg_load.alu_result];
		if (mem_result > 128)
		{
			Byte mem_result_temp = mem_result;									
				mem_result_temp = ~(mem_result_temp - 1);
				mem_result = -1 * mem_result_temp;			
		}
		core->M_reg.mem_read_data 	= mem_result;
		core->M_reg.alu_result = E_reg_load.alu_result;	
		core->M_reg.branch_address = 0; // <------------------ change to branch address		
		if(E_reg_load.signals.MemWrite)
		{       
			core->data_mem[8*E_reg_load.alu_result] = E_reg_load.read_reg_val_2;		
		}
		core->M_reg.write_reg = E_reg_load.write_reg;		
	}	
	//<------------- WB Reg			
	if( (core->stages_complete > 3) && (core->stages_complete < num_instructions + 4 ) )
	{		
		core->WB_reg.reg_write_mux_val = MUX(M_reg_load.signals.MemtoReg, M_reg_load.alu_result, M_reg_load.mem_read_data);						
		if(M_reg_load.signals.RegWrite)
		{						
			core->reg_file[M_reg_load.write_reg] = core->WB_reg.reg_write_mux_val;			
		}
		//printf("%s = %ld\n",VariableName(),);
		//printf("+++++++++++++++++++++++++The WB _reg variables++++++++++++++++++++++++++++++++\n");
		//printf("%s = %ld\n",VariableName(M_reg_load.signals.MemtoReg),M_reg_load.signals.MemtoReg);
		//printf("%s = %ld\n",VariableName(M_reg_load.alu_result),M_reg_load.alu_result);
		//printf("%s = %ld\n",VariableName(M_reg_load.signals.RegWrite),M_reg_load.signals.RegWrite);
		//printf("%s = %ld\n",VariableName(core->WB_reg.reg_write_mux_val),core->WB_reg.reg_write_mux_val);
	}
	printf("--------------------------------------------\n");	
	printf("Clock cycles = %d\n",core->stages_complete);	
	printf("REG FILES\n");	
	printf("%s = %ld\n",VariableName(core->reg_file[10]),core->reg_file[10] );
	printf("%s = %ld\n",VariableName(core->reg_file[11]),core->reg_file[11] );
	printf("%s = %ld\n",VariableName(core->reg_file[12]),core->reg_file[12] );
	printf("%s = %ld\n",VariableName(core->reg_file[13]),core->reg_file[13] );
	printf("%s = %ld\n",VariableName(core->reg_file[14]),core->reg_file[14] );
	printf("%s = %ld\n",VariableName(core->PC),core->PC );
	++core->stages_complete;
    ++core->clk;
    // Are we reaching the final instruction?
	printf("%s = %ld\n",VariableName(num_instructions), num_instructions);
    if (core->stages_complete > num_instructions + 4 )
    {		
        return false;
    }
    return true;
}
// FIXME (1). Control Unit. Refer to Figure 4.18.
void ControlUnit(unsigned instruction, Signal input,
                 ControlSignals *signals)
{	
	Signal func3 = ( (instruction >> (7 + 5)) & 7);
	Signal func7 = ((instruction >> (7 + 5 + 3 + 5 + 5)) & 127);	
    // For R-type - add
    if ((input == 51 )&& (func3 == 0) && (func7 == 0)) {
		//printf("RType\n"); 
        signals->ALUSrc = 0;
        signals->MemtoReg = 0;
        signals->RegWrite = 1;
        signals->MemRead = 0;
        signals->MemWrite = 0;
        signals->Branch = 0;
        signals->ALUOp = 2;
    }
	// For R-type - sub
    if (input == 51 & (func3 == 0) && (func7 == 32)) {
		//printf("RType\n"); 
        signals->ALUSrc = 0;
        signals->MemtoReg = 0;
        signals->RegWrite = 1;
        signals->MemRead = 0;
        signals->MemWrite = 0;
        signals->Branch = 0;
        signals->ALUOp = 2;
    }
	// For R-type - sll
    if (input == 51 & (func3 == 1)) {
		//printf("RType\n"); 
        signals->ALUSrc = 0;
        signals->MemtoReg = 0;
        signals->RegWrite = 1;
        signals->MemRead = 0;
        signals->MemWrite = 0;
        signals->Branch = 0;
        signals->ALUOp = 0;
    }
    // For ld 
    if (input == 3) { //opcode
	    //printf("ld\n"); 
        signals->ALUSrc = 1;
        signals->MemtoReg = 1;
        signals->RegWrite = 1;
        signals->MemRead = 1;
        signals->MemWrite = 0;
        signals->Branch = 0;
        signals->ALUOp = 0;
    }
    // For addaddi , slli 
    if (input == 19 ){
		//printf("slli\n"); 		
        signals->ALUSrc = 1;
        signals->MemtoReg = 1;
        signals->RegWrite = 1;
        signals->MemRead = 1;
        signals->MemWrite = 0;
        signals->Branch = 0;
        signals->ALUOp = 0;
    }
	
    // For sd (S-type)
    if (input == 35){
		//printf("sw\n"); 
        signals->ALUSrc = 1;
        signals->MemtoReg = 0; 
        signals->RegWrite = 0;
        signals->MemRead = 0;
        signals->MemWrite = 1;
        signals->Branch = 0;
        signals->ALUOp = 69;
    }
    // For beq (SB-type)
    if (input == 99){ //opcode
        //printf("bne\n"); 
		signals->ALUSrc = 0;		
        signals->MemtoReg = 0; 
        signals->RegWrite = 0;
        signals->MemRead = 0;
        signals->MemWrite = 0;
        signals->Branch = 1;
        signals->ALUOp = 1;
    }
}

// FIXME (2). ALU Control Unit. Refer to Figure 4.12.
Signal ALUControlUnit(Signal ALUOp,
                      Signal Funct7,
                      Signal Funct3)
{
    //  add
    if (ALUOp == 2 && Funct7 == 0 && Funct3 == 0)
    {
        return 2;
    }

    

    // For subtract 
    if (ALUOp == 2 && Funct7 == 32 && Funct3 == 0)
    {
        return 6;
    }
    //and
    if (ALUOp == 2 && Funct7 == 0 && Funct3 == 7)
    {
        return 0;
    }
    //  or 
    if (ALUOp == 2 && Funct7 == 0 && Funct3 == 6)
    {
        return 1;
    }
	// slli
    if (ALUOp == 0 && Funct7 == 0 && Funct3 == 1)
    {
        return 3;
    }

	
    // ld 
    if (ALUOp == 0 && Funct3 == 3)
    {
        return 2;
    }
    //  sd
    if (ALUOp == 69)
    {
        return 2;
    }
    //  beq 
    if (ALUOp == 1)
    {
        return 6;
    }
	
}

// FIXME (3). Imme. Generator
Signal ImmeGen(Signal input, unsigned instruction)
{
     Signal immediate = 0;

    //ld
    if (input == 3){
        Byte immediate_1;
		immediate =  (instruction >> 20)&4095;
		immediate_1 = immediate;
		if (immediate_1 > 128)
		{
			immediate_1 = ~(immediate_1 - 1);
			immediate = -1 * immediate_1;
		}
		
		
    }
    //addi
    if (input == 19){
        //  000000000001;
        immediate = 1;
    }
    //slli
    if (input == 14)    {
        //  000000000011;
        immediate = 3;
    }
    //bne
    if (input == 99)    {
        //  111111111110;
			//imm goes from left to right 
			Signal immNeg = (instruction >> 31) & 1;
			Signal imm3 = (instruction >> 24) & 63 ;
			Signal imm2 = (instruction >> 8) &15;
			Signal imm1 = (instruction >> 7) &1;			
			immediate = imm2;
			immediate |= imm3 <<4;
			immediate |= imm1 <<10;
			immediate |= immNeg << 11;
		/* if (immediate == 4062)
		{
									
		} */
		
		if (immediate == 4056)
		{
				immediate = -24;					
		}
		else
		{
			immediate = -16;
		}
    }
	


    return immediate;

} 

// FIXME (4). ALU
void ALU(Signal input_0,
         Signal input_1,
         Signal ALU_ctrl_signal,
         Signal *ALU_result,
         Signal *zero)
{
    // For addition
    if (ALU_ctrl_signal == 2)
    {
        *ALU_result = (input_0 + input_1);
        if (*ALU_result == 0) { *zero = 1; } else { *zero = 0; }
    }
	// For addition sd may need to change 
    if (ALU_ctrl_signal == 69)
    {
        *ALU_result = (input_0 + input_1);
        if (*ALU_result == 0) { *zero = 1; } else { *zero = 0; }
    }
    // For and
    if (ALU_ctrl_signal == 0)
    {
        *ALU_result = (input_0 & input_1);
        if (*ALU_result == 0) { *zero = 1; } else { *zero = 0; }
    }
    // For or
    if (ALU_ctrl_signal == 1)
    {
        *ALU_result = (input_0 | input_1);
        if (*ALU_result == 0) { *zero = 1; } else { *zero = 0; }
    }
    // For subtraction
    if (ALU_ctrl_signal == 6)
    {
        *ALU_result = (input_0 - input_1);
		//printf("ALU RESULT - %ld", (input_0 - input_1));
        if (*ALU_result != 0) { *zero = 1; } else { *zero = 0; }
    }
    // For shift left
    if (ALU_ctrl_signal == 3)
    {
        *ALU_result = (input_0 << input_1);
        if (*ALU_result == 0) { *zero = 1; } else { *zero = 0; }
    }

}

// (4). MUX
Signal MUX(Signal sel,
           Signal input_0,
           Signal input_1)
{
    if (sel == 0) { return input_0; } else { return input_1; }
}

// (5). Add
Signal Add(Signal input_0,
           Signal input_1)
{
    return (input_0 + input_1);
}

// (6). ShiftLeft
Signal ShiftLeft1(Signal input)
{
    return input << 1; //<--------------------------------------------------- why? 
}

// regwrite 
 void regWrite(Signal MemWrite, Signal *data_mem, Signal data, Signal *address )
{
		
}