#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#include "bank.h"
#include "parser.h"
#include "timer.h"
#include "transaction.h"
#include "buffer_pool.h"
#include "config.h"

#define MAX_TRANSACTIONS 100

extern Bank bank;

// global flag for verbose logging
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
            accounts_file = argv[++i]; // next arg is accounts file
        }
        else if (strcmp(argv[i], "--trace") == 0 && i + 1 < argc)
        {
            trace_file = argv[++i]; // next arg is trace file
        }

        else if (strcmp(argv[i], "--tick-ms") == 0 && i + 1 < argc)
        {
            char *endptr;
            tick_ms = (int)strtol(argv[++i], &endptr, 10); // parse tick duration

            if (*endptr != '\0' || tick_ms <= 0)
            {
                fprintf(stderr, "Invalid value for --tick-ms\n"); // validate tick duration
                return 1;
            }
        }

        else if (strcmp(argv[i], "--verbose") == 0)
        {
            verbose_flag = 1; // enable verbose logging
        }
        else
        {
            fprintf(stderr, "Unknown argument: %s\n", argv[i]); // handle unknown arguments
            return 1;
        }
    }

    // validate required arguments
    if (!accounts_file || !trace_file)
    {
        fprintf(stderr, "Usage: %s --accounts file --trace file [--tick-ms N] [--verbose]\n",
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

    // validate transaction loading
    if (n < 0)
        return 1;

    // handle case where no transactions were loaded
    if (n == 0)
    {
        fprintf(stderr, "Loaded 0 transactions\n");
        return 0;
    }
    // print loaded transaction count
    fprintf(stderr, "Loaded %d transactions\n", n);

    // print initial account state
    if (verbose_flag)
    {
        fprintf(stderr, "\nExecution Log:\n");
    }

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

    // print transaction summary and final account state
    fprintf(stderr, "\nTransaction Summary:\n");

    // print summary of each transaction with timing info
    for (int i = 0; i < n; i++)
    {
        fprintf(stderr, "TX %d -> %s (start=%d, end=%d, wait=%d)\n",
                txs[i].tx_id,
                txs[i].status == TX_COMMITTED
                    ? "COMMITTED"
                    : "ABORTED",
                txs[i].actual_start,
                txs[i].actual_end,
                txs[i].wait_ticks);
    }

    // print final account state
    fprintf(stderr, "\nFinal Account State:\n");
    print_accounts();

    // compute expected balance changes from committed transactions only
    int total_deposits = 0;
    int total_withdrawals = 0;

    // only consider committed transactions for balance validation
    for (int i = 0; i < n; i++)
    {
        if (txs[i].status == TX_COMMITTED)
        {
            for (int j = 0; j < txs[i].num_ops; j++)
            {
                Operation *op = &txs[i].ops[j];

                if (op->type == OP_DEPOSIT)
                {
                    total_deposits += op->amount_centavos; // track total deposits from committed transactions
                }
                else if (op->type == OP_WITHDRAW)
                {
                    total_withdrawals += op->amount_centavos; // track total withdrawals from committed transactions
                }
            }
        }
    }

    // compute expected final balance based on initial balance and net changes from committed transactions
    int expected_final_balance =
        initial_total_balance +
        total_deposits -
        total_withdrawals;

    // compute final balance
    int final_total_balance = 0;

    for (int i = 0; i < bank.num_accounts; i++)
    {
        final_total_balance += bank.accounts[i].balance_centavos;
    }

    // compare the actual final balance against the expected balance
    // computed from successfully committed transactions only
    fprintf(stderr, "\nBalance Consistency Check:\n");

    printf("Initial Total:  %d\n", initial_total_balance);
    printf("Deposits:       +%d\n", total_deposits);
    printf("Withdrawals:    -%d\n", total_withdrawals);
    printf("Expected Total: %d\n", expected_final_balance);
    printf("Final Total:    %d\n", final_total_balance);

    // validate that the final total balance matches the expected total balance based
    // on the net effect of all successfully committed transactions only
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

    // print current usage, peak usage, total loads, and total evictions from the buffer pool
    printf("Current Usage: %d\n",
           get_buffer_pool()->current_usage);

    printf("Peak Usage:    %d\n",
           get_buffer_pool()->peak_usage);

    printf("Total Loads:   %d\n",
           get_buffer_pool()->total_loads);

    printf("Total Evicts:  %d\n",
           get_buffer_pool()->total_evictions);

    // performance statistics
    int committed_count = 0;
    int aborted_count = 0;
    int total_wait_time = 0;
    int concurrent_execution = 0;

    int earliest_start = txs[0].actual_start;
    int latest_end = txs[0].actual_end;

    // analyze transaction timings to compute performance metrics such as
    // average wait time, total runtime, and throughput
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

        if (txs[i].actual_start < earliest_start)
        {
            earliest_start = txs[i].actual_start;
        }

        if (txs[i].actual_end > latest_end)
        {
            latest_end = txs[i].actual_end;
        }
    }

    // detect overlapping execution windows
    for (int i = 0; i < n; i++)
    {
        for (int j = i + 1; j < n; j++)
        {
            if (txs[i].actual_start < txs[j].actual_end &&
                txs[j].actual_start < txs[i].actual_end)
            {
                concurrent_execution = 1;
            }
        }
    }

    // compute total runtime as the span from the earliest transaction start to the latest transaction end
    int total_runtime = latest_end - earliest_start;

    // compute average wait time across all transactions
    double avg_wait =
        n > 0
            ? (double)total_wait_time / n
            : 0.0;

    // compute throughput as the number of committed transactions divided by total runtime
    double throughput =
        total_runtime > 0
            ? (double)committed_count / total_runtime
            : 0.0;

    // print performance metrics including committed/aborted counts, average wait time,
    // total runtime, concurrency observation, and throughput
    printf("\nPerformance Report:\n");

    printf("Committed TXs:   %d\n", committed_count);
    printf("Aborted TXs:     %d\n", aborted_count);

    printf("Average Wait:    %.2f ticks\n", avg_wait);

    printf("Total Runtime:   %d ticks\n", total_runtime);

    printf("Concurrent Execution Observed: %s\n",
           concurrent_execution ? "YES" : "NO");

    printf("Throughput:      %.2f tx/tick\n", throughput);

    // clean up resources
    destroy_bank();
    // destroy buffer pool resources
    destroy_buffer_pool(get_buffer_pool());

    return 0;
}