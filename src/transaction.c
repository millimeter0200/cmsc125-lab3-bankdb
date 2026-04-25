#include <stdio.h>
#include "transaction.h"
#include "bank.h"
#include "timer.h"

// thread function
void *execute_transaction(void *arg)
{
    Transaction *tx = (Transaction *)arg;

    // wait until scheduled start
    wait_until_tick(tx->start_tick);

    tx->actual_start = get_global_tick();
    tx->status = TX_RUNNING;

    printf("Executing TX %d at tick %d\n", tx->tx_id, tx->actual_start);

    for (int i = 0; i < tx->num_ops; i++)
    {
        Operation *op = &tx->ops[i];
        int result = 0; //to assume success by default, only set to -1 if operation fails

        switch (op->type)
        {
        case OP_DEPOSIT:
            deposit(op->account_id, op->amount_centavos);
            break;

        case OP_WITHDRAW:
            result = withdraw(op->account_id, op->amount_centavos);
            break;

        case OP_TRANSFER:
            result = transfer(op->account_id, op->target_account, op->amount_centavos);
            break;

        case OP_BALANCE:
            printf("Balance of %d = %d\n",
                op->account_id,
                get_balance(op->account_id));
            break;
        }

        //abort if operation fails
        if (result < 0)
        {
            printf("TX %d aborted at operation %d\n", tx->tx_id, i);
            tx->status = TX_ABORTED;
            tx->actual_end = get_global_tick();
            return NULL;
        }
    }

    tx->actual_end = get_global_tick();
    tx->status = TX_COMMITTED;

    return NULL;
}
