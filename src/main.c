
/*
 *
 * main.c
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "fu.h"
#include "tomasulo.h"
#include "output.h"

const char usage[] =
    "usage: %s <options>\n"
    "\toptions:\n"
    "\t-b"
    "\t<binary file>\n"
    "\t-o"
    "\t<reservation station options file>\n";

void parse_args(int, char **);

static char *bin_file_name = NULL;
static char *rs_file_name = NULL;
static FILE *bin_file, *rs_file;
static int wbpi = -1;
static int wbpf = -1;

int main(int argc, char *argv[])
{
    state_t *state;
    int data_count;
    int num_insn, i;
    unsigned char temp;

    parse_args(argc, argv);
    state = state_create(&data_count, bin_file, rs_file);

    if (state == NULL)
    {
        fclose(bin_file);
        fclose(rs_file);
        return -1;
    }

    fclose(bin_file);
    fclose(rs_file);

    /* main sim loop */
    for (i = 0, num_insn = 0; state->finished == 0; i++)
    {

        printf("\n\n*** CYCLE %d\n", i);
        print_state(state, data_count);

        /* allow for single stepping (compile with -DDEBUG, define DEBUG, or 'make debug' to enable) */
        #ifdef DEBUG
            getchar();
            fflush(stdin);
        #endif

        broadcast(state, &num_insn);
        execute(state);
        if (!(state->fetch_lock))
        {
            dispatch(state);

            if (!(state->fu_lock))
                fetch(state);
        }
    }
    num_insn++;

    /* print final machine state */
    printf("\n\n*** FINAL MACHINE STATE\n");
    print_state(state, data_count);
    printf("\n\n");

    printf("SIMULATION COMPLETE!\n");
    printf("EXECUTED %d INSTRUCTIONS IN %d CYCLES\n", num_insn, i);
    printf("CPI:  %.2f\n", (float)i / (float)num_insn);

    return 0;
}

void parse_args(int argc, char **argv)
{
    int i;

    for (i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-b") == 0)
        {
            if (bin_file_name == NULL && i + 1 < argc)
            {
                bin_file_name = argv[++i];
            }
            else
            {
                fprintf(stderr, usage, argv[0]);
                exit(-1);
            }
        }
        else if (strcmp(argv[i], "-o") == 0)
        {
            if (rs_file_name == NULL && i + 1 < argc)
            {
                rs_file_name = argv[++i];
            }
            else
            {
                fprintf(stderr, usage, argv[0]);
                exit(-1);
            }
        }
        else
        {
            fprintf(stderr, usage, argv[0]);
            exit(-1);
        }
    }

    if (bin_file_name == NULL || rs_file_name == NULL)
    {
        fprintf(stderr, usage, argv[0]);
        exit(-1);
    }

    bin_file = fopen(bin_file_name, "r");
    if (bin_file == NULL)
    {
        fprintf(stderr, "error: cannot open binary file '%s'\n", bin_file_name);
        exit(-1);
    }

    rs_file = fopen(rs_file_name, "r");
    if (rs_file == NULL)
    {
        fclose(bin_file);
        fprintf(stderr, "error: cannot open machine options file '%s'\n", rs_file_name);
        exit(-1);
    }
}
