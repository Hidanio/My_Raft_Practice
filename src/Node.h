#pragma once

#include <stack>
#include <random>
#include <chrono>
#include "boost/asio.hpp"
#include "NetworkContext.h"
#include "Contexts.h"

enum class NodeRole {Candidate, Follower, Leader};

class Node {
protected:
    std::stack<int> log_;
    std::mt19937 rng_{std::random_device{}()};
    NodeRole role_;
    int weight_;
    unsigned int votedFor_;
    unsigned currentTerm_;

public:
    virtual bool WriteLog() = 0;
    virtual void HandleElectionTimeout() = 0;
    virtual std::string SendHeartBeat() = 0;
    virtual void HandleVoteRequest(const std::string &message) =0;
    virtual void HandleVoteResponse(const std::string &message) =0;
    virtual void HandleHeartBeat(const std::string &message) =0;
    virtual std::string DoWorkOnTimer(std::unique_ptr<Node> &node) =0;
    // virtual on_timer (uniq_ptr<Node>) <- heartbeat/election and etс

    void ReceiveMessage(RContext r_context, OContext &Zapi) {
    /*    if (message.find("RequestVote") != std::string::npos) {
            auto message=  node_->HandleVoteRequest(message);
            SendMessageToAllPeers();
        } else if (message.find("VoteGranted") != std::string::npos) {
            auto message=  node_->HandleVoteResponse(message);
            SendMessageToAllPeers();
        } else if (message.find("Heartbeat") != std::string::npos) {
            auto message=  node_->HandleHeartBeat(message);
            SendMessageToAllPeers();
        }*/
        if (r_context.message.message.find("RequestVote") != std::string::npos) {
            HandleVoteRequest(r_context.message);
        }
        using namespace std::chrono_literals;
        
        
        Zapi.send_msg("HB term=228 id=MyId");
        Zapi.set_timer(100ms);
    }

    unsigned int ExtractTermFromMessage(const std::string &message) {
        size_t pos = message.find("term=");
        if (pos != std::string::npos) {
            return std::stoi(message.substr(pos + 5));
        }
        return 0;
    }

    void SetRole(NodeRole role){
        role_ = role;
    }

    NodeRole GetRole(){
        return role_;
    }

    unsigned int GetCurrentTerm() {
        return currentTerm_;
    }

    void SetCurrentTerm(unsigned int term){
        currentTerm_ = term;
    }
};
