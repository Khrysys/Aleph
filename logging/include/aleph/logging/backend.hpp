#include <vector>
#include <functional>
#include <atomic>
#include <thread>
#include <chrono>

#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include "frontend.hpp"

class Backend {
public:
    Backend(const char* filename, size_t mmap_size = 1 << 26) // 64MB
        : running_(false),
          mmap_size_(mmap_size),
          write_offset_(0)
    {
        fd_ = open(filename, O_RDWR | O_CREAT, 0644);
        ftruncate(fd_, mmap_size_);

        buffer_ = static_cast<char*>(
            mmap(nullptr, mmap_size_,
                 PROT_READ | PROT_WRITE,
                 MAP_SHARED, fd_, 0));
    }

    ~Backend() {
        munmap(buffer_, mmap_size_);
        close(fd_);
    }

    template <size_t Capacity>
    void add_frontend(Frontend<Capacity>* f) {
        frontends_.emplace_back(
            [f](LogEntry& e) { return f->pop(e); }
        );
    }

    void start() {
        running_.store(true, std::memory_order_release);

        LogEntry entry;

        while (running_.load(std::memory_order_acquire)) {
            bool did_work = false;

            for (auto& pop_fn : frontends_) {
                while (pop_fn(entry)) {
                    write_entry(entry);
                    did_work = true;
                }
            }

            if (!did_work) {
                std::this_thread::sleep_for(std::chrono::microseconds(10));
            }
        }

        drain();
        flush();
    }

    void stop() {
        running_.store(false, std::memory_order_release);
    }

private:
    void write_entry(const LogEntry& e) {
        if (write_offset_ + sizeof(LogEntry) > mmap_size_) {
            flush();
            write_offset_ = 0;
        }

        std::memcpy(buffer_ + write_offset_, &e, sizeof(LogEntry));
        write_offset_ += sizeof(LogEntry);
    }

    void flush() {
        msync(buffer_, mmap_size_, MS_ASYNC);
    }

    void drain() {
        LogEntry entry;
        for (auto& pop_fn : frontends_) {
            while (pop_fn(entry)) {
                write_entry(entry);
            }
        }
    }

private:
    int fd_;
    char* buffer_;
    size_t mmap_size_;
    size_t write_offset_;

    std::atomic<bool> running_;
    std::vector<std::function<bool(LogEntry&)>> frontends_;
};