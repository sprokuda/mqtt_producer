#include <iostream>
#include <cstdlib>
#include <string>
#include <chrono>
#include <cstring>

#include "httpserver.hpp"
#include "mqtt/async_client.h"


using namespace std;
const string TOPIC { "TOPIC" };
const string POST_TOPIC { "POST" };
const string GET_TOPIC_1 { "GET_1" };
const string GET_TOPIC_2 { "GET_2" };

const string DELETE_TOPIC_1 { "DELETE_1" };
const string DELETE_TOPIC_2 { "DELETE_2" };

const char* LWT_PAYLOAD = "Last will and testament.";

const int  QOS = 1;

const auto TIMEOUT = std::chrono::seconds(10);

class mqtt_resource : public httpserver::http_resource
{
public:
     const std::shared_ptr<httpserver::http_response> render_POST(const httpserver::http_request& req)
     {
         std::lock_guard<std::mutex> lck (mtx);
         cout << "POST request with body: " << req.get_content() << endl;
         try
         {
             mqtt::message_ptr pubmsg = mqtt::make_message(POST_TOPIC, req.get_content());
             pubmsg->set_qos(QOS);
             client->publish(pubmsg)->wait_for(TIMEOUT);
         }
         catch(const mqtt::exception& exc)
         {
             cerr << exc.what() << endl;
             return std::shared_ptr<httpserver::http_response>(new httpserver::string_response(exc.what()));
         }
         catch(...)
         {
             cerr << "unknown exception\n" << endl;
             return std::shared_ptr<httpserver::http_response>(new httpserver::string_response("unknown exception\n"));
         }
         cout << "...Ok" << endl;
         return std::shared_ptr<httpserver::http_response>(new httpserver::string_response("success!"));
     }

    const std::shared_ptr<httpserver::http_response> render_GET(const httpserver::http_request& req)
    {
        std::lock_guard<std::mutex> lck (mtx);
        string str = req.get_arg("event_name");
        str.replace(5,3," ");
        cout << "GET request with key parameter: " << str << endl;
        try
        {
            mqtt::message_ptr pubmsg = mqtt::make_message(GET_TOPIC_1, str);
            pubmsg->set_qos(QOS);
            client->publish(pubmsg)->wait_for(TIMEOUT);
            auto msg = client->consume_message();
            if (msg)
            {
                cout << msg->get_topic() <<  msg->to_string() << endl;
                return std::shared_ptr<httpserver::http_response>(new httpserver::string_response(msg->to_string()));
            }
            else
                return std::shared_ptr<httpserver::http_response>(new httpserver::string_response("zero message received"));
        }
        catch(const mqtt::exception& exc)
        {
            cerr << exc.what() << endl;
            return std::shared_ptr<httpserver::http_response>(new httpserver::string_response(exc.what()));
        }
        catch(...)
        {
            cerr << "unknown exception\n" << endl;
            return std::shared_ptr<httpserver::http_response>(new httpserver::string_response("unknown exception\n"));
        }
        cout << "...Ok" << endl;
        return std::shared_ptr<httpserver::http_response>(new httpserver::string_response("success!"));
    }

    const std::shared_ptr<httpserver::http_response> render_DELETE(const httpserver::http_request& req)
    {
        std::lock_guard<std::mutex> lck (mtx);
        cout << "DELETE request with body: " << req.get_content() << endl;
        try
        {
            mqtt::message_ptr pubmsg = mqtt::make_message(DELETE_TOPIC_1, req.get_content());
            pubmsg->set_qos(QOS);
            client->publish(pubmsg)->wait_for(TIMEOUT);
        }
        catch(const mqtt::exception& exc)
        {
            cerr << exc.what() << endl;
            return std::shared_ptr<httpserver::http_response>(new httpserver::string_response(exc.what()));
        }
        catch(...)
        {
            cerr << "unknown exception\n" << endl;
            return std::shared_ptr<httpserver::http_response>(new httpserver::string_response("unknown exception\n"));
        }
        cout << "...Ok" << endl;
        return std::shared_ptr<httpserver::http_response>(new httpserver::string_response("success!"));
    }

    void set_mqtt_client()
    {
        client = make_unique<mqtt::async_client>("tcp://localhost:1883" , "test1", "./persist");

        auto connOpts = mqtt::connect_options_builder()
            .clean_session()
            .will(mqtt::message(TOPIC, LWT_PAYLOAD, QOS))
            .finalize();
        try
        {
            cout << "\nConnecting to the MQTT server..." << endl;
            mqtt::token_ptr conntok = client->connect(connOpts);
            cout << "Waiting for the connection..." << endl;
            conntok->wait();
            client->start_consuming();
            client->subscribe(GET_TOPIC_2,QOS)->wait();
            cout << "  ...OK" << endl;
        }
        catch (const mqtt::exception& exc)
        {
            cerr << exc.what() << endl;
            return;
        }
        catch(...)
        {
            cerr << "unknown exception\n" << endl;
        }
    }

private:
     unique_ptr<mqtt::async_client> client;
     std::mutex mtx;
};

int main() {
    httpserver::webserver ws = httpserver::create_webserver(5880);

    mqtt_resource hwr;
    ws.register_resource("/mqtt", &hwr);
    hwr.set_mqtt_client();

    ws.start(true);

    return 0;
}
