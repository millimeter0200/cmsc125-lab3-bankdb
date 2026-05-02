#include <stdio.h>
#include <pthread.h>
#include "bank.h"
#include "buffer_pool.h"

static BufferPool buffer_pool;

Bank bank;

int load_accounts(const char *filename)
{
    init_buffer_pool(&buffer_pool, 5);
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        perror("Error opening accounts file");
        return -1;
    }

    int id, balance;
    int count = 0;

    bank.num_accounts = 0;

    while (fscanf(file, "%d %d", &id, &balance) == 2 && count < MAX_ACCOUNTS)
    {
        bank.accounts[count].account_id = id;
        bank.accounts[count].balance_centavos = balance;

        pthread_rwlock_init(&bank.accounts[count].lock, NULL);

        count++;
    }

    bank.num_accounts = count;

    fclose(file);
    return count;
}

void print_accounts()
{
    printf("Accounts:\n");
    for (int i = 0; i < bank.num_accounts; i++)
    {
        printf("Account %d -> %d\n",
               bank.accounts[i].account_id,
               bank.accounts[i].balance_centavos);
    }
}

static Account *find_account(int id)
{
    for (int i = 0; i < bank.num_accounts; i++)
    {
        if (bank.accounts[i].account_id == id)
            return &bank.accounts[i];
    }
    return NULL;
}

void deposit(int account_id, int amount_centavos)
{
    load_account(&buffer_pool);

    Account *acc = find_account(account_id);
    if (!acc) {
        unload_account(&buffer_pool);
        return;
    }

    pthread_rwlock_wrlock(&acc->lock);
    acc->balance_centavos += amount_centavos;
    pthread_rwlock_unlock(&acc->lock);

    unload_account(&buffer_pool);
}


int withdraw(int account_id, int amount_centavos)
{
    if (amount_centavos < 0)
        return -1;

    load_account(&buffer_pool);

    Account *acc = find_account(account_id);
    if (!acc) {
        unload_account(&buffer_pool);
        return -1;
    }

    pthread_rwlock_wrlock(&acc->lock);

    if (acc->balance_centavos < amount_centavos)
    {
        pthread_rwlock_unlock(&acc->lock);
        unload_account(&buffer_pool);  
        return -1;
    }

    acc->balance_centavos -= amount_centavos;

    pthread_rwlock_unlock(&acc->lock);
    unload_account(&buffer_pool);

    return 0;
}

int transfer(int from, int to, int amount_centavos)
{
    if (amount_centavos < 0)
        return -1;

    if (from == to){
        load_account(&buffer_pool);
        unload_account(&buffer_pool);
        return 0;
    }

    load_account(&buffer_pool);
    load_account(&buffer_pool);

    Account *a = find_account(from);
    Account *b = find_account(to);

    if (!a || !b) {
        unload_account(&buffer_pool);
        unload_account(&buffer_pool);
        return -1;
    }

    Account *first = (a->account_id < b->account_id) ? a : b;
    Account *second = (a->account_id < b->account_id) ? b : a;

    pthread_rwlock_wrlock(&first->lock);
    pthread_rwlock_wrlock(&second->lock);

    if (a->balance_centavos < amount_centavos)
    {
        pthread_rwlock_unlock(&second->lock);
        pthread_rwlock_unlock(&first->lock);

        unload_account(&buffer_pool);
        unload_account(&buffer_pool);

        return -1;
    }

    a->balance_centavos -= amount_centavos;
    b->balance_centavos += amount_centavos;

    pthread_rwlock_unlock(&second->lock);
    pthread_rwlock_unlock(&first->lock);

    unload_account(&buffer_pool);
    unload_account(&buffer_pool);

    return 0;
} //if buffer size is 1, transfer requires 2 accounts simultaneously, so it must support at least 2 slots

int get_balance(int account_id)
{
    load_account(&buffer_pool);

    Account *acc = find_account(account_id);
    if (!acc){
        unload_account(&buffer_pool);
        return -1;
    }

    pthread_rwlock_rdlock(&acc->lock);
    int bal = acc->balance_centavos;
    pthread_rwlock_unlock(&acc->lock);

    unload_account(&buffer_pool);

    return bal;
}