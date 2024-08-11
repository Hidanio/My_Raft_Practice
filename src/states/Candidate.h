#pragma once

#include "../NetworkContext.h"
#include "Leader.h"

class Candidate : public Node {
private:
    boost::asio::steady_timer electionTimer_;
    int votesReceived_;

    std::string SendVoteRequest() {
        return "RequestVote term=" + std::to_string(currentTerm_) + "\n";
        // SendMessageToAllPeers(voteRequest);
        std::cout << "Vote request send..." << "\n";
    }
public:
    Candidate() {
        StartElection();
    }

    void HandleElectionTimeout() override {
        std::cout << "Handling election timeout for candidates." << "\n";
        StartElection();
    }

    std::string DoWorkOnTimer(std::unique_ptr<Node> &node) override {
        auto new_node = std::make_unique<Candidate>();
        auto message = new_node->StartElection();
        swap(node, node);
        return message;
    }

    std::string StartElection() {
        votesReceived_ = 1;
        ++currentTerm_;
        votedFor_ = currentTerm_;
        std::cout << "Starting election..." << "\n";

        return SendVoteRequest();
      //  ResetElectionTimeout();
    }

    void HandleVoteResponse(const std::string &message) override {
        if (message.find("VoteGranted") != std::string::npos) {
            ++votesReceived_;
            std::cout << "Total votes: " << votesReceived_ << "\n";
            if (votesReceived_ > peers_.size() / 2) {
                SetRole(NodeRole::Leader);
                std::cout << "The king is dead, long live the king!" << "\n";
                auto leader = std::make_shared<Leader>(io_context_, socket_.local_endpoint().port(), weight_);
                leader->SetCurrentTerm(currentTerm_);
                leader->StartHeartBeat();
            }
        }
    }

    void HandleHeartBeat(const std::string &message) override {
        std::cout << "Received heartbeat as candidate: " << message << "\n";
        unsigned int receivedTerm = ExtractTermFromMessage(message);

        if (receivedTerm > currentTerm_) {
            currentTerm_ = receivedTerm;
            SetRole(NodeRole::Follower);
            ResetElectionTimeout();
        }
    }

    void HandleVoteRequest(const std::string &message) override {
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

    bool WriteLog() override {

        return false;
    }


    /*
     * struct TimerContext {
     *   string to_send;
     *   Time next_timer_time_to_sleep;
     *   unique_ptr<Node>& ref_to_node;
     * }
     *
     *
     * void setup_timer() {
     *   timer.on(300ms, [](){
     *     string s = node_strategy->get_on_timer();
     *     tcp_send(s)
     *     setup_timer();
     *   });
     * }
     *
     * leader:
     * get_on_timer(TimerContext &ctx) {
     *  ctx.to_send = "Send_all $term";
     *  ctx.next_time = rand(100);
     * }
     *
     * follower: {
     *   ctx.to_send = "Lets select new leader"
     *   auto new_node = make_unique<Candidate>(...);
     *   swap(new_node, ref_to_node);
     *   ctx.next_time = rand(300 ... 500);
     * }
     *
     *
     *
     * */

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