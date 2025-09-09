#!/bin/bash
sql1="CREATE DATABASE sabbath;"

psql -U postgres -c "$sql1"


sql2="
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
);"


psql -U postgres -d sabbath -c "$sql2"


