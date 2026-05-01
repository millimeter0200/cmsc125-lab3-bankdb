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
        // skip comments or empty lines
        if (line[0] == '#' || line[0] == '\n')
            continue;

        Transaction tx;
        char op[20];

        int parsed = sscanf(line, "T%d %d %s",
                            &tx.tx_id,
                            &tx.start_tick,
                            op);

        if (parsed != 3)
            continue;

        tx.num_ops = 1;
        tx.status = TX_RUNNING;

        Operation *o = &tx.ops[0];

        if (strcmp(op, "DEPOSIT") == 0)
        {
            o->type = OP_DEPOSIT;
            sscanf(line, "T%d %d %*s %d %d",
                   &tx.tx_id,
                   &tx.start_tick,
                   &o->account_id,
                   &o->amount_centavos);
        }
        else if (strcmp(op, "WITHDRAW") == 0)
        {
            o->type = OP_WITHDRAW;
            sscanf(line, "T%d %d %*s %d %d",
                   &tx.tx_id,
                   &tx.start_tick,
                   &o->account_id,
                   &o->amount_centavos);
        }
        else if (strcmp(op, "TRANSFER") == 0)
        {
            o->type = OP_TRANSFER;
            sscanf(line, "T%d %d %*s %d %d %d",
                   &tx.tx_id,
                   &tx.start_tick,
                   &o->account_id,
                   &o->target_account,
                   &o->amount_centavos);
        }
        else if (strcmp(op, "BALANCE") == 0)
        {
            o->type = OP_BALANCE;
            sscanf(line, "T%d %d %*s %d",
                   &tx.tx_id,
                   &tx.start_tick,
                   &o->account_id);
        }

        txs[count++] = tx;
    }

    fclose(file);
    return count;
}