#pragma once
#include "../NetworkNode.h"

class Candidate : public NetworkNode{
private:

public:
    Candidate(boost::asio::io_context &io_context, short port) : NetworkNode(io_context, port) {

    }
    void HandleElectionTimeout() override{

    }

    bool WriteLog() override{

        return false;
    }

    // no
    void SendHeartBeat() override {

    }
};