#include "Candidate.h"

Candidate::Candidate(unsigned term) : votesReceived_(0) {
    currentTerm_ = term;
    SetRole(NodeRole::Candidate);
}

void Candidate::HandleElectionTimeout(RContext r_context, OContext &o_context) {
    std::cout << "Handling election timeout for candidates." << "\n";
    StartElection(r_context, o_context);
}

void Candidate::StartElection(RContext r_context, OContext &o_context) {
    votesReceived_ = 1;
    ++currentTerm_;
    votedFor_ = r_context.id;
    std::cout << "Starting election..." << "\n";

    o_context.notifyAll = true;
    auto timeout = std::uniform_int_distribution<>(150, 300)(rng_);
    o_context.set_timer(std::chrono::milliseconds(timeout));
    o_context.send_msg("RequestVote term=" + std::to_string(currentTerm_) + "\n");
}

void Candidate::HandleVoteResponse(RContext r_context, OContext &o_context) {
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

void Candidate::HandleHeartBeat(RContext r_context, OContext &o_context) {
    auto message = r_context.message.message;

    std::cout << "Received heartbeat as candidate: " << message << "\n";
    unsigned int receivedTerm = ExtractTermFromMessage(message);

    if (receivedTerm > currentTerm_) {
        currentTerm_ = receivedTerm;

        auto new_follower_node = std::make_unique<Follower>(currentTerm_);
        std::unique_ptr<Node> base_ptr = std::move(new_follower_node);

        auto timeout = std::uniform_int_distribution<>(150, 300)(rng_);
        o_context.set_timer(std::chrono::milliseconds(timeout));

        std::swap(r_context.node_, base_ptr);
    }
}

void Candidate::HandleVoteRequest(RContext r_context, OContext &o_context) {
    // Реализация метода
}

bool Candidate::WriteLog() {
    return false;
}

void Candidate::SendHeartBeat(RContext r_context, OContext &o_context) {
    // Реализация метода
}
