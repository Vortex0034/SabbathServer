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
- PostgreSQL libpq library
- uWebSockets and uSockets libraries

### Database create

```sql

CREATE TABLE Users ( 
id SERIAL PRIMARY KEY,
unique_name VARCHAR(256) NOT NULL,
first_name VARCHAR(128),
second_name VARCHAR(128),
third_name VARCHAR(128),
status BOOLEAN);

CREATE TABLE Messages (
sender_id INT NOT NULL,
recipient_id INT NOT NULL,
CONSTRAINT fkey_se FOREIGN KEY (sender_id) REFERENCES Users(id),
CONSTRAINT fkey_re FOREIGN KEY (recipient_id) REFERENCES Users(id),
message_text VARCHAR(1024)
);

CREATE TABLE subscriptions (
user_id INT NOT NULL,
user_channel_id INT NOT NULL,
CONSTRAINT fkey_us FOREIGN KEY (user_id) REFERENCES Users(id),
CONSTRAINT fkey_ch FOREIGN KEY (user_channel_id) REFERENCES Users(id),
CONSTRAINT prke PRIMARY KEY (user_id, user_channel_id)
);

```
### Building from source code

```bash
git clone https://github.com/Vortex0034/SabbathServer.git
cd SabbathServer

mkdir build
cd build

cmake ..
cmake --build .
```
🚧 Early Development - Core functionality is implemented but the project lacks many features expected from a production chat system.
