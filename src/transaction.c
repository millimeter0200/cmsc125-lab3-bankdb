#include <stdio.h>
#include <unistd.h>
#include "transaction.h"
#include "bank.h"
#include "timer.h"

extern int verbose_flag;

void *execute_transaction(void *arg)
{
    Transaction *tx = (Transaction *)arg;

    // wait for scheduled time
    wait_until_tick(tx->start_tick);

    tx->actual_start = get_global_tick();
    tx->wait_ticks = tx->actual_start - tx->start_tick;
    tx->status = TX_RUNNING;

    if (verbose_flag)
    {
        printf("Executing TX %d at tick %d (scheduled=%d, wait=%d)\n",
               tx->tx_id,
               tx->actual_start,
               tx->start_tick,
               tx->wait_ticks);
    }

    for (int i = 0; i < tx->num_ops; i++)
    {
        Operation *op = &tx->ops[i];
        int result = 0;

        switch (op->type)
        {
        case OP_DEPOSIT:
            if (verbose_flag)
                printf("TX %d: Deposit %d to %d\n",
                       tx->tx_id, op->amount_centavos, op->account_id);
            deposit(op->account_id, op->amount_centavos);
            break;

        case OP_WITHDRAW:
            if (verbose_flag)
                printf("TX %d: Withdraw %d from %d\n",
                       tx->tx_id, op->amount_centavos, op->account_id);
            result = withdraw(op->account_id, op->amount_centavos);
            break;

        case OP_TRANSFER:
            if (verbose_flag)
                printf("TX %d: Transfer %d from %d to %d\n",
                       tx->tx_id,
                       op->amount_centavos,
                       op->account_id,
                       op->target_account);
            result = transfer(op->account_id,
                              op->target_account,
                              op->amount_centavos);
            break;

        case OP_BALANCE:
        {
            int bal = get_balance(op->account_id);

            printf("TX %d: Balance of %d = %d\n",
                   tx->tx_id, op->account_id, bal);
            break;
        }

        default:
            if (verbose_flag)
                printf("TX %d: Unknown operation\n", tx->tx_id);
            break;
        }

        // abort condition
        if (result < 0)
        {
            tx->status = TX_ABORTED;
            tx->actual_end = get_global_tick();

            if (verbose_flag)
                printf("TX %d aborted at tick %d\n",
                       tx->tx_id, tx->actual_end);

            return NULL;
        }

        // simulate operation time
        usleep(300000 + (tx->tx_id % 3) * 100000);
        // 300–400ms (less uniform)
    }

    tx->actual_end = get_global_tick();
    tx->status = TX_COMMITTED;

    if (verbose_flag)
        printf("TX %d committed at tick %d\n",
               tx->tx_id, tx->actual_end);

    return NULL;
}