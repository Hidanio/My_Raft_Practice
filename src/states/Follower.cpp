#include "Follower.h"
#include "Candidate.h"

Follower::Follower(unsigned term) {
    currentTerm_ = term;
    SetRole(NodeRole::Follower);
}

bool Follower::WriteLog() {
    return false;
}

void Follower::HandleVoteResponse(RContext r_context, OContext &o_context) {

}

void Follower::HandleVoteRequest(RContext r_context, OContext &o_context) {
    std::cout << "Received vote request: " << r_context.message.message << "\n";

    unsigned int receivedTerm = ExtractTermFromMessage(r_context.message.message);

    if (receivedTerm > currentTerm_) {
        currentTerm_ = receivedTerm;
        votedFor_ = {};  // Сбросить голос, если receivedTerm новый
        isVoted = false;
    }

    if (isVoted) {
        std::string voteNotGranted = "VoteGranted=false receivedTerm=" + std::to_string(currentTerm_) + "\n";
        o_context.send_msg(voteNotGranted);
    }

    auto sender_endpoint = r_context.message.sender.value();
    if (IsDefaultEndpoint(votedFor_) || votedFor_ == sender_endpoint) {
        isVoted = true;
        std::cout << "Voted for: " << sender_endpoint.address() << ": " << sender_endpoint.port() << "\n";
        votedFor_ = sender_endpoint;
        std::string voteGranted = "VoteGranted=true receivedTerm=" + std::to_string(currentTerm_) + "\n";

        o_context.send_msg(voteGranted);

        auto timeout = std::uniform_int_distribution<>(TIMEOUT_FROM, TIMEOUT_TO)(rng_);
        o_context.set_timer(std::chrono::milliseconds(timeout));
    }
}

void Follower::HandleElectionTimeout(RContext r_context, OContext &o_context) {
    std::cout << "Follower election timeout. Becoming candidate..." << '\n';

    auto new_candidate_node = std::make_unique<Candidate>(currentTerm_);

    new_candidate_node->StartElection(r_context, o_context);
    std::unique_ptr<Node> base_ptr = std::move(new_candidate_node);

    std::swap(r_context.node_, base_ptr);
}

//TODO: Here we can sync our LOG, если не совпадает, то вызовем ?HandleAppendEntries?
void Follower::HandleHeartBeat(RContext r_context, OContext &o_context) {
    auto message = r_context.message.message;
    unsigned int receivedTerm = ExtractTermFromMessage(message);

    if (receivedTerm >= currentTerm_) {
        currentTerm_ = receivedTerm;

        auto timeout = std::uniform_int_distribution<>(TIMEOUT_FROM, TIMEOUT_TO)(rng_);
        o_context.set_timer(std::chrono::milliseconds(timeout));
    }

    std::cout << "Received heartBeat message: " << message << "\n";
}

void Follower::HandleAppendEntries(RContext r_context, OContext &o_context) {
    // Извлечение полей из AppendEntries запроса
    // Если лог подписчика совпадает с prevLogIndex и prevLogTerm,
    // добавить новые записи (если есть) и обновить commit index, а также выполнить действие (выведем в текстовой файл например для тестов)
    // Если нет – отправить ?false?

    // Сбросить таймер выборов (так как пришёл своего рода heartbeat)
    auto timeout = std::uniform_int_distribution<>(TIMEOUT_FROM, TIMEOUT_TO)(rng_);
    o_context.set_timer(std::chrono::milliseconds(timeout));
}

void Follower::SendHeartBeat(RContext r_context, OContext &o_context) {

}

void Follower::ReceiveDataFromClient(RContext r_context, OContext &o_context) {

}

void Follower::HandleAnswerAppendFromFollower(RContext r_context, OContext &o_context) {

}

