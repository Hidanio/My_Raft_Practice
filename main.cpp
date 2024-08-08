#include <iostream>
#include "src/Raft_server.h"

int main(int argc, char *argv[]) {
    if (argc != 5) {
        std::cerr << "Usage: raft_node <port> <peer_port> <role> <weight>\n";
        return 1;
    }

    short port = std::atoi(argv[1]);
    short peer_port = std::atoi(argv[2]);
    std::string role = argv[3];
    int weight = std::atoi(argv[4]);

    RaftServer server;
    server.AddNode(port, role, weight);

    if (peer_port != 0) {
        server.ConnectNodes("127.0.0.1", peer_port);
    }

    server.Run();

    return 0;

}