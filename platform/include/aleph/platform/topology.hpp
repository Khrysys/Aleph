#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include <libassert/assert.hpp>

namespace aleph::platform::topology {
    class LocalMemory;
    class MCTSTree;
    class PolicyCache;
    class ThreadGroup;

    class Accelerator {  // Owned by PolicyCache}
        private:
            PolicyCache* pPolicyCache;
    };

    class LocalMemory {  // Owned by Topology
        public:
            LocalMemory(std::size_t node);

            [[nodiscard]] auto getBytes() const { return bytes; }

        private:
            std::vector<MCTSTree> mctsTrees;
            PolicyCache policyCache;

            std::size_t node;
            std::size_t bytes;
    };

    class MCTSTree {  // Owned by LocalMemory
        private:
            ThreadGroup threadGroup;
            LocalMemory* pLocalMemory;
    };

    class PolicyCache {  // Owned by LocalMemory
        private:
            LocalMemory* localMemory;
            std::vector<ThreadGroup*> pThreadGroups;
            std::vector<Accelerator> pAccelerators;
    }

    class ThreadGroup {  // Owned by MCTSTree
        private:
            MCTSTree* pMCTSTree;
            std::vector<PolicyCache*> pPolicyCaches;
    };

    class Topology {
        private:
            std::vector<LocalMemory> localMemory;
    };
}  // namespace aleph::platform::topology