#pragma once
#include "boost/asio.hpp"
#include "NetworkNode.h"
#include "states/Follower.h"
#include "states/Leader.h"
#include "states/Candidate.h"

class RaftServer {
private:
    std::vector<std::shared_ptr<NetworkNode>> nodes_;
    boost::asio::io_context io_context_;

public:
    RaftServer() = default;

    void AddNode(short port, const std::string& role, int weight) {
        if (role == "follower") {
            nodes_.emplace_back(std::make_shared<Follower>(io_context_, port, weight));
        } else if (role == "leader") {
            nodes_.emplace_back(std::make_shared<Leader>(io_context_, port, weight));
        } else if (role == "candidate") {
            nodes_.emplace_back(std::make_shared<Candidate>(io_context_, port, weight));
        }
    }

    void ConnectNodes(const std::string& host, short port) {
        for (auto& node : nodes_) {
            node->ConnectToPeer(host, port);
        }
    }

    void Run() {
        io_context_.run();
    }
};