/*
 * 
 * tomasulo.c
 * 
 */

#include <stdlib.h>
#include <string.h>
#include "fu.h"
#include "tomasulo.h"



void fetch(state_t *state)
{   
    // check if fetch stage is locked
    if ((state->fetch_lock == FALSE && state->halted != TRUE)) {
        // fetch instruction
        uint32_t instruction;
        memcpy(&instruction, &state->mem[state->pc], sizeof(uint32_t));
        // update pipeline register
        state->if_id.instr = (int)instruction;
        state->if_id.insn_num = state->insn_num;
        // increment instruction number
        state->insn_num++;
        // increment program counter
        state->pc += 4;
    }
    // handle cases where fetch still runs once before any branch instruction or halt
    if (state->pending_fetch_lock == TRUE) {
        // set fetch lock flag
        state->fetch_lock = TRUE;
        // disable pending fetch lock flag
        state->pending_fetch_lock = FALSE;
    }

    if (state->pending_halt_lock) {
        state->halted = TRUE;
        state->pending_halt_lock = FALSE;
    }    
    
}

void dispatch(state_t *state)
{
    // local variables
    int use_imm;
    uint32_t instr;
    fu_t *fu = NULL;
    const op_info_t *op_info;
    int r1, r2, r3;

    // if halted, stop dispatching
    if (state->halted) {
        state->fu_lock = TRUE;
        return;
    }

    // clear structural hazard stall 
    state->fu_lock = FALSE;
    // decode the instruction
    instr = (uint32_t)state->if_id.instr;
    op_info = decode_instr(instr, &use_imm);

    // handle any NOP or INVALID instructions
    if (op_info->fu_group_num == FU_GROUP_NONE || op_info->fu_group_num == FU_GROUP_INVALID) {
        // do nothing
        return;
    }
    // handle HALT instructions
    if (op_info->fu_group_num == FU_GROUP_HALT) {
        // enable pending halt flag
        state->pending_halt_lock = TRUE;

        return;
    }
    
    // allocate functional units 
    switch (op_info->fu_group_num) {
        case FU_GROUP_INT:
        case FU_GROUP_BRANCH:  fu = find_available_fu(state->int_rs_list);      break;
        case FU_GROUP_ADD:     fu = find_available_fu(state->fp_add_rs_list);   break;
        case FU_GROUP_MULT:    fu = find_available_fu(state->fp_mult_rs_list);  break;
        case FU_GROUP_DIV:     fu = find_available_fu(state->fp_div_rs_list);   break;
        case FU_GROUP_MEM:     fu = find_available_fu(state->mem_fu_list);      break;
    }
    // check for structural hazards; if there is one, stall
    if (fu == NULL) {
        state->fu_lock = TRUE;
        return;
    }
    // issue an instruction to the functional unit
    fu->busy = TRUE;
    fu->instr = instr;
    fu->cycles = fu->max_cycles;
    fu->insn_num = state->if_id.insn_num;
    fu->fu_1 = NULL;
    fu->fu_2 = NULL;
    r1 = FIELD_R1(instr);
    r2 = FIELD_R2(instr);
    r3 = FIELD_R3(instr);

    // read operands and handle any RAW hazards
    if (use_imm) {
        switch (op_info->fu_group_num) {
            case FU_GROUP_INT:
                // destination = R2, source = R1, imm
                if (state->reg_map_int[r1] != NULL) {
                    fu->fu_1 = state->reg_map_int[r1];  // RAW hazard, wait until ready
                }
                else {
                    fu->operand_1.integer.w = state->rf_int.reg_int[r1].w;  // value is ready
                }
                // now, get the immediate value
                // for unsigned instructions
                if (op_info->operation == OPERATION_ADDU || op_info->operation == OPERATION_SGTU
                || op_info->operation == OPERATION_SLTU || op_info->operation == OPERATION_SUBU) {
                    fu->operand_2.integer.wu = (unsigned long)FIELD_IMMU(instr);
                }
                // signed instructions
                else {
                    fu->operand_2.integer.w = (long)FIELD_IMM(instr);
                }
                // update register mapping
                state->reg_map_int[r2] = fu;
                break;
            case FU_GROUP_MEM:
                // base = R1, data = R2, offset = imm
                if (state->reg_map_int[r1] != NULL) {
                    fu->fu_1 = state->reg_map_int[r1];  // RAW hazard, wait until ready
                }
                else {
                    fu->operand_1.integer.w = state->rf_int.reg_int[r1].w;
                }
                // handle store instructions
                if (op_info->operation == OPERATION_STORE) {
                    // floating point type?
                    if (op_info->data_type == DATA_TYPE_F) {
                        if (state->reg_map_fp[r2] != NULL) {
                            fu->fu_2 = state->reg_map_fp[r2];
                        }
                        else {
                            fu->operand_2.flt = state->rf_fp.reg_fp[r2];
                        }
                    }
                    // integer type
                    else {
                        if (state->reg_map_int[r2] != NULL) {
                            fu->fu_2 = state->reg_map_int[r2];
                        }
                        else {
                            fu->operand_2.integer.w = state->rf_int.reg_int[r2].w;
                        }                        
                    }
                }
                // handle load instructions
                else {
                    if (op_info->data_type == DATA_TYPE_F) {
                        state->reg_map_fp[r2] = fu;
                    }
                    else {
                        state->reg_map_int[r2] = fu;
                    }
                }
                break;
            case FU_GROUP_BRANCH:
                // stall pipeline
                state->pending_fetch_lock = TRUE;
                // handle different branch instructions
                switch (op_info->operation) {
                    case OPERATION_BEQZ:
                    case OPERATION_BNEZ:
                    // condtion register: R1
                    if (state->reg_map_int[r1] != NULL) {
                        fu->fu_1 = state->reg_map_int[r1];
                    }
                    else {
                        fu->operand_1.integer.w = state->rf_int.reg_int[r1].w;  
                    }
                        break;

                    case OPERATION_JR:
                    // target register: R1
                    if (state->reg_map_int[r1] != NULL) {
                        fu->fu_1 = state->reg_map_int[r1];
                    }
                    else {
                        fu->operand_1.integer.w = state->rf_int.reg_int[r1].w;
                    }
                        break;  

                    case OPERATION_JALR:
                        if (state->reg_map_int[r1] != NULL) {
                            fu->fu_1 = state->reg_map_int[r1];
                        }
                        else {
                            fu->operand_1.integer.w = state->rf_int.reg_int[r1].w;
                        }
                        state->reg_map_int[31] = fu;
                        break;

                    case OPERATION_JAL:
                        // store PC of instruction after JAL so broadcast can save return address
                        fu->operand_1.integer.wu = state->pc;
                        state->reg_map_int[31] = fu;
                        break;
                    
                    default:
                        break;
                }

                break;

            default:
                break;
        }

    }
    // handle non-immediate/R-type instructions
    else {
        // destination: R3
        switch(op_info->fu_group_num) {
            case FU_GROUP_INT:
                if (state->reg_map_int[r1] != NULL) {
                    fu->fu_1 = state->reg_map_int[r1];
                }
                else {
                    fu->operand_1.integer.w = state->rf_int.reg_int[r1].w;
                }
                if (state->reg_map_int[r2] != NULL) {
                    fu->fu_2 = state->reg_map_int[r2];
                }
                else {
                    fu->operand_2.integer.w = state->rf_int.reg_int[r2].w;
                }
                // update register alias table
                state->reg_map_int[r3] = fu;
                break;

            case FU_GROUP_ADD:
            case FU_GROUP_MULT:
            case FU_GROUP_DIV:
                if (state->reg_map_fp[r1] != NULL) {
                    fu->fu_1 = state->reg_map_fp[r1];
                }
                else {
                    fu->operand_1.flt = state->rf_fp.reg_fp[r1];
                }
                if (state->reg_map_fp[r2] != NULL) {
                    fu->fu_2 = state->reg_map_fp[r2];
                }
                else {
                    fu->operand_2.flt = state->rf_fp.reg_fp[r2];
                }
                // update register alias table
                state->reg_map_fp[r3] = fu;
                break; 
            // NOP instruction: throw away instruction
            case FU_GROUP_NONE:
                return;    
            default:
                break;       
        }
    }
}


