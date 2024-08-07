#pragma once


#include "Follower.h"
#include <chrono>

class Leader : public NetworkNode {
private:
    boost::asio::steady_timer heartbeatTimer_;

public:
    Leader(boost::asio::io_context &io_context, short port, int weight)
            : NetworkNode(io_context, port, weight),
              heartbeatTimer_(io_context) {
        StartHeartBeat();
    }

    bool WriteLog() override {
        // sendMessage to followers -> wait answer
        // -> if all of them ok -> write to myself
        //          if no -> rollback
        return false;
    }

    void HandleVoteRequest(const std::string &message) override{

    }

    void HandleHeartBeat(const std::string &message) override{

    }

    void HandleVoteResponse(const std::string &message) override{

    }

    void SendHeartBeat() override{
        std::string heartbeat = "HeartBeat";
        SendMessageToAllPeers(heartbeat);
        std::cout << "Heartbeat send.." << "\n";
    }

    void StartHeartBeat() {
        SendHeartBeat();
        heartbeatTimer_.expires_after(std::chrono::milliseconds(std::chrono::milliseconds(100)));

        heartbeatTimer_.async_wait([this](const boost::system::error_code &error) {
            if (!error) {
                StartHeartBeat();
            }
        });
    }
};