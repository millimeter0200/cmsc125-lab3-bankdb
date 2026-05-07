#ifndef BANK_H
#define BANK_H
#include "buffer_pool.h"

#include <pthread.h>

#define MAX_ACCOUNTS 100

BufferPool* get_buffer_pool();

typedef struct
{
    int account_id;
    int balance_centavos;  // balance stored in centavos
    pthread_rwlock_t lock; // per-account lock
} Account;

typedef struct
{
    Account accounts[MAX_ACCOUNTS];
    int num_accounts;
    pthread_mutex_t bank_lock; // protects bank metadata
} Bank;

extern Bank bank;

int load_accounts(const char *filename);
void print_accounts();

// banking operations
void deposit(int account_id, int amount_centavos);
int withdraw(int account_id, int amount_centavos);
int transfer(int from, int to, int amount_centavos);
int get_balance(int account_id);

#endif
