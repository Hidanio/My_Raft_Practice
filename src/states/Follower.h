#pragma once

#include <iostream>
#include "../Node.h"
#include "../NetworkNode.h"
#include "Candidate.h"

class Follower : public NetworkNode {
private:
    boost::asio::steady_timer electionTimer_;

public:
    Follower(boost::asio::io_context &io_context, short port, int weight) : NetworkNode(
            io_context, port, weight), electionTimer_(io_context) {
        ResetElectionTimeout();
    }

    void ResetElectionTimeout() {
        auto timeout = std::uniform_int_distribution<>(150, 300)(rng_);
        electionTimer_.expires_after(std::chrono::milliseconds(timeout));
        std::cout << "Election timeout is " << timeout << "ms" "\n";
        electionTimer_.async_wait([this](const boost::system::error_code &error) {
            if(!error){
                HandleElectionTimeout();
            }
        });
    }

    void HandleElectionTimeout() override{
        // Follower cast => Candidate and start election
        SetRole(NodeRole::Candidate);
        std::cout << "Election timeout, starting new election..." << "\n";
        auto candidate = std::make_shared<Candidate>(io_context_
        ,socket_.local_endpoint().port(), weight_);
        candidate->StartElection();
    }

    void SendHeartBeat() override{

    }

    bool WriteLog() override{
        return false;
    }

    void HandleVoteResponse(const std::string &message) override{

    }

    void HandleVoteRequest(const std::string &message) override{
        std::cout << "Received vote request: " << message << "\n";

        unsigned int term = extractTermFromMessage(message);

        if (term > currentTerm_) {
            currentTerm_ = term;
            votedFor_ = 0;
        }

        if (votedFor_ == 0 || votedFor_ == term) {
            votedFor_ = term;
            std::string voteGranted = "VoteGranted term=" + std::to_string(currentTerm_) + "\n";
            SendMessageToAllPeers(voteGranted);
            ResetElectionTimeout();
        }
    }

    void HandleHeartBeat(const std::string &message) override{
        unsigned int receivedTerm = extractTermFromMessage(message);

        if (receivedTerm >= currentTerm_) {
            currentTerm_ = receivedTerm;
          // SetRole(NodeRole::Follower);
            ResetElectionTimeout();
        }
        std::cout << "Received heartBeat message: " << message << "\n";
    }
};