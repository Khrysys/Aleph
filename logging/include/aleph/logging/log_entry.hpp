#pragma once

#include <cstdint>
#include <cstring>

constexpr size_t LOG_MSG_SIZE = 96;

struct LogEntry {
    uint64_t timestamp;
    uint32_t thread_id;
    char message[LOG_MSG_SIZE];

    void set(const char* msg, uint64_t ts, uint32_t tid) {
        timestamp = ts;
        thread_id = tid;

        std::strncpy(message, msg, LOG_MSG_SIZE - 1);
        message[LOG_MSG_SIZE - 1] = '\0';
    }
};