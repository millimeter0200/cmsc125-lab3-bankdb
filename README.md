# CMSC 125 Lab 3 – BankDB

## Contributors
- Christel Hope S. Ong
- Mae Maricar R. Yap

## Final Implementation & Design Overview

---

## 1. Problem Overview

This project implements a **concurrent banking system** that processes multiple transactions on shared account data. Transactions may execute simultaneously, requiring proper synchronization to maintain correctness.

### Key Challenges Addressed
- **Race Conditions** – multiple threads accessing shared accounts  
- **Synchronization** – ensuring atomic updates to balances  
- **Deadlocks** – preventing circular waiting in transfers  
- **Resource Constraints** – limiting concurrent access using a buffer pool  
- **Scheduling** – executing transactions based on simulated time  

---

## 2. System Architecture

### 2.1 Core Components

- **Accounts Module (`bank.c`)**
  - Stores account data (ID and balance)
  - Implements deposit, withdraw, transfer, and balance operations
  - Uses reader-writer locks for safe concurrent access

- **Transaction Module (`transaction.c`)**
  - Executes transactions in separate threads
  - Handles operation flow, abort conditions, and execution timing

- **Parser Module (`parser.c`)**
  - Parses input files into structured transactions
  - Supports multi-operation transactions

- **Timer Module (`timer.c`)**
  - Maintains a global logical clock (`global_tick`)
  - Synchronizes transaction start times

- **Buffer Pool (`buffer_pool.c`)**
  - Controls limited resource access using semaphores

- **Main Controller (`main.c`)**
  - Initializes system components
  - Creates and joins transaction threads
  - Handles CLI arguments and program flow
  - Validates CLI input using `strtol()` for safer numeric parsing
  - Supports runtime flags such as `--verbose` and `--deadlock=prevention`

---

## 3. Concurrency Design

- Each transaction is executed as a **separate thread**
- Threads wait until their assigned `start_tick` using:
  - `pthread_mutex`
  - `pthread_cond_t`
- Ensures transactions follow timing constraints while running concurrently

---

## 4. Timer-Based Scheduling

A dedicated **timer thread** simulates system time:

- Increments `global_tick` at fixed intervals
- Uses condition variables to wake waiting threads
- Transactions call `wait_until_tick()` before execution

### Additional Behavior
- Introduced **scheduling jitter** to simulate realistic thread delays
- Prevents perfectly synchronized execution
- Allows observation of **wait times and contention**

---

## 5. Synchronization Mechanisms

### Reader-Writer Locks (`pthread_rwlock_t`)
- Multiple threads can read simultaneously
- Write operations are exclusive
- Used for account-level protection

### Mutex + Condition Variable
- Protects access to `global_tick`
- Enables efficient waiting (no busy waiting)

## 5.1 Synchronization Benchmark Results

The system was tested using both `pthread_mutex_t` and `pthread_rwlock_t` under different workloads.

### Observed Results

Workload Type   Mutex    RWLock 
Read-heavy      Slower   Faster 
Mixed workload  Moderate Slightly Faster 
Write-heavy     Similar  Similar 

### Conclusion

Reader-writer locks improved concurrency during balance-check-heavy workloads because multiple reader threads could access accounts simultaneously. Under write-heavy workloads, the performance difference was minimal since write operations still require exclusive access.


---

## 6. Deadlock Prevention

Deadlocks are avoided using **lock ordering**:

- Accounts are always locked in ascending order of account ID
- Prevents circular wait conditions
- Ensures safe transfer operations

Deadlock requires the four Coffman conditions:
1. Mutual exclusion
2. Hold and wait
3. No preemption
4. Circular wait

The implementation removes the circular wait condition by forcing all transactions to acquire locks in ascending account ID order. Since threads follow the same lock order consistently, cyclic lock dependencies cannot form.

---

## 7. Buffer Pool Design

A **fixed-size buffer pool** limits concurrent access:

- Implemented using semaphores
- Transactions must acquire a slot before accessing accounts
- If full, transactions block until a slot becomes available

### Purpose
- Simulates limited system resources
- Introduces realistic contention

### Buffer Pool Access Timing

`load_account()` is called before account access and `unload_account()` is called immediately after operations complete. This simulates temporary occupation of limited resources while transactions are actively using account data. Releasing the slot after the operation prevents resource leaks and allows waiting transactions to continue execution.

---

## 8. Transaction Execution Behavior

Each transaction:

1. Waits until its scheduled `start_tick`
2. Records:
   - Actual start time
   - Wait time (`actual_start - start_tick`)
3. Executes operations sequentially
4. Aborts immediately if any operation fails
5. Records completion time

### Execution Features
- Transactions run concurrently
- Operations simulate execution time using delays
- Wait times reflect scheduling and contention

---

## 9. Testing & Validation

The system was tested using concurrent transaction traces containing:

- simultaneous transfers
- overlapping withdrawals
- read-heavy balance checks
- mixed workloads
- buffer pool contention scenarios

```bash
make test
```
---

## 10. Results and Conclusion

- No race conditions observed  
- No deadlocks  
- Correct and consistent final balances across runs  
- Transactions execute concurrently with observable overlap  
- Non-zero wait times demonstrate realistic scheduling  

Overall, the system successfully demonstrates a concurrent transaction processing environment with proper synchronization, deadlock prevention, and realistic timing behavior. The combination of timer-based scheduling, thread execution, and controlled resource access ensures both correctness and efficiency.
