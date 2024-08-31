#pragma once
#include "boost/asio.hpp"
#include "NetworkContext.h"
#include "states/Follower.h"
#include "states/Leader.h"
#include "states/Candidate.h"

class RaftServer {
private:
    std::vector<std::unique_ptr<NetworkContext>> nodes_;
    boost::asio::io_context io_context_;

public:
    RaftServer() = default;

    void AddNode(short port) {
        auto networkContext = std::make_unique<NetworkContext>(io_context_, port);

        auto node = std::make_unique<Follower>(0);
        networkContext->SetNode(std::move(node));

        nodes_.emplace_back(std::move(networkContext));
    }

    void ConnectNodes(const std::string &host, const std::vector<short>& ports) {
        for (auto &node: nodes_) {
            for (const short port: ports) {
                node->ConnectToPeer(host, port);
            }
        }
    }

    void Run() {
        // it is from one thread!
        io_context_.run();
    }
};
