#pragma once
#include "iostream" 
using namespace std;
#include <string>
#include <mutex>

class PortableServer {
public:
    PortableServer();
    void waitForClient();
  
    void receiveMultithreaded();
    void sendToClient(const char* message);

    shared_ptr<string> getLastMessage() const;
    bool isConnected() const;
    shared_ptr<mutex> getMutex() const;
    bool newMessage() const;
};