# Operating Systems Project 3: Multi-User Chat Server

**Course:** Operating Systems  

**Project:** Network Programming - Implement A Simple Chat Server  

**Authors:** Nathnael Bereketab and Jonathan Abeje  

**Date:** November 2024

---

## Project Overview

A chat server that lets multiple people connect at the same time and talk to each other. Users can create chat rooms, join different rooms, and send private messages to specific people. The server handles multiple users safely without crashing or losing messages.

---

## Available Commands

| Command | Description | Example |
|---------|-------------|---------|
| `login <username>` | Change from guest to custom username | `login Alice` |
| `create <room>` | Create a new chat room | `create Study` |
| `join <room>` | Join an existing room | `join Study` |
| `leave <room>` | Leave a room | `leave Lobby` |
| `users` | List all connected users | `users` |
| `rooms` | List all available rooms | `rooms` |
| `connect <user>` | Establish DM connection with user | `connect Bob` |
| `disconnect <user>` | Remove DM connection with user | `disconnect Bob` |
| `exit` or `logout` | Disconnect from server | `exit` |
| `help` | Show available commands | `help` |

---

## How to Compile and Run

### Prerequisites
- GCC compiler
- POSIX threads library (pthread)
- Telnet client

### Compilation
```bash
make server
```

### Running the Server
```bash
./server
```

The server will start on **PORT 8888** and display:
```
Default room 'Lobby' created.
Server Launched! Listening on PORT: 8888
```

### Connecting Clients
Open a **new terminal** for each client and connect using telnet:
```bash
telnet localhost 8888
```

### Stopping the Server
Press `Ctrl+C` in the server terminal for graceful shutdown.

---

## Usage Examples

### Example Session 1: Basic Room Chat
```bash
# Terminal 1 (Alice)
telnet localhost 8888
login Alice
create Study
join Study
Hello everyone in Study!

# Terminal 2 (Bob)
telnet localhost 8888
login Bob
join Study
Hi Alice!
```

### Example Session 2: Direct Messaging
```bash
# Alice's terminal
connect Bob
This is a private message to Bob

# Bob's terminal
connect Alice
Private reply to Alice
```

### Example Session 3: Multiple Rooms
```bash
# Alice can be in multiple rooms
join Study
join Lobby
join GameRoom
# Messages sent by Alice go to users in ALL three rooms
```

---

## Implementation Details

### Message Routing Algorithm
1. Identify sender by socket descriptor
2. Find all rooms sender is currently in
3. Add all users in those rooms to recipient list (avoiding duplicates)
4. Find all DM connections involving sender
5. Add DM partners to recipient list (avoiding duplicates)
6. Send message to all unique recipients

### Error Handling
- Validates room existence before join/leave operations
- Checks for duplicate usernames on login
- Prevents self-connection for DMs
- Handles invalid commands gracefully

---

## Testing

### Test Scenario 1: Room Isolation
1. Alice joins Study room (and leaves Lobby)
2. Bob joins Study room (and leaves Lobby)
3. Charlie stays in Lobby only
4. **Result:** Messages from Alice/Bob only visible to each other, not Charlie

### Test Scenario 2: DM Privacy
1. Alice connects to Charlie (DM)
2. Alice sends message
3. **Result:** Only Charlie sees the message, not Bob

### Test Scenario 3: Multiple Rooms
1. Bob joins both Lobby and Study
2. Alice (in Study only) sends message
3. Charlie (in Lobby only) sends message
4. **Result:** Bob sees both messages; Alice and Charlie only see messages from their respective rooms

---

##  Team Members

- **Nathnael Bereketab**
- **Jonathan Abeje**

---
