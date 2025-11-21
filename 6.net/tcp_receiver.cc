// Copyright 2024
// TCP message receiver using boost::asio

#include <boost/asio.hpp>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <memory>
#include <vector>

#include "message.h"

namespace net {

using boost::asio::ip::tcp;

class Session : public std::enable_shared_from_this<Session> {
 public:
  explicit Session(tcp::socket socket) : socket_(std::move(socket)) {}

  void Start() { ReadHeader(); }

 private:
  void ReadHeader() {
    auto self = shared_from_this();
    boost::asio::async_read(
        socket_,
        boost::asio::buffer(&message_.header, sizeof(MessageHeader)),
        [this, self](boost::system::error_code ec, std::size_t /*length*/) {
          if (!ec) {
            ReadData();
          } else if (ec != boost::asio::error::eof) {
            std::cerr << "Error reading header: " << ec.message() << std::endl;
          }
        });
  }

  void ReadData() {
    message_.data.resize(message_.header.length);
    auto self = shared_from_this();
    boost::asio::async_read(
        socket_,
        boost::asio::buffer(message_.data.data(), message_.header.length),
        [this, self](boost::system::error_code ec, std::size_t /*length*/) {
          if (!ec) {
            PrintMessage();
            ReadHeader();  // Continue reading next message
          } else if (ec != boost::asio::error::eof) {
            std::cerr << "Error reading data: " << ec.message() << std::endl;
          }
        });
  }

  void PrintMessage() {
    std::cout << "Position: " << message_.header.position
              << ", Length: " << message_.header.length
              << ", Data: ";

    // Print data as hex
    for (const auto& byte : message_.data) {
      std::cout << std::hex << std::setw(2) << std::setfill('0')
                << static_cast<int>(byte);
    }
    std::cout << std::dec << std::endl;
  }

  tcp::socket socket_;
  Message message_;
};

class Server {
 public:
  Server(boost::asio::io_context& io_context, uint16_t port)
      : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)) {
    std::cout << "Server listening on port " << port << std::endl;
    Accept();
  }

 private:
  void Accept() {
    acceptor_.async_accept(
        [this](boost::system::error_code ec, tcp::socket socket) {
          if (!ec) {
            std::cout << "New connection from "
                      << socket.remote_endpoint().address().to_string()
                      << std::endl;
            std::make_shared<Session>(std::move(socket))->Start();
          } else {
            std::cerr << "Accept error: " << ec.message() << std::endl;
          }
          Accept();  // Continue accepting new connections
        });
  }

  tcp::acceptor acceptor_;
};

}  // namespace net

int main(int argc, char* argv[]) {
  try {
    uint16_t port = 8080;
    if (argc > 1) {
      port = static_cast<uint16_t>(std::stoi(argv[1]));
    }

    boost::asio::io_context io_context;
    net::Server server(io_context, port);
    io_context.run();
  } catch (const std::exception& e) {
    std::cerr << "Exception: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
