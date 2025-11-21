// Copyright 2024
// TCP message sender (synchronous) using boost::asio

#include <boost/asio.hpp>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <vector>

#include "message.h"

namespace net {

using boost::asio::ip::tcp;

class Sender {
 public:
  Sender(boost::asio::io_context& io_context,
         const std::string& host, uint16_t port)
      : socket_(io_context) {
    tcp::resolver resolver(io_context);
    auto endpoints = resolver.resolve(host, std::to_string(port));
    boost::asio::connect(socket_, endpoints);
    std::cout << "Connected to " << host << ":" << port << std::endl;
  }

  void Send(uint64_t position, const void* data, uint32_t length) {
    // Prepare header
    MessageHeader header;
    header.position = position;
    header.length = length;

    // Send header
    boost::asio::write(socket_,
                       boost::asio::buffer(&header, sizeof(MessageHeader)));

    // Send data
    boost::asio::write(socket_,
                       boost::asio::buffer(data, length));
  }

 private:
  tcp::socket socket_;
};

}  // namespace net

int main(int argc, char* argv[]) {
  try {
    std::string host = "localhost";
    uint16_t port = 8080;
    uint64_t num_messages = 1000000;  // 1 million messages by default

    if (argc > 1) {
      host = argv[1];
    }
    if (argc > 2) {
      port = static_cast<uint16_t>(std::stoi(argv[2]));
    }
    if (argc > 3) {
      num_messages = std::stoull(argv[3]);
    }

    boost::asio::io_context io_context;
    net::Sender sender(io_context, host, port);

    // Sample data to send
    const char* payload = "Hello, World!";
    uint32_t payload_len = static_cast<uint32_t>(std::strlen(payload));

    std::cout << "Sending " << num_messages << " messages..." << std::endl;

    auto start = std::chrono::high_resolution_clock::now();

    for (uint64_t i = 0; i < num_messages; ++i) {
      sender.Send(i, payload, payload_len);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        end - start).count();

    double rate = (duration > 0)
        ? (static_cast<double>(num_messages) / duration * 1000.0)
        : 0.0;

    std::cout << "Sent " << num_messages << " messages in "
              << duration << " ms (" << rate << " msg/s)" << std::endl;

  } catch (const std::exception& e) {
    std::cerr << "Exception: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
