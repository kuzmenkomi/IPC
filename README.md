# UNIX Inter-Process Communication (IPC)

A UNIX-based application demonstrating inter-process communication, process synchronization, and data exchange using various IPC mechanisms.

## 📌 Overview

The project implements a multi-process system where independent processes communicate and transfer data through pipes, shared memory, and network sockets.

## ✨ Features

- Multi-process architecture
- Inter-process communication using pipes
- Shared memory for data exchange
- Semaphore-based process synchronization
- UNIX signals (`SIGUSR1`)
- TCP and UDP socket communication
- File-based input and output

## 🛠️ Technologies

- C
- UNIX/Linux
- POSIX APIs
- Pipes and Shared Memory
- Semaphores
- TCP/UDP Sockets

## ⚙️ How It Works

The application consists of several interconnected processes:

- **P1 & P2** – Read words from input files.
- **Pr** – Collects and processes incoming data.
- **T** – Transfers data from pipes to shared memory.
- **S** – Processes data between shared memory segments.
- **D** – Sends processed data over TCP.
- **Serv1** – Receives TCP data and forwards it via UDP.
- **Serv2** – Receives UDP data and writes the results to a file.

### Data Flow

```text
P1 / P2
   |
   v
  Pr
   |
   v
 Pipe R2
   |
   v
   T
   |
   v
Shared Memory SM1
   |
   v
   S
   |
   v
Shared Memory SM2
   |
   v
   D
   |
   v
TCP Server (Serv1)
   |
   v
UDP Server (Serv2)
   |
   v
serv2.txt
```

## 🚀 Usage

Run the main application with two port numbers:

```bash
./zadanie <tcp_port> <udp_port>
```

Example:

```bash
./zadanie 8080 8081
```

The application initializes the IPC resources, launches the required processes, and manages communication between them.

## 📂 Input & Output

- `p1.txt` – Input data for process P1.
- `p2.txt` – Input data for process P2.
- `serv2.txt` – Final processed output.

## 🎯 Purpose

Demonstrates practical implementation of UNIX process management, synchronization, and communication across multiple independent processes.
