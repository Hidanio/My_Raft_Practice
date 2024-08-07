#pragma once

#include <stack>
#include <random>
#include <chrono>
#include "boost/asio.hpp"

enum class NodeRole {Candidate, Follower, Leader};

class Node {
protected:
    uint id_; // guid better i think
    uint leaderId_;
    std::stack<int> log_;
    std::mt19937 rng_{std::random_device{}()};
    NodeRole role_;

public:
    bool isLeader = false;

    virtual bool WriteLog() = 0;
    virtual void HandleElectionTimeout() = 0;
    virtual void SendHeartBeat() = 0;

    void SetRole(NodeRole role){
        role_ = role;
    }

    NodeRole GetRole(){
        return role_;
    }
};
