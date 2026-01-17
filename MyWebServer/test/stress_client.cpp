// stress_client.cpp - simple multithreaded blocking client
// Usage: g++ -std=c++11 -O2 -pthread stress_client.cpp -o stress_client
// ./stress_client 127.0.0.1 8888 1000 4096

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <atomic>

using namespace std::chrono;

struct Result { bool ok; double latency; size_t received; std::string err; };

void worker(const std::string& host, int port, size_t size, int idx, Result &res) {
    auto start = high_resolution_clock::now();
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) { res = {false, 0.0, 0, "socket() failed"}; return; }
    sockaddr_in serv{};
    serv.sin_family = AF_INET;
    serv.sin_port = htons(port);
    if (inet_pton(AF_INET, host.c_str(), &serv.sin_addr) <= 0) { close(sock); res = {false,0.0,0,"inet_pton failed"}; return; }
    if (connect(sock, (sockaddr*)&serv, sizeof(serv)) < 0) { close(sock); res = {false,0.0,0,std::strerror(errno)}; return; }
    std::vector<char> payload(size, 'A');
    ssize_t s = send(sock, payload.data(), payload.size(), 0);
    if (s <= 0) { close(sock); res = {false,0.0,0,std::strerror(errno)}; return; }
    size_t total = 0;
    while (total < payload.size()) {
        char buf[4096];
        ssize_t r = recv(sock, buf, sizeof(buf), 0);
        if (r <= 0) break;
        total += r;
    }
    close(sock);
    auto end = high_resolution_clock::now();
    double lat = duration_cast<duration<double>>(end - start).count();
    res = { true, lat, total, "" };
}

int main(int argc, char** argv) {
    if (argc < 5) {
        std::cerr << "Usage: " << argv[0] << " host port connections size\n";
        return 1;
    }
    std::string host = argv[1];
    int port = std::stoi(argv[2]);
    int connections = std::stoi(argv[3]);
    int size = std::stoi(argv[4]);

    std::vector<std::thread> threads;
    std::vector<Result> results(connections);
    threads.reserve(connections);

    auto t0 = high_resolution_clock::now();
    for (int i = 0; i < connections; ++i) {
        threads.emplace_back(worker, host, port, (size_t)size, i, std::ref(results[i]));
    }
    for (auto &t : threads) if (t.joinable()) t.join();
    auto t1 = high_resolution_clock::now();
    double total_time = duration_cast<duration<double>>(t1 - t0).count();

    int succ = 0; size_t bytes = 0; double sumlat = 0; double maxlat = 0;
    for (auto &r : results) {
        if (r.ok) { succ++; sumlat += r.latency; if (r.latency > maxlat) maxlat = r.latency; bytes += r.received; }
    }
    std::cout << "Requested: " << connections << " succeeded: " << succ << " failed: " << (connections - succ) << "\n";
    if (succ) {
        std::cout << "avg latency: " << (sumlat / succ) << " s, max latency: " << maxlat << " s\n";
    }
    std::cout << "total time: " << total_time << " s, total bytes received: " << bytes << "\n";
    return 0;
}
