#include "Candidate.h"

Candidate::Candidate(unsigned term) : votesReceived_(0) {
    currentTerm_ = term;
    SetRole(NodeRole::Candidate);
}

void Candidate::HandleElectionTimeout(RContext r_context, OContext &o_context) {
    std::cout << "Handling election timeout for candidates." << "\n";
    StartElection(r_context, o_context);
}

void Candidate::StartElection(const RContext& r_context, OContext &o_context) {
    votesReceived_ = 1;
    ++currentTerm_;
    votedFor_ = r_context.id;
    std::cout << "Voted myself: " << votedFor_ << "\n";

    std::cout << "Starting election..." << "\n";

    o_context.notifyAll = true;
    auto timeout = std::uniform_int_distribution<>(TIMEOUT_FROM, TIMEOUT_TO)(rng_);
    o_context.set_timer(std::chrono::milliseconds(timeout));

    std::string voteRequest = "RequestVote receivedTerm=" + std::to_string(currentTerm_) + "\n";
    o_context.send_msg(voteRequest);
    std::cout << "Vote request sent to all peers." << "\n";
}

void Candidate::HandleVoteResponse(RContext r_context, OContext &o_context) {
    auto message = r_context.message.message;
    unsigned int receivedTerm = ExtractTermFromMessage(message);

    std::cout << "HandleVoteResponse:" << "receivedTerm= " << receivedTerm << " && " << "currentTerm= " << currentTerm_ << "\n";

    if (receivedTerm > currentTerm_) {
        currentTerm_ = receivedTerm;
        std::cout << "Received vote response with higher term. Becoming Follower." << "\n";

        auto new_follower_node = std::make_unique<Follower>(currentTerm_);
        std::unique_ptr<Node> base_ptr = std::move(new_follower_node);

        auto timeout = std::uniform_int_distribution<>(TIMEOUT_FROM, TIMEOUT_TO)(rng_);
        o_context.set_timer(std::chrono::milliseconds(timeout));

        std::swap(r_context.node_, base_ptr);
        return;
    }

    std::cout << "HandleVoteResponse:" << "receivedTerm= " << receivedTerm << " && " << "currentTerm= " << currentTerm_ << "\n";
    if (receivedTerm == currentTerm_ && message.find("VoteGranted=true") != std::string::npos) {
        ++votesReceived_;
        std::cout << "Total votes: " << votesReceived_ << "\n";

        if (votesReceived_ > r_context.totalPeers / 2) {
            std::cout << "The king is dead, long live the king!" << "\n";

            auto new_leader_node = std::make_unique<Leader>(currentTerm_);
            new_leader_node->SendHeartBeat(r_context, o_context);
            std::unique_ptr<Node> base_ptr = std::move(new_leader_node);

            std::swap(r_context.node_, base_ptr);
        }
    } else {
        std::cout << "Vote not granted or term mismatch. Ignoring response." << "\n";
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

        auto timeout = std::uniform_int_distribution<>(TIMEOUT_FROM, TIMEOUT_TO)(rng_);
        o_context.set_timer(std::chrono::milliseconds(timeout));

        std::swap(r_context.node_, base_ptr);
    } else if (receivedTerm == currentTerm_) {
        //Received heartbeat with equal term, candidate should become Follower (another node become leader)
        std::cout << "Received heartbeat with equal term. Becoming Follower." << "\n";

        auto new_follower_node = std::make_unique<Follower>(currentTerm_);
        std::unique_ptr<Node> base_ptr = std::move(new_follower_node);

        auto timeout = std::uniform_int_distribution<>(TIMEOUT_FROM, TIMEOUT_TO)(rng_);
        o_context.set_timer(std::chrono::milliseconds(timeout));

        std::swap(r_context.node_, base_ptr);
    } else {
        std::cout << "Received heartbeat with lower term. Ignoring." << "\n";
    }
}


void Candidate::HandleVoteRequest(RContext r_context, OContext &o_context) {
    auto message = r_context.message.message;

    unsigned int receivedTerm = ExtractTermFromMessage(message);

    std::cout << "HandleVoteRequest: " << "receivedTerm= " << receivedTerm << " && " << "currentTerm= " << currentTerm_ << "\n";

    if (receivedTerm > currentTerm_) {
        currentTerm_ = receivedTerm;

        auto new_follower_node = std::make_unique<Follower>(currentTerm_);
        new_follower_node->HandleVoteRequest(r_context, o_context);
        std::unique_ptr<Node> base_ptr = std::move(new_follower_node);

        auto timeout = std::uniform_int_distribution<>(TIMEOUT_FROM, TIMEOUT_TO)(rng_);
        o_context.set_timer(std::chrono::milliseconds(timeout));

        std::swap(r_context.node_, base_ptr);
    } else if (receivedTerm < currentTerm_) {
        // The request is out of date, ignore it and send a refusal
        std::string voteNotGranted = "VoteGranted=false receivedTerm=" + std::to_string(currentTerm_) + "\n";
        o_context.send_msg(voteNotGranted);
    } else {
        // The term matches, the candidate ignores the request, since he himself is participating in the elections
        std::cout << "Ignoring vote request with equal term." << "\n";
        std::string voteNotGranted = "VoteGranted=false receivedTerm=" + std::to_string(currentTerm_) + "\n";
        o_context.send_msg(voteNotGranted);
    }
}

bool Candidate::WriteLog() {
    return false;
}

void Candidate::SendHeartBeat(RContext r_context, OContext &o_context) {

}
