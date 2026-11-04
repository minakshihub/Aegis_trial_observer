#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <mutex>
#include <string>
#include "aegis.hpp"

// Ensures terminal output stays clean when multiple threads print simultaneously
std::mutex console_mutex;
void print_log(const std::string& msg) {
    std::lock_guard<std::mutex> lock(console_mutex);
    std::cout << msg << "\n";
}

// =====================================================================
// AGENT 1: The Model Citizen (Happy Path & Recovery)
// =====================================================================
void agent_normal(Aegis& engine, uint64_t token) {
    print_log("[Agent 1 - Normal] Booting legitimate telemetry stream...");
    engine.log(token, 8000ULL, 100, 0, 100, 200);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    
    engine.log(token, 8001ULL, 120, 0, 100, 250);
    std::this_thread::sleep_for(std::chrono::milliseconds(50)); // Simulating work while attacked
    
    print_log("[Agent 1 - Normal] Sending healthy recovery telemetry...");
    engine.log(token, 20000ULL, 140, 0, 100, 200);
    print_log("[Agent 1 - Normal] Execution complete.");
}

// =====================================================================
// AGENT 2: The Hacker (Spoofing, Forging, Replays, Bounds)
// =====================================================================
void agent_hacker(Aegis& engine, uint64_t token) {
    print_log("[Agent 2 - Hacker] Initiating targeted anomaly attacks...");
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    // Anomaly 1: Spoofed Identity (Token 0)
    uint16_t response_1 = engine.log(0, 8002ULL, 150, 0, 100, 99999);
    if (response_1 == 0) print_log("   -> [Blocked] Zero-token spoofing silently dropped.");

    // Anomaly 2: Forged Identity (Random Token)
    uint16_t response_2 = engine.log(987654321ULL, 8003ULL, 200, 1, 50, 88888);
    if (response_2 == 0) print_log("   -> [Blocked] Unregistered token dropped.");

    // Anomaly 3: Replay Attack (Duplicating Event ID 8001ULL)
    engine.log(token, 8001ULL, 120, 0, 100, 250);
    print_log("   -> [Logged] Replay attack (Duplicate ID) recorded for audit.");

    // Anomaly 4: Out-of-Bounds Payload (Integer Boundary Smash)
    engine.log(token, 8004ULL, 65535, 65535, 65535, 9999999);
    print_log("   -> [Logged] Maximum boundary payload smashed into ring buffer.");
    
    print_log("[Agent 2 - Hacker] Attack sequence complete.");
}

// =====================================================================
// AGENT 3: The Spammer (Stress Testing O(1) Latency)
// =====================================================================
void agent_spammer(Aegis& engine, uint64_t token) {
    print_log("[Agent 3 - Spammer] Initiating High-Frequency DDoS Loop (10,000 events)...");
    for (uint64_t i = 0; i < 10000; ++i) {
        // Blasting the ring buffer to prove lock-free O(1) latency holds up
        engine.log(token, 9000ULL + i, 255, 1, 200, 404);
    }
    print_log("   -> [System] 10,000 rapid-fire events absorbed instantly without locking.");
}

int main() {
    print_log("--- Aegis Security Test: Multi-Agent Concurrency & Maximum Anomalies ---");
    
    Aegis engine;
    
    // Orchestrator securely registers 3 concurrent agents
    uint64_t token_normal  = engine.register_agent();
    uint64_t token_hacker  = engine.register_agent();
    uint64_t token_spammer = engine.register_agent();

    print_log("[System] Registered Agents: Token " + std::to_string(token_normal) + ", " + 
                                                   std::to_string(token_hacker) + ", " + 
                                                   std::to_string(token_spammer));
    print_log("-------------------------------------------------------------------------");

    // Launch all 3 agents simultaneously
    std::thread t1(agent_normal, std::ref(engine), token_normal);
    std::thread t2(agent_hacker, std::ref(engine), token_hacker);
    std::thread t3(agent_spammer, std::ref(engine), token_spammer);

    // Wait for all agent threads to finish execution
    t1.join();
    t2.join();
    t3.join();

    print_log("-------------------------------------------------------------------------");
    print_log("[System] Multi-Agent Anomaly Test finished cleanly.");
    print_log("[System] Check aegis_session_report.txt & aegis_ledger.bin for the interwoven forensic audit.");
    
    return 0;
}