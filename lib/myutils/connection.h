#ifndef _MYUTILS_CONNECTION_H
#define _MYUTILS_CONNECTION_H

#include <string>

class Connection_Callback {
public:
    virtual void on_recv(const char* data, size_t size) = 0;
    virtual void on_idle() = 0;
    virtual void on_conn_status(bool connected) = 0;
};

class Connection {
public:
    Connection() {}
    virtual ~Connection() {}

    virtual bool connect(const std::string& addr, Connection_Callback* callback) = 0;
    virtual void reconnect() = 0;
    virtual void close() = 0;
    virtual void send(const char* data, size_t size) = 0;
    virtual void send(const std::string& data) = 0;
    virtual bool is_connected() = 0;
};



#endif