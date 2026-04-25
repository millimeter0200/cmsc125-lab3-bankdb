#include <stdio.h>
#include <pthread.h>

#include "bank.h"
#include "parser.h"
#include "timer.h"
#include "transaction.h"

#define MAX_TRANSACTIONS 100

//moved thread function to transaction.c

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
        if (pthread_create(&txs[i].thread, NULL, execute_transaction, &txs[i]) != 0)
        {
            perror("Failed to create thread");
            return 1;
        }
    }

    // wait for all threads
    for (int i = 0; i < n; i++)
    {
        pthread_join(txs[i].thread, NULL);
    }

    printf("\nFinal account state:\n");
    print_accounts();

    return 0;
}