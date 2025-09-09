#pragma once
#include <string>
#include <libpq-fe.h>
void process_set_user( uWS::App& app, websock ws, json parsed_data, uWS::OpCode opcode);
void process_public_message(websock ws, json parsed_data);
void log_info(std::string info, std::string grade, std::string from_user = "common", std::string to_user = "common");
void process_private_message(websock ws, json parsed);
void process_add_sub_user(uWS::App& app, websock ws, json data, uWS::OpCode opcode);

class DBManager {
private:
    PGconn* conn;
    std::string db;
    std::string user;
    int port;
    PGresult* result;

    void connection(std::string conninfo);
    void log(std::message, std::status, std::string error_info = "NoN");

public:
    DBManager(std::string conninfo);

};
