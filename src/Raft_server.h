#pragma once

#include <utility>

#include "boost/asio.hpp"
#include "NetworkContext.h"
#include "states/Follower.h"

struct Peers_to_connect {
    std::string host;
    short port;

    Peers_to_connect(std::string host, uint port) : host(std::move(host)), port(port) {

    }
};

class RaftServer {
private:
    std::shared_ptr<NetworkContext> networkContext_;
    boost::asio::io_context io_context_;

public:
    explicit RaftServer(short port) {
        networkContext_ = std::make_shared<NetworkContext>(io_context_, port);

        auto node = std::make_unique<Follower>(0);
        networkContext_->SetNode(std::move(node));
    }

    void ConnectToPeers(const std::vector<Peers_to_connect>& pears) {
        for (const auto& pear : pears) {
            networkContext_->ConnectToPeer(pear.host, pear.port);
        }
    }

    void Run() {
        io_context_.run();
    }
};