#pragma once

#include <iostream>
#include <vector>
#include <memory>
#include <boost/asio.hpp>
#include "Node.h"

using boost::asio::ip::tcp;

class NetworkNode : public Node {
protected:
    boost::asio::io_context &io_context_;
    tcp::acceptor acceptor_;
    tcp::socket socket_;
    std::vector<tcp::endpoint> peers_;
    int weight_;
    unsigned int votedFor_;
    unsigned currentTerm_;
    tcp::endpoint leaderId_;

public:
    NetworkNode(boost::asio::io_context &io_context, short port, int weight)
            : io_context_(io_context), acceptor_(io_context, tcp::endpoint(tcp::v4(), port)), socket_(io_context), weight_(weight), currentTerm_(0){
        std::cout << "Node created on port " << port << " with weight " << weight_ << "\n";
        role_ = NodeRole::Follower;
        StartAccept();
    }

    void StartAccept() {
        acceptor_.async_accept(socket_, [this](boost::system::error_code ec) {
            if (!ec) {
                auto peer = socket_.remote_endpoint();
                peers_.push_back(peer);
                std::make_shared<Session>(std::move(socket_), this)->Start();
                std::cout << "Accepted connection from " << peer << "\n";
            }
            StartAccept();
        });
    }

    void ConnectToPeer(const std::string &host, short port) {
        tcp::resolver resolver(io_context_);
        auto endpoints = resolver.resolve(host, std::to_string(port));
        boost::asio::async_connect(socket_, endpoints,
                                   [this, &host, port](boost::system::error_code ec, const tcp::endpoint &) {
                                       if (!ec) {
                                           std::cout << "Connected to peer at " << host << ":" << port << "\n";
                                           peers_.push_back(socket_.remote_endpoint());
                                       }
                                   });
    }

    void SendMessageToPeer(const tcp::endpoint &peer_endpoint, const std::string &message) {
        auto peer_socket = std::make_shared<tcp::socket>(io_context_);
        peer_socket->async_connect(peer_endpoint, [this, peer_socket, message](boost::system::error_code ec) {
            if (!ec) {
                boost::asio::async_write(*peer_socket, boost::asio::buffer(message),
                                         [&message, peer_socket](boost::system::error_code ec, std::size_t length) {
                                             if (!ec) {
                                                 std::cout << "Message sent to peer " << peer_socket->remote_endpoint() << ": " << message << "\n";
                                             }
                                         });
            }
        });
    }

    void SendMessageToAllPeers(const std::string &message) {
        for (const auto &peer: peers_) {
            SendMessageToPeer(peer, message);
        }
    }

    void ReceiveMessage(tcp::endpoint publisherId, const std::string &message){
        if(message.find("RequestVote") != std::string::npos){
            HandleVoteRequest(message);
        } else if(message.find("VoteGranted") != std::string::npos){
            HandleVoteResponse(message);
        } else if(message.find("Heartbeat") != std::string::npos){
            HandleHeartBeat(message);
        }
    }

    void HandleElectionTimeout() override {
        std::cout << "Election timeout, starting election process..." << "\n";
// Start election
    }

    bool WriteLog() override {
        return true;
    }

    void SendHeartBeat() override {
// heartBeat
    }

private:
    class Session : public std::enable_shared_from_this<Session> {
    private:
        tcp::socket socket_;
        std::string data_;
        NetworkNode *node_;

        void ReadMessage() {
            auto self(shared_from_this());
            boost::asio::async_read_until(socket_, boost::asio::dynamic_buffer(data_), '\n',
                                          [this, self](boost::system::error_code ec, std::size_t length) {
                                              if (!ec) {
                                                  std::cout << "Received message: " << data_ << "\n";
                                                  node_->ReceiveMessage(node_->leaderId_, data_);
                                                  data_.clear();
                                                  ReadMessage();
                                              }
                                          });
        }

    public:
        Session(tcp::socket socket, NetworkNode *node)
                : socket_(std::move(socket)), node_(node) {}

        void Start() {
            ReadMessage();
        }
    };
};