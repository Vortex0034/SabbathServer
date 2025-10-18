# Sabbath Chat Server

A simple educational chat server implementation in C++ using WebSockets and PostgreSQL.

## Overview

This is a basic chat server project that demonstrates real-time communication using WebSockets with a PostgreSQL backend for data persistence. The project is currently in early development stage.

## Dependencies

- **uWebSockets** - WebSocket server library
- **uSockets** - Underlying socket library for uWebSockets
- **libpq** - PostgreSQL client library

## Installation on Linux

### Prerequisites

Ensure you have the following installed:
- C++ compiler with C++17 support
- PostgreSQL development libraries
- uWebSockets and uSockets libraries

### Database Setup
Make sure PostgreSQL is running. db_create.sh will create a user named "rufus" using psql

```bash
./db_create.sh

mkdir build
cd build

cmake ..
cmake --build .
```

🚧 Early Development - Core functionality is implemented but the project lacks many features expected from a production chat system.
