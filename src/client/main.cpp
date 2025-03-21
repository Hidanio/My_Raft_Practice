#include <iostream>
#include <boost/asio.hpp>


using boost::asio::ip::tcp;

class RaftClient : public std::enable_shared_from_this<RaftClient> {
private:


#pragma region fields
    boost::asio::io_context &io_context_; // Reference to io_context
    tcp::socket socket_;
    boost::asio::steady_timer reconnect_timer_;
    std::string read_msg_;
    std::deque<std::string> write_msgs_;

    std::string host_;
    short port_;

#pragma endregion fields

    void DoWrite() {
        if (write_msgs_.empty()) {
            return;
        }

        std::string msg = write_msgs_.front();
        std::cout << "Sending message: " << msg << "\n";

        auto self = shared_from_this();
        boost::asio::async_write(socket_, boost::asio::buffer(msg),
                                 [self](boost::system::error_code ec, std::size_t) {
                                     if (!ec) {
                                         std::cout << "Message sent successfully." << "\n";

                                         self->write_msgs_.pop_front();
                                         if (!self->write_msgs_.empty()) {
                                             self->DoWrite();
                                         }
                                     } else {
                                         std::cerr << "Write error: " << ec.message() << "\n";
                                         if (ec == boost::asio::error::eof ||
                                             ec == boost::asio::error::connection_reset ||
                                             ec == boost::asio::error::broken_pipe)
                                         {
                                             std::cout << "Something wrong. Please restart the client!" << "\n";
                                         }
                                     }
                                 });
    }

public:
    RaftClient(boost::asio::io_context &io_context, std::string host, short port)
            : io_context_(io_context),
              socket_(io_context), reconnect_timer_(io_context),
              host_(std::move(host)), port_(port) {
    }

    void Start() {
        StartConnect();
    }

    void StartConnect() {
        socket_ = tcp::socket(io_context_);

        tcp::resolver resolver(io_context_);
        auto endpoints = resolver.resolve(host_, std::to_string(port_));

        auto self = shared_from_this();
        boost::asio::async_connect(socket_, endpoints,
                                   [self](boost::system::error_code ec, const tcp::endpoint &) {
                                       if (!ec) {
                                           std::cout << "Connected to the server." << "\n";

                                           if (!self->write_msgs_.empty()) {
                                               self->DoWrite();
                                           }
                                       } else {
                                           std::cerr << "Connect failed: " << ec.message() << "\n";
                                       }
                                   });
    }


    void SendMessage(const std::string &message) {
        boost::asio::post(io_context_,
                          [self = shared_from_this(), message]() {
                              if (!self->socket_.is_open()) {
                                  std::cout << "Socket is not open. Message not sent: " << message << "\n";
                                  return;
                              }
                              bool write_in_progress = !self->write_msgs_.empty();
                              self->write_msgs_.push_back(message + "\n");

                              if (!write_in_progress) {
                                  self->DoWrite();
                              }
                          });
    }
};


int main() {
    try {
        boost::asio::io_context io_context;

        // hardcoded leader
        std::string leaderIp = "127.0.0.1";
        short LeaderPort = 5001;


        auto client = std::make_shared<RaftClient>(io_context, leaderIp, LeaderPort);
        client->Start();

        // Run io_context in a separate thread because input will block our thread
        std::thread io_thread([&io_context]() {
            io_context.run();
        });

        while (true) {
            std::string message = "Client: ";
            std::cout << "Enter a message to send (empty line to exit): ";
            std::getline(std::cin, message);

            if (message.empty()) {
                break;
            }

            client->SendMessage(message);
        }

        // Stop the client and join the thread
        io_context.stop();
        io_thread.join();

    } catch (const std::exception &e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
