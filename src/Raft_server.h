#pragma once
#include "boost/asio.hpp"
#include "NetworkContext.h"
#include "states/Follower.h"

class RaftServer {
private:
    std::unique_ptr<NetworkContext> networkContext_;
    boost::asio::io_context io_context_;

public:
    RaftServer(short port) {
        networkContext_ = std::make_unique<NetworkContext>(io_context_, port);

        auto node = std::make_unique<Follower>(0);
        networkContext_->SetNode(std::move(node));
    }

    void ConnectToPeers(const std::string& host, const std::vector<short>& ports) {
        for (const short port : ports) {
            networkContext_->ConnectToPeer(host, port);
        }
    }

    void Run() {
        io_context_.run();
    }
};