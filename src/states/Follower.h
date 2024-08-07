#pragma once

#include <iostream>
#include "../Node.h"
#include "../NetworkNode.h"

class Follower : public NetworkNode {
private:
    boost::asio::steady_timer electionTimer_;

public:
    Follower(boost::asio::io_context &io_context, short port) : NetworkNode(
            io_context, port), electionTimer_(io_context) {
        ResetElectionTimeout();
    }

    void receiveMessage(uint publisherId, int data) {
        if (publisherId != leaderId_) {
            return;
        }

        log_.push(data);
        ResetElectionTimeout();
    }

    void ResetElectionTimeout() {
        auto timeout = std::uniform_int_distribution<>(150, 300)(rng_);
        electionTimer_.expires_after(std::chrono::milliseconds(timeout));
        electionTimer_.async_wait([this](const boost::system::error_code &error) {
            if(!error){
                HandleElectionTimeout();
            }
        });
    }
    void HandleElectionTimeout() override{
        // Follower cast => Candidate and start election
        std::cout << "Election timeout, starting new election..." << "\n";
    }

    void SendHeartBeat() override{

    }

    bool WriteLog() override{
        return false;
    }
};