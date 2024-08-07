#pragma once


#include "Follower.h"
#include <chrono>

class Leader : public NetworkNode {
private:
    std::vector<Follower *> followers;
    boost::asio::steady_timer heartbeatTimer_;

public:
    Leader(boost::asio::io_context &io_context, short port)
            : NetworkNode(io_context, port),
              heartbeatTimer_(io_context) {
        StartHeartBeat();
    }

    [[nodiscard]] int getFollowersCount() const {
        return followers.size();
    }

    void registerFollower(Follower *follower) {
        followers.push_back(follower);
    }

    void sendMessage() {
        for (auto &follower: followers) {
            follower->WriteLog();
        }
      //  SendMessageToPeer("testestest");
    }

    bool WriteLog() override {
        // sendMessage to followers -> wait answer
        // -> if all of them ok -> write to myself
        //          if no -> rollback
    }

    void SendHeartBeat() override {
        for (auto &follower: followers) {
            // send heartBeat
          //  SendMessageToPeer("I am alive");
            std::cout << "HeartBeating to follower" << "\n";
        }
    }

    void StartHeartBeat() {
        SendHeartBeat();

        heartbeatTimer_.expires_after(std::chrono::milliseconds(std::chrono::milliseconds(100)));
        heartbeatTimer_.async_wait([this](const boost::system::error_code &error) {
            if (!error) {
                HandleElectionTimeout();
            }
        });
    }
};