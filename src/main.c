#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#include "bank.h"
#include "parser.h"
#include "timer.h"
#include "transaction.h"
#include "buffer_pool.h"

#define MAX_TRANSACTIONS 100

extern Bank bank;
extern BufferPool buffer_pool;

int verbose_flag = 0;
int initial_total_balance = 0;

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

    if (!accounts_file || !trace_file)
    {
        printf("Usage: %s --accounts file --trace file [--tick-ms N] [--verbose]\n",
               argv[0]);
        return 1;
    }

    // load accounts
    if (load_accounts(accounts_file) < 0)
        return 1;

    // compute initial total balance
    for (int i = 0; i < bank.num_accounts; i++)
    {
        initial_total_balance += bank.accounts[i].balance_centavos;
    }

    // load transactions
    Transaction txs[MAX_TRANSACTIONS];

    int n = load_transactions(trace_file,
                              txs,
                              MAX_TRANSACTIONS);

    if (n < 0)
        return 1;

    if (n == 0)
    {
        printf("Loaded 0 transactions\n");
        return 0;
    }

    printf("Loaded %d transactions\n", n);

    if (verbose_flag)
    {
        printf("\nExecution Log:\n");
    }

    // compute expected balance changes
    int total_deposits = 0;
    int total_withdrawals = 0;

    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < txs[i].num_ops; j++)
        {
            Operation *op = &txs[i].ops[j];

            if (op->type == OP_DEPOSIT)
            {
                total_deposits += op->amount_centavos;
            }
            else if (op->type == OP_WITHDRAW)
            {
                total_withdrawals += op->amount_centavos;
            }
        }
    }

    int expected_final_balance =
        initial_total_balance +
        total_deposits -
        total_withdrawals;

    // start timer thread
    start_timer(tick_ms);

    // create transaction threads
    for (int i = 0; i < n; i++)
    {
        if (pthread_create(&txs[i].thread,
                           NULL,
                           execute_transaction,
                           &txs[i]) != 0)
        {
            perror("Failed to create thread");
            stop_timer();
            return 1;
        }
    }

    // wait for all transaction threads
    for (int i = 0; i < n; i++)
    {
        pthread_join(txs[i].thread, NULL);
    }

    // stop timer
    stop_timer();

    printf("\nTransaction Summary:\n");

    for (int i = 0; i < n; i++)
    {
        printf("TX %d -> %s (start=%d, end=%d, wait=%d)\n",
               txs[i].tx_id,
               txs[i].status == TX_COMMITTED
                   ? "COMMITTED"
                   : "ABORTED",
               txs[i].actual_start,
               txs[i].actual_end,
               txs[i].wait_ticks);
    }

    printf("\nFinal Account State:\n");
    print_accounts();

    // compute final balance
    int final_total_balance = 0;

    for (int i = 0; i < bank.num_accounts; i++)
    {
        final_total_balance += bank.accounts[i].balance_centavos;
    }

    // consistency validation
    printf("\nBalance Consistency Check:\n");

    printf("Initial Total:  %d\n", initial_total_balance);
    printf("Deposits:       +%d\n", total_deposits);
    printf("Withdrawals:    -%d\n", total_withdrawals);
    printf("Expected Total: %d\n", expected_final_balance);
    printf("Final Total:    %d\n", final_total_balance);

    if (expected_final_balance == final_total_balance)
    {
        printf("CONSISTENT: Balance validation passed\n");
    }
    else
    {
        printf("ERROR: Balance mismatch detected\n");
    }

    // buffer pool statistics
    printf("\nBuffer Pool Report:\n");

    printf("Current Usage: %d\n",
           buffer_pool.current_usage);

    printf("Peak Usage:    %d\n",
           buffer_pool.peak_usage);

    printf("Total Loads:   %d\n",
           buffer_pool.total_loads);

    printf("Total Evicts:  %d\n",
           buffer_pool.total_evictions);

    // performance statistics
    int committed_count = 0;
    int aborted_count = 0;
    int total_wait_time = 0;
    int concurrent_execution = 0;

    int earliest_start = txs[0].actual_start;
    int latest_end = txs[0].actual_end;

    for (int i = 0; i < n; i++)
    {
        if (txs[i].status == TX_COMMITTED)
        {
            committed_count++;
        }
        else
        {
            aborted_count++;
        }

        total_wait_time += txs[i].wait_ticks;

        if (txs[i].wait_ticks > 0)
        {
            concurrent_execution = 1;
        }

        if (txs[i].actual_start < earliest_start)
        {
            earliest_start = txs[i].actual_start;
        }

        if (txs[i].actual_end > latest_end)
        {
            latest_end = txs[i].actual_end;
        }
    }

    int total_runtime = latest_end - earliest_start;

    double avg_wait =
        n > 0
            ? (double)total_wait_time / n
            : 0.0;

    double throughput =
        total_runtime > 0
            ? (double)committed_count / total_runtime
            : 0.0;

    printf("\nPerformance Report:\n");

    printf("Committed TXs:   %d\n", committed_count);
    printf("Aborted TXs:     %d\n", aborted_count);

    printf("Average Wait:    %.2f ticks\n", avg_wait);

    printf("Total Runtime:   %d ticks\n", total_runtime);

    printf("Concurrent Execution Observed: %s\n",
           concurrent_execution ? "YES" : "NO");

    printf("Throughput:      %.2f tx/tick\n", throughput);

    // cleanup locks
    for (int i = 0; i < bank.num_accounts; i++)
    {
        pthread_rwlock_destroy(&bank.accounts[i].lock);
    }

    return 0;
}