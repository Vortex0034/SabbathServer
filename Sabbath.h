#pragma once
#include <string>
#include <libpq-fe.h>

using json = nlohmann::json;

struct UserData {
    int id;
    std::string unique_name = "null";
    std::string first_name;
    bool status = false;
};

typedef uWS::WebSocket<false, true, UserData>* websock;
using res_tuple =  std::map<std::string, std::map<std::string, std::vector<std::string>>>;

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
    const std::string UNIQUE_NAME_FIELD = "unique_name";
    const std::string COMMAND_FIELD = "command";
    const std::string TEXT_FIELD = "text";
    const std::string USER_FROM_FIELD = "user_from";
    const std::string USER_TO_FIELD = "user_to";
    const std::string FIRST_NAME_FIELD = "first_name";
    const std::string SECOND_NAME_FIELD = "second_name";
    const std::string THIRD_NAME_FIELD = "third_name";
    const std::string REQ_STATUS_FIELD = "status";

    const std::string UNIQUE_NAME_DB_COL = UNIQUE_NAME_FIELD;
    const std::string FIRST_NAME_DB_COL = FIRST_NAME_FIELD;
    const std::string SECOND_NAME_DB_COL = SECOND_NAME_FIELD;
    const std::string THIRD_NAME_DB_COL = THIRD_NAME_FIELD;
    const std::string USERS_DB_TABLE = "users";
    const std::string SB_TABLE = "subscriptions";
    const std::string SB_USER_ID_COL = "user_id";
    const std::string SB_CHANNEL_ID_COL = "user_channel_id";
    
    const std::string REG_USER_COMMAND = "REG_USER_MSG";
    const std::string PUBLIC_MSG_COMMAND = "PUBLIC_MSG";
    const std::string ADD_CHAT_COMMAND = "ADD_CHAT";
    const std::string PRIVATE_MSG_COMMAND = "PRIVATE_MSG";

    DBManager dbm = DBManager("dbname=sabbath");
    uWS::App app = uWS::App();
    int temp_id = 0;
    uWS::TemplatedApp<false>::WebSocketBehavior<UserData> wsb;

    bool unique_exist(std::string unique_name); 
    void process_set_user(websock ws, json parsed_data, uWS::OpCode opcode);
    void process_public_message(websock ws, json parsed_data);
    void log_info(std::string info, std::string grade, std::string from_user = "common", std::string to_user = "common");
    void process_private_message(websock ws, json parsed);
    void process_add_sub_user(websock ws, json data, uWS::OpCode opcode);
    int get_id_from_unique_name(std::string unique_name);
    void add_chat_db(int user_id, int channel_id);
public:
    AManager();
    void start(std::string address, int port);
};


