#pragma once

#include <iostream>
#include "../Node.h"
#include "../NetworkContext.h"
#include "Candidate.h"

class Follower : public Node {
private:

public:
    Follower(boost::asio::io_context &io_context, short port, int weight) {
        ResetElectionTimeout();
    }

    void ResetElectionTimeout() {
        auto timeout = std::uniform_int_distribution<>(150, 300)(rng_);
        electionTimer_.expires_after((timeout));
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

        unsigned int term = ExtractTermFromMessage(message);

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
    // remove it and move this logic (read Rcontext) and write to (Ocontext)
    std::string DoWorkOnTimer(std::unique_ptr<Node> &node) override{
        /*
         * auto new_node = make_unique<Follower>(tis.weight, this.log, ...)
         * swap(node, new_node)
         * */
    }

    void HandleHeartBeat(const std::string &message) override{
        unsigned int receivedTerm = ExtractTermFromMessage(message);

        if (receivedTerm >= currentTerm_) {
            currentTerm_ = receivedTerm;
          // SetRole(NodeRole::Follower);
            ResetElectionTimeout();
        }
        std::cout << "Received heartBeat message: " << message << "\n";
    }
};