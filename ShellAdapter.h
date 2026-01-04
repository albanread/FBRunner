//
// ShellAdapter.h
// FBRunner3 - Shell Adapter for Interactive Mode
//
// Adapts fbsh's shell functionality (CommandParser, ProgramManager) for GUI use.
// Provides command execution, program management, and compilation/execution
// without requiring terminal I/O (routes output to TextGrid instead).
//

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <functional>

// Forward declarations
extern "C" {
    struct lua_State;
}

namespace SuperTerminal {
    class TextGrid;
    class GraphicsLayer;
    class AudioManager;
    class TextGridOutputStream;
}

namespace FasterBASIC {
    class ProgramManagerV2;
    class CommandParser;
}

/// Callback for output messages
using OutputCallback = std::function<void(const std::string&)>;

/// Callback for error messages
using ErrorCallback = std::function<void(const std::string&)>;

/// ShellAdapter - Adapts fbsh functionality for GUI interactive mode
///
/// Responsibilities:
/// - Parse and execute shell commands (LIST, RUN, LOAD, SAVE, etc.)
/// - Manage BASIC program in memory (via ProgramManager)
/// - Compile and execute programs using FasterBASICT compiler
/// - Route output to TextGridOutputStream instead of stdout
/// - Provide callbacks for status updates
///
class ShellAdapter {
public:
    /// Constructor
    /// @param outputStream Output stream for PRINT statements
    ShellAdapter(std::shared_ptr<SuperTerminal::TextGridOutputStream> outputStream);
    
    /// Destructor
    ~ShellAdapter();
    
    // =========================================================================
    // Command Execution
    // =========================================================================
    
    /// Execute a line of input (command or program line)
    /// @param input Line entered by user
    /// @return true if successful, false if error
    bool executeLine(const std::string& input);
    
    /// Get prompt string for current state
    /// @return Prompt string (e.g., "Ready." or "10 ")
    std::string getPrompt() const;
    
    /// Check if in AUTO mode (automatic line numbering)
    /// @return true if AUTO mode is active
    bool isAutoMode() const;
    
    /// Get suggested next line number (for AUTO mode)
    /// @return Next line number to suggest
    int getSuggestedLineNumber() const;
    
    // =========================================================================
    // Program Management
    // =========================================================================
    
    /// Check if program is currently running
    /// @return true if program is executing
    bool isProgramRunning() const;
    
    /// Stop running program
    void stopProgram();
    
    /// Clear program from memory (NEW command)
    void newProgram();
    
    /// Check if program has been modified
    /// @return true if program has unsaved changes
    bool isProgramModified() const;
    
    /// Get program listing as vector of lines
    /// @return Vector of program lines with line numbers
    std::vector<std::string> getProgramLines() const;
    
    /// Get raw program source (without line numbers)
    /// @return Program source code
    std::string getProgramSource() const;
    
    // =========================================================================
    // Callbacks
    // =========================================================================
    
    /// Set callback for output messages
    /// @param callback Function to call for output
    void setOutputCallback(OutputCallback callback);
    
    /// Set callback for error messages
    /// @param callback Function to call for errors
    void setErrorCallback(ErrorCallback callback);
    
    /// Set callback for program status changes
    /// @param callback Function to call when program starts/stops
    void setStatusCallback(std::function<void(bool running)> callback);
    
    // =========================================================================
    // Configuration
    // =========================================================================
    
    /// Set BASIC scripts directory
    /// @param path Path to directory containing .bas files
    void setScriptsDirectory(const std::string& path);
    
    /// Get BASIC scripts directory
    /// @return Path to scripts directory
    std::string getScriptsDirectory() const;
    
    /// Set whether to show verbose output
    /// @param verbose If true, show compilation details
    void setVerbose(bool verbose);
    
    /// Set whether to enable optimizers
    /// @param enableAST Enable AST optimizer
    /// @param enablePeephole Enable peephole optimizer
    void setOptimizers(bool enableAST, bool enablePeephole);

private:
    // =========================================================================
    // Internal State
    // =========================================================================
    
    // Output handling
    std::shared_ptr<SuperTerminal::TextGridOutputStream> outputStream_;
    OutputCallback outputCallback_;
    ErrorCallback errorCallback_;
    std::function<void(bool)> statusCallback_;
    
    // Command processing
    std::unique_ptr<FasterBASIC::CommandParser> parser_;
    std::unique_ptr<FasterBASIC::ProgramManagerV2> programManager_;
    
    // Execution state
    lua_State* luaState_;
    std::mutex luaStateMutex_;
    bool programRunning_;
    bool shouldStop_;
    
    // Configuration
    std::string scriptsDirectory_;
    bool verbose_;
    bool enableASTOptimizer_;
    bool enablePeepholeOptimizer_;
    
    // AUTO mode state
    bool autoMode_;
    int autoLineNumber_;
    int autoIncrement_;
    
    // Thread safety
    mutable std::mutex mutex_;
    
    // =========================================================================
    // Internal Methods
    // =========================================================================
    
    // Command handlers
    bool handleProgramLine(int lineNumber, const std::string& code);
    bool handleListCommand(const std::string& args);
    bool handleRunCommand(const std::string& args);
    bool handleLoadCommand(const std::string& filename);
    bool handleSaveCommand(const std::string& filename);
    bool handleNewCommand();
    bool handleAutoCommand(const std::string& args);
    bool handleDeleteCommand(const std::string& args);
    bool handleRenumCommand(const std::string& args);
    bool handleDirCommand();
    
    // Compilation and execution
    bool compileAndRun(int startLine = -1);
    std::string compileProgram();
    bool executeLuaCode(const std::string& luaCode);
    void setupLuaState();
    void cleanupLuaState();
    
    // Output helpers
    void output(const std::string& message);
    void outputLine(const std::string& message);
    void error(const std::string& message);
    
    // File I/O helpers
    std::string resolveFilename(const std::string& filename) const;
    bool fileExists(const std::string& path) const;
    std::vector<std::string> listBasicFiles() const;
    
    // Formatting helpers
    std::string formatProgramListing() const;
    std::string formatLineRange(int start, int end) const;
};