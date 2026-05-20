/*
 * 
 * fu.c
 * 
   This module was originally written by Paul Kohout and adapted for
   this simulator.

 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "fu.h"
#include "tomasulo.h"

#define MAX_FIELD_LENGTH 100

const char rs_group_int_name[] = "ALU";
const char rs_group_add_name[] = "ADD";
const char rs_group_mult_name[] = "MULT";
const char rs_group_div_name[] = "DIV";
const char fu_group_mem_name[] = "MEM";

/*
  {{name, fu_group_num, operation, data_type}, sub_table}
*/

const op_t op_table[] = {
    {{NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE}, op_special_table},
    {{NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE}, op_fparith_table},
    {{"J", FU_GROUP_BRANCH, OPERATION_J, DATA_TYPE_NONE}, NULL},
    {{"JAL", FU_GROUP_BRANCH, OPERATION_JAL, DATA_TYPE_NONE}, NULL},
    {{"BEQZ", FU_GROUP_BRANCH, OPERATION_BEQZ, DATA_TYPE_NONE}, NULL},
    {{"BNEZ", FU_GROUP_BRANCH, OPERATION_BNEZ, DATA_TYPE_NONE}, NULL},
    {{NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE}, NULL},
    {{NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE}, NULL},
    {{"ADDI", FU_GROUP_INT, OPERATION_ADD, DATA_TYPE_NONE}, NULL},
    {{"ADDUI", FU_GROUP_INT, OPERATION_ADDU, DATA_TYPE_NONE}, NULL},
    {{"SUBI", FU_GROUP_INT, OPERATION_SUB, DATA_TYPE_NONE}, NULL},
    {{"SUBUI", FU_GROUP_INT, OPERATION_SUBU, DATA_TYPE_NONE}, NULL},
    {{"ANDI", FU_GROUP_INT, OPERATION_AND, DATA_TYPE_NONE}, NULL},
    {{"ORI", FU_GROUP_INT, OPERATION_OR, DATA_TYPE_NONE}, NULL},
    {{"XORI", FU_GROUP_INT, OPERATION_XOR, DATA_TYPE_NONE}, NULL},
    {{NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE}, NULL},
    {{NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE}, NULL},
    {{NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE}, NULL},
    {{"JR", FU_GROUP_BRANCH, OPERATION_JR, DATA_TYPE_NONE}, NULL},
    {{"JALR", FU_GROUP_BRANCH, OPERATION_JALR, DATA_TYPE_NONE}, NULL},
    {{"SLLI", FU_GROUP_INT, OPERATION_SLL, DATA_TYPE_NONE}, NULL},
    {{NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE}, NULL},
    {{"SRLI", FU_GROUP_INT, OPERATION_SRL, DATA_TYPE_NONE}, NULL},
    {{NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE}, NULL},
    {{NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE}, NULL},
    {{NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE}, NULL},
    {{"SLTI", FU_GROUP_INT, OPERATION_SLT, DATA_TYPE_NONE}, NULL},
    {{"SGTI", FU_GROUP_INT, OPERATION_SGT, DATA_TYPE_NONE}, NULL},
    {{NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE}, NULL},
    {{NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE}, NULL},
    {{NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE}, NULL},
    {{NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE}, NULL},
    {{NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE}, NULL},
    {{NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE}, NULL},
    {{NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE}, NULL},
    {{"LW", FU_GROUP_MEM, OPERATION_LOAD, DATA_TYPE_W}, NULL},
    {{NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE}, NULL},
    {{NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE}, NULL},
    {{"L.S", FU_GROUP_MEM, OPERATION_LOAD, DATA_TYPE_F}, NULL},
    {{NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE}, NULL},
    {{NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE}, NULL},
    {{NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE}, NULL},
    {{NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE}, NULL},
    {{"SW", FU_GROUP_MEM, OPERATION_STORE, DATA_TYPE_W}, NULL},
    {{NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE}, NULL},
    {{NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE}, NULL},
    {{"S.S", FU_GROUP_MEM, OPERATION_STORE, DATA_TYPE_F}, NULL},
    {{NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE}, NULL},
    {{NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE}, NULL},
    {{NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE}, NULL},
    {{"SLTUI", FU_GROUP_INT, OPERATION_SLTU, DATA_TYPE_NONE}, NULL},
    {{"SGTUI", FU_GROUP_INT, OPERATION_SGTU, DATA_TYPE_NONE}, NULL},
    {{NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE}, NULL},
    {{NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE}, NULL},
    {{NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE}, NULL},
    {{NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE}, NULL},
    {{NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE}, NULL},
    {{NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE}, NULL},
    {{NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE}, NULL},
    {{NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE}, NULL},
    {{NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE}, NULL},
    {{NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE}, NULL},
    {{NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE}, NULL},
    {{"HALT", FU_GROUP_HALT, OPERATION_NONE, DATA_TYPE_NONE}, NULL}};

