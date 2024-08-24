#pragma once

#include <iostream>
#include "../Node.h"
#include "../NetworkContext.h"
#include "Candidate.h"

class Follower : public Node {
private:

    bool isVoted = false;
public:
    Follower(boost::asio::io_context &io_context, short port, int weight) {
    }

/*    void HandleElectionTimeout() override{
        // Follower cast => Candidate and start election
        SetRole(NodeRole::Candidate);
        std::cout << "Election timeout, starting new election..." << "\n";
        auto candidate = std::make_shared<Candidate>(io_context_
        ,socket_.local_endpoint().port(), weight_);
        candidate->StartElection();
    }*/

/*    void SendHeartBeat() override{

    }*/

    bool WriteLog() override{
        return false;
    }

    void HandleVoteResponse(const std::string &message) override{

    }

    void HandleVoteRequest(RContext r_context, OContext &Zapi) override{
        std::cout << "Received vote request: " << r_context.message.message << "\n";

        unsigned int term = ExtractTermFromMessage(r_context.message.message);

        if (term > currentTerm_) {
            currentTerm_ = term;
            votedFor_ = {};  // Сбросить голос, если term новый
            isVoted = false;
        }

        if(isVoted){
            // return false
            std::string voteNotGranted = "VoteGranted=false term=" + std::to_string(currentTerm_) + "\n";
            Zapi.send_msg(voteNotGranted);
        }

        if (IsDefaultEndpoint(votedFor_) || votedFor_ == r_context.message.sender) {
            isVoted = true;
            std::string voteGranted = "VoteGranted=true term=" + std::to_string(currentTerm_) + "\n";

            Zapi.send_msg(voteGranted);

            auto timeout = std::uniform_int_distribution<>(150, 300)(rng_);
            Zapi.set_timer(std::chrono::milliseconds(timeout));
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