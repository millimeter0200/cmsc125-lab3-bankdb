#include <stdio.h>
#include <pthread.h>
#include "bank.h"
#include "buffer_pool.h"

// global buffer pool instance
static BufferPool buffer_pool;

Bank bank;

// load accounts from file, returns number of accounts loaded or -1 on error
int load_accounts(const char *filename)
{
    init_buffer_pool(&buffer_pool, 5);
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        perror("Error opening accounts file");
        return -1;
    }

    int id, balance; // temporary variables for reading account data
    int count = 0;   // track number of accounts loaded

    bank.num_accounts = 0;

    // read accounts from file, expecting lines in format: <account_id> <balance_centavos>
    while (fscanf(file, "%d %d", &id, &balance) == 2 && count < MAX_ACCOUNTS)
    {
        bank.accounts[count].account_id = id;
        bank.accounts[count].balance_centavos = balance;

        if (pthread_rwlock_init(&bank.accounts[count].lock, NULL) != 0)
        {
            perror("pthread_rwlock_init failed");
            fclose(file);
            return -1;
        }

        count++;
    }

    // set the total number of accounts loaded into the bank structure
    bank.num_accounts = count;

    fclose(file);
    return count;
}

// helper function to print current state of all accounts
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

// helper function to find account by ID, returns pointer or NULL if not found
static Account *find_account(int id)
{
    for (int i = 0; i < bank.num_accounts; i++)
    {
        if (bank.accounts[i].account_id == id)
            return &bank.accounts[i];
    }
    return NULL;
}

// banking operations
void deposit(int account_id, int amount_centavos)
{
    load_account(&buffer_pool);

    Account *acc = find_account(account_id);
    if (!acc)
    {
        unload_account(&buffer_pool);
        return;
    }

    pthread_rwlock_wrlock(&acc->lock);
    acc->balance_centavos += amount_centavos;
    pthread_rwlock_unlock(&acc->lock);

    unload_account(&buffer_pool);
}

// returns 0 on success, -1 on failure (e.g. insufficient funds or account not found)
int withdraw(int account_id, int amount_centavos)
{
    load_account(&buffer_pool);

    Account *acc = find_account(account_id);
    if (!acc)
    {
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

// returns 0 on success, -1 on failure (e.g. insufficient funds or account not found)
int transfer(int from, int to, int amount_centavos)
{
    load_account(&buffer_pool);

    Account *a = find_account(from);
    Account *b = find_account(to);

    if (!a || !b)
    {
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
        return -1;
    }

    a->balance_centavos -= amount_centavos;
    b->balance_centavos += amount_centavos;

    pthread_rwlock_unlock(&second->lock);
    pthread_rwlock_unlock(&first->lock);

    unload_account(&buffer_pool);
    return 0;
}

// returns balance in centavos, or -1 if account not found
int get_balance(int account_id)
{
    load_account(&buffer_pool);

    Account *acc = find_account(account_id);
    if (!acc)
    {
        unload_account(&buffer_pool);
        return -1;
    }

    pthread_rwlock_rdlock(&acc->lock);
    int bal = acc->balance_centavos;
    pthread_rwlock_unlock(&acc->lock);

    unload_account(&buffer_pool);
    return bal;
}

// clean up resources (destroy locks)
void destroy_bank()
{
    for (int i = 0; i < bank.num_accounts; i++)
    {
        pthread_rwlock_destroy(&bank.accounts[i].lock);
    }
}

// provide access to global buffer pool instance
BufferPool *get_buffer_pool()
{
    return &buffer_pool;
}