#pragma once

#include "Follower.h"
#include <chrono>

class Leader : public Node {
public:
    Leader(unsigned term);

    bool WriteLog() override;

    void HandleVoteRequest(RContext r_context, OContext &o_context) override;

    void HandleHeartBeat(RContext r_context, OContext &o_context) override;

    void HandleVoteResponse(RContext r_context, OContext &o_context) override;

    void HandleElectionTimeout(RContext r_context, OContext &o_context) override;

    void SendHeartBeat(RContext r_context, OContext &o_context) override;
};
