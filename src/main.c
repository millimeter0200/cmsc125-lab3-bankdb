#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#include "bank.h"
#include "parser.h"
#include "timer.h"
#include "transaction.h"

#define MAX_TRANSACTIONS 100

int verbose_flag = 0;
int main(int argc, char *argv[])
{
    const char *accounts_file = NULL;
    const char *trace_file = NULL;
    int tick_ms = 100; // default

    for (int i = 1; i < argc; i++) // parse CLI arguments
    {
        if (strcmp(argv[i], "--accounts") == 0 && i + 1 < argc)
        {
            accounts_file = argv[++i];
        }
        else if (strcmp(argv[i], "--trace") == 0 && i + 1 < argc)
        {
            trace_file = argv[++i];
        }
        else if (strcmp(argv[i], "--tick-ms") == 0 && i + 1 < argc)
        {
            tick_ms = atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "--verbose") == 0)
        {
            verbose_flag = 1;
        }
        else
        {
            printf("Unknown argument: %s\n", argv[i]);
            return 1;
        }
    }

    if (!accounts_file || !trace_file) // validate required args
    {
        printf("Usage: %s --accounts file --trace file [--tick-ms N] [--verbose]\n", argv[0]);
        return 1;
    }

    // load accounts
    if (load_accounts(accounts_file) < 0)
        return 1;

    // load transactions
    Transaction txs[MAX_TRANSACTIONS];
    int n = load_transactions(trace_file, txs, MAX_TRANSACTIONS);

    if (n < 0)
        return 1;

    if (n == 0)
    {
        printf("Loaded 0 transactions\n");
        return 0;
    }

    printf("Loaded %d transactions\n", n);

    // start timer thread
    start_timer(tick_ms);

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
        printf("TX %d -> %s (start=%d, end=%d, wait=%d)\n",
               txs[i].tx_id,
               txs[i].status == TX_COMMITTED ? "COMMITTED" : "ABORTED",
               txs[i].actual_start,
               txs[i].actual_end,
               txs[i].wait_ticks);
    }

    // stop timer thread cleanly
    stop_timer();

    printf("\nFinal account state:\n");
    print_accounts();

    return 0;
}