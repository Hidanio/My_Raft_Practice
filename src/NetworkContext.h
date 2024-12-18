#pragma once

#include <iostream>
#include <utility>
#include <vector>
#include <memory>
#include <boost/asio.hpp>
#include "Node.h"

using boost::asio::ip::tcp;

class NetworkContext : public std::enable_shared_from_this<NetworkContext> {
protected:
    boost::asio::io_context &io_context_;
    tcp::acceptor acceptor_;
    std::vector<std::shared_ptr<tcp::socket>> peers_;
    tcp::endpoint leaderId_;
    std::unique_ptr<Node> node_;

    boost::asio::steady_timer timer_;
    boost::asio::steady_timer reconnect_timer_;

    std::unordered_map<std::string, std::shared_ptr<tcp::socket>> host_port_to_socket_;
    std::unordered_map<std::shared_ptr<tcp::socket>, std::string> socket_to_host_port_;


    void SetupTimer(const RContext &r_context, std::chrono::milliseconds ms) {
        timer_.expires_after(ms);
        std::cout << "Election timeout is " << ms.count() << "ms\n";
        timer_.async_wait([this, r_context](const boost::system::error_code &error) {
            if (!error) {
                OContext o_context;
                node_->OnTimer(r_context, o_context);

                if (o_context.next_time_out) {
                    SetupTimer(r_context, o_context.next_time_out.value());
                }

                if (o_context.message) {
                    if (o_context.notifyAll) {
                        SendMessageToAllPeers(o_context.message.value());
                    } else {
                        SendMessageToPeer(socket_by_peer(r_context.message.sender.value()), o_context.message.value());
                    }
                }
            } else if (error != boost::asio::error::operation_aborted) {
                std::cerr << "Timer error: " << error.message() << "\n";
            }
        });
    }

public:
    NetworkContext(boost::asio::io_context &io_context, short port)
            : io_context_(io_context), acceptor_(io_context, tcp::endpoint(tcp::v4(), port)),
              timer_(io_context), reconnect_timer_(io_context) {
        std::cout << "Node created on port " << port << "\n";
        StartAccept();
    }

    void SetNode(std::unique_ptr<Node> node) {
        node_ = std::move(node);
        Message message{
                acceptor_.local_endpoint(),
                ""
        };

        RContext r_context{
                message,
                node_,
                acceptor_.local_endpoint(),
                static_cast<unsigned int>(peers_.size())
        };
        using namespace std::chrono_literals;
        SetupTimer(r_context, 150ms);
    }

    void StartAccept() {
        auto new_socket = std::make_shared<tcp::socket>(io_context_);
        acceptor_.async_accept(*new_socket, [this, new_socket](boost::system::error_code ec) {
            if (!ec) {
                auto peer = new_socket->remote_endpoint();
                peers_.emplace_back(new_socket);
                std::make_shared<Session>(new_socket, this)->Start(); // one session - one socket
                std::cout << "Accepted connection from " << peer << "\n";
            }
            StartAccept();
        });
    }

    void ConnectToPeer(const std::string &host, short port) {
        auto peer_socket = std::make_shared<tcp::socket>(io_context_);
        tcp::resolver resolver(io_context_);
        auto endpoints = resolver.resolve(host, std::to_string(port));

        boost::asio::async_connect(*peer_socket, endpoints,
                                   [this, peer_socket, host, port](boost::system::error_code ec,
                                                                   const tcp::endpoint &) {
                                       if (!ec) {
                                           std::cout << "Connected to peer at " << host << ":" << port << "\n";
                                           std::string key = host + ":" + std::to_string(port);
                                           host_port_to_socket_[key] = peer_socket;
                                           socket_to_host_port_[peer_socket] = key;
                                           peers_.push_back(peer_socket);
                                           std::make_shared<Session>(peer_socket, this)->Start();
                                       } else {
                                           std::cerr << "Error connecting to peer: " << ec.message() << "\n";
                                       }
                                   });
    }

