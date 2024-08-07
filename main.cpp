#include <iostream>
#include "boost/asio.hpp"

void test_timer() {
    boost::asio::io_context io_context;
    boost::asio::steady_timer timer(io_context, std::chrono::seconds(1));

    timer.async_wait([](const boost::system::error_code& error) {
        if (!error) {
            std::cout << "Timer expired!" << std::endl;
        }
    });

    io_context.run();
}

int main() {
    test_timer();
    return 0;
}