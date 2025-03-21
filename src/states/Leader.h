#pragma once

#include "Follower.h"
#include <chrono>

class Leader : public Node {
public:
    //  we have maps that storing for each follower two indexes:
    //  - nextIndex[follower] : index of the next record, which we want to send to this follower
    //  - matchIndex[follower]: index of the last record, which the follower 100% has
    std::unordered_map<tcp::endpoint, unsigned> nextIndex_;
    std::unordered_map<tcp::endpoint, unsigned> matchIndex_;

    // commitIndex: last commited index (that was fixed)
    // lastApplied: last applied index (not fixed yet and can be reverted)
    unsigned commitIndex = 0;
    unsigned lastApplied = 0;


    Leader(unsigned term);

    bool WriteLog() override;

    void HandleVoteRequest(RContext r_context, OContext &o_context) override;

    void HandleHeartBeat(RContext r_context, OContext &o_context) override;

    void HandleVoteResponse(RContext r_context, OContext &o_context) override;

    void HandleElectionTimeout(RContext r_context, OContext &o_context) override;

    void SendHeartBeat(RContext r_context, OContext &o_context) override;

    void ReceiveDataFromClient(RContext r_context, OContext &o_context) override;

    // Replication
    void SendAppendEntries(const RContext& r_context, OContext &o_context);

    void HandleAnswerAppendFromFollower(RContext r_context, OContext &o_context) override;

    std::string ExtractDataFromClientMessage(const std::string &message);

    void ApplyLogEntries();

    void UpdateCommitIndex();

    void HandleAppendEntries(RContext r_context, OContext &o_context) override;
};
