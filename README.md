# TCP Client-Server Chat Room

A multi-client chat application developed using **C programming and TCP socket programming**. The application follows a client-server architecture and uses POSIX threads to handle multiple client connections concurrently.

The server manages client authentication, active connections, and message routing between users. Clients can communicate through group chats or private messages.

## Features

- **Multiple Client Support:** Supports up to 10 simultaneous client connections.
- **User Authentication:** Validates usernames and passwords during login and registration.
- **Duplicate Login Prevention:** Prevents multiple simultaneous sessions using the same username.
- **Persistent User Storage:** Stores user information separately in a file (`users.txt`), maintaining user records across server restarts.
- **Group Chat:** Allows users to broadcast messages to all connected clients.
- **Private Messaging:** Enables one-to-one communication between online users.
- **Online User List:** Displays the list of currently connected users.
- **Session Management:** Handles user login, logout, and client disconnections.
- **Concurrent Client Handling:** Uses POSIX threads to manage multiple clients independently.

## Technologies Used

| Technology | Purpose |
|---|---|
| C Programming | Application development |
| TCP Sockets | Reliable client-server communication |
| POSIX Threads (Pthreads) | Concurrent client handling |
| Linux System Calls | Socket communication and file operations |
| File Handling | Persistent user information storage |
| Makefile | Automated compilation |

## Project Structure

```text
TCP-Chat-Room/
│
├── bin/
│   ├── server
│   └── client
│
├── data/
│   └── users.txt
│
├── include/
│   ├── client.h
│   ├── common.h
│   └── server.h
│
├── src/
│   ├── client/
│   │
│   └── server/
│       ├── auth_manager.c
│       ├── chat_logic.c
│       └── server_main.c
│
├── Makefile
└── README.md
```

## Architecture

The application follows a multi-threaded client-server architecture.

**Server:**
- Creates a TCP listening socket.
- Accepts incoming client connections.
- Creates a dedicated thread for each connected client.
- Validates user credentials.
- Maintains a list of active users.
- Routes group and private messages to the appropriate clients.
- Handles client disconnections and resource cleanup.

**Client:**
- Establishes a TCP connection with the server.
- Performs user authentication.
- Provides options for group chat, private chat, and viewing online users.
- Uses a separate receiving thread to continuously receive messages from the server while the main thread handles user input.

## Compilation

### Prerequisites

- GCC compiler
- Linux environment
- POSIX Threads library

### Build

Clone the repository:

```bash
git clone https://github.com/pathakuntalokeswarreddy/LI_3_TCP_Chat_Room_Project.git
```

Navigate to the project directory:

```bash
cd LI_3_TCP_Chat_Room_Project
```

Compile the server and client:

```bash
make
```

This generates two executables:

```text
bin/server
bin/client
```

### Run the Application

**1. Start the Server**

From the project root directory:

```bash
./bin/server
```

**2. Start the Client**

Open another terminal and execute:

```bash
./bin/client
```

Launch additional client instances in separate terminals to test simultaneous connections and chat functionality.

**Note:** Run the executables from the project root directory because the user-data file is accessed using a relative path.

### Clean Build

To remove generated executables and the user-data file:

```bash
make clean
```

## Chat Operations

After successful login, users can perform the following operations:

| Option | Operation | Description |
|---|---|---|
| 1 | Group Chat | Send messages to all online users. |
| 2 | Private Chat | Send messages to a specific online user. |
| 3 | Online Users | View currently connected users. |
| 4 | Logout | Disconnect from the server gracefully. |

## Concepts Demonstrated

This project demonstrates practical implementation of several Linux system programming and networking concepts:

- TCP socket programming (`socket`, `bind`, `listen`, `accept`, `connect`, `send`, `recv`)
- Client-server communication
- Concurrent programming using POSIX threads
- Thread creation and detachment (`pthread_create`, `pthread_detach`)
- File handling using system calls (`open`, `read`, `write`, `close`)
- Process and thread synchronization
- Network byte order conversion
- IP address conversion (`inet_ntop`)
- Dynamic memory allocation and resource management
- Client connection and disconnection handling

## Future Enhancements

- Implement secure password hashing.
- Improve TCP message framing and partial send/receive handling.
- Add message delivery acknowledgements.
- Introduce encrypted communication using SSL/TLS.
- Add support for chat history.

## Author

**Lokeswar Reddy Pathakunta**

GitHub: [pathakuntalokeswarreddy](https://github.com/pathakuntalokeswarreddy)
