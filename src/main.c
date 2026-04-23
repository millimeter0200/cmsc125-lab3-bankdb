#include <stdio.h>
#include <pthread.h>

#include "bank.h"
#include "parser.h"
#include "timer.h"
#include "transaction.h"

#define MAX_TRANSACTIONS 100

// thread function
void *execute_transaction(void *arg)
{
    Transaction *tx = (Transaction *)arg;

    // wait until scheduled start
    wait_until_tick(tx->start_tick);

    tx->actual_start = get_global_tick();
    tx->status = TX_RUNNING;

    // print execution start
    printf("Executing TX %d at tick %d\n", tx->tx_id, tx->actual_start);

    for (int i = 0; i < tx->num_ops; i++)
    {
        Operation *op = &tx->ops[i];

        printf("TX %d - Op %d: type=%d, acc=%d, amt=%d, target=%d\n",
               tx->tx_id,
               i,
               op->type,
               op->account_id,
               op->amount_centavos,
               op->target_account);
    }

    tx->actual_end = get_global_tick();
    tx->status = TX_COMMITTED;

    return NULL;
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("Usage: %s accounts.txt trace.txt\n", argv[0]);
        return 1;
    }

    // load accounts
    if (load_accounts(argv[1]) < 0)
        return 1;

    // load transactions
    Transaction txs[MAX_TRANSACTIONS];
    int n = load_transactions(argv[2], txs, MAX_TRANSACTIONS);

    if (n < 0)
        return 1;

    printf("Loaded %d transactions\n", n);

    // start timer thread
    start_timer();

    // create thread per transaction
    for (int i = 0; i < n; i++)
    {
        pthread_create(&txs[i].thread, NULL, execute_transaction, &txs[i]);
    }

    // wait for all threads
    for (int i = 0; i < n; i++)
    {
        pthread_join(txs[i].thread, NULL);
    }

    print_accounts();

    return 0;
}