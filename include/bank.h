#ifndef BANK_H
#define BANK_H

#define MAX_ACCOUNTS 100

typedef struct {
    int account_id;
    int balance;
} Account;

typedef struct {
    Account accounts[MAX_ACCOUNTS];
    int num_accounts;
} Bank;

extern Bank bank;

// functions
int load_accounts(const char *filename);
void print_accounts();

#endif
