#include <aleph/platform.hpp>

#include <atomic>
#include <cstdint>
#include <thread>
#include <vector>
#include <unordered_set>
#include <mutex>

using namespace aleph::platform::allocation;

// ===== Config =====
static constexpr size_t MAX_OBJECTS = 1024;
static constexpr size_t MAX_THREADS = 8;

// ===== Operation Encoding =====
enum class OpType : uint8_t {
    Allocate = 0,
    Copy     = 1,
    Destroy  = 2,
    Assign   = 3,
    PushChar = 4
};

struct Op {
    OpType type;
    uint16_t a;
    uint16_t b;
    char c;
};

// ===== Global Debug State =====
struct DebugState {
    std::mutex mtx;
    std::unordered_set<const void*> activePtrs;
    std::atomic<bool> failed{false};

    void insert(const void* p) {
        std::lock_guard<std::mutex> lock(mtx);
        if (!activePtrs.insert(p).second) {
            failed = true; // duplicate slot detected
        }
    }

    void erase(const void* p) {
        std::lock_guard<std::mutex> lock(mtx);
        activePtrs.erase(p);
    }
};

// ===== Worker =====
void run_worker(const std::vector<Op>& ops,
                StringArena* arena,
                DebugState* dbg)
{
    std::vector<NUMAString> objects;
    objects.reserve(MAX_OBJECTS);

    for (const auto& op : ops) {
        if (dbg->failed.load(std::memory_order_relaxed)) return;

        try {
            switch (op.type) {

            case OpType::Allocate: {
                if (objects.size() >= MAX_OBJECTS) break;

                auto s = arena->allocate();
                dbg->insert(s.c_str());
                objects.emplace_back(std::move(s));
                break;
            }

            case OpType::Copy: {
                if (objects.empty()) break;
                auto idx = op.a % objects.size();
                objects.emplace_back(objects[idx]);
                break;
            }

            case OpType::Assign: {
                if (objects.size() < 2) break;
                auto a = op.a % objects.size();
                auto b = op.b % objects.size();
                objects[a] = objects[b];
                break;
            }

            case OpType::Destroy: {
                if (objects.empty()) break;
                auto idx = op.a % objects.size();

                dbg->erase(objects[idx].c_str());
                objects.erase(objects.begin() + idx);
                break;
            }

            case OpType::PushChar: {
                if (objects.empty()) break;
                auto idx = op.a % objects.size();
                objects[idx].push_back(op.c);
                break;
            }

            default:
                break;
            }
        } catch (...) {
            // Allocation failure is allowed ONLY if arena exhausted
            // but repeated unexpected failures are suspicious
            dbg->failed = true;
            return;
        }
    }

    // cleanup
    for (auto& obj : objects) {
        dbg->erase(obj.c_str());
    }
}

// ===== Input decoding =====
std::vector<Op> decode_ops(const uint8_t* data, size_t size)
{
    std::vector<Op> ops;

    for (size_t i = 0; i + 4 < size; i += 5) {
        Op op;
        op.type = static_cast<OpType>(data[i] % 5);
        op.a    = (data[i+1] << 8) | data[i+2];
        op.b    = (data[i+3] << 8) | data[i+4];
        op.c    = static_cast<char>(data[i]);

        ops.push_back(op);
    }

    return ops;
}

// ===== Entry Point =====
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < 16) return 0;

    auto ops = decode_ops(data, size);

    Allocation alloc(detail::STRING_ALLOCATION_SIZE, 0);
    StringArena arena(&alloc);

    DebugState dbg;

    size_t threadCount = (data[0] % MAX_THREADS) + 1;

    std::vector<std::thread> threads;

    for (size_t t = 0; t < threadCount; ++t) {
        threads.emplace_back(run_worker, std::cref(ops), &arena, &dbg);
    }

    for (auto& t : threads) {
        t.join();
    }

    // ===== Final invariants =====
    if (dbg.failed.load()) {
        __builtin_trap(); // crash = bug found
    }

    return 0;
}