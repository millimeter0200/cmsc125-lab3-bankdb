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
    int tick_ms = 100;

    for (int i = 1; i < argc; i++)
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
            char *endptr;
            tick_ms = strtol(argv[++i], &endptr, 10);


            if (*endptr != '\0')
            {
                fprintf(stderr, "Invalid tick-ms value\n");
                return 1;
            }
        }
        else if (strcmp(argv[i], "--verbose") == 0)
        {
            verbose_flag = 1;
        }
        else if (strncmp(argv[i], "--deadlock=", 11) == 0)
        {
            const char *mode = argv[i] + 11;


            if (strcmp(mode, "prevention") == 0)
            {
                // already implemented
            }
            else if (strcmp(mode, "detection") == 0)
            {
                fprintf(stderr, "Deadlock detection not implemented, using prevention\n");
            }
            else
            {
                fprintf(stderr, "Invalid deadlock mode: %s\n", mode);
                return 1;
            }
        }
        else
        {
            fprintf(stderr, "Unknown argument: %s\n", argv[i]);
            return 1;
        }
    }

    if (!accounts_file || !trace_file)
    {
        fprintf(stderr, "Usage: %s --accounts file --trace file [--tick-ms N] [--verbose]\n", argv[0]);
        return 1;
    }

    if (load_accounts(accounts_file) < 0)
        return 1;

    // balance before execution
    int initial_total = 0;
    for (int i = 0; i < bank.num_accounts; i++)
        initial_total += bank.accounts[i].balance_centavos;

    Transaction txs[MAX_TRANSACTIONS];
    int n = load_transactions(trace_file, txs, MAX_TRANSACTIONS);

    if (n <= 0)
        return 1;

    start_timer(tick_ms);

    for (int i = 0; i < n; i++)
    {
        if (pthread_create(&txs[i].thread, NULL, execute_transaction, &txs[i]) != 0)
        {
            perror("Failed to create thread");
            stop_timer();
            return 1;
        }
    }

    for (int i = 0; i < n; i++)
        pthread_join(txs[i].thread, NULL);

    stop_timer();

    printf("\n=== Transaction Performance Metrics ===\n");
    printf("ID\tStatus\tStart\tEnd\tWait\n");


    for (int i = 0; i < n; i++)
    {
        printf("%d\t%s\t%d\t%d\t%d\n",
            txs[i].tx_id,
            txs[i].status == TX_COMMITTED ? "COMMITTED" : "ABORTED",
            txs[i].actual_start,
            txs[i].actual_end,
            txs[i].wait_ticks);
    }

    printf("\nFinal account state:\n");
    print_accounts();

    // balance after execution
    int final_total = 0;
    for (int i = 0; i < bank.num_accounts; i++)
        final_total += bank.accounts[i].balance_centavos;

    printf("\n=== Balance Check ===\n");
    printf("Initial Total: %d\n", initial_total);
    printf("Final Total: %d\n", final_total);
    printf("Result: %s\n", (initial_total == final_total) ? "PASS" : "FAIL");

    print_buffer_pool_report(get_buffer_pool());
    destroy_bank();

    return 0;
}