    void SendMessageToPeer(std::shared_ptr<tcp::socket> socket, const std::string &message) {
        if (socket && socket->is_open()) {
            try {
                std::cout << "Trying to send message to peer " << socket->remote_endpoint() << ": " << message << "\n";
                auto self = shared_from_this();
                boost::asio::async_write(*socket, boost::asio::buffer(message),
                                         [message, socket, self](boost::system::error_code ec, std::size_t length) {
                                             if (!ec) {
                                                 std::cout << "Message sent to peer " << socket->remote_endpoint()
                                                           << ": "
                                                           << message << "\n";
                                             } else {
                                                 std::cerr << "Error sending message: " << ec.message() << "\n";
                                                 if (ec == boost::asio::error::eof ||
                                                     ec == boost::asio::error::connection_reset ||
                                                     ec == boost::asio::error::broken_pipe) {
                                                     self->HandleDisconnect(socket);
                                                 }
                                             }
                                         });
            } catch (const boost::system::system_error &e) {
                std::cerr << "Error: remote_endpoint failed: " << e.what() << "\n";
                auto it = std::find(peers_.begin(), peers_.end(), socket);
                peers_.erase(it);
            }
        } else {
            std::cerr << "Socket is not open. Cannot send message.\n";
        }
    }

    void AttemptReconnectToHostPort(const std::string &host_port) {
        auto colonPos = host_port.find(':');
        std::string host = host_port.substr(0, colonPos);
        short port = std::stoi(host_port.substr(colonPos + 1));

        reconnect_timer_.expires_after(std::chrono::seconds(5));
        auto self = shared_from_this();
        reconnect_timer_.async_wait([self, host, port](boost::system::error_code ec) {
            if (!ec) {
                std::cout << "Reconnecting to specific host:port " << host << ":" << port << "\n";
                self->ConnectToPeer(host, port);
            } else {
                std::cerr << "Reconnect timer error: " << ec.message() << "\n";
            }
        });
    }

    void HandleDisconnect(const std::shared_ptr<tcp::socket> &socket) {
        std::cout << "Connection lost. Attempting to reconnect..." << "\n";

        boost::system::error_code ec;
        auto it = std::find(peers_.begin(), peers_.end(), socket);
        if (it != peers_.end()) peers_.erase(it);

        socket->close(ec);

        if (ec) {
            std::cerr << "Error closing socket: " << ec.message() << "\n";
        }

        // Let's find host:port for this socket
        auto it2 = socket_to_host_port_.find(socket);
        if (it2 != socket_to_host_port_.end()) {
            std::string host_port = it2->second;
            socket_to_host_port_.erase(it2);
            host_port_to_socket_.erase(host_port);

            AttemptReconnectToHostPort(host_port);
        } else {
            // Something if we can not find?
            //   AttemptReconnect();
        }
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
                node_,
                acceptor_.local_endpoint(),
                static_cast<unsigned int>(peers_.size())
        };

        node_->ReceiveMessage(r_context, o_context);
        if (o_context.next_time_out) {
            SetupTimer(r_context, o_context.next_time_out.value());
        }

        if (o_context.notifyAll) {
            SendMessageToAllPeers(o_context.message.value());
        } else {
            if (o_context.message) {
                std::cout << "New message to sent" << "\n";
                SendMessageToPeer(socket_by_peer(r_context.message.sender.value()), o_context.message.value());
            }
            //   SendMessageToPeer(socket_by_peer(r_context.message.sender.value()), o_context.message.value());
        }
    }

private:
    std::shared_ptr<tcp::socket> socket_by_peer(const tcp::endpoint &endpoint) {
        for (auto &socket: peers_) {
            if (socket->remote_endpoint() != endpoint) continue;
            return socket;
        }
        // handle properly
        throw "Some problems!";
    }

    class Session : public std::enable_shared_from_this<Session> {
    private:
        std::shared_ptr<tcp::socket> socket_;
        std::string data_;
        NetworkContext *node_;

        void ReadMessage() {
            auto self(shared_from_this());
            boost::asio::async_read_until(*socket_, boost::asio::dynamic_buffer(data_), '\n',
                                          [this, self](boost::system::error_code ec, std::size_t length) {
                                              if (!ec) {
                                                  std::cout << "Received message: " << data_ << "\n";
                                                  node_->ReceiveMessage(data_, *socket_);
                                                  data_.clear();
                                                  ReadMessage();  // wait again message
                                              }
                                          });
        }

    public:
        Session(std::shared_ptr<tcp::socket> socket, NetworkContext *node)
                : socket_(std::move(socket)), node_(node) {}

        void Start() {
            ReadMessage();
        }
    };
};
