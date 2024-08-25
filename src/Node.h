#pragma once

#include <stack>
#include <random>
#include <chrono>
#include "boost/asio.hpp"
#include "Contexts.h"

enum class NodeRole {Candidate, Follower, Leader};

class Node {
protected:
    std::stack<int> log_;
    std::mt19937 rng_{std::random_device{}()};
    NodeRole role_;
    tcp::endpoint votedFor_;
    unsigned currentTerm_;

public:
    virtual bool WriteLog() = 0;
    virtual void HandleElectionTimeout(RContext r_context,OContext &o_context) = 0;
    virtual void SendHeartBeat(RContext r_context, OContext &o_context) = 0;
    virtual void HandleVoteRequest(RContext r_context, OContext &o_context) =0;
    virtual void HandleVoteResponse(RContext r_context, OContext &o_context) =0;
    virtual void HandleHeartBeat(RContext r_context, OContext &o_context) =0;

    void ReceiveMessage(const RContext& r_context, OContext &o_context) {
        if (r_context.message.message.find("RequestVote") != std::string::npos) {
            HandleVoteRequest(r_context, o_context);
        } else if (r_context.message.message.find("VoteGranted") != std::string::npos) {
            HandleVoteResponse(r_context, o_context);
        } else if (r_context.message.message.find("Heartbeat") != std::string::npos) {
            HandleHeartBeat(r_context, o_context);
        }

/*        using namespace std::chrono_literals;
        
        
        o_context.send_msg("HB term=228 id=MyId");
        o_context.set_timer(100ms);*/
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

    bool IsDefaultEndpoint(const boost::asio::ip::tcp::endpoint& endpoint) {
        return (endpoint.address().is_unspecified() && endpoint.port() == 0);
    }

    void OnTimer(const RContext& r_context, OContext &o_context) {
        switch (role_) {
            case NodeRole::Follower:
                HandleElectionTimeout(r_context, o_context);
                break;
            case NodeRole::Candidate:
                HandleElectionTimeout(r_context, o_context);
                break;
            case NodeRole::Leader:
                SendHeartBeat(r_context, o_context);
                break;
        }
    }

    virtual ~ Node()= default;
};
