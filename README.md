# Dining Philosophers Problem (with Waiter)
<!-- Submitted on 2025-03-28 | Grade: 3.5/3.5 -->

Author: Kamil Nicoś  
Student ID: 263753

This program is a multithreaded implementation of the classic Dining Philosophers Problem.

Each philosopher is simulated as a separate thread and must pick up two shared forks (represented as binary semaphores) to eat.

To avoid deadlock, the program uses a simple waiter mechanism based on a counting semaphore, which limits the number of philosophers who can try to eat at the same time.


## Compilation and Usage

Tested on: **Ubuntu Linux**

To compile the program, run:

```bash
make
```

To run the program with a given number of philosophers (e.g. 5):

```bash
./philosophers 5
```

To remove the compiled file:

```bash
make clean
```

## Limited Resources in the Project

This simulation involves three main types of limited (shared) resources:

### 1. Forks (Binary Semaphores)

- Each fork is modeled as a **binary semaphore** (`sem_t`), shared between two neighboring philosophers.
- **Each philosopher must hold two forks simultaneously** (left and right) to begin eating.
- **However, any given fork can only be held by one philosopher at a time**, which means access must be synchronized.
- To achieve this, the program uses `sem_wait` to acquire a fork and `sem_post` to release it.
- In the code, each philosopher **first picks up the right fork**, then the left. After eating, the philosopher **first puts down the left fork**, then the right.

### 2. Table Access (Waiter Semaphore)

- To avoid **deadlock**, the program uses a **waiter mechanism**, implemented as a **counting semaphore**.
- The waiter allows a maximum of **N - 1 philosophers** (where N is the total number of philosophers) to attempt to pick up forks simultaneously.
- This ensures that at least one philosopher can always eat, breaking the possibility of circular waiting and preventing deadlock.

### 3. Threads (Philosophers)

- Each philosopher is represented as a **separate thread**, which shares access to the program’s global data.
- While threads themselves are not limited in the same way as forks, they require **synchronization** when accessing shared resources.
- Printing philosopher states to the console is protected using a **mutex** (`printMutex`), ensuring that only one thread writes to standard output at a time, preventing output interleaving.

