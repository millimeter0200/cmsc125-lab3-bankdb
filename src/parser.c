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

    char op[20];
    int count = 0;

    while (count < max)
    {
        Transaction tx;

        if (fscanf(file, "T%d %d %s",
                   &tx.tx_id,
                   &tx.start_tick,
                   op) != 3)
            break;

        tx.num_ops = 1;

        Operation *o = &tx.ops[0];

        if (strcmp(op, "DEPOSIT") == 0)
        {
            o->type = OP_DEPOSIT;
            fscanf(file, "%d %d", &o->account_id, &o->amount_centavos);
        }
        else if (strcmp(op, "WITHDRAW") == 0)
        {
            o->type = OP_WITHDRAW;
            fscanf(file, "%d %d", &o->account_id, &o->amount_centavos);
        }
        else if (strcmp(op, "TRANSFER") == 0)
        {
            o->type = OP_TRANSFER;
            fscanf(file, "%d %d %d",
                   &o->account_id,
                   &o->target_account,
                   &o->amount_centavos);
        }
        else if (strcmp(op, "BALANCE") == 0)
        {
            o->type = OP_BALANCE;
            fscanf(file, "%d", &o->account_id);
        }

        txs[count++] = tx;
    }

    fclose(file);
    return count;
}
