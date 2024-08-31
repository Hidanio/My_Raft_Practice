#pragma once

#include <iostream>
#include <boost/asio.hpp>
#include <optional>


struct Node;
using boost::asio::ip::tcp;


struct Message {
    //if not set -> message to all || if set -> only one
    std::optional<tcp::endpoint> sender;
    std::string message;
};
// write (output)
struct OContext {
    bool notifyAll = false;
    std::optional<std::chrono::milliseconds> next_time_out;
    std::optional<std::string> message;

    void send_msg(std::string s) {
        message = std::move(s);
    }

    void set_timer(std::chrono::milliseconds s) {
        next_time_out = s;
    }
};

struct CContext{
    tcp::endpoint id;
    std::unique_ptr<Node> &node_;
};

//only read
struct RContext{
    Message message;
    std::unique_ptr<Node> &node_;
    tcp::endpoint id;
    unsigned totalPeers;
};
