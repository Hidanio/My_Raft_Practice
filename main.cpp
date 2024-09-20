#include <iostream>
#include "src/Raft_server.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: raft_node <port> [peer_port1 peer_port2 ...]\n";
        return 1;
    }

    short port = std::atoi(argv[1]);
    std::vector<short> peer_ports;
    for (int i = 2; i < argc; ++i) {
        peer_ports.push_back(std::atoi(argv[i]));
    }

    try {
        RaftServer server(port);
        server.ConnectToPeers("127.0.0.1", peer_ports);
        server.Run();
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}