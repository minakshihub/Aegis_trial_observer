# Aegis Observer Engine
Aegis is a bare-metal C++17 telemetry and anomaly-detection firewall engineered for autonomous AI agent swarms.

# The Architectural Thesis

# The I/O Serialization Tax in Agentic Workflows
Standard enterprise agent architectures fail at the observability layer. When attempting to monitor the non-deterministic state space of probabilistic agents, standard frameworks rely on synchronous, inline logging. This introduces a critical I/O serialization tax.

Attempting to log complex reasoning loops synchronously creates an observer effect: the telemetry itself destroys the compute throughput and alters the latency of the system it is trying to measure.

# The Bare-Metal Solution: Out-of-Process Telemetry
Aegis is a bare-metal C++17 telemetry and anomaly-detection engine engineered to solve this exact physical boundary. It strictly separates the Execution Path from the Diagnostic Path.

Instead of inline blocking, Aegis operates entirely out-of-process via a lock-free shared-memory ring buffer. It passively caches an agent's rolling execution cycles with nanosecond overhead on the happy path. It dumps the causal forensic sequence to disk strictly when an anomaly or execution invariant trips—ensuring zero latency drag on the host application.

Designed to sit invisibly between your application layer and the upstream LLM, Aegis acts as a microsecond-latency circuit breaker. It prevents runaway loops, token bloat, and application deadlocks before they exhaust your API budget or crash your local infrastructure.


# Capabilities & Anomaly Detection
Aegis evaluates real-time telemetry across ##65,536## concurrent agents without introducing network latency or utilizing blocking mutexes. It instantly flags and intercepts the following critical failure states:

# 1. Repetition & Logic Trap 

# Unresolved Loop Traps: 
Detects agents caught in infinite loops, including those attempting to evade detection by slightly permuting tool arguments on every turn.

# Action Thrashing:
 Identifies A-B-A oscillation where an agent rapidly bounces between two actions without making forward progress.

# Zombie Streaks:
Tracks consecutive execution failures (e.g., hallucinated files or broken JSON) to halt agents banging against a failure wall.

# 2. Context Window & Memory Protection

# Context Hemorrhage:
Detects sudden, massive spikes in prompt tokens (e.g., an agent accidentally dumping a massive server log into its scratchpad).

# Data Bombs:
 Aggressively limits and flags maximum-size payload returns to prevent Out-Of-Memory (OOM) application crashes.

# 3. Temporal & Swarm Anomalies

# Cognitive Stalls (Apology Spirals):
Calculates a dynamic time budget to cut off agents that get stuck generating polite conversational text instead of executing required tools.

# Velocity Spikes:
Identifies unthrottled local compute loops (sub-50ms turnarounds) where an agent is spinning CPU cycles without waiting for valid I/O.

# warm Deadlocks:
Detects zero-entropy ping-ponging in multi-agent architectures (e.g., Agent A and Agent B trapped in an infinite query/reject loop).

# Integration: Architecture & Deployment
⚠️ Architecture Note: Trial Harness vs. Production Deployment
# A. True Production Deployment (POSIX Shared Memory IPC)
In a true production deployment, Aegis runs as a completely separate, standalone daemon. We do not use OS-level hooking (eBPF, ptrace) as kernel context switches destroy our 1.4-nanosecond latency target. Instead, the Aegis daemon allocates the MPSC ring buffer in RAM via shm_open/mmap. Your host agent simply #includes our lightweight header and explicitly pushes 64-byte structs into shared memory at your tool-call boundaries, entirely out-of-band. (For production configuration, contact the repository owner).

# B. The 72-Hour Trial Evaluation (Single-Process Harness)
To instantly demonstrate the lock-free math and anomaly detection without forcing you to configure multi-process POSIX IPC locally, the libaegis.a trial binary compiles both the dummy AI agent (producer) and the Aegis core (consumer) into a single executable.