void execute(state_t *state)
{
    state->cdb_fp = NULL;
    state->cdb_int = NULL;

    // advance reservation stations
    advance_rs(state->int_rs_list, &state->cdb_int);
    advance_rs(state->fp_add_rs_list, &state->cdb_fp);
    advance_rs(state->fp_div_rs_list, &state->cdb_fp);
    advance_rs(state->fp_mult_rs_list, &state->cdb_fp);
    // advance memory buffer
    advance_memory_buffer(state->mem_fu_list, &state->cdb_int, &state->cdb_fp);
    // if halted and every FU is idle, we are done
    if (state->halted && state->cdb_fp == NULL && state->cdb_int == NULL && fu_list_done(state->int_rs_list)
    && fu_list_done(state->fp_add_rs_list) && fu_list_done(state->fp_div_rs_list) && fu_list_done(state->fp_mult_rs_list)
    && fu_list_done(state->mem_fu_list)) {
        // mark the state as finished
        state->finished = TRUE;
    }
 
}

void broadcast(state_t *state, int *num_insn)
{
    // local variables
    fu_t           *cdb;
    int             use_imm;
    const op_info_t *op_info;
    operand_t       result;
    int             r1, r2, r3;
    unsigned long   addr;
    

    // handle integer CDB
    if (state->cdb_int != NULL)
    {
        cdb     = state->cdb_int;
        op_info = decode_instr(cdb->instr, &use_imm);
        r1      = FIELD_R1(cdb->instr);
        r2      = FIELD_R2(cdb->instr);
        r3      = FIELD_R3(cdb->instr);
 
        result = perform_operation(cdb->instr, state->pc, cdb->operand_1, cdb->operand_2);
 
        if (op_info->fu_group_num == FU_GROUP_BRANCH)
        {
            // clear fetch lock
            state->fetch_lock = FALSE;
 
            switch (op_info->operation)
            {
            case OPERATION_J:
                // set the current PC to the result of operation
                state->pc = result.integer.wu;
                // issue NOP
                state->if_id.instr = 0;
                break;
 
            case OPERATION_JAL:
                // compute target using the branch instruction's own PC
                // return address is the PC saved in operand_1 at dispatch time
                state->rf_int.reg_int[31].wu = cdb->operand_1.integer.wu;
                if (state->reg_map_int[31] == cdb)
                    state->reg_map_int[31] = NULL;
                // jump target = result from perform_operation
                state->pc = result.integer.wu;
                // issue NOP
                state->if_id.instr = 0;
                break;
 
            case OPERATION_JR:
                state->pc = result.integer.wu;
                state->if_id.instr = 0;
                break;
 
            case OPERATION_JALR:
                // return address is the PC saved in operand_1 at dispatch time
                state->rf_int.reg_int[31].wu = cdb->operand_1.integer.wu;
                if (state->reg_map_int[31] == cdb)
                    state->reg_map_int[31] = NULL;
                // jump target is the register value = result
                state->pc = result.integer.wu;
                // issue NOP
                state->if_id.instr = 0;
                break;
 
            case OPERATION_BEQZ:
                // compute target using the branch instruction's own PC
                
                if (result.integer.wu != state->pc)
                {
                    state->pc = result.integer.wu;
                    state->if_id.instr = 0;
                }
                break;
 
            case OPERATION_BNEZ:
                // compute target using the branch instruction's own PC
                if (result.integer.wu != 0)
                {
                    state->pc = result.integer.wu;
                    state->if_id.instr = 0;
                }
                break;
 
            default:
                break;
            }
        }
        else if (op_info->fu_group_num == FU_GROUP_MEM)
        {
            // result holds the computed address from perform_operation
            addr = result.integer.wu;
            if (op_info->operation == OPERATION_LOAD)
            {
                // read 4 bytes from memory at the computed address
                result.integer.wu = ((unsigned long)state->mem[addr + 3] << 24) |
                                    ((unsigned long)state->mem[addr + 2] << 16) |
                                    ((unsigned long)state->mem[addr + 1] <<  8) |
                                    ((unsigned long)state->mem[addr]);
                if (state->reg_map_int[r2] == cdb) {
                    state->rf_int.reg_int[r2] = result.integer;
                    state->reg_map_int[r2] = NULL;
                }
            }
            else
            {
                // store: write operand_2 (data register) to memory
                state->mem[addr]     = (cdb->operand_2.integer.wu)        & 0xFF;
                state->mem[addr + 1] = (cdb->operand_2.integer.wu >>  8)  & 0xFF;
                state->mem[addr + 2] = (cdb->operand_2.integer.wu >> 16)  & 0xFF;
                state->mem[addr + 3] = (cdb->operand_2.integer.wu >> 24)  & 0xFF;
            }
        }
        else
        {
            if (use_imm)
            {
                if (state->reg_map_int[r2] == cdb) {
                state->rf_int.reg_int[r2] = result.integer;
                state->reg_map_int[r2] = NULL;   
                }
            }
            else
            {
                if (state->reg_map_int[r3] == cdb) {
                state->rf_int.reg_int[r3] = result.integer;
                state->reg_map_int[r3] = NULL;
                }
            }
        }
 
        update_fu_list(state->int_rs_list,     cdb, result);
        update_fu_list(state->fp_add_rs_list,  cdb, result);
        update_fu_list(state->fp_mult_rs_list, cdb, result);
        update_fu_list(state->fp_div_rs_list,  cdb, result);
        update_fu_list(state->mem_fu_list,     cdb, result);
 
        (*num_insn)++;
        cdb->busy  = FALSE;
        state->cdb_int = NULL;
    }
    
    // handle Floating Point CDB
    if (state->cdb_fp != NULL)
    {
        cdb     = state->cdb_fp;
        op_info = decode_instr(cdb->instr, &use_imm);
        r2      = FIELD_R2(cdb->instr);
        r3      = FIELD_R3(cdb->instr);
 
        result = perform_operation(cdb->instr, state->pc,
                                   cdb->operand_1, cdb->operand_2);
 
        if (op_info->fu_group_num == FU_GROUP_MEM)
        {
            // result holds the computed address from perform_operation
            addr = result.integer.wu;
            if (op_info->operation == OPERATION_LOAD)
            {
                // read 4 bytes from memory and reinterpret as float
                uint32_t raw = ((uint32_t)state->mem[addr + 3] << 24) |
                               ((uint32_t)state->mem[addr + 2] << 16) |
                               ((uint32_t)state->mem[addr + 1] <<  8) |
                               ((uint32_t)state->mem[addr]);
                memcpy(&result.flt, &raw, sizeof(float));
                if (state->reg_map_fp[r2] == cdb) {
                    state->rf_fp.reg_fp[r2] = result.flt;
                    state->reg_map_fp[r2] = NULL;
                }

            }
            else
            {
                // store: write float operand_2 to memory
                uint32_t raw;
                memcpy(&raw, &cdb->operand_2.flt, sizeof(float));
                state->mem[addr]     = (raw)        & 0xFF;
                state->mem[addr + 1] = (raw >>  8)  & 0xFF;
                state->mem[addr + 2] = (raw >> 16)  & 0xFF;
                state->mem[addr + 3] = (raw >> 24)  & 0xFF;
            }
        }
        else
        {
            if (state->reg_map_fp[r3] == cdb) {
                state->rf_fp.reg_fp[r3] = result.flt;
                state->reg_map_fp[r3] = NULL;
            }
        }
        
        update_fu_list(state->int_rs_list,     cdb, result);
        update_fu_list(state->fp_add_rs_list,  cdb, result);
        update_fu_list(state->fp_mult_rs_list, cdb, result);
        update_fu_list(state->fp_div_rs_list,  cdb, result);
        update_fu_list(state->mem_fu_list,     cdb, result);
 
        (*num_insn)++;
        cdb->busy = FALSE;
        state->cdb_fp = NULL;
    }
}