const sub_op_t op_special_table[] = {
    {"NOP", FU_GROUP_NONE, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {"SLL", FU_GROUP_INT, OPERATION_SLL, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {"SRL", FU_GROUP_INT, OPERATION_SRL, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {"SLTU", FU_GROUP_INT, OPERATION_SLTU, DATA_TYPE_NONE},
    {"SGTU", FU_GROUP_INT, OPERATION_SGTU, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {"ADD", FU_GROUP_INT, OPERATION_ADD, DATA_TYPE_NONE},
    {"ADDU", FU_GROUP_INT, OPERATION_ADDU, DATA_TYPE_NONE},
    {"SUB", FU_GROUP_INT, OPERATION_SUB, DATA_TYPE_NONE},
    {"SUBU", FU_GROUP_INT, OPERATION_SUBU, DATA_TYPE_NONE},
    {"AND", FU_GROUP_INT, OPERATION_AND, DATA_TYPE_NONE},
    {"OR", FU_GROUP_INT, OPERATION_OR, DATA_TYPE_NONE},
    {"XOR", FU_GROUP_INT, OPERATION_XOR, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {"SLT", FU_GROUP_INT, OPERATION_SLT, DATA_TYPE_NONE},
    {"SGT", FU_GROUP_INT, OPERATION_SGT, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE}};

const sub_op_t op_fparith_table[] = {
    {"ADD.S", FU_GROUP_ADD, OPERATION_ADD, DATA_TYPE_F},
    {"SUB.S", FU_GROUP_ADD, OPERATION_SUB, DATA_TYPE_F},
    {"MULT.S", FU_GROUP_MULT, OPERATION_MULT, DATA_TYPE_F},
    {"DIV.S", FU_GROUP_DIV, OPERATION_DIV, DATA_TYPE_F},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE},
    {NULL, FU_GROUP_INVALID, OPERATION_NONE, DATA_TYPE_NONE}};

state_t * state_create(int *data_count, FILE *bin_file, FILE *rs_file)
{
    state_t *state;
    int i;
    char field[MAX_FIELD_LENGTH];
    int fu_index = 0;

    state = (state_t *)malloc(sizeof(state_t));
    if (state == NULL)
    {
        fprintf(stderr, "error: unable to allocate resources\n");
        return NULL;
    }

    memset(state, 0, sizeof(state_t));
    
    for (i = 0; i < NUMREGS; i++)
    {
        state->reg_map_int[i] = NULL;
        state->reg_map_fp[i] = NULL;
    }

    /* read machine-code file into instruction/data memory (starting at address 0) */
    i = 0;

    while (!feof(bin_file))
    {
        if (fread(&state->mem[i], 1, 1, bin_file) != 0)
        {
            i++;
        }
        else if (!feof(bin_file))
        {
            fprintf(stderr, "error: cannot read address 0x%X from binary file\n", i);
            return NULL;
        }
    }

    if (data_count != NULL)
    {
        *data_count = i;
    }

    /* initialize reservation station linked lists from machine file */
    while (fgets(field, MAX_FIELD_LENGTH, rs_file))
    {
        int num_rs;
        int total_cycles;
        char *pt;
        char *name;

        pt = strtok(field, ",");
        if (pt != NULL)
        {
            /* name field first */
            name = pt;
        }
        else
        {
            fprintf(stderr, "error: invalid name for fu group\n");
            return NULL;
        }

        pt = strtok(NULL, ",");
        if (pt != NULL)
        {
            /* num_rs next */
            num_rs = atoi(pt);
        }
        else
        {
            fprintf(stderr, "error: invalid num_rs for fu group\n");
            return NULL;
        }


        pt = strtok(NULL, ",");
        if (pt != NULL)
        {
            /* num_cycles next */
            total_cycles = atoi(pt);
        }
        else
        {
            fprintf(stderr, "error: invalid num_cycles for fu group\n");
            return NULL;
        }


        if (strcmp(name, rs_group_int_name) == 0)
        {
            if (fu_init(&state->int_rs_list, num_rs, total_cycles, &fu_index) != 0)
            {
                return NULL;
            }
        }
        else if (strcmp(name, rs_group_add_name) == 0)
        {
            if (fu_init(&state->fp_add_rs_list, num_rs, total_cycles, &fu_index) != 0)
            {
                return NULL;
            }
        }
        else if (strcmp(name, rs_group_mult_name) == 0)
        {
            if (fu_init(&state->fp_mult_rs_list, num_rs, total_cycles, &fu_index) != 0)
            {
                return NULL;
            }
        }
        else if (strcmp(name, rs_group_div_name) == 0) 
        {
            if (fu_init(&state->fp_div_rs_list, num_rs, total_cycles, &fu_index) != 0)
            {
                return NULL;
            }
        }
        else if (strcmp(name, fu_group_mem_name) == 0) 
        {
            if (fu_init(&state->mem_fu_list, num_rs, total_cycles, &fu_index) != 0)
            {
                return NULL;
            }
        }
    }

    if (state->int_rs_list == NULL)
    {
        fprintf(stderr, "error: no %s functional units\n", rs_group_int_name);
        return NULL;
    }

    if (state->fp_add_rs_list == NULL)
    {
        fprintf(stderr, "error: no %s functional units\n", rs_group_add_name);
        return NULL;
    }

    if (state->fp_div_rs_list == NULL)
    {
        fprintf(stderr, "error: no %s functional units\n", rs_group_mult_name);
        return NULL;
    }

    if (state->fp_mult_rs_list == NULL)
    {
        fprintf(stderr, "error: no %s functional units\n", rs_group_div_name);
        return NULL;
    }

    if (state->mem_fu_list == NULL)
    {
        fprintf(stderr, "error: no %s functional units\n", fu_group_mem_name);
        return NULL;
    }

    return state;
}

int fu_init(fu_t **list, int num_fu, int num_cycles, int *fu_index)
{   
    fu_t *fu;
    fu_t *last;
    int i;

    if (list == NULL)
    {
        fprintf(stderr, "error: invalid list\n");
        return -1;
    }

    if (num_fu < 1)
    {
        fprintf(stderr, "error: no functional units specified\n");
        return -1;
    }

    if (num_cycles < 1)
    {
        fprintf(stderr, "warning: functional units specified to have 0-cycle (or less) execution time\n");
        return -1;
    }

    /* initialize list head */
    fu = (fu_t *)malloc(sizeof(fu_t));
    if (fu == NULL)
    {
        fprintf(stderr, "error: unable to allocate resources\n");
        return -1;
    }
    memset(fu, 0, sizeof(fu_t));
    fu->id = *fu_index;
    fu->max_cycles = num_cycles;
    (*fu_index)++;

    *list = fu;
    last = fu;

    /* intialize fu */
    for (i = 0; i < num_fu - 1; i++)
    {
        fu = (fu_t *)malloc(sizeof(fu_t));
        if (fu == NULL)
        {
            fprintf(stderr, "error: unable to allocate resources\n");
            return -1;
        }
        memset(fu, 0, sizeof(fu_t));
        fu->id = *fu_index;
        fu->max_cycles = num_cycles;
        (*fu_index)++;
        last->next = fu;
        last = fu;
    }

    return 0;
}

/* functions to allocate reservation stations */
fu_t * find_available_fu(fu_t *rs_list)
{
    fu_t *fu;

    fu = rs_list;
    while (fu != NULL)
    {
        /* finding first unallocated fu */
        if (fu->busy != TRUE)
        {
            /* unallocated fu found, allocate for this instruction */
            return fu;
        }

        fu = fu->next;
    }

    /* no fu are remaining, must stall until available */
    return NULL;
}

/* functions to cycle reservation stations */
void advance_rs(fu_t * rs_list, fu_t ** wb)
{
    fu_t *rs;

    rs = rs_list;
    
    while (rs != NULL)
    {
        if (rs->busy == TRUE)
        {
            /* advance fu only if operand_1 & operand_2 are available */
            if (rs->cycles > 0 && rs->fu_1 == NULL && rs->fu_2 == NULL)
            {
                rs->cycles--;
            }

            /* check if fu is ready for broadcasting */
            if (rs->cycles == 0)
            {
                /* find single oldest non-written instruction to broadcast */
                if ((*wb) == NULL || ((*wb)->insn_num > rs->insn_num))
                {
                    (*wb) = rs;
                }
            }
        }

        rs = rs->next;
    }
}

/* functions to advance memory function units stations */
void advance_memory_buffer(fu_t * rs_list, fu_t ** cdb_int, fu_t ** cdb_fp)
{
    fu_t *fu;
    fu = rs_list;
    fu_t *oldest = NULL;
    const op_info_t *op_info;
    int use_imm;

    
    /* memory buffer MUST be in order, so we only advance the oldest insn */
    while (fu != NULL)
    {
        if (fu->busy == TRUE && (oldest == NULL || oldest->insn_num > fu->insn_num))
            oldest = fu;
        fu = fu->next;
    }

    
    if (oldest != NULL)
    {
        /* advance fu only if operand_1 & operand_2 are available */
        if (oldest->cycles > 0 && oldest->fu_1 == NULL && oldest->fu_2 == NULL)
        {
            oldest->cycles--;
        }

        /* check if fu is ready for broadcasting */
        if (oldest->cycles == 0)
        {
            /* check data type */
            op_info = decode_instr(oldest->instr, &use_imm);

            if (op_info->data_type == DATA_TYPE_F)
            {
                /* floating point instruction; set to broadcast if it is the oldest */
                if ((*cdb_fp) == NULL || ((*cdb_fp)->insn_num > oldest->insn_num))
                {
                    (*cdb_fp) = oldest;
                }
            }
            else
            {
                /* integer instruction; set to broadcast if it is the oldest */
                if ((*cdb_int) == NULL || ((*cdb_int)->insn_num > oldest->insn_num))
                {
                    (*cdb_int) = oldest;
                }
            }
        }
    }
}

/* functions to check if a specific fu list is fully completed */
int fu_list_done(fu_t * list)
{
    fu_t *fu;
    fu = list;
    while (fu != NULL)
    {
        if (fu->busy)
            return FALSE;
        
        fu = fu->next;
    }
    
    return TRUE;
}

/* decode an instruction */
const op_info_t *
decode_instr(uint32_t instr, int *use_imm)
{
    const op_info_t *op_info;

    if (op_table[FIELD_OPCODE(instr)].sub_table == NULL)
    {
        op_info = &op_table[FIELD_OPCODE(instr)].info;
        *use_imm = 1;
    }
    else
    {
        op_info = &op_table[FIELD_OPCODE(instr)].sub_table[FIELD_RSNC(instr)].info;
        *use_imm = 0;
    }
    return op_info;
}

/* update any functional units that have the source functional unit listed as a dependency */
void update_fu_list(fu_t *list, fu_t *source, operand_t new_operand)
{
    fu_t *fu;

    fu = list;

    while (fu != NULL)
    {
        if (fu->fu_1 == source)
        {
            fu->fu_1 = NULL;
            fu->operand_1 = new_operand;
        }

        if (fu->fu_2 == source)
        {
            fu->fu_2 = NULL;
            fu->operand_2 = new_operand;
        }

        fu = fu->next;
    }
}

/* perform a computation (not implemented) */
operand_t perform_operation(uint32_t instr, unsigned long pc, operand_t operand1, operand_t operand2)
{
    // local variables
    operand_t result;
    int use_imm;
    memset(&result, 0, sizeof(operand_t));

    // decode instruction
    const op_info_t *op_info = decode_instr(instr, &use_imm);

    switch (op_info->operation) {
        // Integer & FP Add
        case OPERATION_ADD: 
            // handle floating point addition
            if (op_info->data_type == DATA_TYPE_F) {
                result.flt = operand1.flt + operand2.flt;
            }
            else {
                result.integer.w = operand1.integer.w + operand2.integer.w;
            }
            break;
        // unsigned add
        case OPERATION_ADDU:
            result.integer.wu = operand1.integer.wu + operand2.integer.wu;
            break;
        
        // Integer & FP Subtract
        case OPERATION_SUB:
            if (op_info->data_type == DATA_TYPE_F) {
                result.flt = operand1.flt - operand2.flt;
            }
            else {
                result.integer.w = operand1.integer.w - operand2.integer.w;
            }
            break;
        // unsigned subtract
        case OPERATION_SUBU:
            result.integer.wu = operand1.integer.wu - operand2.integer.wu;
            break;

        // Integer & FP Multiply
        case OPERATION_MULT:
            if (op_info->data_type == DATA_TYPE_F) {
                result.flt = operand1.flt * operand2.flt;
            }
            else {
                result.integer.w = operand1.integer.w * operand2.integer.w;
            }
            break;
        
        // Integer & FP Divide
        case OPERATION_DIV:
            if (op_info->data_type == DATA_TYPE_F) {
                result.flt = (operand2.flt != 0.0f) ? operand1.flt / operand2.flt : 0.0f;
            }
            else {
                result.integer.w = (operand2.integer.w != 0) ? operand1.integer.w / operand2.integer.w : 0;
            }
            break;

        // Shift left logical
        case OPERATION_SLL:
            result.integer.wu = operand1.integer.wu << operand2.integer.wu;
            break;

        // Shift right logical    
        case OPERATION_SRL:
            result.integer.wu = operand1.integer.wu >> operand2.integer.wu;
            break;
            
        // bitwise xor
        case OPERATION_XOR:
            result.integer.wu = operand1.integer.wu ^ operand2.integer.wu;
            break;
        
        // bitwise and
        case OPERATION_AND:
            result.integer.wu = operand1.integer.wu & operand2.integer.wu;
            break;
        
        // bitwise or
        case OPERATION_OR:
            result.integer.wu = operand1.integer.wu | operand2.integer.wu;
            break;
        
        // set less than unsigned
        case OPERATION_SLTU:
            result.integer.wu = (operand1.integer.wu < operand2.integer.wu) ? 1 : 0;
            break;
        
        // set greater than unsigned
        case OPERATION_SGTU:
            result.integer.wu = (operand1.integer.wu > operand2.integer.wu) ? 1 : 0;
            break;
        
        // set less than signed
        case OPERATION_SLT:
            result.integer.w = (operand1.integer.w < operand2.integer.w) ? 1 : 0;
            break;
        
        // set greater than signed
        case OPERATION_SGT:
            result.integer.w = (operand1.integer.w > operand2.integer.w) ? 1 : 0;
            break;
        
        // load
        case OPERATION_LOAD:
            result.integer.wu = (unsigned long) ((long)operand1.integer.w + (long)FIELD_IMM(instr));
            break;
        
        // store
        case OPERATION_STORE:
            result.integer.wu = (unsigned long) ((long)operand1.integer.w + (long)FIELD_IMM(instr));
            break;
        
        // jump and link
        case OPERATION_JAL:
            result.integer.wu = (unsigned long)((long)pc + (long)FIELD_OFFSET(instr));
            break;
        
        // jump
        // pc = pc + off + 4
        case OPERATION_J:
            result.integer.wu = (unsigned long)((long)pc + (long)FIELD_OFFSET(instr)) - 4;
            break;            
        
        // jump register
        case OPERATION_JR:
            result.integer.wu = operand1.integer.wu - 4;
            break;
        
        // jump and link register
        // pc = pc + off + 4
         case OPERATION_JALR:
            result.integer.wu = (unsigned long)((long)pc + (long)FIELD_OFFSET(instr)) - 4;
            break;
        
        // branch if equal to zero
        case OPERATION_BEQZ:
            result.integer.wu = (operand1.integer.w == 0) ? (unsigned long) ((long)pc + (long) FIELD_IMM(instr)) - 4 : pc;
            break;
        
        // branch if not equal to zero
        case OPERATION_BNEZ:
            result.integer.wu = (operand1.integer.w != 0) ? (unsigned long) ((long)pc + (long) FIELD_IMM(instr)) - 4 : pc;
            break;
        default:
            break;
    }
    return result;


}