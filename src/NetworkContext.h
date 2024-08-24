#pragma once

#include <iostream>
#include <utility>
#include <vector>
#include <memory>
#include <boost/asio.hpp>
#include "Node.h"

using boost::asio::ip::tcp;


class NetworkContext {
protected:
    boost::asio::io_context &io_context_;
    tcp::acceptor acceptor_;
    tcp::socket socket_;
    std::vector<tcp::endpoint> peers_;
    tcp::endpoint leaderId_;
    std::unique_ptr<Node> node_;
    boost::asio::steady_timer timer_;

    void SetupTimer(std::chrono::milliseconds ms) {
        timer_.expires_after(ms);
        std::cout << "Election timeout is " << ms.count() << "ms\n";
        timer_.async_wait([this](const boost::system::error_code &error) {
            if (!error) {
                OContext o_context;
                node_->OnTimer(o_context);

                if (o_context.next_time_out) {
                    SetupTimer(o_context.next_time_out.value());
                }

                if (o_context.message) {
                    if (o_context.notifyAll) {
                        SendMessageToAllPeers(o_context.message.value());
                    } else {
                        // maybe not leader ?
                        SendMessageToPeer(leaderId_, o_context.message.value());
                    }
                }
            } else if (error != boost::asio::error::operation_aborted) {
                std::cerr << "Timer error: " << error.message() << "\n";
            }
        });
    }
public:
    NetworkContext(boost::asio::io_context &io_context, short port)
            : io_context_(io_context), acceptor_(io_context, tcp::endpoint(tcp::v4(), port)), socket_(io_context),
              timer_(io_context) {
        std::cout << "Node created on port " << port << "\n";
        StartAccept();
    }

    void StartAccept() {
        acceptor_.async_accept(socket_, [this](boost::system::error_code ec) {
            if (!ec) {
                auto peer = socket_.remote_endpoint();
                peers_.push_back(peer);
                std::make_shared<Session>(std::move(socket_), this)->Start();
                std::cout << "Accepted connection from " << peer << "\n";
                //potential node-> onAccept; <- should we understand which node?

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

    void ReceiveMessage(std::string data, tcp::socket &socket) {
        OContext o_context;
        auto rem = socket.remote_endpoint();
        //extract data
        Message message{
            std::optional{rem},
            std::move(data)
        };

        RContext r_context{
                message,
                node_
        };

        node_->ReceiveMessage(std::move(r_context), o_context);
        if (o_context.next_time_out) {
            SetupTimer(o_context.next_time_out.value());
        }

        if (o_context.notifyAll) {
            SendMessageToAllPeers(o_context.message.value());
        } else {
            SendMessageToPeer(r_context.message.sender.value(), o_context.message.value());
        }
    }

private:
    class Session : public std::enable_shared_from_this<Session> {
    private:
        tcp::socket socket_;
        std::string data_;
        NetworkContext *node_;

        void ReadMessage() {
            auto self(shared_from_this());
            boost::asio::async_read_until(socket_, boost::asio::dynamic_buffer(data_), '\n',
                                          [this, self, &socket_=socket_](boost::system::error_code ec, std::size_t length) {
                                              if (!ec) {
                                                  std::cout << "Received message: " << data_ << "\n";
                                                  node_->ReceiveMessage(data_, socket_);
                                                  data_.clear();
                                                  ReadMessage();
                                              }
                                          });
        }

    public:
        Session(tcp::socket socket, NetworkContext *node)
                : socket_(std::move(socket)), node_(node) {}

        void Start() {
            ReadMessage();
        }
    };
};
