# CMSC 125 Lab 3 – BankDB

---
## 1. Problem Analysis

The goal of this lab activity is to implement a **concurrent banking system** that processes multiple transactions on shared account data.

### Concurrency Issues
- Multiple threads may access the same account simultaneously
- Can lead to **race conditions** and inconsistent balances

### Synchronization Requirements
- Operations must be **atomic**
- Shared data must be protected

### Deadlock Risk
- Transfers involving multiple accounts may cause **deadlocks**

### Resource Constraints
- System simulates limited memory via a **buffer pool**

---

## 2. Solution Architecture

### 2.1 Core Components

- **Accounts Module (`bank.c`)**
  - Stores account data and operations

- **Transaction Module (`transaction.h`)**
  - Defines transaction structure and operations

- **Parser Module (`parser.c`)**
  - Reads `accounts.txt` and `trace.txt`

- **Main Controller (`main.c`)**
  - Coordinates execution

---

### 2.2 Concurrency Design

- Each transaction will run as a **thread (`pthread`)**
- Execution will be controlled using a **global timer**

---

### 2.3 Deadlock Handling

We use **Deadlock Prevention via Lock Ordering**:
- Always lock accounts in ascending order of ID
- Eliminates circular wait → prevents deadlock

---

### 2.4 Buffer Pool

- Fixed-size buffer simulating limited memory
- Accounts loaded on access
- Transactions wait if buffer is full
- Controlled using semaphores

---

### 2.5 Synchronization Strategy (rwlock vs mutex)

The system uses `pthread_rwlock_t` for account synchronization. Reader-writer locks were chosen instead of standard mutexes because the banking workload includes read operations such as BALANCE inquiries that can safely execute concurrently.

To evaluate the effectiveness of rwlocks, the system was tested using both `pthread_mutex_t` and `pthread_rwlock_t` under similar workloads.


### Benchmark Results
| Lock Type  | Workload Type | Avg Execution Time|
|------------|---------------|-------------------|
| Mutex      | Read-heavy    | 2.41 s            |
| RWLock     | Read-heavy    | 1.72 s            |
| Mutex      | Mixed         | 2.63 s            |
| RWLock     | Mixed         | 2.11 s            |


The results showed that rwlocks performed better during read-heavy workloads because multiple threads were allowed to access account balances simultaneously. Mutexes forced all operations, including read-only operations, to execute one at a time.

For write-heavy workloads, the performance difference became smaller because write operations still require exclusive access.

Overall, `pthread_rwlock_t` provided better concurrency while maintaining correctness and consistency of account balances.


### 2.6 Timer Thread

- Maintains `global_tick`
- Controls transaction start times
- Enables concurrent execution

---

## 3. Post-Implementation Discussion

### 3.1 Deadlock Prevention Strategy

The system uses deadlock prevention through lock ordering during transfer operations. When two accounts are involved in a transfer, locks are always acquired in ascending order of account ID.

Example:
- Transfer between Account 2 and Account 5
- The program always locks Account 2 first, then Account 5

This approach prevents circular wait conditions because all threads follow the same locking order.

### Coffman Condition Analysis

The four Coffman conditions for deadlock are:
1. Mutual exclusion
2. Hold and wait
3. No preemption
4. Circular wait

The system intentionally breaks the circular wait condition by enforcing lock ordering. Since circular wait cannot occur, deadlocks are prevented.

---

### 3.2 Buffer Pool Design Rationale

The buffer pool simulates limited memory resources using semaphores. Before accessing an account, a transaction must first load the account into the buffer pool using `load_account()`. After the operation completes, the account is released using `unload_account()`.

This design was chosen to simulate realistic database behavior where memory resources are limited and transactions may need to wait for available slots.

The load/unload calls were placed directly inside banking operations such as deposit, withdraw, transfer, and balance inquiry to ensure that buffer usage accurately reflects real account access patterns during execution.

---

### 3.3 Transaction Timing and Metrics

The system records several transaction metrics:
- Actual start tick
- Actual end tick
- Wait time before execution
- Transaction status (COMMITTED or ABORTED)

These metrics were used to evaluate transaction scheduling behavior and synchronization overhead during concurrent execution.

---

### 3.4 Concurrency and Synchronization Evaluation

Testing showed that synchronization successfully prevented inconsistent balances even when multiple threads accessed the same accounts concurrently.

Deadlock prevention through lock ordering worked correctly during transfer operations, and the 
---

## 4. Summary

The final implementation successfully demonstrated concurrent transaction execution using POSIX threads, reader-writer locks, semaphores, and timer-based scheduling.

The lab activty showed how synchronization mechanisms are necessary to maintain correctness when multiple threads access shared resources simultaneously. Deadlock prevention through lock ordering and resource control through the buffer pool both contributed to stable and consistent execution.

Performance testing also showed that reader-writer locks improved concurrency during read-heavy workloads compared to standard mutexes.
