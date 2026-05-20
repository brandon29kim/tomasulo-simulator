#ifndef _rs_h
#define _rs_h

/*
 *     $Revision: 1.4 $
 *
 * fu.h
   This module was originally written by Paul Kohout and adapted for
   this simulator.
 *
 */

#include <stdio.h>
#include <stdint.h>

#define MAXMEMORY 65536 /* maximum number of data words in memory */
#define NUMREGS 32      /* number of machine registers */

#define REG_INVALID -1 /* invalid id for reservation stations used in rs mapping and ids */

#define FIELD_OPCODE(instr) ((unsigned int)(((instr) >> 26) & 0x003F))
#define FIELD_RSNC(instr) ((unsigned int)((instr)&0x07FF))
#define FIELD_R1(instr) ((unsigned int)(((instr) >> 21) & 0x001F))
#define FIELD_R2(instr) ((unsigned int)(((instr) >> 16) & 0x001F))
#define FIELD_R3(instr) ((unsigned int)(((instr) >> 11) & 0x001F))
#define FIELD_IMM(instr) ((signed short)((instr)&0xFFFF))
#define FIELD_IMMU(instr) ((unsigned short)((instr)&0xFFFF))
#define FIELD_OFFSET(instr) ((signed int)(((instr)&0x02000000) ? ((instr) | 0xFC000000) : ((instr)&0x03FFFFFF)))

#define FU_GROUP_INT 0
#define FU_GROUP_ADD 1
#define FU_GROUP_MULT 2
#define FU_GROUP_DIV 3
#define FU_GROUP_MEM 4
#define FU_GROUP_BRANCH 5
#define FU_GROUP_NONE -1
#define FU_GROUP_INVALID -2
#define FU_GROUP_HALT -3

#define OPERATION_ADD 0
#define OPERATION_ADDU 1
#define OPERATION_SUB 2
#define OPERATION_SUBU 3
#define OPERATION_MULT 4
#define OPERATION_DIV 5
#define OPERATION_SLL 6
#define OPERATION_SRL 7
#define OPERATION_AND 8
#define OPERATION_OR 9
#define OPERATION_XOR 10
#define OPERATION_LOAD 11
#define OPERATION_STORE 12
#define OPERATION_J 13
#define OPERATION_JAL 14
#define OPERATION_JR 15
#define OPERATION_JALR 16
#define OPERATION_BEQZ 17
#define OPERATION_BNEZ 18
#define OPERATION_SLT 19
#define OPERATION_SGT 20
#define OPERATION_SLTU 21
#define OPERATION_SGTU 22
#define OPERATION_NONE -1

#define DATA_TYPE_W 1
#define DATA_TYPE_F 2
#define DATA_TYPE_NONE -1

typedef struct _op_info_t
{
    const char *name;
    const int fu_group_num;
    const int operation;
    const int data_type;
} op_info_t;

typedef struct _op_t
{
    const struct _op_info_t info;
    const struct _sub_op_t *const sub_table;
} op_t;

typedef struct _sub_op_t
{
    const struct _op_info_t info;
} sub_op_t;

extern const op_t op_table[];
extern const sub_op_t op_special_table[];
extern const sub_op_t op_fparith_table[];

extern const char rs_group_int_name[];
extern const char rs_group_add_name[];
extern const char rs_group_mult_name[];
extern const char rs_group_div_name[];
extern const char fu_group_mem_name[];

/* union to handle multiple fixed-point types */
typedef union _int_t
{
    signed long w;
    unsigned long wu;
} int_t;

typedef union _operand_t
{
    int_t integer;
    float flt;
} operand_t;

typedef struct _fu_t
{
    int32_t id;     // unique integer id for rs
    uint32_t instr; // instruction storage
    struct _fu_t *fu_1;    // location of fu_1 rs (set to NULL if operand_1 is ready)
    struct _fu_t *fu_2;    // location of fu_2 rs (set to NULL is operand_2 is ready)
    operand_t operand_1; // true value of operand 1 (exclusive with fu_1)
    operand_t operand_2; // true value of operand 2 (exclusive with fu_2)
    uint8_t busy;   // flag for in use
    int32_t cycles; // remaining time until completion
    uint32_t max_cycles; // cycles required to perform operation
    unsigned long insn_num; // instruction number (tracks oldest instruction for wb)

    struct _fu_t *next; // linked list of alus
} fu_t;

int fu_init(fu_t **, int,int,int *);

fu_t *find_available_fu(fu_t *);

void advance_rs(fu_t *, fu_t **);
void advance_memory_buffer(fu_t *, fu_t **, fu_t **);

int fu_list_done(fu_t *);

const op_info_t *decode_instr(uint32_t, int *);

void update_fu_list(fu_t *, fu_t *, operand_t);

extern operand_t perform_operation(uint32_t, unsigned long, operand_t, operand_t);


#endif