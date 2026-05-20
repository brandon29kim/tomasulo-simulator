
#ifndef _tomasulo_h
#define _tomasulo_h

/*
 * 
 * tomasulo.h
 * 
 */

#define TRUE 1
#define FALSE 0

#include "fu.h"

/* fetch/decode pipeline register */
typedef struct _if_id_t
{
    int instr;
    unsigned long insn_num;
} if_id_t;

/* register state */
typedef struct _rf_int_t 
{
    int_t reg_int[NUMREGS];
} rf_int_t;

typedef struct _rf_fp_t
{
    float reg_fp[NUMREGS];
} rf_fp_t;

/* overall processor state */
typedef struct _state_t
{
    /* memory */
    unsigned char mem[MAXMEMORY];

    /* register files */
    rf_int_t rf_int;
    rf_fp_t rf_fp;

    /* pipeline registers */
    unsigned long pc;
    if_id_t if_id;

    /* reservation station list */
    fu_t *int_rs_list;
    fu_t *fp_add_rs_list;
    fu_t *fp_mult_rs_list;
    fu_t *fp_div_rs_list;

    /* memory buffers (simulated using the rs structure) */
    fu_t *mem_fu_list;

    /* register to reservation station mapping */
    fu_t *reg_map_int[NUMREGS];
    fu_t *reg_map_fp[NUMREGS];

    /* common data bus */
    fu_t *cdb_int;
    fu_t *cdb_fp;

    /* control variables */
    int finished;
    int fetch_lock;
    int fu_lock;
    int halted;
    int pending_fetch_lock;
    int pending_halt_lock;

    unsigned long insn_num;
} state_t;

extern state_t *state_create(int *, FILE *, FILE *);

extern void fetch(state_t *);
extern void dispatch(state_t *);
extern void execute(state_t *);
extern void broadcast(state_t *, int *);

#endif