To reduce friction for evaluators, this repository contains a simplified Trial Harness. Please note the critical distinction between testing the trial binary locally and deploying the industrial engine in production.
Integration: Production vs. Trial Architecture
Aegis enforces a strict physical boundary between the Execution Path and the Diagnostic Path. To understand how Aegis achieves 1.4-nanosecond latency, it is critical to understand how the architecture differs between a Production deployment and this local Trial Harness.

# 1. True Production Deployment (POSIX Shared Memory IPC)
In production, Aegis is split into two entirely different executables communicating over a physical RAM segment (shm_open/mmap). The Aegis Observer runs as a completely standalone background daemon.

The AI agent does not initialize the Aegis engine. Instead, the agent's code becomes incredibly lightweight. It simply includes a tiny producer header, connects to the daemon's shared memory mailbox, and pushes a 64-byte struct at its tool-call boundaries.

# Production Code Example:

C++
#include "aegis_producer.hpp" // Lightweight producer header (NO heavy engine)

int main() { 
    // 1. Connect to the shared memory ring the standalone Daemon already created
    AegisProducer producer("/aegis_ring"); 
    
    // ... AI Agent runs its normal code ...

    // 2. The Explicit Instrumentation: 
    // At the tool-call boundary, the agent drops a struct into RAM in 1.4ns
    producer.push_event(tool_call_data); 

    return 0; 
}
Because the heavy engine is entirely out-of-process, if the agent suffers an OOM-kill or SIGKILL, the physical RAM segment remains intact and zero telemetry is lost.

# 2. The 72-Hour Trial Evaluation (Single-Process Harness)
If we required evaluators to configure standalone background daemons and multi-process POSIX IPC locally just to run a 72-hour trial, most would abandon the setup.

To instantly demonstrate the lock-free math and anomaly detection locally, the libaegis.a trial binary deliberately compiles both the producer and the heavy consumer engine into a single executable.

# STEPWISE: Running the Single-Process Trial
Integrating the trial into your local test environment takes three concrete steps:

# 1. Drop the files into your project
Copy two items into your existing test codebase:

The header file (aegis.hpp) into your include folder.

The static binary (libaegis.a) into your lib folder.

# 2. Add two lines of code (Trial Configuration)
Inside your test application's main() function, add these two lines.(Note: As explained above, this object is removed in production).

C++
#include "aegis.hpp"

int main() { 
    // [TRIAL ONLY]: Spins up the Observer consumer in-process for easy evaluation.
    // [PRODUCTION]: This line is deleted. The Observer runs out-of-band.
    Aegis engine; 
    
    // ... Your AI agent test/simulation code runs here unchanged ...
    // In this trial, the engine automatically intercepts test telemetry.

    return 0; // <--- Trial recorder shuts down automatically here (C++ RAII)
}
# 3. Compile with one extra flag
When you compile your project, add -laegis so the compiler stitches the pre-built trial binary into your executable:

Bash
g++ -std=c++17 your_app.cpp -I./include -L./lib -laegis -o your_app


## What happens when it runs 
 AI application will run exactly as it did before, at full speed.

Aegis sits silently in shared memory, capturing execution cycles without slowing down the primary workload.

When the run finishes (or is interrupted), a readable text file named ## aegis_session_report.txt ## appears in the folder. The engineer opens it to see the exact sequence of events leading up to any failure.

# Stopping the Trial & Log Analysis
The Trial Harness relies on C++ RAII (Resource Acquisition Is Initialization) to manage its lifecycle safely.

##Graceful Interruption (Ctrl+C):## If you need to halt the AI application mid-way, simply press Ctrl+C in your terminal. The Aegis destructor automatically triggers a graceful shutdown: it halts new ingestion, safely joins the background threads, and flushes all telemetry to the ledger.

##Forceful Termination (kill -9):## As engineered in the production architecture, even under a hard SIGKILL or Segfault, zero telemetry is lost. The physical RAM segment remains intact for the daemon to drain.

# 📄 Telemetry Output: aegis_session_report.txt
Upon the completion of every run (or after an interruption), Aegis translates the raw binary ledger into parsed events and generates a fresh, human-readable log file in your execution directory named ##aegis_session_report.txt##

