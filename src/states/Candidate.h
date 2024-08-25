#pragma once

#include "../NetworkContext.h"
#include "Leader.h"
#include "Follower.h"

class Candidate : public Node {
private:
    int votesReceived_ = 0;

public:
    Candidate(unsigned term) : votesReceived_(0) {
        currentTerm_ = term;
        SetRole(NodeRole::Candidate);
    }

    void HandleElectionTimeout(RContext r_context, OContext &o_context) override {
        std::cout << "Handling election timeout for candidates." << "\n";
        StartElection(r_context, o_context);
    }

/*    std::string DoWorkOnTimer(std::unique_ptr<Node> &node) override {
        auto new_node = std::make_unique<Candidate>();
        auto message = new_node->StartElection();
        swap(node, node);
        return message;
    }*/

    void StartElection(RContext r_context, OContext &o_context) {
        votesReceived_ = 1;
        ++currentTerm_;
        votedFor_ = r_context.id;
        std::cout << "Starting election..." << "\n";

        o_context.notifyAll = true;
        auto timeout = std::uniform_int_distribution<>(150, 300)(rng_);
        o_context.set_timer(std::chrono::milliseconds(timeout));
        o_context.send_msg("RequestVote term=" + std::to_string(currentTerm_) + "\n");
    }

    void HandleVoteResponse(RContext r_context, OContext &o_context) override {
        if (r_context.message.message.find("VoteGranted") != std::string::npos) {
            ++votesReceived_;
            std::cout << "Total votes: " << votesReceived_ << "\n";
            if (votesReceived_ > r_context.totalPeers / 2) {
                SetRole(NodeRole::Leader);
                std::cout << "The king is dead, long live the king!" << "\n";

                auto new_leader_node = std::make_unique<Leader>(currentTerm_);

                new_leader_node->SendHeartBeat(r_context, o_context);
                std::unique_ptr<Node> base_ptr = std::move(new_leader_node);

                std::swap(r_context.node_, base_ptr);
            }
        }
    }

    //TODO: if we have message with data we should answer
    void HandleHeartBeat(RContext r_context, OContext &o_context) override {
        auto message = r_context.message.message;

        std::cout << "Received heartbeat as candidate: " << message << "\n";
        unsigned int receivedTerm = ExtractTermFromMessage(message);

        if (receivedTerm > currentTerm_) {
            currentTerm_ = receivedTerm;

            auto new_follower_node = std::make_unique<Follower>(currentTerm_);

            new_follower_node->StartElection(r_context, o_context);
            std::unique_ptr<Node> base_ptr = std::move(new_follower_node);

            auto timeout = std::uniform_int_distribution<>(150, 300)(rng_);
            o_context.set_timer(std::chrono::milliseconds(timeout));

            std::swap(r_context.node_, base_ptr);
        }

    }

    void HandleVoteRequest(RContext r_context, OContext &o_context) override {
/*        std::cout << "Received vote request: " << message << "\n";

        unsigned int term = ExtractTermFromMessage(message);

        if (term > currentTerm_) {
            currentTerm_ = term;
            votedFor_ = 0;
        }

        if (votedFor_ == 0 || votedFor_ == term) {
            votedFor_ = term;
            std::string voteGranted = "VoteGranted term=" + std::to_string(currentTerm_) + "\n";

        }*/
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

    // no
    void SendHeartBeat(RContext r_context, OContext &o_context) override {

    }
};