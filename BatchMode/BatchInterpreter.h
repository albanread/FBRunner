//
// BatchInterpreter.h
// FasterBASIC - Batch Mode Command Interpreter
//
// Synchronous command interpreter for batch/scripting mode.
// Handles BASIC program commands (numbered lines, LIST, SAVE, LOAD, NEW, RUN)
// and cart operations (CREATECART, USECART, SAVECART, etc.) synchronously
// without requiring GUI components or asynchronous dispatch.
//
// Usage:
//   BatchInterpreter interpreter;
//   interpreter.initialize();
//   interpreter.executeLine("10 PRINT \"HELLO\"");
//   interpreter.executeLine("20 END");
//   interpreter.executeLine("LIST");
//   interpreter.executeLine("RUN");
//   interpreter.shutdown();
//

#ifndef BATCHINTERPRETER_H
#define BATCHINTERPRETER_H

#include <string>
#include <vector>
#include <memory>
#include <iostream>

// Forward declarations to avoid heavy includes in header
namespace FasterBASIC {
    class CommandParser;
    class ProgramManagerV2;
}

namespace SuperTerminal {
    class CartManager;
}

extern "C" {
    struct lua_State;
}

namespace FBRunner3 {
namespace BatchMode {

/**
 * @brief Result of executing a batch command
 */
struct BatchCommandResult {
    bool success = true;
    std::string output;
    std::string error;
    
    BatchCommandResult() = default;
    BatchCommandResult(bool s, const std::string& out, const std::string& err = "")
        : success(s), output(out), error(err) {}
};

/**
 * @brief Synchronous batch command interpreter
 * 
 * This interpreter executes commands synchronously and writes output to stdout.
 * It is designed for scripting and automation, not interactive GUI use.
 * 
 * Supported commands:
 *   - Numbered lines (e.g., "10 PRINT \"HELLO\"")
 *   - LIST: Display program listing
 *   - NEW: Clear program
 *   - RUN: Execute program
 *   - SAVE <file>: Save program to filesystem
 *   - LOAD <file>: Load program from filesystem
 *   - CREATECART <path>: Create new cart file
 *   - USECART <path>: Open existing cart
 *   - SAVECART: Save current cart
 *   - CLOSECART: Close current cart
 *   - COPY SCRIPT <src> <dest>: Copy script to cart
 *   - COPY IMAGE/SOUND/DATA: Copy assets to cart
 */
class BatchInterpreter {
public:
    BatchInterpreter();
    ~BatchInterpreter();
    
    // Prevent copying
    BatchInterpreter(const BatchInterpreter&) = delete;
    BatchInterpreter& operator=(const BatchInterpreter&) = delete;
    
    /**
     * @brief Initialize the interpreter (command parser, Lua state, etc.)
     * @return true if initialization succeeded
     */
    bool initialize();
    
    /**
     * @brief Shutdown the interpreter and clean up resources
     */
    void shutdown();
    
    /**
     * @brief Execute a single line of input
     * @param line Command line to execute
     * @return Result of execution (success, output, error)
     */
    BatchCommandResult executeLine(const std::string& line);
    
    /**
     * @brief Execute multiple lines (newline-separated)
     * @param lines Multiple commands separated by newlines
     * @return Combined result of all executions
     */
    BatchCommandResult executeLines(const std::string& lines);
    
    /**
     * @brief Get the current program listing
     * @return Vector of program lines
     */
    std::vector<std::string> getProgramListing() const;
    
    /**
     * @brief Clear the current program
     */
    void clearProgram();
    
    /**
     * @brief Check if interpreter is initialized
     */
    bool isInitialized() const { return _initialized; }
    
    /**
     * @brief Set output stream (default: std::cout)
     */
    void setOutputStream(std::ostream* stream) { _outputStream = stream; }
    
    /**
     * @brief Set error stream (default: std::cerr)
     */
    void setErrorStream(std::ostream* stream) { _errorStream = stream; }
    
private:
    // Initialization state
    bool _initialized;
    
    // Output streams
    std::ostream* _outputStream;
    std::ostream* _errorStream;
    
    // Core components
    std::unique_ptr<FasterBASIC::CommandParser> _commandParser;
    std::unique_ptr<FasterBASIC::ProgramManagerV2> _programManager;
    lua_State* _luaState;
    
    // Cart system
    std::unique_ptr<SuperTerminal::CartManager> _cartManager;
    
    // Command handlers
    BatchCommandResult handleNumberedLine(int lineNumber, const std::string& content);
    BatchCommandResult handleList();
    BatchCommandResult handleNew();
    BatchCommandResult handleRun();
    BatchCommandResult handleSave(const std::string& filename);
    BatchCommandResult handleLoad(const std::string& filename);
    BatchCommandResult handleCreateCart(const std::string& path);
    BatchCommandResult handleUseCart(const std::string& path);
    BatchCommandResult handleSaveCart();
    BatchCommandResult handleCloseCart();
    BatchCommandResult handleCopyScript(const std::string& src, const std::string& dest);
    BatchCommandResult handleCopyAsset(const std::string& type, const std::string& src, const std::string& dest);
    
    // Helper methods
    void initializeLua();
    void shutdownLua();
    std::string compileProgram();
    bool executeCompiledLua(const std::string& luaCode);
    void writeOutput(const std::string& message);
    void writeError(const std::string& message);
};

} // namespace BatchMode
} // namespace FBRunner3

#endif // BATCHINTERPRETER_H