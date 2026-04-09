#ifndef PARSER_H
#define PARSER_H

#include "transaction.h"

int load_transactions(const char *filename, Transaction txs[], int max);

#endif
