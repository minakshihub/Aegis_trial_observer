# Aegis Observer Engine: 72-Hour Evaluation Edition

Aegis is a bare-metal C++17 telemetry and anomaly-detection firewall engineered for autonomous AI agent swarms. 

Designed to sit invisibly between your application layer and the upstream LLM, Aegis acts as a microsecond-latency circuit breaker. It prevents runaway loops, token bloat, and application deadlocks before they exhaust your API budget or crash your local infrastructure.

### ⚠️ Evaluation Notice: 72-Hour Time Lock
This repository contains the **Trial Edition** of the Aegis Industrial Core. The `libaegis.a` binary provided in this repository is cryptographically time-locked. It contains the complete, unrestricted feature set of the industrial engine, but will automatically permanently halt execution exactly **72 hours** after its compilation timestamp. 

For the permanent industrial license and source integration, please contact the repository owner.

---

## Capabilities & Anomaly Detection

Aegis evaluates real-time telemetry across 65,536 concurrent agents without introducing network latency or utilizing blocking mutexes. It instantly flags and intercepts the following critical failure states:

### 1. Repetition & Logic Traps
* **Unresolved Loop Traps:** Detects agents caught in infinite loops, including those attempting to evade detection by slightly permuting tool arguments on every turn.
* **Action Thrashing:** Identifies A-B-A oscillation where an agent rapidly bounces between two actions without making forward progress.
* **Zombie Streaks:** Tracks consecutive execution failures (e.g., hallucinated files or broken JSON) to halt agents banging against a failure wall.

### 2. Context Window & Memory Protection
* **Context Hemorrhage:** Detects sudden, massive spikes in prompt tokens (e.g., an agent accidentally dumping a massive server log into its scratchpad).
* **Data Bombs:** Aggressively limits and flags maximum-size payload returns to prevent Out-Of-Memory (OOM) application crashes.

### 3. Temporal & Swarm Anomalies
* **Cognitive Stalls (Apology Spirals):** Calculates a dynamic time budget to cut off agents that get stuck generating polite conversational text instead of executing required tools.
* **Velocity Spikes:** Identifies unthrottled local compute loops (sub-50ms turnarounds) where an agent is spinning CPU cycles without waiting for valid I/O.
* **Swarm Deadlocks:** Detects zero-entropy ping-ponging in multi-agent architectures (e.g., Agent A and Agent B trapped in an infinite query/reject loop).

---

## Integration

Aegis is distributed as a pre-compiled static library to ensure zero dependencies and instantaneous integration into your C++ stack.

1. Include the headers from the `include/` directory.
2. Link against the time-locked binary in the `lib/` directory:
   ```bash
   g++ -std=c++17 your_app.cpp -I./include -L./lib -laegis -o your_app


(Note: A 72-hour time-locked trial build is triggered)

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