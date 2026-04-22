#include <cstdint>
#include <cstddef>
#include <vector>

#include <aleph/platform/allocation/fifo_queue.hpp>

using aleph::platform::allocation::FIFOQueue;

// Simple byte-stream interpreter:
// 0x00–0x7F -> push(value)
// 0x80      -> pop()
// others     -> ignored

extern "C" auto LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) -> int {
    FIFOQueue<int, 32> q{}; // NOTE: assumes trivial initialization (may still be UB in current impl)

    for (size_t i = 0; i < size; ++i) {
        uint8_t const op = data[i];

        try {
            if ((op & 0x80) == 0) {
                // push
                q.push(static_cast<int>(op));
            } else {
                // pop
                (void)q.pop();
            }
        } catch (...) {
            // pop on empty queue or any internal issue is allowed for fuzzing
        }
    }

    return 0;
}