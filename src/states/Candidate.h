#pragma once

#include "../NetworkNode.h"
#include "Leader.h"

class Candidate : public NetworkNode {
private:
    boost::asio::steady_timer electionTimer_;
    int votesReceived_;

    void SendVoteRequest() {
        std::string voteRequest = "RequestVote\n";
        SendMessageToAllPeers(voteRequest);
        std::cout << "Vote request send..." << "\n";
    }
public:
    Candidate(boost::asio::io_context &io_context, short port, int weight) : NetworkNode(io_context, port, weight),
                                                                 electionTimer_(io_context), votesReceived_(0) {
        StartElection();
    }

    void HandleElectionTimeout() override {
        std::cout << "Handling election timeout for candidates." << "\n";
        StartElection();
    }

    void StartElection() {
        votesReceived_ = 0;
        std::cout << "Starting election..." << "\n";

        SendVoteRequest();
        ResetElectionTimeout();
    }

    void HandleVoteResponse(const std::string &message) override{
        if(message.find("VoteGranted") != std::string::npos){
            ++votesReceived_;
            std::cout << "Total votes: " << votesReceived_ << "\n";
            if(votesReceived_ > peers_.size() / 2){
                SetRole(NodeRole::Leader);
                std::cout << "The king is dead, long live the king!" << "\n";
                auto leader = std::make_shared<Leader>(io_context_,socket_.local_endpoint().port(),weight_);
                leader->StartHeartBeat();
            }
        }
    }

    void HandleHeartBeat(const std::string &message) override {
        std::cout << "Received heartbeat as candidate: " << message << "\n";
        SetRole(NodeRole::Follower);
    }

    void HandleVoteRequest(const std::string &message) override{

    }

    bool WriteLog() override {

        return false;
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

    // no
    void SendHeartBeat() override {

    }
};