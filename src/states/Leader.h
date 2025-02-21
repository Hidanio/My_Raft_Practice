#pragma once

#include "Follower.h"
#include <chrono>

class Leader : public Node {
public:
    unsigned commitIndex = -1;


    Leader(unsigned term);

    bool WriteLog() override;

    void HandleVoteRequest(RContext r_context, OContext &o_context) override;

    void HandleHeartBeat(RContext r_context, OContext &o_context) override;

    void HandleVoteResponse(RContext r_context, OContext &o_context) override;

    void HandleElectionTimeout(RContext r_context, OContext &o_context) override;

    void SendHeartBeat(RContext r_context, OContext &o_context) override;

    void ReceiveDataFromClient(RContext r_context, OContext &o_context) override;

    std::string ExtractDataFromClientMessage(const std::string &message);

    void HandleAnswerAppendFromFollower(RContext r_context, OContext &o_context) override;

    void UpdateCommitIndex();

    void SendAppendEntries(const RContext& r_context, OContext &o_context);

    void HandleAppendEntries(RContext r_context, OContext &o_context) override;
};
