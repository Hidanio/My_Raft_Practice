#pragma once
#include "boost/asio.hpp"
#include "NetworkContext.h"
#include "states/Follower.h"
#include "states/Leader.h"
#include "states/Candidate.h"

class RaftServer {
private:
    std::vector<std::shared_ptr<NetworkContext>> nodes_;
    boost::asio::io_context io_context_;

public:
    RaftServer() = default;

    void AddNode(short port, int weight) {
        nodes_.emplace_back(std::make_shared<Follower>(io_context_, port, weight));
    }

    void ConnectNodes(const std::string &host, const std::vector<short>& ports) {
        for (auto &node: nodes_) {
            for (const short port: ports) {
                node->ConnectToPeer(host, port);
            }
        }
    }

    void Run() {
        io_context_.run();
    }
};