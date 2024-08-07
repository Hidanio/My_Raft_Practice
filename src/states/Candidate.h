#pragma once

#include "../NetworkNode.h"

class Candidate : public NetworkNode {
private:
    boost::asio::steady_timer electionTimer_;
    int votesReceived_;

public:
    Candidate(boost::asio::io_context &io_context, short port) : NetworkNode(io_context, port),
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

    void SendVoteRequest(){
        std::string voteRequest = "RequestVote";
        SendMessageToPeer(voteRequest);
        std::cout << "Vote request send..." << "\n";
    }

    void ReceiveVoteResponse(bool voteGranted){
        if (voteGranted){
            ++votesReceived_;
        }
        std::cout << "Total votes: " << votesReceived_ << "\n";
        // logic for checking count of votes and become leader
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