#include <stdio.h>
#include <pthread.h>
#include "bank.h"

Bank bank;

int load_accounts(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening accounts file");
        return -1;
    }

    int id, balance;
    int count = 0;

    while (fscanf(file, "%d %d", &id, &balance) == 2) {
        bank.accounts[count].account_id = id;
        bank.accounts[count].balance_centavos = balance;
        
        pthread_rwlock_init(&bank.accounts[count].lock, NULL);
        
          count++;
    }

    bank.num_accounts = count;

    pthread_mutex_init(&bank.bank_lock, NULL);

    fclose(file);
    return count;
}

void print_accounts()
{
    printf("Accounts:\n");
    for (int i = 0; i < bank.num_accounts; i++) {
        printf("Account %d -> %d\n",
               bank.accounts[i].account_id,
               bank.accounts[i].balance_centavos);
    }
}
