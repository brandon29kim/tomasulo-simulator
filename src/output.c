
/*
 *
 * output.c
 *
 */

#include <stdio.h>
#include "fu.h"
#include "tomasulo.h"
#include "output.h"

void print_state(state_t *state, int num_memory)
{
    int i, index;

    printf("Memory\n");
    printf("\tAddress\t\tData");
    for (i = 0; i < num_memory; i++)
    {
        if ((i & 0x0000000F) == 0)
        {
            printf("\n\t0x%.8X\t%.2X", i, (unsigned int)state->mem[i]);
        }
        else
        {
            printf(" %.2X", (unsigned int)state->mem[i]);
        }
    }
    printf("\n");

    printf("Registers (integer):\n");
    for (i = 0; i < NUMREGS; i += 4)
        printf("\tR%d=0x%.8lX\tR%d=0x%.8lX\tR%d=0x%.8lX\tR%d=0x%.8lX\n",
               i, state->rf_int.reg_int[i].wu,
               i + 1, state->rf_int.reg_int[i + 1].wu,
               i + 2, state->rf_int.reg_int[i + 2].wu,
               i + 3, state->rf_int.reg_int[i + 3].wu);

    printf("Registers (floating point):\n");
    for (i = 0; i < NUMREGS; i += 4)
        printf("\tF%d=%-10.6g\tF%d=%-10.6g\tF%d=%-10.6g\tF%d=%-10.6g\n",
               i, state->rf_fp.reg_fp[i],
               i + 1, state->rf_fp.reg_fp[i + 1],
               i + 2, state->rf_fp.reg_fp[i + 2],
               i + 3, state->rf_fp.reg_fp[i + 3]);

    printf("pc:\n");
    printf("\tpc\t0x%.8lX\n", state->pc);

    printf("ifid:\n");
    printf("\tinstr\t");
    print_instruction(state->if_id.instr);
    printf("\n");

    printf("%s rs:\n", rs_group_int_name);
    print_fu_list(state->int_rs_list);
    printf("%s rs:\n", rs_group_add_name);
    print_fu_list(state->fp_add_rs_list);
    printf("%s rs:\n", rs_group_mult_name);
    print_fu_list(state->fp_mult_rs_list);
    printf("%s rs:\n", rs_group_div_name);
    print_fu_list(state->fp_div_rs_list);
    printf("%s fu:\n", fu_group_mem_name);
    print_fu_list(state->mem_fu_list);

    printf("reg_map_int:\n");
    for (i = 0; i < NUMREGS; i++)
    {
        if (state->reg_map_int[i] != NULL)
        {
            printf("\t\tR%d mapped to fu%d\n", i, state->reg_map_int[i]->id);
        }
    }
    printf("reg_map_fp:\n");
    for (i = 0; i < NUMREGS; i++)
    {
        if (state->reg_map_fp[i] != NULL)
        {
            printf("\t\tF%d mapped to fu%d\n", i, state->reg_map_fp[i]->id);
        }
    }
}

void print_fu_list(fu_t *fu_list)
{
    while (fu_list)
    {
        if (fu_list->busy)
        {
            printf("\t\tfu%d, cycles %d (from end), ", fu_list->id, fu_list->cycles);
            print_instruction(fu_list->instr);

            if (fu_list->fu_1 != NULL)
            {
                printf(", op1 in fu%d", fu_list->fu_1->id);
            }

            if (fu_list->fu_2 != NULL)
            {
                printf(", op2 in fu%d", fu_list->fu_2->id);
            }

            printf("\n");
        }

        fu_list = fu_list->next;
    }
}

void print_instruction(uint32_t instr)
{
    const op_info_t *op_info;

    if (op_table[FIELD_OPCODE(instr)].sub_table == NULL)
    {
        op_info = &op_table[FIELD_OPCODE(instr)].info;
        if (op_info->name == NULL)
            printf("0x%.8X", instr);
        else
        {
            switch (op_info->fu_group_num)
            {
            case FU_GROUP_INT:
                printf("%s R%d R%d #%d", op_info->name, FIELD_R2(instr), FIELD_R1(instr), FIELD_IMM(instr));
                break;
            case FU_GROUP_MEM:
                switch (op_info->data_type)
                {
                case DATA_TYPE_W:
                    printf("%s R%d (%d)R%d", op_info->name, FIELD_R2(instr), FIELD_IMM(instr), FIELD_R1(instr));
                    break;
                case DATA_TYPE_F:
                    printf("%s F%d (%d)R%d", op_info->name, FIELD_R2(instr), FIELD_IMM(instr), FIELD_R1(instr));
                    break;
                }
                break;
            case FU_GROUP_BRANCH:
                switch (op_info->operation)
                {

                case OPERATION_JAL:
                case OPERATION_J:
                    printf("%s #%d", op_info->name, FIELD_OFFSET(instr));
                    break;

                case OPERATION_JALR:
                case OPERATION_JR:
                    printf("%s R%d", op_info->name, FIELD_R1(instr));
                    break;

                case OPERATION_BEQZ:
                case OPERATION_BNEZ:
                    printf("%s R%d #%d", op_info->name, FIELD_R1(instr), FIELD_IMM(instr));
                    break;
                }
                break;
            default:
                printf("%s", op_info->name);
            }
        }
    }
    else
    {
        op_info = &op_table[FIELD_OPCODE(instr)].sub_table[FIELD_RSNC(instr)].info;
        if (op_info->name == NULL)
            printf("0x%.8X", instr);
        else
        {
            switch (op_info->fu_group_num)
            {
            case FU_GROUP_INT:
                printf("%s R%d R%d R%d", op_info->name, FIELD_R3(instr), FIELD_R1(instr), FIELD_R2(instr));
                break;
            case FU_GROUP_ADD:
            case FU_GROUP_MULT:
            case FU_GROUP_DIV:
                printf("%s F%d F%d F%d", op_info->name, FIELD_R3(instr), FIELD_R1(instr), FIELD_R2(instr));
                break;
            default:
                printf("%s", op_info->name);
            }
        }
    }
}
