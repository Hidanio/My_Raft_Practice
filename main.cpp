#include <iostream>
#include "src/Raft_server.h"


struct config {
    short port;
};

std::vector<short> SplitStringByComma(const std::string& str) {
    std::vector<short> result;
    std::stringstream ss(str);
    std::string item;

    while (std::getline(ss, item, ',')) {
        result.push_back(std::atoi(item.c_str()));
    }

    return result;
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: raft_node <port> <peer_ports_comma_sep>\n";
        return 1;
    }

    short port = std::atoi(argv[1]);
 //   short peer_port = std::atoi(argv[2]);
    auto ports = SplitStringByComma(argv[2]);


    RaftServer server;
    server.AddNode(port);

    if (ports.size() != 1 || ports[0] != 0) {
        server.ConnectNodes("127.0.0.1", ports);
    }

    server.Run();

    return 0;

}