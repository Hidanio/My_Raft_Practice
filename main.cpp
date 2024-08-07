#include <iostream>
#include "src/Raft_server.h"

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: raft_node <port> <peer_port> <role>\n";
        return 1;
    }

    short port = std::atoi(argv[1]);
    short peer_port = std::atoi(argv[2]);
    std::string role = argv[3];

    RaftServer server;
    server.AddNode(port, role);

    if (peer_port != 0) {
        server.ConnectNodes("127.0.0.1", peer_port);
    }

    server.Run();

    return 0;
}