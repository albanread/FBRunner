// =============================================================================
// SignalManager.cpp
// =============================================================================
// Cross-platform signal handling implementation for FasterBASIC script interruption
// Provides safe, reliable script termination via Ctrl+C (SIGINT) and other signals

#include "SignalManager.h"
#include <chrono>
#include <cstring>
#include <iostream>

#ifdef _WIN32
    #include <io.h>
    #include <fcntl.h>
#else
    #include <sys/select.h>
    #include <errno.h>
#endif

// Include logging if available, otherwise fallback to cout
#ifndef LOG_INFO
    #define LOG_INFO(msg) std::cout << "[INFO] " << msg << std::endl
    #define LOG_INFOF(fmt, ...) printf("[INFO] " fmt "\n", __VA_ARGS__)
    #define LOG_ERROR(msg) std::cerr << "[ERROR] " << msg << std::endl
    #define LOG_ERRORF(fmt, ...) printf("[ERROR] " fmt "\n", __VA_ARGS__)
    #define LOG_DEBUG(msg) std::cout << "[DEBUG] " << msg << std::endl
    #define LOG_DEBUGF(fmt, ...) printf("[DEBUG] " fmt "\n", __VA_ARGS__)
#endif

namespace FasterBASIC {

// =============================================================================
// Static members
// =============================================================================

std::atomic<bool> SignalManager::s_signalReceived{false};
std::atomic<int> SignalManager::s_signalType{0};
SignalManager* SignalManager::s_instance = nullptr;

// =============================================================================
// Constructor/Destructor
// =============================================================================

SignalManager::SignalManager() 
    : m_initialized(false)
    , m_shouldExit(false)
    , m_callback(nullptr)
    , m_monitorThread(nullptr)
    , m_stats{}
#ifdef _WIN32
    , m_consoleHandle(INVALID_HANDLE_VALUE)
#else
    , m_pipeCreated(false)
#endif
{
    s_instance = this;
    
#ifndef _WIN32
    m_signalPipe[0] = -1;
    m_signalPipe[1] = -1;
    memset(&m_oldSigintAction, 0, sizeof(m_oldSigintAction));
    memset(&m_oldSigtermAction, 0, sizeof(m_oldSigtermAction));
    memset(&m_oldSigusr1Action, 0, sizeof(m_oldSigusr1Action));
#endif

    SIGNAL_LOG_DEBUG("SignalManager constructed");
}

SignalManager::~SignalManager() {
    shutdown();
    s_instance = nullptr;
    SIGNAL_LOG_DEBUG("SignalManager destroyed");
}

// =============================================================================
// Public Interface
// =============================================================================

bool SignalManager::initialize() {
    if (m_initialized) {
        SIGNAL_LOG_DEBUG("Already initialized");
        return true;
    }

    SIGNAL_LOG_INFO("Initializing signal handling...");

    // Reset atomic flags
    s_signalReceived.store(false);
    s_signalType.store(0);
    
    // Initialize platform-specific signal handling
    if (!initializePlatform()) {
        SIGNAL_LOG_ERROR("Platform-specific initialization failed");
        return false;
    }

    m_initialized = true;
    m_shouldExit = false;
    
    SIGNAL_LOG_INFO("Signal handling initialized successfully");
    return true;
}

void SignalManager::shutdown() {
    if (!m_initialized) {
        return;
    }

    SIGNAL_LOG_INFO("Shutting down signal handling...");
    
    // Stop monitoring thread first
    stopMonitoring();
    
    // Platform-specific cleanup
    shutdownPlatform();
    
    // Reset state
    m_initialized = false;
    m_callback = nullptr;
    
    // Clear statistics
    m_stats = Statistics{};
    
    SIGNAL_LOG_INFO("Signal handling shutdown complete");
}

void SignalManager::setSignalCallback(SignalCallback callback) {
    m_callback = callback;
    SIGNAL_LOG_DEBUG("Signal callback set");
}

bool SignalManager::hasSignal() const {
    return s_signalReceived.load();
}

SignalType SignalManager::getSignalType() const {
    int sigType = s_signalType.load();
    switch (sigType) {
        case SIGINT:  return SignalType::INTERRUPT;
        case SIGTERM: return SignalType::TERMINATE;
#ifndef _WIN32
        case SIGUSR1: return SignalType::USER1;
#endif
        default:      return SignalType::INTERRUPT;
    }
}

void SignalManager::clearSignal() {
    s_signalReceived.store(false);
    s_signalType.store(0);
}

void SignalManager::simulateSignal(SignalType signalType) {
    int signum;
    switch (signalType) {
        case SignalType::INTERRUPT: signum = SIGINT; break;
        case SignalType::TERMINATE: signum = SIGTERM; break;
        case SignalType::USER1:     signum = SIGUSR1; break;
        default:                    signum = SIGINT; break;
    }
    
    s_signalReceived.store(true);
    s_signalType.store(signum);
    m_stats.signalsReceived++;
    
    SIGNAL_LOG_DEBUGF("Simulated signal: %d", signum);
}

void SignalManager::startMonitoring(int pollIntervalMs) {
    if (!m_initialized) {
        SIGNAL_LOG_ERROR("Cannot start monitoring - not initialized");
        return;
    }
    
    if (m_monitorThread && m_monitorThread->joinable()) {
        SIGNAL_LOG_DEBUG("Monitoring already active");
        return;
    }
    
    m_shouldExit = false;
    m_monitorThread = std::make_unique<std::thread>(
        &SignalManager::monitoringThreadFunc, this, pollIntervalMs);
    
    m_stats.monitoringActive = true;
    SIGNAL_LOG_INFOF("Started signal monitoring (poll interval: %dms)", pollIntervalMs);
}

void SignalManager::stopMonitoring() {
    if (!m_monitorThread) {
        return;
    }
    
    SIGNAL_LOG_DEBUG("Stopping signal monitoring...");
    m_shouldExit = true;
    
    if (m_monitorThread->joinable()) {
        m_monitorThread->join();
    }
    
    m_monitorThread.reset();
    m_stats.monitoringActive = false;
    
    SIGNAL_LOG_DEBUG("Signal monitoring stopped");
}

// =============================================================================
// Platform-Specific Implementation
// =============================================================================

#ifdef _WIN32

bool SignalManager::initializePlatform() {
    // Set up Windows console control handler
    if (!SetConsoleCtrlHandler(consoleHandler, TRUE)) {
        DWORD error = GetLastError();
        SIGNAL_LOG_ERRORF("Failed to set console handler (error: %lu)", error);
        return false;
    }
    
    m_consoleHandle = GetStdHandle(STD_INPUT_HANDLE);
    return true;
}

void SignalManager::shutdownPlatform() {
    // Remove console control handler
    SetConsoleCtrlHandler(consoleHandler, FALSE);
    
    if (m_consoleHandle != INVALID_HANDLE_VALUE) {
        m_consoleHandle = INVALID_HANDLE_VALUE;
    }
}

BOOL WINAPI SignalManager::consoleHandler(DWORD dwCtrlType) {
    if (!s_instance) {
        return FALSE;
    }
    
    int signum = 0;
    switch (dwCtrlType) {
        case CTRL_C_EVENT:
        case CTRL_BREAK_EVENT:
            signum = SIGINT;
            break;
        case CTRL_CLOSE_EVENT:
        case CTRL_SHUTDOWN_EVENT:
            signum = SIGTERM;
            break;
        default:
            return FALSE;
    }
    
    s_signalReceived.store(true);
    s_signalType.store(signum);
    s_instance->m_stats.signalsReceived++;
    
    return TRUE; // Signal handled
}

#else // POSIX (macOS/Linux)

bool SignalManager::initializePlatform() {
    // Create self-pipe for signal-safe communication
    if (pipe(m_signalPipe) == -1) {
        SIGNAL_LOG_ERRORF("Failed to create signal pipe: %s", strerror(errno));
        return false;
    }
    
    // Make pipe non-blocking
    int flags = fcntl(m_signalPipe[0], F_GETFL);
    if (flags == -1 || fcntl(m_signalPipe[0], F_SETFL, flags | O_NONBLOCK) == -1) {
        SIGNAL_LOG_ERRORF("Failed to set pipe non-blocking: %s", strerror(errno));
        close(m_signalPipe[0]);
        close(m_signalPipe[1]);
        return false;
    }
    
    m_pipeCreated = true;
    
    // Install signal handlers
    struct sigaction sa;
    sa.sa_handler = signalHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    
    // Install SIGINT handler
    if (sigaction(SIGINT, &sa, &m_oldSigintAction) == -1) {
        SIGNAL_LOG_ERRORF("Failed to install SIGINT handler: %s", strerror(errno));
        shutdownPlatform();
        return false;
    }
    
    // Install SIGTERM handler
    if (sigaction(SIGTERM, &sa, &m_oldSigtermAction) == -1) {
        SIGNAL_LOG_ERRORF("Failed to install SIGTERM handler: %s", strerror(errno));
        shutdownPlatform();
        return false;
    }
    
    // Install SIGUSR1 handler (optional)
    if (sigaction(SIGUSR1, &sa, &m_oldSigusr1Action) == -1) {
        SIGNAL_LOG_ERRORF("Failed to install SIGUSR1 handler: %s", strerror(errno));
        // Continue anyway - SIGUSR1 is optional
    }
    
    return true;
}

void SignalManager::shutdownPlatform() {
    // Restore original signal handlers
    if (m_initialized) {
        sigaction(SIGINT, &m_oldSigintAction, nullptr);
        sigaction(SIGTERM, &m_oldSigtermAction, nullptr);
        sigaction(SIGUSR1, &m_oldSigusr1Action, nullptr);
    }
    
    // Close signal pipe
    if (m_pipeCreated) {
        close(m_signalPipe[0]);
        close(m_signalPipe[1]);
        m_signalPipe[0] = -1;
        m_signalPipe[1] = -1;
        m_pipeCreated = false;
    }
}

void SignalManager::signalHandler(int signum) {
    if (!s_instance) {
        return;
    }
    
    // Signal-safe operations only
    s_signalReceived.store(true);
    s_signalType.store(signum);
    s_instance->m_stats.signalsReceived++;
    
    // Wake up monitoring thread via self-pipe
    if (s_instance->m_pipeCreated) {
        char byte = 1;
        ssize_t result = write(s_instance->m_signalPipe[1], &byte, 1);
        (void)result; // Suppress unused variable warning
    }
}

#endif

// =============================================================================
// Internal Implementation
// =============================================================================

void SignalManager::monitoringThreadFunc(int pollIntervalMs) {
    SIGNAL_LOG_DEBUGF("Signal monitoring thread started (poll: %dms)", pollIntervalMs);
    
    auto sleepDuration = std::chrono::milliseconds(pollIntervalMs);
    
    while (!m_shouldExit) {
        m_stats.pollCycles++;
        
        // Check for signals
        if (hasSignal()) {
            SignalType sigType = getSignalType();
            handleSignalInternal(sigType);
            clearSignal();
        }
        
#ifndef _WIN32
        // On POSIX, also check the signal pipe for immediate wake-ups
        if (m_pipeCreated) {
            fd_set readfds;
            FD_ZERO(&readfds);
            FD_SET(m_signalPipe[0], &readfds);
            
            struct timeval timeout;
            timeout.tv_sec = 0;
            timeout.tv_usec = pollIntervalMs * 1000; // Convert to microseconds
            
            int result = select(m_signalPipe[0] + 1, &readfds, nullptr, nullptr, &timeout);
            if (result > 0 && FD_ISSET(m_signalPipe[0], &readfds)) {
                // Drain the pipe
                char buffer[256];
                while (read(m_signalPipe[0], buffer, sizeof(buffer)) > 0);
                
                // Process any pending signals immediately
                if (hasSignal()) {
                    SignalType sigType = getSignalType();
                    handleSignalInternal(sigType);
                    clearSignal();
                }
            }
        } else
#endif
        {
            // Fallback: simple polling
            std::this_thread::sleep_for(sleepDuration);
        }
    }
    
    SIGNAL_LOG_DEBUG("Signal monitoring thread exiting");
}

void SignalManager::handleSignalInternal(SignalType signalType) {
    SIGNAL_LOG_DEBUGF("Handling signal type: %d", static_cast<int>(signalType));
    
    if (m_callback) {
        try {
            m_callback(signalType);
            m_stats.callbacksCalled++;
        } catch (const std::exception& e) {
            SIGNAL_LOG_ERRORF("Signal callback threw exception: %s", e.what());
        } catch (...) {
            SIGNAL_LOG_ERROR("Signal callback threw unknown exception");
        }
    }
}

// =============================================================================
// RAII Helper Implementation
// =============================================================================

SignalHandler::SignalHandler() : m_manager(nullptr) {
}

SignalHandler::~SignalHandler() {
    if (m_manager) {
        m_manager->shutdown();
    }
}

bool SignalHandler::setup(SignalCallback callback) {
    m_manager = std::make_unique<SignalManager>();
    
    if (!m_manager->initialize()) {
        m_manager.reset();
        return false;
    }
    
    m_manager->setSignalCallback(callback);
    m_manager->startMonitoring();
    
    return true;
}

bool SignalHandler::isActive() const {
    return m_manager && m_manager->isInitialized();
}

} // namespace FasterBASIC