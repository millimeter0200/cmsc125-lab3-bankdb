#include <stdio.h>
#include <pthread.h>

#include "bank.h"
#include "parser.h"
#include "timer.h"
#include "transaction.h"

#define MAX_TRANSACTIONS 100

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

    if (n == 0)
    {
        printf("Loaded 0 transactions\n");
        return 0;
    }

    printf("Loaded %d transactions\n", n);

    // start timer thread
    start_timer();

    // create thread per transaction
    for (int i = 0; i < n; i++)
    {
        if (pthread_create(&txs[i].thread, NULL, execute_transaction, &txs[i]) != 0)
        {
            perror("Failed to create thread");
            stop_timer(); // cleanup if thread creation fails
            return 1;
        }
    }

    // wait for all transaction threads
    for (int i = 0; i < n; i++)
    {
        pthread_join(txs[i].thread, NULL);
    }

    printf("\nTransaction summary:\n");
    for (int i = 0; i < n; i++)
    {
        printf("TX %d -> %s\n",
               txs[i].tx_id,
               txs[i].status == TX_COMMITTED ? "COMMITTED" : "ABORTED");
    }

    // stop timer thread cleanly
    stop_timer();

    printf("\nFinal account state:\n");
    print_accounts();

    return 0;
}