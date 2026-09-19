# Aegis Observer Core

A True Zero-Configuration C++ telemetry and diagnostic engine for Agentic AI. 

The Observer Core passively monitors AI agent behavior, flags diagnostic anomalies in real-time, and generates binary ledgers on isolated background threads without blocking or degrading the host application's main thread.

## Architecture & Philosophy

**The Why:** Modern Agentic AI systems lack standardized, low-overhead observability. Traditional logging requires manual path configurations, explicit file handling, and heavy main-thread synchronization. Aegis was built to eliminate this friction.

**The What (True Zero-Configuration):** The engine is entirely self-sufficient. It internally initializes a lock-free `RingBuffer`, an autonomous `VfsWorker`, and a `PassiveObserver`. It manages its own memory, thread lifecycle, and file I/O operations autonomously. 

## Integration & Build Instructions

Aegis is built using standard CMake. It is designed to be linked statically to the host AI application.

### Building the Industrial Engine
```bash
mkdir build && cd build
cmake ..
make


(Note: A 72-hour time-locked trial build can be triggered via cmake -DBUILD_TRIAL=ON ..)

Running the Engine
Simply include the header and instantiate the engine at the start of your host application. No file paths or thread management required.

#include "aegis.hpp"

int main() {
    // 1. Ignite the engine
    Aegis engine; 

    // 2. Host AI application logic runs here...
    
    return 0;
}

Execution & Graceful Shutdown (Stopping Mid-Way)
The engine relies on C++ RAII (Resource Acquisition Is Initialization) to manage its lifecycle safely.

If you need to halt the AI application mid-way (e.g., pressing Ctrl+C or a sudden process interrupt), the Aegis destructor automatically triggers a graceful shutdown sequence:

It signals the RingBuffer to halt new ingestion.

It safely joins the VfsWorker and PassiveObserver background threads.

It flushes all remaining telemetry data to the binary ledger before the application exits.

No data is lost during an unexpected mid-way termination.


Telemetry & Log Analysis
Upon the completion of every run (or after a graceful shutdown), Aegis automatically generates a fresh, human-readable log file in the execution directory: aegis_session_report.txt.

Always review this file after a run. It translates the raw binary ledger into parsed agent events and appends an Anomaly Flag to highlight errant AI behavior.

### Diagnostic Legend (Anomaly Flags)
* **0**    : `STATUS_HEALTHY` - Normal operation
* **1**    : `LOOP_UNRESOLVED_TRAP` - Agent repeating exact actions/permutations
* **2**    : `COGNITIVE_STALL` - Agent stuck in text-generation/apology loop
* **4**    : `VELOCITY_SPIKE` - Actions occurring faster than physical limits
* **8**    : `APP_IDLE_STALL` - Dead air, timeouts, or API 429 Rate Limits
* **16**   : `CONTEXT_HEMORRHAGE` - Sudden massive spike or drop in tokens
* **32**   : `ACTION_THRASHING` - Agent bouncing between two states (A-B-A)
* **64**   : `ZOMBIE_STREAK` - Consecutive standard application errors
* **128**  : `PHANTOM_STRIKEOUT` - Hallucinated tools or out-of-order execution
* **256**  : `SLOW_BOIL_CREEP` - Derivative token waste over time
* **512**  : `READ_ONLY_SPIRAL` - Endless reading/researching without action
* **1024** : `SWARM_DEADLOCK` - Multi-agent infinite ping-pong loop
* **2048** : `PAYLOAD_FLOOD` - Sudden massive data ingestion/output spike

License
Proprietary and Confidential
Copyright (c) 2026. All Rights Reserved.

This repository and its contents are strictly proprietary. Unauthorized copying, modification, distribution, or execution of this software, via any medium, is strictly prohibited without an explicit commercial license agreement.