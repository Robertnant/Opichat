
# Opichat

## Project Description
**Opichat** is a command-line chat application written in C, featuring both server and client components. It uses socket programming to establish communication between multiple clients and a server. The application demonstrates fundamental network programming concepts, such as socket creation, binding, and message handling.

## Features
- **Client-Server Architecture:** Employs a dedicated server to manage client connections, handle message routing, and maintain chat rooms.
- **Concurrent Client Handling:** Utilizes epoll for efficient management of multiple client connections, enabling simultaneous communication.
- **User Authentication:**  Supports basic user login to associate messages with usernames and prevent duplication.
- **Direct Messaging (DM):** Enables private one-on-one conversations between users.
- **Room-Based Chat:**  Provides functionality to create, join, leave, and delete chat rooms for group discussions. 
- **Message Broadcasting:** Supports broadcasting messages to all connected users or within specific rooms.
- **Robust Message Parsing:**  Implements a custom message lexer for structuring and interpreting communication between client and server.
- **Error Handling:** Includes error checks and handling mechanisms throughout to ensure stability and provide informative messages.

### Technical Implementation:

- **Socket Programming:** Leverages TCP sockets for reliable, bidirectional communication between clients and the server.
- **Epoll:** Employs the `epoll` system call for efficient event-driven I/O handling, allowing the server to manage multiple clients concurrently.
- **Custom Data Structures:**  Utilizes custom linked list implementations for storing and managing client connections, chat rooms, and message parameters. 
- **Message Format:** Defines a specific message format for communication, parsed using a custom lexer to extract payload, parameters, and command types. 

### Project Structure:

- **`opichat_server.c`, `opichat_server.h`:** Contains the server-side implementation.
- **`opichat_client.c`, `opichat_client.h`:** Contains the client-side implementation.
- **`connection.c`, `connection.h`:**  Manages client connections, including adding, removing, and finding clients.
- **`lexer.c`, `lexer.h`:**  Handles message tokenization, validation, and generation of structured messages.
- **`tools.c`, `tools.h`:** Provides utility functions for managing chat rooms and associated operations.
- **`xalloc.c`, `xalloc.h`:**  Implements memory allocation wrappers for error handling.

## Installation
To compile the project, you need `gcc` installed. Clone the repository and use the provided `Makefile`.

```bash
cd opichat
make
```

This will generate two executables: `opichat_server` and `opichat_client`.

## Usage

### Running the Server
Start the server by specifying an IP address and port.

```bash
./opichat_server <IP_ADDRESS> <PORT>
```

For example:

```bash
./opichat_server 127.0.0.1 8080
```

### Running the Client
Once the server is running, clients can connect to it using the following command:

```bash
./opichat_client <SERVER_IP_ADDRESS> <PORT>
```

For example:

```bash
./opichat_client 127.0.0.1 8080
```

After connecting, the client can send messages to the server, which will be broadcasted to all other connected clients.
