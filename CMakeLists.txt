cmake_minimum_required(VERSION 3.5)

set(Project "producer")
project(${Project} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(Project_IncludeDirs
    /home/ubuntu/work/libhttpserver/src/
    /home/ubuntu/work/libhttpserver/src/httpserver/
    /home/ubuntu/work/paho.mqtt.c/src/
    /home/ubuntu/work/paho.mqtt.cpp/src/
    /home/ubuntu/work/paho.mqtt.cpp/src/mqtt
        )

    add_executable(${Project} main.cpp)


target_include_directories(${Project} PRIVATE ${Project_IncludeDirs})
target_link_libraries(${Project}
       /home/ubuntu/work/libhttpserver-install/libhttpserver.so.0.19.0
       /home/ubuntu/work/mqtt-c-build/src/libpaho-mqtt3c.so.1.3.9
       /home/ubuntu/work/mqtt-c-build/src/libpaho-mqtt3a.so.1.3.9
       /home/ubuntu/work/mqtt-cpp-build/src/libpaho-mqttpp3.so.1.2.0
        )
