# CMSC 125 Lab 3 – BankDB

## Design Notes (Week 1)

---

## 1. Problem Analysis

The goal of this project is to implement a **concurrent banking system** that processes multiple transactions on shared account data.

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

We consider two synchronization mechanisms:

- **pthread_mutex_t (mutex)**
  - Simpler locking mechanism
  - Only one thread can access a resource at a time

- **pthread_rwlock_t (reader-writer lock)**
  - Allows multiple concurrent readers
  - Ensures exclusive access for writers

Since banking workloads may involve frequent balance checks (read-heavy operations), using **reader-writer locks** can improve performance by allowing multiple threads to read simultaneously while still maintaining correctness during write operations.

### Planned Benchmark Methodology

To evaluate the performance difference between pthread_mutex_t and pthread_rwlock_t, we will:

- Run identical workloads using both locking strategies  
- Use multiple trace files simulating:
  - read-heavy workloads  
  - write-heavy workloads  
  - mixed workloads  
- Measure:
  - total execution time  
  - average transaction wait time (wait_ticks)  
- Perform multiple runs and compute averages  

This will help determine whether reader-writer locks provide measurable performance benefits over mutexes under different contention scenarios.

---

### 2.6 Timer Thread

- Maintains `global_tick`
- Controls transaction start times
- Enables concurrent execution

---

## 3. Estimated Implementation Timeline

### Week 1: Core Setup & Basic Execution
- Set up project structure and Makefile  
- Implement data structures (Account, Transaction, Operation)  
- Implement parsing for `accounts.txt` and `trace.txt`  
- Begin basic single-threaded execution  
- Verify correctness of operations  

### Week 2: Concurrency Implementation
- Implement multi-threading using `pthread`  
- Add synchronization (mutex and rwlocks)  
- Implement timer thread  
- Ensure correct concurrent execution  

### Week 3: Advanced Features & Optimization
- Implement deadlock prevention (lock ordering)  
- Add buffer pool with semaphores  
- Perform testing and debugging  
- Measure performance and optimize  

### Week 4: Finalization and Evaluation
- Perform comprehensive testing of all features  
- Debug and fix remaining issues  
- Evaluate system performance under different workloads  
- Refine code structure and documentation  
- Prepare final submission and presentation  

---

## 4. Summary

The system is designed to safely handle concurrent transactions using:
- Lock ordering for deadlock prevention  
- Reader-writer locks to optimize performance under read-heavy workloads  
- A buffer pool for controlled resource usage  
- A timer thread for realistic concurrency  