Always ##review## this file after a run. It contains the exact causal forensic sequence leading up to any failure and appends an Anomaly Flag to highlight errant behavior.

# B. True Production Deployment (POSIX Shared Memory IPC)
In a true production deployment, Aegis runs as a completely separate, standalone out-of-process daemon.

We do not use OS-level hooking (eBPF, ptrace, LD_PRELOAD) as kernel context switches destroy our 1.4-nanosecond latency target. Instead, we trade kernel-level visibility for extreme user-space speed using explicit instrumentation over POSIX Shared Memory.

# The Observer (Standalone Daemon): The Aegis background process allocates the MPSC ring buffer in RAM via shm_open and mmap.

The Producer (Host AI Agent): You do not initialize Aegis inside your main(). You simply #include our lightweight producer header. At your tool-call boundaries, your host agent uses our lock-free push functions to pad a struct to 64 bytes, drop it into the shared memory segment, and immediately move on.

For instructions on configuring the production POSIX IPC Observer daemon, please contact the repository owner.

# Failure Modes & Crash Guarantees
Because production telemetry lives entirely in POSIX shared memory, it exists independently of your host application's lifecycle.

If your AI agent suffers an abrupt forced termination (e.g., an OOM-kill, Segfault, or SIGKILL), the OS reclaims the agent's process space, but the physical RAM segment remains intact. The out-of-process Aegis daemon simply drains the remaining bytes from the buffer. Zero telemetry is lost during an unexpected host crash.

For graceful shutdowns during the Trial evaluation, Aegis relies on C++ RAII. Catching an exit signal (like Ctrl+C) safely joins the background threads and flushes remaining data to the binary ledger before exiting.

Upon the completion of a run (or after a shutdown), Aegis translates the raw binary ledger into parsed agent events and generates aegis_session_report.txt, appending Anomaly Flags to highlight errant AI behavior.

# Diagnostic Legend (Anomaly Flags)
Always review this file after a run. It translates the raw binary ledger into parsed agent events and appends an Anomaly Flag to highlight errant AI behavior.


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


# Stopping the Trial Mid-Way (Shutdown Procedures)
The Trial Harness relies on C++ RAII (Resource Acquisition Is Initialization) to manage its lifecycle safely.

Graceful Interruption (Ctrl+C): If you need to halt the AI application mid-way, simply press Ctrl+C in your terminal. The Aegis destructor automatically triggers a graceful shutdown sequence: it halts new RingBuffer ingestion, safely joins the background threads, and flushes all remaining telemetry to the ledger before the application exits.

Forceful Termination (kill -9): As detailed in the production architecture, even under a hard SIGKILL or Segfault, zero telemetry is lost. The physical RAM segment remains intact for the daemon to drain.
No data is lost during an unexpected mid-way termination.

It signals the RingBuffer to halt new ingestion.

It safely joins the VfsWorker and PassiveObserver background threads.

It flushes all remaining telemetry data to the binary ledger before the application exits.



# Telemetry Output & Log Analysis
Upon the completion of every run (or after an interruption), Aegis automatically translates the raw binary ledger into parsed agent events and generates a fresh, human-readable log file in your execution directory:

# 📄 aegis_session_report.txt

Always review this file after a run. It contains the exact causal forensic sequence leading up to any failure and appends an Anomaly Flag to highlight errant AI behavior.

# ⚠️ Evaluation Notice: 72-Hour Time Lock
This repository contains the Trial Edition of the Aegis Industrial Core. The libaegis.a binary provided in this repository is cryptographically time-locked. It contains the complete, unrestricted feature set of the industrial engine, but will automatically permanently halt execution exactly 72 hours after its compilation timestamp.

# For the permanent industrial license and production POSIX deployment architecture, please contact the repository owner.

# License Proprietary and Confidential | Copyright (c) 2026. All Rights Reserved.
# This repository and its contents are strictly proprietary. Unauthorized copying, modification, distribution, or execution of this software, via any medium, is strictly prohibited without an explicit commercial license agreement.


















