#pragma once

#include <iostream>
#include "Node.h"

using boost::asio::ip::tcp;
// eto pizdets
class NetworkNode : public Node {
private:
    boost::asio::io_context &io_context_;
    tcp::acceptor acceptor_;
    tcp::socket socket_;
    std::vector<tcp::endpoint> peers_;

public:
    NetworkNode(boost::asio::io_context &io_context, short port)
            : io_context_(io_context), acceptor_(io_context, tcp::endpoint(tcp::v4(), port)), socket_(io_context) {
        StartAccept();
    }

    void StartAccept() {
        acceptor_.async_accept(socket_, [this](boost::system::error_code ec) {
            if (!ec) {
                std::make_shared<Session>(std::move(socket_), this)->Start();
            }
            StartAccept();
        });
    }

    void ConnectToPeer(const std::string &host, short port) {
        tcp::resolver resolver(io_context_);
        auto endpoints = resolver.resolve(host, std::to_string(port));
        boost::asio::async_connect(socket_, endpoints,
                                   [this](boost::system::error_code ec, const tcp::endpoint &) {
                                       if (!ec) {
                                           std::cout << "Connected to peer." << std::endl;
                                       }
                                   });
    }

    void SendMessageToPeer(const std::string &message) {
        boost::asio::async_write(socket_, boost::asio::buffer(message),
                                 [this](boost::system::error_code ec, std::size_t /*length*/) {
                                     if (!ec) {
                                         std::cout << "Message sent to peer." << std::endl;
                                     }
                                 });
    }

    void HandleElectionTimeout() override {
        std::cout << "Election timeout, starting election process..." << std::endl;
    // Lets start election
    }

    bool WriteLog() override {
        return true;
    }

    void SendHeartBeat() override {
    }

private:
    class Session : public std::enable_shared_from_this<Session> {
    private:
        void ReadMessage() {
            auto self(shared_from_this());
            boost::asio::async_read_until(socket_, boost::asio::dynamic_buffer(data_), '\n',
                                          [this, self](boost::system::error_code ec, std::size_t length) {
                                              if (!ec) {
                                                  std::cout << "Received message: " << data_ << std::endl;


                                                  data_.clear();
                                                  ReadMessage();
                                              }
                                          });
        }

        tcp::socket socket_;
        std::string data_;
        NetworkNode *node_;

    public:
        Session(tcp::socket socket, NetworkNode *node)
                : socket_(std::move(socket)), node_(node) {}

        void Start() {
            ReadMessage();
        }
    };
};