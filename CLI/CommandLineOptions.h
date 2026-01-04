//
// CommandLineOptions.h
// FasterBASIC - Command Line Options Parser
//
// Centralized command-line argument parsing for FBRunner3.
// Separates CLI concerns from main application logic.
//

#ifndef COMMANDLINEOPTIONS_H
#define COMMANDLINEOPTIONS_H

#include <string>
#include <vector>
#include <optional>

namespace FBRunner3 {
namespace CLI {

/**
 * @brief Parsed command-line options for FBRunner3
 */
struct CommandLineOptions {
    // Script file to execute (positional argument)
    std::optional<std::string> scriptFile;
    
    // -s flag: Script source code to execute directly
    std::optional<std::string> scriptSource;
    
    // -o flag: Output file for execution results
    std::optional<std::string> outputFile;
    
    // -e flag: Execute a single interactive command
    std::optional<std::string> executeCommand;
    
    // -i flag: Execute multiple interactive commands (newline-separated)
    std::optional<std::string> interactiveCommands;
    
    // --help flag: Show help message
    bool showHelp = false;
    
    // --version flag: Show version information
    bool showVersion = false;
    
    // Batch mode: Run without showing GUI window
    bool batchMode = false;
    
    // Parsed successfully
    bool valid = true;
    
    // Error message if parsing failed
    std::string errorMessage;
};

/**
 * @brief Command-line argument parser
 */
class CommandLineParser {
public:
    /**
     * @brief Parse command-line arguments
     * @param argc Argument count
     * @param argv Argument values
     * @return Parsed options structure
     */
    static CommandLineOptions parse(int argc, const char* argv[]);
    
    /**
     * @brief Print usage information to stdout
     */
    static void printUsage();
    
    /**
     * @brief Print version information to stdout
     */
    static void printVersion();
    
private:
    /**
     * @brief Check if argument is a flag (starts with -)
     */
    static bool isFlag(const std::string& arg);
    
    /**
     * @brief Get next argument value, handling errors
     */
    static std::optional<std::string> getNextArg(int& i, int argc, const char* argv[], 
                                                   const std::string& flagName);
};

} // namespace CLI
} // namespace FBRunner3

#endif // COMMANDLINEOPTIONS_H