#ifndef BANK_H
#define BANK_H

#include <pthread.h>

#define MAX_ACCOUNTS 100

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
void deposit(int account_id, int amount);
int withdraw(int account_id, int amount);
int transfer(int from, int to, int amount);
int get_balance(int account_id);

#endif
