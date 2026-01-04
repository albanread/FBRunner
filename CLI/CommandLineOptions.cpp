//
// CommandLineOptions.cpp
// FasterBASIC - Command Line Options Parser
//
// Implementation of command-line argument parsing for FBRunner3.
//

#include "CommandLineOptions.h"
#include <iostream>
#include <sstream>

extern "C" {
#include <luajit.h>
}

namespace FBRunner3 {
namespace CLI {

CommandLineOptions CommandLineParser::parse(int argc, const char* argv[]) {
    CommandLineOptions options;
    
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        
        // Help flag
        if (arg == "--help" || arg == "-h") {
            options.showHelp = true;
            return options;
        }
        
        // Version flag
        if (arg == "--version" || arg == "-v") {
            options.showVersion = true;
            return options;
        }
        
        // Script source (-s)
        if (arg == "-s") {
            auto value = getNextArg(i, argc, argv, "-s");
            if (!value) {
                options.valid = false;
                options.errorMessage = "Missing argument for -s flag";
                return options;
            }
            options.scriptSource = *value;
            options.batchMode = true;
            continue;
        }
        
        // Output file (-o)
        if (arg == "-o") {
            auto value = getNextArg(i, argc, argv, "-o");
            if (!value) {
                options.valid = false;
                options.errorMessage = "Missing argument for -o flag";
                return options;
            }
            options.outputFile = *value;
            continue;
        }
        
        // Execute command (-e)
        if (arg == "-e") {
            auto value = getNextArg(i, argc, argv, "-e");
            if (!value) {
                options.valid = false;
                options.errorMessage = "Missing argument for -e flag";
                return options;
            }
            options.executeCommand = *value;
            options.batchMode = true;
            continue;
        }
        
        // Interactive commands (-i)
        if (arg == "-i") {
            auto value = getNextArg(i, argc, argv, "-i");
            if (!value) {
                options.valid = false;
                options.errorMessage = "Missing argument for -i flag";
                return options;
            }
            options.interactiveCommands = *value;
            options.batchMode = true;
            continue;
        }
        
        // Unknown flag
        if (isFlag(arg)) {
            options.valid = false;
            options.errorMessage = "Unknown flag: " + arg;
            return options;
        }
        
        // Positional argument (script file)
        if (!options.scriptFile) {
            options.scriptFile = arg;
        } else {
            options.valid = false;
            options.errorMessage = "Multiple script files specified: " + 
                                   *options.scriptFile + " and " + arg;
            return options;
        }
    }
    
    // Validate: can't have both script file and script source
    if (options.scriptFile && options.scriptSource) {
        options.valid = false;
        options.errorMessage = "Cannot specify both script file and -s flag";
        return options;
    }
    
    // If we have a script file or -e/-i, we're in batch mode
    if (options.scriptFile || options.executeCommand || options.interactiveCommands) {
        options.batchMode = true;
    }
    
    return options;
}

void CommandLineParser::printUsage() {
    std::cout << R"(FasterBASIC - SuperTerminal FasterBASICT Runtime

USAGE:
    FasterBASIC [OPTIONS] [script.bas]

OPTIONS:
    -s <source>       Execute BASIC source code directly
    -o <file>         Write output to file
    -e <command>      Execute a single interactive command
    -i <commands>     Execute multiple interactive commands (newline-separated)
    -h, --help        Show this help message
    -v, --version     Show version information

ARGUMENTS:
    script.bas        BASIC script file to execute

EXAMPLES:
    # Run in GUI mode
    FasterBASIC
    
    # Execute a script file
    FasterBASIC myscript.bas
    
    # Execute source code directly
    FasterBASIC -s "10 PRINT \"HELLO\"\n20 END"
    
    # Execute interactive commands
    FasterBASIC -i $'10 PRINT "TEST"\n20 END\nLIST\nRUN'
    
    # Save output to file
    FasterBASIC -i $'10 PRINT "HELLO"\nRUN' -o output.txt

INTERACTIVE COMMANDS:
    When using -e or -i flags, you can use interactive shell commands:
    
    Program editing:
        <number> <line>   Add/replace numbered line
        LIST              List program
        NEW               Clear program
        RUN               Execute program
        SAVE <file>       Save program to file
        LOAD <file>       Load program from file
        
    Cart operations:
        CREATECART <path> Create new cart
        USECART <path>    Open existing cart
        SAVECART          Save current cart
        CLOSECART         Close current cart
        
    Asset operations:
        COPY SCRIPT <src> <dest>
        COPY IMAGE <src> <dest>
        COPY SOUND <src> <dest>

SCRIPT STYLE:
    10 REM Initialize
    20 LET X = 0
    30 PRINT "Hello from BASIC!"
    40 END

For more information, see documentation at:
    https://github.com/yourusername/FasterBASIC
)";
}

void CommandLineParser::printVersion() {
    std::cout << "FasterBASIC v1.0.0\n";
    std::cout << "FasterBASICT Compiler with SuperTerminal Runtime\n";
    std::cout << "Built with LuaJIT " << LUAJIT_VERSION << "\n";
}

bool CommandLineParser::isFlag(const std::string& arg) {
    return !arg.empty() && arg[0] == '-';
}

std::optional<std::string> CommandLineParser::getNextArg(int& i, int argc, 
                                                          const char* argv[], 
                                                          const std::string& flagName) {
    if (i + 1 >= argc) {
        return std::nullopt;
    }
    i++;
    return std::string(argv[i]);
}

} // namespace CLI
} // namespace FBRunner3