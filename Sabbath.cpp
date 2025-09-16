#include "App.h"
#include "nlohmann/json.hpp"
#include <iostream>
#include <ctime>
#include <string>
#include "Sabbath.h"
#include <libpq-fe.h>
using json = nlohmann::json;

DBManager::DBManager(const char* conninfo) {
    connection(conninfo);
}

void DBManager::connection(const char* conninfo) {
    conn = PQconnectdb(conninfo);

    if (PQstatus(conn) != CONNECTION_OK) {
        log("failed to connection", "ERROR", PQerrorMessage(conn));
        PQfinish(conn);
    }
}

void DBManager::log(std::string message, std::string status, std::string error_info) {
    std::cout << message << " -- " << status << " -- " << error_info << std::endl;
}

typedef uWS::WebSocket<false, true, UserData>* websock;

void DBManager::make_record(std::map<std::string, std::string> attr, std::string table_name) {
    int args_count = attr.size();
    std::string query = "insert into " + table_name + "(";
    std::string args_part = "";
    int count = 0;
    for (auto& p : attr) {
        count += 1;
        if (count < args_count)
        {
            query += p.first + ",";
            args_part += "'" + p.second + "',";
        }
        else {
            query += p.first + ")";
            args_part += "'" + p.second + "'";
        }
            
    }
    query += "values (" + args_part + ");"; 

    const char* query_massive = query.c_str(); 
    PGresult* result = PQexec(conn, query_massive);
    if (PQresultStatus(result) != PGRES_COMMAND_OK) {
        std::cout << PQresultStatus(result);
        log("error to insert", "ERROR", PQerrorMessage(conn));
        PQclear(result);
    }
}

void AManager::log_info(std::string info, std::string grade, std::string from_user, std::string to_user) {
    time_t now;
    std::time(&now);
    char* formatted_now = ctime(&now);
    formatted_now[strlen(formatted_now) - 1] = '\0';
    std::cout << info << " -- " << formatted_now << " -- " << grade;
    if (from_user != "common") std::cout << " -- " << from_user;
    if (to_user != "common") std::cout << " -- " << to_user;
    std::cout << std::endl;
}

void AManager::process_public_message( websock ws, json parsed_data)
{
    UserData* data = ws->getUserData();
    json payload = {
        {COMMAND_FIELD, "PUBLIC_MSG"},
        {TEXT_FIELD, parsed_data[TEXT_FIELD]},
        {USER_FROM_FIELD, data->unique_name}
    };

    ws->publish("PUBLIC_CHANNEL", payload.dump());
    log_info("send public message", "INFO", data->unique_name);
}

void AManager::process_set_user(websock ws, json parsed_data, uWS::OpCode opcode)
{
    UserData* data = ws->getUserData(); 
        
    json payload2;
    payload2[COMMAND_FIELD] = "SET_USER_MSG";
    payload2[UNIQUE_NAME_FIELD] = parsed_data[UNIQUE_NAME_FIELD];
    
    bool found = unique_exist(parsed_data[UNIQUE_NAME_FIELD]); 

    if (!found) { 
        data->unique_name = parsed_data[UNIQUE_NAME_FIELD];
        payload2[REQ_STATUS_FIELD] = "OK";
        ws->subscribe(data->unique_name);
        ws->unsubscribe(data->first_name);
        std::map<std::string, std::string> rec;
        rec[UNIQUE_NAME_DB_COL] = parsed_data[UNIQUE_NAME_FIELD];
        rec[FIRST_NAME_DB_COL] = data->first_name;
        rec[THIRD_NAME_DB_COL] = data->first_name;
        rec[SECOND_NAME_DB_COL] = data->first_name;
        rec["status"] = "true";
        this->dbm.make_record(rec, USERS_DB_TABLE);

        log_info("set user info", "INFO", data->unique_name);
        app.publish(data->unique_name, payload2.dump(), opcode);
    } else {
        payload2[REQ_STATUS_FIELD] = "USR_EXS";
        log_info("set user info", "ERROR", data->first_name, parsed_data[UNIQUE_NAME_FIELD]);
        app.publish(data->first_name, payload2.dump(), opcode);
    }
}

res_tuple DBManager::make_select_request(std::vector<std::string> args, std::string table_name, std::string where_filter) {
    std::string request_columns = "";
    int args_count = args.size();
    for (int i = 0; i < args_count - 1; i++) {
        request_columns += args[i] + ", "; 
    }
    request_columns += " " + args[args_count - 1];

    std::string query = "select " + request_columns + " from " + table_name;
    if (where_filter != "null") query += "  where " + where_filter;
    query += ";";
    const char* query_massive = query.c_str(); 
    PGresult* res = PQexec(conn, query_massive);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        log("error to select", "ERROR", PQerrorMessage(conn));
        PQclear(res);
        
        return make_map_frompg(res, false);
    } else return make_map_frompg(res);
}

res_tuple DBManager::make_map_frompg(PGresult* res, bool success) {
    res_tuple result;
    if (success){
        std::map<std::string, std::vector<std::string>> map_res;
                
        int rows_count = PQntuples(res);
        int cols_count = PQnfields(res);
        result["info"] = {};
        result["info"]["error"] = {"false"};
        result["info"]["empty"] = {"false"};
        if (rows_count == 0) {
             result["info"]["empty"] = {"true"};
             return result;
        }

        for (int i = 0; i < cols_count; i++)
        {
            std::vector<std::string> row;

            for (int j = 0; j < rows_count; j++) {
                row.push_back(PQgetvalue(res, j, i));
            }
            map_res[PQfname(res, i)] = row;
        }

         
        result["result"] = map_res;
        return result;
    }
    result["info"]["error"] = {"true"};
    return result;
}

