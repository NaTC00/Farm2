# Farm2

## Project Overview
Farm2 implements a concurrent processing system based on the Master–Worker pattern, supported by a dedicated Collector process that receives and manages computation results.

The architecture consists of:

### MasterWorker (parent process)
- Manages a dynamic threadpool  
- Scans files and directories to generate tasks  
- Sends computation results to the Collector via a UNIX domain socket  

### Collector (child process)
- Acts as a UNIX socket server  
- Receives results from the MasterWorker  
- Stores them in a sorted list  
- Prints the list periodically  

Communication between the two processes uses a UNIX socket (`AF_UNIX`).

---

## MasterWorker Functionality
The MasterWorker is responsible for:
- Creating a socket and connecting to the Collector  
- Initializing a threadpool with a configurable number of worker threads  
- Scanning input files or recursively exploring directories  
- Inserting tasks into a thread-safe queue protected by mutexes and condition variables  

---

## Command-line Options

-n <nthreads> Number of worker threads
-q <queuesize> Maximum size of the task queue
-d <directory> Root directory for recursive file search
-t <ms> Delay in milliseconds between consecutive task submissions


---

## Signal Handling
Signals are masked at startup and managed by a dedicated signal handler thread using `sigwait()`.

| Signal                                   | Action                            |
| ---------------------------------------- | --------------------------------- |
| SIGINT, SIGTERM, SIGQUIT, SIGHUP         | Start program termination         |
| SIGUSR1                                  | Increase number of worker threads |
| SIGUSR2                                  | Decrease number of worker threads |
| SIGPIPE                                  | Ignored                           |

---

## Collector Process
The Collector acts as a UNIX domain socket server:
- Creates a socket and binds it to a filesystem path  
- Uses `select()` to monitor descriptors  
- Accepts new client connections  
- For each received message:  
  - Reads computation results  
  - If a termination message is received → stops  
  - Otherwise inserts the result into an ascending sorted list  

The Collector prints the sorted list every second during execution.

---

## Program Termination
The system terminates when:
- All tasks have been submitted and processed  
- A termination signal is received  
- A critical runtime error occurs (e.g., memory allocation failure)  

---

# Build & Run

## Requirements
- GCC or Clang  
- POSIX-compliant operating system (Linux recommended)  
- Make  

## Build the Project
Compile all executables using:

---

# Makefile Commands

| Command         | Description                              |
|----------------|------------------------------------------|
| `make`         | Compile the project                      |
| `make exec`    | Run the program with default parameters  |
| `make test`    | Run test suite (`test.sh`)               |
| `make mytest`  | Run custom tests (`my_test.sh`)          |
| `make valg`    | Run under Valgrind                       |
| `make clean`   | Remove compiled files                    |
| `make cleanall`| Restore the project to initial state     |

---

# Example Usage

Run MasterWorker with:
./masterworker -n 4 -q 10 -d ./data -t 100


Example Collector output:
64834211 data/file100.dat
103453975 data/file2.dat
153259244 data/file1.dat


---

# Author
**Natalia Ceccarini**  
Academic Year: 2023/2024  

