#pragma once

#include <stack>
#include <random>
#include <chrono>
#include "boost/asio.hpp"
#include "Contexts.h"

enum class NodeRole {Candidate, Follower, Leader};
constexpr inline auto TIMEOUT_FROM = 1500;
constexpr inline auto TIMEOUT_TO = 3000;

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

    void ReceiveMessage(RContext& r_context, OContext &o_context) {
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
        size_t pos = message.find("receivedTerm=");
        if (pos != std::string::npos) {
            pos += 13;
            std::cout << "Position after 'receivedTerm=': " << pos << std::endl;
            std::string termStr;
            while (pos < message.size() && std::isdigit(message[pos])) {
                termStr += message[pos];
                ++pos;
            }
            std::cout << "Extracted term string: '" << termStr << "'" << std::endl;

            if (!termStr.empty()) {
                try {
                    return std::stoi(termStr);
                } catch (const std::exception &e) {
                    std::cerr << "Error extracting term: " << e.what() << std::endl;
                }
            }
        } else {
            std::cerr << "Substring 'receivedTerm=' not found in message." << std::endl;
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