bool AManager::unique_exist(std::string unique_name) {
    std::vector<std::string> attrs = {UNIQUE_NAME_DB_COL};
    std::string where = UNIQUE_NAME_DB_COL + " = '" + unique_name + "'";
    res_tuple r_result = dbm.make_select_request(attrs, "users", where);

    if (r_result["info"]["error"][0] == "false" and r_result["info"]["empty"][0] == "false")
        return true;
    return false;
}

void AManager::process_private_message(websock ws, json parsed)
{
    UserData* data = ws->getUserData();
    json payload;
    payload[COMMAND_FIELD] = "PRIVATE_MSG";
    payload[USER_FROM_FIELD] = data->unique_name;
    payload[TEXT_FIELD] = parsed[TEXT_FIELD];
    std::string subscriber_name = parsed[USER_TO_FIELD];
    ws->publish(subscriber_name, payload.dump());
    
    log_info("send private message", "INFO", data->unique_name, subscriber_name);
}

std::string user_status(UserData* data, bool online) //
{
    json payload = {
        {"command", "STATUS"},
        {"online", online},
        {"user_id", data->id},
        {"first_name", data->first_name},
    };

    return payload.dump();
}

void process_user_connect(uWS::App& app, websock ws, json parsed_data, uWS::OpCode opcode) //
{
    UserData* d = ws->getUserData();
    json message = {
	{"command", "GET_STATUS"},    
        
    };
    app.publish(d->unique_name, message.dump(), opcode);
}

void AManager::process_add_sub_user(websock ws, json data, uWS::OpCode opcode)
{
    UserData* user_data = ws->getUserData();
    json payload = {{COMMAND_FIELD, "ADD_CHAT"},
                     {REQ_STATUS_FIELD, "failed"}};
    
    bool found = unique_exist(data[UNIQUE_NAME_FIELD]); 
    if (data[TEXT_FIELD] != "") payload[TEXT_FIELD] = data[TEXT_FIELD];
    if (found)
    {
        payload[REQ_STATUS_FIELD] = "done";
        payload[UNIQUE_NAME_FIELD] = data[UNIQUE_NAME_FIELD];
        int channel_id = get_id_from_unique_name(data[UNIQUE_NAME_FIELD]);
        int user = get_id_from_unique_name(user_data->unique_name);
        add_chat_db(user, channel_id);
	
        log_info("add chat request", "INFO", user_data->unique_name, std::string(data[UNIQUE_NAME_FIELD]));
    } else {
        log_info("add chat request", "ERROR", user_data->unique_name, std::string(data[UNIQUE_NAME_FIELD]));
    }
    
    app.publish(user_data->unique_name, payload.dump(), opcode);   
}

int AManager::get_id_from_unique_name(std::string unique_name) {
    res_tuple result = dbm.make_select_request({"id"}, USERS_DB_TABLE, UNIQUE_NAME_DB_COL + " = '" + unique_name + "'");
    return std::stoi(result["result"]["id"][0]);
}

void AManager::add_chat_db(int user_id, int channel_id) {
    std::map<std::string, std::string> query;
    query[SB_USER_ID_COL] = std::to_string(user_id);
    query[SB_CHANNEL_ID_COL] = std::to_string(channel_id);
    
    dbm.make_record(query, SB_TABLE);
}

AManager::AManager() {

    
    wsb = {
        .open = [this](websock ws) {
            UserData* data = ws->getUserData();
            data->status = true;
            data->first_name = "UnnamedUser" + std::to_string(temp_id);
            this->log_info("new user:", "INFO", data->first_name);
            ws->subscribe("PUBLIC_CHANNEL");
            ws->subscribe(data->first_name);
            ws->publish("PUBLIC_CHANNEL", user_status(data, true));
            
        },

        .message = [this](websock ws, std::string_view message, uWS::OpCode opcode) {
	    UserData* data = ws->getUserData();
        std::string command;
        json parsed_data;
        
	    parsed_data = json::parse(message); // exept

        command = parsed_data[COMMAND_FIELD];
     
        
        std::string str_mes = std::string(message);
	    if (command.empty()) {
            std::string info = "no command in message";
            if (data->unique_name == "null")
		        this->log_info(info + str_mes, "ERROR", data->unique_name);
            else
                this->log_info(info + str_mes, "ERROR", data->first_name);
        } else {

            if (command == "PUBLIC_MSG")
            {
                this->process_public_message(ws, parsed_data);
            }
            else if (command == "PRIVATE_MSG")
            {
                this->process_private_message(ws, parsed_data);
            } else if (command == "GET_STATUS")
	        {
                process_user_connect(this->app, ws, parsed_data, opcode); 
	        } else if (command == "SET_USER_MSG")
	        {
		        this->process_set_user(ws, parsed_data, opcode);
	        } else if (command == "ADD_CHAT")
	        {
		        this->process_add_sub_user(ws, parsed_data, opcode);
            } else {
                std::string info = "wrong command in message";
                
                if (data->unique_name == "null")
		            this->log_info(info + str_mes, "ERROR", data->unique_name);
                else
                    this->log_info(info + str_mes, "ERROR", data->first_name);
            }
        } 
            }, .close = [](websock ws, int, std::string_view) {},
    };
}

void AManager::start(std::string address, int port) {
    app.ws<UserData>(address, std::move(wsb));
    
    app.listen(port, [](auto*) {});
    std::cout << "start" << std::endl;
    app.run();
}


int main()
{
    AManager manager = AManager();
    manager.start("/*", 8887);
}
    
