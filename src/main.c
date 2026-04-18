#include <stdio.h>
#include "bank.h"
#include "parser.h"
#include "timer.h"

#define MAX_TRANSACTIONS 100

// TODO (Week 2):
// - Implement multithreaded transaction execution (pthread)
// - Create one thread per transaction
// - Apply lock ordering for safe transfers (deadlock prevention)
// - Implement buffer pool using semaphores
// - Integrate timer thread for transaction scheduling

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
    print_accounts();

    return 0;
}
