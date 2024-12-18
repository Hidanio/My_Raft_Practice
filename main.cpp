#include <iostream>
#include "src/Raft_server.h"

int main(int argc, char* argv[]) {
    if (argc < 1) {
        std::cerr << "Usage: raft_node <port> [peer_port1 peer_port2 ...]\n"; // TODO: rewrite
        return 1;
    }

    short port = std::atoi(argv[1]);
    std::vector<short> peer_ports;
    for (int i = 2; i < argc; ++i) {
        peer_ports.push_back(std::atoi(argv[i]));
    }

    std::vector<Peers_to_connect> peers_to_connect{
            Peers_to_connect("127.0.0.1",5001),
            Peers_to_connect("127.0.0.1",5002),
            Peers_to_connect("127.0.0.1",5003),
    };

    peers_to_connect.erase(
            std::remove_if(peers_to_connect.begin(), peers_to_connect.end(),
                           [port](const Peers_to_connect& peer) {
                               return peer.port == port;
                           }),
            peers_to_connect.end()
    );


    try {
        RaftServer server(port);
        server.ConnectToPeers(peers_to_connect);
        server.Run();
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}