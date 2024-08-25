#pragma once

#include "Follower.h"
#include <chrono>

class Leader : public Node {
private:

public:
    Leader(unsigned term) {
        currentTerm_ = term;
        SetRole(NodeRole::Leader);
    }

    bool WriteLog() override {
        // sendMessage to followers -> wait answer
        // -> if all of them ok -> write to myself
        //          if no -> rollback
        return false;
    }

    void HandleVoteRequest(RContext r_context, OContext &o_context) override {

    }

    void HandleHeartBeat(RContext r_context, OContext &o_context) override {

    }

    void HandleVoteResponse(RContext r_context, OContext &o_context) override {

    }

    void HandleElectionTimeout(RContext r_context, OContext &o_context) override {

    }

    void SendHeartBeat(RContext r_context, OContext &o_context) override {
        std::string heartbeat = "HeartBeat term=" + std::to_string(currentTerm_) + "RAVE RAVE HEARTBEAT" + "\n";

        o_context.send_msg(heartbeat);
        o_context.notifyAll = true;
        auto timeout = std::uniform_int_distribution<>(50, 100)(rng_);
        o_context.set_timer(std::chrono::milliseconds(timeout));
        std::cout << "Heartbeat sent.." << "\n";
    }
};