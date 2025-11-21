// Copyright 2024
// Message structure definitions for TCP communication

#ifndef MESSAGE_H_
#define MESSAGE_H_

#include <cstdint>
#include <vector>

namespace net {

// Header structure for incoming messages
struct MessageHeader {
  uint64_t position;  // Opaque position identifier
  uint32_t length;    // Length of following data
} __attribute__((packed));

// Complete message with header and data
struct Message {
  MessageHeader header;
  std::vector<uint8_t> data;
};

}  // namespace net

#endif  // MESSAGE_H_
