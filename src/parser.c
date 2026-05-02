#include <stdio.h>
#include <string.h>
#include "parser.h"

int load_transactions(const char *filename, Transaction txs[], int max)
{
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        perror("Error opening trace file");
        return -1;
    }

    char line[256];
    int count = 0;

    while (fgets(line, sizeof(line), file) && count < max)
    {
        // skip comments and empty lines
        while (line[0] == '#' || line[0] == '\n')
        {
            if (!fgets(line, sizeof(line), file))
                break;
        }

        Transaction tx;
        tx.num_ops = 0;

        tx.status = TX_RUNNING;
        tx.actual_start = 0;
        tx.actual_end = 0;
        tx.wait_ticks = 0;

        char op[32];

        // read first operation
        if (sscanf(line, "T%d %d %s",
                   &tx.tx_id,
                   &tx.start_tick,
                   op) != 3)
            continue;

        while (1)
        {
            if (tx.num_ops >= MAX_OPS)
                break;

            Operation *o = &tx.ops[tx.num_ops];

            if (strcmp(op, "DEPOSIT") == 0)
            {
                o->type = OP_DEPOSIT;
                sscanf(line, "T%d %d %*s %d %d",
                       &tx.tx_id, &tx.start_tick,
                       &o->account_id,
                       &o->amount_centavos);
            }
            else if (strcmp(op, "WITHDRAW") == 0)
            {
                o->type = OP_WITHDRAW;
                sscanf(line, "T%d %d %*s %d %d",
                       &tx.tx_id, &tx.start_tick,
                       &o->account_id,
                       &o->amount_centavos);
            }
            else if (strcmp(op, "TRANSFER") == 0)
            {
                o->type = OP_TRANSFER;
                sscanf(line, "T%d %d %*s %d %d %d",
                       &tx.tx_id, &tx.start_tick,
                       &o->account_id,
                       &o->target_account,
                       &o->amount_centavos);
            }
            else if (strcmp(op, "BALANCE") == 0)
            {
                o->type = OP_BALANCE;
                sscanf(line, "T%d %d %*s %d",
                       &tx.tx_id, &tx.start_tick,
                       &o->account_id);
            }

            tx.num_ops++;

            // save position before reading next line
            long pos = ftell(file);

            if (!fgets(line, sizeof(line), file))
                break;

            // skip comments again
            while (line[0] == '#' || line[0] == '\n')
            {
                if (!fgets(line, sizeof(line), file))
                    break;
            }

            int next_id;
            if (sscanf(line, "T%d", &next_id) != 1)
                break;

            // if new transaction, go back
            if (next_id != tx.tx_id)
            {
                fseek(file, pos, SEEK_SET);
                break;
            }

            // read next operation type
            sscanf(line, "T%d %d %s", &tx.tx_id, &tx.start_tick, op);
        }

        txs[count++] = tx;
    }

    fclose(file);
    return count;
}
