# BankDB Design Document

## 1. Deadlock Strategy Choice

We chose **Deadlock Prevention via Lock Ordering** as our strategy.

In this approach, locks on accounts are always acquired in a consistent order based on account ID (i.e., the account with the smaller ID is locked first). This guarantees that no circular wait can occur among transactions.

### Coffman Conditions for Deadlock
1. Mutual exclusion  
2. Hold and wait  
3. No preemption  
4. Circular wait  

Lock ordering eliminates the **circular wait condition**, thereby preventing deadlock entirely.

We chose this strategy because:
- It is simpler and less error-prone than deadlock detection  
- It avoids the overhead of maintaining a wait-for graph  
- It guarantees deadlock-free execution without needing recovery mechanisms  

---

## 2. Buffer Pool Integration

We plan to implement a fixed-size buffer pool using semaphores to simulate limited memory for account access.

### Strategy
- Accounts will be **loaded into the buffer pool upon first access**  
- Accounts will be **unloaded after the transaction completes**  

### Behavior
- If the buffer pool is full when a transaction attempts to load an account, the transaction will **block until a slot becomes available**  
- Semaphores (`empty_slots` and `full_slots`) will be used to synchronize access to the buffer  

### Justification
- Loading on first access minimizes unnecessary memory usage  
- Unloading at the end of a transaction allows other transactions to reuse buffer slots  
- This approach balances performance and simplicity without requiring complex eviction policies  

---

## 3. Reader-Writer Lock Performance

We will compare the performance of:
- `pthread_mutex_t` (exclusive locking)  
- `pthread_rwlock_t` (reader-writer locking)  

### Expected Outcome
Reader-writer locks should perform better on **read-heavy workloads**, such as multiple concurrent `BALANCE` operations.

### Reason
- `pthread_rwlock_t` allows multiple threads to read the same account simultaneously  
- `pthread_mutex_t` forces all operations (including reads) to execute one at a time  

### Plan
- Run the system using a read-heavy trace file  
- Measure total execution time in ticks  
- Compare results between mutex and rwlock implementations  

---

## 4. Timer Thread Design

A separate timer thread will be used to simulate time progression via a global tick counter.

### Purpose
- To control when each transaction starts based on its `start_tick`  
- To enable realistic concurrent execution  

### Why not sequential execution?
Without a timer thread:
- Transactions would execute immediately in order  
- There would be no overlap between transactions  
- Concurrency issues (race conditions, deadlocks) would not be observable  

### How it enables concurrency
- The timer thread increments `global_tick` at fixed intervals  
- Transactions wait until their scheduled `start_tick`  
- Multiple transactions may start at the same tick, enabling parallel execution  

---

## Summary

This design ensures safe and correct concurrent execution in a multi-threaded banking system. By using lock ordering, reader-writer locks, a buffer pool, and a timer thread, the system can handle concurrent transactions efficiently while avoiding race conditions and deadlocks.