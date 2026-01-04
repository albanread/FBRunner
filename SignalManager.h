// =============================================================================
// SignalManager.h
// =============================================================================
// Cross-platform signal handling for FasterBASIC script interruption
// Provides safe, reliable script termination via Ctrl+C (SIGINT) and other signals

#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <thread>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <signal.h>
    #include <unistd.h>
#endif

namespace FasterBASIC {

/// Signal types that can be handled
enum class SignalType {
    INTERRUPT = 1,  // SIGINT (Ctrl+C)
    TERMINATE = 2,  // SIGTERM (system shutdown)
    USER1 = 3       // SIGUSR1 (custom debug signal)
};

/// Signal handler callback function type
using SignalCallback = std::function<void(SignalType signalType)>;

/// Signal manager for safe script interruption
class SignalManager {
public:
    SignalManager();
    ~SignalManager();

    /// Initialize signal handling
    /// @return true if initialization succeeded, false otherwise
    bool initialize();

    /// Shutdown signal handling and cleanup resources
    void shutdown();

    /// Check if signals are properly initialized
    /// @return true if signal handling is active
    bool isInitialized() const { return m_initialized; }

    /// Set callback function for signal events
    /// @param callback Function to call when signals are received
    void setSignalCallback(SignalCallback callback);

    /// Check if a signal has been received
    /// @return true if a signal is pending
    bool hasSignal() const;

    /// Get the type of the most recent signal
    /// @return SignalType of pending signal, or INTERRUPT if none
    SignalType getSignalType() const;

    /// Clear any pending signals
    void clearSignal();

    /// Manually trigger a signal (for testing)
    /// @param signalType Type of signal to simulate
    void simulateSignal(SignalType signalType);

    /// Start background monitoring thread
    /// @param pollIntervalMs How often to check for signals (default: 10ms)
    void startMonitoring(int pollIntervalMs = 10);

    /// Stop background monitoring thread
    void stopMonitoring();

    /// Get signal statistics (for debugging)
    struct Statistics {
        uint64_t signalsReceived = 0;
        uint64_t callbacksCalled = 0;
        uint64_t pollCycles = 0;
        bool monitoringActive = false;
    };
    Statistics getStatistics() const { return m_stats; }

private:
    // Signal-safe atomic flags
    static std::atomic<bool> s_signalReceived;
    static std::atomic<int> s_signalType;
    static SignalManager* s_instance; // For static signal handler access

    // Instance state
    bool m_initialized;
    bool m_shouldExit;
    SignalCallback m_callback;
    std::unique_ptr<std::thread> m_monitorThread;
    mutable Statistics m_stats;

#ifdef _WIN32
    // Windows-specific
    static BOOL WINAPI consoleHandler(DWORD dwCtrlType);
    HANDLE m_consoleHandle;
#else
    // POSIX-specific
    static void signalHandler(int signum);
    struct sigaction m_oldSigintAction;
    struct sigaction m_oldSigtermAction;
    struct sigaction m_oldSigusr1Action;
    
    // Self-pipe for signal-safe communication
    int m_signalPipe[2];
    bool m_pipeCreated;
#endif

    // Platform-specific initialization
    bool initializePlatform();
    void shutdownPlatform();
    
    // Signal monitoring thread function
    void monitoringThreadFunc(int pollIntervalMs);
    
    // Internal signal handling
    void handleSignalInternal(SignalType signalType);
    
    // Disable copy/move
    SignalManager(const SignalManager&) = delete;
    SignalManager& operator=(const SignalManager&) = delete;
    SignalManager(SignalManager&&) = delete;
    SignalManager& operator=(SignalManager&&) = delete;
};

/// RAII helper for signal management
class SignalHandler {
public:
    SignalHandler();
    ~SignalHandler();

    /// Initialize with callback
    /// @param callback Function to call on signal reception
    /// @return true if successfully initialized
    bool setup(SignalCallback callback);

    /// Check if handler is active
    bool isActive() const;

    /// Get access to underlying signal manager
    SignalManager* getManager() { return m_manager.get(); }

private:
    std::unique_ptr<SignalManager> m_manager;
};

} // namespace FasterBASIC

// Convenience macros for logging (assuming LOG_* macros are available)
#define SIGNAL_LOG_INFO(msg) LOG_INFO("[SignalManager] " msg)
#define SIGNAL_LOG_INFOF(fmt, ...) LOG_INFOF("[SignalManager] " fmt, __VA_ARGS__)
#define SIGNAL_LOG_ERROR(msg) LOG_ERROR("[SignalManager] " msg)
#define SIGNAL_LOG_ERRORF(fmt, ...) LOG_ERRORF("[SignalManager] " fmt, __VA_ARGS__)
#define SIGNAL_LOG_DEBUG(msg) LOG_DEBUG("[SignalManager] " msg)
#define SIGNAL_LOG_DEBUGF(fmt, ...) LOG_DEBUGF("[SignalManager] " fmt, __VA_ARGS__)