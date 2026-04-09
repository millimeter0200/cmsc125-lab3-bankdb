#include <stdio.h>
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
        bank.accounts[count].balance = balance;
        count++;
    }

    bank.num_accounts = count;
    fclose(file);
    return count;
}

void print_accounts()
{
    printf("Accounts:\n");
    for (int i = 0; i < bank.num_accounts; i++) {
        printf("Account %d -> %d\n",
               bank.accounts[i].account_id,
               bank.accounts[i].balance);
    }
}
