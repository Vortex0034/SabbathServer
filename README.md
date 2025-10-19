# Sabbath Chat Server

A simple educational chat server implementation in C++ using WebSockets and PostgreSQL.

## Overview

This is a basic chat server project that demonstrates real-time communication using WebSockets with a PostgreSQL backend for data persistence. The project is currently in early development stage.

## Dependencies

- **uWebSockets** - WebSocket server library
- **uSockets** - Underlying socket library for uWebSockets
- **libpq** - PostgreSQL client library
- **nlohmann/json.hpp** - JSON parsing

## Installation on Linux

### Database

database_schema.sql file contains the database structure for the Sabbath application.

- **Database Name**: sabbath
- **Tables**: 3 (users, messages, subscriptions)

### Building from source code

```bash
git clone https://github.com/MikhelsonVladislava/SabbathServer.git
cd SabbathServer

mkdir build
cd build

cmake ..
cmake --build .
```

## Short-term Goals

- Improvements to user registration: organizing the process of hashing passwords and adding them to the database;
- Providing the authorization process;
- Adding the ability to save message history;
- Test coverage of existing code;
- Ensure data transfer via SSL protocol.

🚧 Early Development - Core functionality is implemented but the project lacks many features expected from a production chat system.
