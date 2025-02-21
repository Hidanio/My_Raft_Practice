#pragma once

#include "../NetworkContext.h"
#include "Leader.h"
#include "Follower.h"

class Candidate : public Node {
private:
    int votesReceived_ = 0;

public:
    Candidate(unsigned term);

    void HandleElectionTimeout(RContext r_context, OContext &o_context) override;

    void StartElection(const RContext& r_context, OContext &o_context);

    void HandleVoteResponse(RContext r_context, OContext &o_context) override;

    void HandleHeartBeat(RContext r_context, OContext &o_context) override;

    void HandleVoteRequest(RContext r_context, OContext &o_context) override;

    bool WriteLog() override;

    void SendHeartBeat(RContext r_context, OContext &o_context) override;

    void HandleAppendEntries(RContext r_context, OContext &o_context) override;

    void ReceiveDataFromClient(RContext r_context, OContext &o_context) override;

    void HandleAnswerAppendFromFollower(RContext r_context, OContext &o_context) override;

};
