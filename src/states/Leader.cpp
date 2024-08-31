#include "Leader.h"

Leader::Leader(unsigned term) {
    currentTerm_ = term;
    SetRole(NodeRole::Leader);
}

bool Leader::WriteLog() {
    // sendMessage to followers -> wait answer
    // -> if all of them ok -> write to myself
    //          if no -> rollback
    return false;
}

void Leader::HandleVoteRequest(RContext r_context, OContext &o_context) {

}

void Leader::HandleHeartBeat(RContext r_context, OContext &o_context) {

}

void Leader::HandleVoteResponse(RContext r_context, OContext &o_context) {

}

void Leader::HandleElectionTimeout(RContext r_context, OContext &o_context) {

}

void Leader::SendHeartBeat(RContext r_context, OContext &o_context) {
    std::string heartbeat = "HeartBeat receivedTerm=" + std::to_string(currentTerm_) + " RAVE RAVE HEARTBEAT" + "\n";

    o_context.send_msg(heartbeat);
    o_context.notifyAll = true;
    auto timeout = std::uniform_int_distribution<>(TIMEOUT_FROM / 3, TIMEOUT_TO / 3)(rng_);
    o_context.set_timer(std::chrono::milliseconds(timeout));
    std::cout << "Heartbeat sent.." << "\n";
}
