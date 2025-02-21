#pragma once

#include <iostream>
#include "../Node.h"
#include "../NetworkContext.h"

class Candidate;  // Forward declaration

class Follower : public Node {
private:
    bool isVoted = false;

public:
    Follower(unsigned term);

    bool WriteLog() override;

    void HandleVoteResponse(RContext r_context, OContext &o_context) override;

    void HandleVoteRequest(RContext r_context, OContext &o_context) override;

    void HandleElectionTimeout(RContext r_context, OContext &o_context) override;

    void HandleHeartBeat(RContext r_context, OContext &o_context) override;

    void SendHeartBeat(RContext r_context, OContext &o_context) override;

    void ReceiveDataFromClient(RContext r_context, OContext &o_context) override;

    void HandleAppendEntries(RContext r_context, OContext &o_context) override;

    void HandleAnswerAppendFromFollower(RContext r_context, OContext &o_context) override;
};
