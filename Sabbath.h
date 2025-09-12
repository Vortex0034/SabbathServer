#pragma once
#include <string>
#include <libpq-fe.h>

using json = nlohmann::json;

struct UserData {
    int id;
    std::string unique_name = "null";
    std::string first_name;
    std::string second_name;
    std::string third_name;
    std::string key;
    bool status = false;
};

typedef uWS::WebSocket<false, true, UserData>* websock;
using res_tuple =  std::map<std::string, std::map<std::string, std::vector<std::string>>>;
std::map<std::string, websock> connected = {};

std::map<std::string, std::string> app_data = {};

class DBManager {
private:
    PGconn* conn;
    std::string db;
    std::string user;
    int port;
    PGresult* result;

    // void push_message(std::string user_from, std::string user_to, std::strin)
    void connection(const char* conninfo);
    void log(std::string message, std::string status, std::string error_info = "NoN");

public:
    DBManager(const char* conninfo);
    void make_record(std::map<std::string, std::string> attr, std::string table_name);
    res_tuple make_select_request(std::vector<std::string> args, std::string table_name, std::string where_filter = "null");
    res_tuple make_map_frompg(PGresult* res, bool success = true);
};

class AManager {
private:
    DBManager dbm = DBManager("dbname=sabbath");
    uWS::App app = uWS::App(); 
    uWS::TemplatedApp<false>::WebSocketBehavior<UserData> wsb;

    bool unique_exist(std::string unique_name); 
    void process_set_user(websock ws, json parsed_data, uWS::OpCode opcode);
    void process_public_message(websock ws, json parsed_data);
    void log_info(std::string info, std::string grade, std::string from_user = "common", std::string to_user = "common");
    void process_private_message(websock ws, json parsed);
    void process_add_sub_user(websock ws, json data, uWS::OpCode opcode);

public:
    AManager();
    void start(std::string address, int port);
};


