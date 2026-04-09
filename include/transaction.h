#ifndef TRANSACTION_H
#define TRANSACTION_H

#define MAX_OPS 256
#define MAX_TRANSACTIONS 100

typedef enum {
    OP_DEPOSIT,
    OP_WITHDRAW,
    OP_TRANSFER,
    OP_BALANCE
} OpType;

typedef struct {
    OpType type;
    int account_id;
    int amount;
    int target_account;
} Operation;

typedef enum {
    TX_RUNNING,
    TX_COMMITTED,
    TX_ABORTED
} TxStatus;

typedef struct {
    int tx_id;
    Operation ops[MAX_OPS];
    int num_ops;
    int start_tick;

    int actual_start;
    int actual_end;
    int wait_ticks;

    TxStatus status;
} Transaction;

#endif
