#include "App.h"
#include "nlohmann/json.hpp"
#include <iostream>
#include <ctime>
#include <string>
#include "Sabbath.h"
#include <libpq-fe.h>
using json = nlohmann::json;

DBManager(std::string conninfo) {
    connection(conninfo);
}

void DBManager::connection(std::string conninfo) {
    conn = PQconnectdb(conninfo);

    if (PQstatus(conn) != CONNECTION_OK) {
        log("failed to connection", "ERROR", PQerrorMessage(conn));
        PQfinish(conn);
    }
}

void DBManager::log(std::message, std::status, std::string error_info = "NoN") {
    std::cout << message << " -- " << status << " -- " << error_info << std::endl;
}

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

std::map<std::string, websock> connected = {};

std::map<std::string, std::string> app_data = {};

void log_info(std::string info, std::string grade, std::string from_user, std::string to_user) {
    time_t now;
    std::time(&now);
    char* formatted_now = ctime(&now);
    formatted_now[strlen(formatted_now) - 1] = '\0';
    std::cout << info << " -- " << formatted_now << " -- " << grade;
    if (from_user != "common") std::cout << " -- " << from_user;
    if (to_user != "common") std::cout << " -- " << to_user;
    std::cout << std::endl;
}

void process_public_message(websock ws, json parsed_data)
{
    UserData* data = ws->getUserData();
    json payload = {
        {"command", "PUBLIC_MSG"},
        {"text", parsed_data["text"]},
        {"user_from", data->unique_name}
    };

    ws->publish("PUBLIC_CHANNEL", payload.dump());
    log_info("send public message", "INFO", data->unique_name);
}

void process_set_user( uWS::App& app, websock ws, json parsed_data, uWS::OpCode opcode)
{
    UserData* data = ws->getUserData(); 
        
    json payload2;
    payload2["command"] = "SET_USER_MSG";
    payload2["name"] = parsed_data["unique_name"];
    
    bool found = connected.count(parsed_data["unique_name"]);

    if (!found) { 
        data->unique_name = parsed_data["unique_name"];
        payload2["status"] = "OK";
        connected[parsed_data["unique_name"]] = ws;
        ws->subscribe(data->unique_name);
        ws->unsubscribe(data->first_name);
        log_info("set user info", "INFO", data->unique_name);
        app.publish(data->unique_name, payload2.dump(), opcode);
    } else {
        payload2["status"] = "USR_EXS";
        log_info("set user info", "ERROR", data->first_name, parsed_data["unique_name"]);
        app.publish(data->first_name, payload2.dump(), opcode);
    }
    
}


void process_private_message(websock ws, json parsed)
{
    UserData* data = ws->getUserData();
    json payload;
    payload["command"] = "PRIVATE_MSG";
    payload["user_from"] = data->unique_name;
    payload["text"] = parsed["text"];
    std::string subscriber_name = parsed["user_to"];
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

void process_add_sub_user(uWS::App& app, websock ws, json data, uWS::OpCode opcode)
{
    UserData* user_data = ws->getUserData();
    json payload = {{"command", "ADD_CHAT"},
                     {"result", "failed"}};
    
    bool found = connected.count(data["name"]); 
    if (data["message"] != "") payload["message"] = data["message"];
    if (found && connected[(data["name"])]->getUserData()->status)
    {
        payload["result"] = "done";
        payload["name"] = data["name"];
	
        log_info("add chat request", "INFO", user_data->unique_name, std::string(data["name"]));
    } else {
        log_info("add chat request", "ERROR", user_data->unique_name, std::string(data["name"]));
    }
    
    app.publish(user_data->unique_name, payload.dump(), opcode);   
}

int main()
{
    int latest_user_id = 0;
    uWS::App app = uWS::App();

    uWS::TemplatedApp<false>::WebSocketBehavior<UserData> wsb = {

        .open = [&latest_user_id](websock ws) {
            UserData* data = ws->getUserData();
            data->id = latest_user_id++;
            data->status = true;
            data->first_name = "UnnamedUser#" + std::to_string(data->id);
            log_info("new user:", "INFO", data->first_name);
            ws->subscribe("PUBLIC_CHANNEL");
            ws->subscribe(data->first_name);
            ws->publish("PUBLIC_CHANNEL", user_status(data, true));
        },

        .message = [&app](websock ws, std::string_view message, uWS::OpCode opcode) {
	    UserData* data = ws->getUserData();
        std::string command;
        json parsed_data;
        
	    parsed_data = json::parse(message);

        command = parsed_data["command"];
     
        
        std::string str_mes = std::string(message);
	    if (command.empty()) {
            std::string info = "no command in message";
            if (data->unique_name == "null")
		        log_info(info + str_mes, "ERROR", data->unique_name);
            else
                log_info(info + str_mes, "ERROR", data->first_name);
        } else {

            if (command == "PUBLIC_MSG")
            {
                process_public_message(ws, parsed_data);
            }
            else if (command == "PRIVATE_MSG")
            {
                process_private_message(ws, parsed_data);
            } else if (command == "GET_STATUS")
	        {
               process_user_connect(app, ws,  parsed_data, opcode); 
	        } else if (command == "SET_USER_MSG")
	        {
		       process_set_user(app, ws, parsed_data, opcode);
	        } else if (command == "ADD_CHAT")
	        {
            std::cout << "add chat";
		    process_add_sub_user(app,ws, parsed_data, opcode);
            } else {
                std::string info = "wrong command in message";
                
                if (data->unique_name == "null")
		            log_info(info + str_mes, "ERROR", data->unique_name);
                else
                    log_info(info + str_mes, "ERROR", data->first_name);            }
        } 
            }, .close = [](websock ws, int, std::string_view) {},
    };

    app.ws<UserData>("/*", std::move(wsb));

    app.listen(8887, [](auto*) {});
    std::cout << "start" << std::endl;
    app.run();
}
