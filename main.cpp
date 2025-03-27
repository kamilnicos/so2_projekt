using namespace std;
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <semaphore.h>
#include <unistd.h>

int philosophersCount = 0;
vector<string> currentState;
vector<string> lastCurrentState;
vector<sem_t> forks;
mutex printMutex;

sem_t waiter;

int leftFork(int i) {
    return i;
}

int rightFork(int i) {
    return (i + 1) % philosophersCount;
}

//Update state only if it changed
void updateState(int id, const string& newState) {
    lock_guard<mutex> lock(printMutex);
    if (currentState[id] != newState) {
        currentState[id] = newState;
        cout << "Philosopher " << id << " is " << newState << endl;
    }
}

//Main loop for each philosopher: think > wait > eat > repeat
void philosopherCycle(int id) {
    while (true) {
        updateState(id, "thinking");
        usleep(3000000); //in μs - microseconds

        updateState(id, "waiting");
        sem_wait(&waiter); //Waiter limits the number of philosophers sitting at the table to avoid deadlock

        sem_wait(&forks[rightFork(id)]); //Philosopher picks up right and then left fork (semaphore)
        sem_wait(&forks[leftFork(id)]); //Philosopher picks up left fork (semaphore)

        updateState(id, "eating");
        usleep(5000000);

        sem_post(&forks[leftFork(id)]); //Philosopher puts down the fork after eating
        sem_post(&forks[rightFork(id)]); //Philosopher puts down the fork after eating

        sem_post(&waiter);
    }
}

bool checkArguments(int argc, char* argv[]) {
    if (argc < 2) {
        cout << "Usage: " << argv[0] << " <philosophers>" << endl;
        return false;
    }
    philosophersCount = stoi(argv[1]);
    
    return true;
}

int main(int argc, char* argv[]) {
    if (!checkArguments(argc, argv)) {
        return 1;
    }

    currentState.resize(philosophersCount, "thinking"); //Resize vectors to match the number of philosophers
    lastCurrentState.resize(philosophersCount, ""); //Empty to ensure first state is always printed
    forks.resize(philosophersCount); //One semaphore per fork

    cout << "Dining Philosophers Problem (with waiter)" << endl;
    cout << "Kamil Nicos 263753" << endl;
    cout << "The programme's argument: " << philosophersCount << " philosophers." << endl;

    //Initialize the waiter semaphore: allows up to (N-1) philosophers to sit at the table simultaneously, which prevents deadlock
    if (sem_init(&waiter, 0, philosophersCount - 1) != 0) {
        cout << "Error: sem_init (waiter)" << endl;
        return 1;
    }

    //One binary semaphore per fork (1=available, only one philosopher can hold it)
    for (int i = 0; i < philosophersCount; i++) {
        if (sem_init(&forks[i], 0, 1) != 0) {
            cout << "Error: sem_init (fork " << i << ")" << endl;
            return 1;
        }
    }

    vector<thread> philosophers;
    for (int i = 0; i < philosophersCount; i++) {
        philosophers.emplace_back(philosopherCycle, i);
    }

    for (int i = 0; i < philosophersCount; i++) {
        philosophers[i].join();
    }

    for (int i = 0; i < philosophersCount; i++) {
        sem_destroy(&forks[i]);
    }

    sem_destroy(&waiter);

    return 0;
}