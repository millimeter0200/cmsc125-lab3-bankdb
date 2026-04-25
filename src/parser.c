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

        char op[20];

        // read first operation line
        sscanf(line, "T%d %d %s",
               &tx.tx_id,
               &tx.start_tick,
               op);

        while (1)
        {
            Operation *o = &tx.ops[tx.num_ops];
            
            if (tx.num_ops >= MAX_OPS) {
                break;
            }

            if (strcmp(op, "DEPOSIT") == 0) {
                o->type = OP_DEPOSIT;
                sscanf(line, "T%d %d %*s %d %d",
                       &tx.tx_id, &tx.start_tick,
                       &o->account_id,
                       &o->amount_centavos);
            }
            else if (strcmp(op, "WITHDRAW") == 0) {
                o->type = OP_WITHDRAW;
                sscanf(line, "T%d %d %*s %d %d",
                       &tx.tx_id, &tx.start_tick,
                       &o->account_id,
                       &o->amount_centavos);
            }
            else if (strcmp(op, "TRANSFER") == 0) {
                o->type = OP_TRANSFER;
                sscanf(line, "T%d %d %*s %d %d %d",
                       &tx.tx_id, &tx.start_tick,
                       &o->account_id,
                       &o->target_account,
                       &o->amount_centavos);
            }
            else if (strcmp(op, "BALANCE") == 0) {
                o->type = OP_BALANCE;
                sscanf(line, "T%d %d %*s %d",
                       &tx.tx_id, &tx.start_tick,
                       &o->account_id);
            }
            tx.num_ops++;

            // peek next line
            long pos = ftell(file);
            if (!fgets(line, sizeof(line), file))
                break;

            if (line[0] == '#' || line[0] == '\n')
            {
                if(!fgets(line, sizeof(line), file))
                    break;
            }

            int next_id;
            sscanf(line, "T%d", &next_id);

            if (next_id != tx.tx_id) {
                fseek(file, pos, SEEK_SET);
                break;
            }

            sscanf(line, "T%d %d %s",
                   &tx.tx_id,
                   &tx.start_tick,
                   op);
        }
        txs[count++] = tx;
    }
    fclose(file);
    return count;
}
