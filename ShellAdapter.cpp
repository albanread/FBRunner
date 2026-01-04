//
// ShellAdapter.cpp
// FBRunner3 - Shell Adapter for Interactive Mode
//
// Implementation of shell adapter for GUI interactive mode.
//

#include "ShellAdapter.h"
#include "../Framework/UI/TextGridOutputStream.h"
#include "../FasterBASICT/shell/command_parser.h"
#include "../FasterBASICT/shell/program_manager_v2.h"
#include "../FasterBASICT/src/fasterbasic_lexer.h"
#include "../FasterBASICT/src/fasterbasic_parser.h"
#include "../FasterBASICT/src/fasterbasic_semantic.h"
#include "../FasterBASICT/src/fasterbasic_optimizer.h"
#include "../FasterBASICT/src/fasterbasic_peephole.h"
#include "../FasterBASICT/src/fasterbasic_cfg.h"
#include "../FasterBASICT/src/fasterbasic_ircode.h"
#include "../FasterBASICT/src/fasterbasic_lua_codegen.h"
#include "../FasterBASICT/src/basic_formatter_lib.h"
#include "../FasterBASICT/runtime/DataManager.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <algorithm>

#include <lua.hpp>

// Runtime module registration (from FasterBASICT)
extern "C" void register_unicode_module(lua_State* L);
extern "C" void register_bitwise_module(lua_State* L);
extern "C" void register_constants_module(lua_State* L);
extern "C" void set_constants_manager(FasterBASIC::ConstantsManager* manager);

namespace FasterBASIC {
    void register_fileio_functions(lua_State* L);
    void clear_fileio_state();
    void registerDataBindings(lua_State* L);
    void registerTerminalBindings(lua_State* L);
}

#include "../FasterBASICT/src/modular_commands.h"

using namespace FasterBASIC;

// Simple Lua registration for modular commands
static int lua_modular_cls(lua_State* L) {
   lua_getglobal(L, "text_clear");
   if (lua_isfunction(L, -1)) {
       lua_call(L, 0, 0);
   } else {
       lua_pop(L, 1);
   }
   return 0;
}

static void registerModularCommandsWithLua(lua_State* L) {
   lua_pushcfunction(L, lua_modular_cls);
   lua_setglobal(L, "basic_cls");
    
   lua_pushcfunction(L, lua_modular_cls);
   lua_setglobal(L, "cls");
}

// Default BASIC scripts directories
static const std::string DEFAULT_SCRIPTS_DIR = "~/SuperTerminal/BASIC/";
static const std::string DEFAULT_LIB_DIR = "~/SuperTerminal/BASIC/lib/";

ShellAdapter::ShellAdapter(std::shared_ptr<SuperTerminal::TextGridOutputStream> outputStream)
    : outputStream_(outputStream)
    , outputCallback_(nullptr)
    , errorCallback_(nullptr)
    , statusCallback_(nullptr)
    , luaState_(nullptr)
    , programRunning_(false)
    , shouldStop_(false)
    , verbose_(false)
    , enableASTOptimizer_(false)
    , enablePeepholeOptimizer_(false)
    , autoMode_(false)
    , autoLineNumber_(10)
    , autoIncrement_(10)
{
    // Create command parser
    parser_ = std::make_unique<CommandParser>();
    
    // Create program manager
    programManager_ = std::make_unique<ProgramManagerV2>();
    
    // Expand home directory in default path
    scriptsDirectory_ = DEFAULT_SCRIPTS_DIR;
    if (scriptsDirectory_[0] == '~') {
        const char* home = getenv("HOME");
        if (home) {
            scriptsDirectory_ = std::string(home) + scriptsDirectory_.substr(1);
        }
    }
    
    // Ensure scripts directory exists
    std::filesystem::create_directories(scriptsDirectory_);
    
    // Output welcome message
    if (outputStream_) {
        outputStream_->println("FasterBASIC Interactive Mode");
        outputStream_->println("");
    }
}

ShellAdapter::~ShellAdapter() {
    stopProgram();
    cleanupLuaState();
}

// =============================================================================
// Command Execution
// =============================================================================

bool ShellAdapter::executeLine(const std::string& input) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (input.empty()) {
        return true;
    }
    
    // Parse the input
    ParsedCommand cmd = parser_->parse(input);
    
    if (parser_->hasError()) {
        error(parser_->getLastError());
        return false;
    }
    
    // Execute based on command type
    switch (cmd.type) {
        case ShellCommandType::PROGRAM_LINE:
            return handleProgramLine(cmd.lineNumber, cmd.code);
            
        case ShellCommandType::DELETE_LINE:
            programManager_->deleteLine(cmd.lineNumber);
            return true;
            
        case ShellCommandType::LIST:
        case ShellCommandType::LIST_RANGE:
        case ShellCommandType::LIST_LINE:
        case ShellCommandType::LIST_FROM:
        case ShellCommandType::LIST_TO:
            return handleListCommand(cmd.args);
            
        case ShellCommandType::RUN:
        case ShellCommandType::RUN_FROM:
            return handleRunCommand(cmd.args);
            
        case ShellCommandType::LOAD:
            return handleLoadCommand(cmd.filename);
            
        case ShellCommandType::SAVE:
            return handleSaveCommand(cmd.filename);
            
        case ShellCommandType::NEW:
            return handleNewCommand();
            
        case ShellCommandType::AUTO:
        case ShellCommandType::AUTO_PARAMS:
            return handleAutoCommand(cmd.args);
            
        case ShellCommandType::DELETE:
        case ShellCommandType::DELETE_RANGE:
            return handleDeleteCommand(cmd.args);
            
        case ShellCommandType::RENUM:
        case ShellCommandType::RENUM_PARAMS:
            return handleRenumCommand(cmd.args);
            
        case ShellCommandType::DIR:
            return handleDirCommand();
            
        case ShellCommandType::QUIT:
            // QUIT is handled by the application, not here
            return true;
            
        default:
            error("Unknown command");
            return false;
    }
}

std::string ShellAdapter::getPrompt() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (autoMode_) {
        return std::to_string(autoLineNumber_) + " ";
    }
    
    return "Ready.";
}

bool ShellAdapter::isAutoMode() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return autoMode_;
}

int ShellAdapter::getSuggestedLineNumber() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return autoLineNumber_;
}

// =============================================================================
// Program Management
// =============================================================================

bool ShellAdapter::isProgramRunning() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return programRunning_;
}

void ShellAdapter::stopProgram() {
    std::lock_guard<std::mutex> lock(mutex_);
    shouldStop_ = true;
    programRunning_ = false;
    
    if (statusCallback_) {
        statusCallback_(false);
    }
}

void ShellAdapter::newProgram() {
    std::lock_guard<std::mutex> lock(mutex_);
    programManager_->clear();
    autoMode_ = false;
}

bool ShellAdapter::isProgramModified() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return programManager_->isModified();
}

std::vector<std::string> ShellAdapter::getProgramLines() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<std::string> lines;
    auto lineNumbers = programManager_->getLineNumbers();
    
    for (int lineNum : lineNumbers) {
        std::string code = programManager_->getLine(lineNum);
        lines.push_back(std::to_string(lineNum) + " " + code);
    }
    
    return lines;
}

std::string ShellAdapter::getProgramSource() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return programManager_->generateProgram();
}

// =============================================================================
// Callbacks
// =============================================================================

void ShellAdapter::setOutputCallback(OutputCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    outputCallback_ = callback;
}

void ShellAdapter::setErrorCallback(ErrorCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    errorCallback_ = callback;
}

void ShellAdapter::setStatusCallback(std::function<void(bool)> callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    statusCallback_ = callback;
}

// =============================================================================
// Configuration
// =============================================================================

void ShellAdapter::setScriptsDirectory(const std::string& path) {
    std::lock_guard<std::mutex> lock(mutex_);
    scriptsDirectory_ = path;
}

std::string ShellAdapter::getScriptsDirectory() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return scriptsDirectory_;
}

void ShellAdapter::setVerbose(bool verbose) {
    std::lock_guard<std::mutex> lock(mutex_);
    verbose_ = verbose;
}

void ShellAdapter::setOptimizers(bool enableAST, bool enablePeephole) {
    std::lock_guard<std::mutex> lock(mutex_);
    enableASTOptimizer_ = enableAST;
    enablePeepholeOptimizer_ = enablePeephole;
}

// =============================================================================
// Command Handlers
// =============================================================================

bool ShellAdapter::handleProgramLine(int lineNumber, const std::string& code) {
    if (code.empty()) {
        programManager_->deleteLine(lineNumber);
    } else {
        programManager_->setLine(lineNumber, code);
    }
    
    // Update auto mode
    if (autoMode_) {
        autoLineNumber_ = lineNumber + autoIncrement_;
    }
    
    return true;
}

bool ShellAdapter::handleListCommand(const std::string& args) {
    std::string listing = formatProgramListing();
    
    if (listing.empty()) {
        outputLine("No program in memory");
    } else {
        output(listing);
    }
    
    return true;
}

bool ShellAdapter::handleRunCommand(const std::string& args) {
    if (programManager_->isEmpty()) {
        error("No program to run");
        return false;
    }
    
    // Parse start line if provided
    int startLine = -1;
    if (!args.empty()) {
        try {
            startLine = std::stoi(args);
        } catch (...) {
            error("Invalid line number");
            return false;
        }
    }
    
    return compileAndRun(startLine);
}

bool ShellAdapter::handleLoadCommand(const std::string& filename) {
    if (filename.empty()) {
        error("Filename required");
        return false;
    }
    
    std::string fullPath = resolveFilename(filename);
    
    if (!fileExists(fullPath)) {
        error("File not found: " + filename);
        return false;
    }
    
    std::ifstream file(fullPath);
    if (!file.is_open()) {
        error("Cannot open file: " + filename);
        return false;
    }
    
    // Clear current program
    programManager_->clear();
    
    // Load lines
    std::string line;
    int lineCount = 0;
    
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        
        std::istringstream iss(line);
        int lineNum;
        
        if (iss >> lineNum) {
            std::string code;
            std::getline(iss, code);
            
            // Remove leading space
            if (!code.empty() && code[0] == ' ') {
                code = code.substr(1);
            }
            
            if (!code.empty()) {
                programManager_->setLine(lineNum, code);
                lineCount++;
            }
        }
    }
    
    file.close();
    
    outputLine("Loaded " + std::to_string(lineCount) + " lines from " + filename);
    programManager_->setModified(false);
    
    return true;
}

bool ShellAdapter::handleSaveCommand(const std::string& filename) {
    if (filename.empty()) {
        error("Filename required");
        return false;
    }
    
    if (programManager_->isEmpty()) {
        error("No program to save");
        return false;
    }
    
    std::string fullPath = resolveFilename(filename);
    
    std::ofstream file(fullPath);
    if (!file.is_open()) {
        error("Cannot create file: " + filename);
        return false;
    }
    
    // Write program
    auto lines = programManager_->getLineNumbers();
    for (int lineNum : lines) {
        std::string code = programManager_->getLine(lineNum);
        file << lineNum << " " << code << "\n";
    }
    
    file.close();
    
    outputLine("Saved to " + filename);
    programManager_->setModified(false);
    
    return true;
}

bool ShellAdapter::handleNewCommand() {
    programManager_->clear();
    autoMode_ = false;
    outputLine("New program");
    return true;
}

bool ShellAdapter::handleAutoCommand(const std::string& args) {
    if (args.empty()) {
        // Toggle auto mode
        autoMode_ = !autoMode_;
        if (autoMode_) {
            autoLineNumber_ = 10;
            autoIncrement_ = 10;
            outputLine("AUTO mode on");
        } else {
            outputLine("AUTO mode off");
        }
    } else {
        // Parse parameters
        std::istringstream iss(args);
        iss >> autoLineNumber_;
        if (iss >> autoIncrement_) {
            // Both provided
        } else {
            autoIncrement_ = 10;
        }
        autoMode_ = true;
        outputLine("AUTO " + std::to_string(autoLineNumber_) + ", " + std::to_string(autoIncrement_));
    }
    
    return true;
}

bool ShellAdapter::handleDeleteCommand(const std::string& args) {
    // Parse line range
    std::istringstream iss(args);
    int start, end;
    
    if (iss >> start) {
        if (iss.peek() == '-') {
            iss.ignore();
            if (iss >> end) {
                // Range delete
                for (int i = start; i <= end; i++) {
                    programManager_->deleteLine(i);
                }
            } else {
                error("Invalid range");
                return false;
            }
        } else {
            // Single line delete
            programManager_->deleteLine(start);
        }
        
        outputLine("Deleted");
        return true;
    }
    
    error("Line number required");
    return false;
}

bool ShellAdapter::handleRenumCommand(const std::string& args) {
    int newStart = 10;
    int newStep = 10;
    
    if (!args.empty()) {
        std::istringstream iss(args);
        iss >> newStart;
        if (iss >> newStep) {
            // Both provided
        }
    }
    
    programManager_->renumber(newStart, newStep);
    outputLine("Renumbered");
    
    return true;
}

bool ShellAdapter::handleDirCommand() {
    auto files = listBasicFiles();
    
    if (files.empty()) {
        outputLine("No BASIC files found");
    } else {
        outputLine("BASIC files:");
        for (const auto& file : files) {
            outputLine("  " + file);
        }
    }
    
    return true;
}

// =============================================================================
// Compilation and Execution
// =============================================================================

bool ShellAdapter::compileAndRun(int startLine) {
    // Compile program
    std::string luaCode = compileProgram();
    
    if (luaCode.empty()) {
        return false;
    }
    
    // Execute
    return executeLuaCode(luaCode);
}

std::string ShellAdapter::compileProgram() {
    if (programManager_->isEmpty()) {
        error("No program to compile");
        return "";
    }
    
    // Get source
    std::string source = programManager_->generateProgram();
    
    if (verbose_) {
        outputLine("Compiling...");
    }
    
    // Lexer
    Lexer lexer(source);
    std::vector<Token> tokens = lexer.tokenize();
    
    if (lexer.hasErrors()) {
        error("Lexer error: " + lexer.getErrors()[0]);
        return "";
    }
    
    // Parser
    Parser parser(tokens);
    auto program = parser.parse();
    
    if (parser.hasErrors()) {
        error("Parser error: " + parser.getErrors()[0]);
        return "";
    }
    
    // Semantic analysis
    SemanticAnalyzer analyzer;
    auto symbols = analyzer.analyze(*program);
    
    if (analyzer.hasErrors()) {
        error("Semantic error: " + analyzer.getErrors()[0]);
        return "";
    }
    
    // Optional: AST optimizer
    if (enableASTOptimizer_) {
        ASTOptimizer optimizer;
        program = optimizer.optimize(std::move(program));
    }
    
    // Control Flow Graph
    CFGBuilder cfgBuilder;
    auto cfg = cfgBuilder.build(*program);
    
    // IR Code generation
    IRGenerator irGen(cfg.get(), symbols.get());
    auto irCode = irGen.generate();
    
    // Optional: Peephole optimizer
    if (enablePeepholeOptimizer_) {
        PeepholeOptimizer peephole;
        irCode = peephole.optimize(std::move(irCode));
    }
    
    // Lua code generation
    LuaCodeGenConfig config;
    config.emitComments = verbose_;
    
    LuaCodeGenerator codegen(config);
    std::string luaCode = codegen.generate(*irCode);
    
    if (verbose_) {
        outputLine("Compilation successful");
    }
    
    return luaCode;
}

bool ShellAdapter::executeLuaCode(const std::string& luaCode) {
    // Setup Lua state
    setupLuaState();
    
    if (!luaState_) {
        error("Failed to create Lua state");
        return false;
    }
    
    // Load and execute
    int result = luaL_loadstring(luaState_, luaCode.c_str());
    
    if (result != LUA_OK) {
        const char* err = lua_tostring(luaState_, -1);
        error(std::string("Load error: ") + (err ? err : "unknown"));
        cleanupLuaState();
        return false;
    }
    
    // Execute
    programRunning_ = true;
    if (statusCallback_) {
        statusCallback_(true);
    }
    
    result = lua_pcall(luaState_, 0, 0, 0);
    
    programRunning_ = false;
    if (statusCallback_) {
        statusCallback_(false);
    }
    
    if (result != LUA_OK) {
        const char* err = lua_tostring(luaState_, -1);
        error(std::string("Runtime error: ") + (err ? err : "unknown"));
        cleanupLuaState();
        return false;
    }
    
    cleanupLuaState();
    return true;
}

void ShellAdapter::setupLuaState() {
    std::lock_guard<std::mutex> lock(luaStateMutex_);
    
    if (luaState_) {
        cleanupLuaState();
    }
    
    luaState_ = luaL_newstate();
    luaL_openlibs(luaState_);
    
    // Override os.exit to prevent it from killing the entire app
    // When a BASIC script does END or an error occurs, the generated Lua
    // code calls os.exit(). We intercept this and throw a Lua error instead.
    lua_pushcfunction(luaState_, [](lua_State* L) -> int {
        int exitcode = luaL_optinteger(L, 1, 0);
        if (exitcode == 0) {
            luaL_error(L, "Script ended normally (os.exit called)");
        } else {
            luaL_error(L, "Script ended with error code %d (os.exit called)", exitcode);
        }
        return 0;
    });
    lua_setglobal(luaState_, "exit");  // Set as global function 'exit'
    
    // Also override it in the os table
    lua_getglobal(luaState_, "os");
    lua_pushcfunction(luaState_, [](lua_State* L) -> int {
        int exitcode = luaL_optinteger(L, 1, 0);
        if (exitcode == 0) {
            luaL_error(L, "Script ended normally (os.exit called)");
        } else {
            luaL_error(L, "Script ended with error code %d (os.exit called)", exitcode);
        }
        return 0;
    });
    lua_setfield(luaState_, -2, "exit");
    lua_pop(luaState_, 1);  // Pop the os table
    
    // Register runtime modules
    register_unicode_module(luaState_);
    register_bitwise_module(luaState_);
    register_constants_module(luaState_);
    
    // Register FBT bindings
    FasterBASIC::register_fileio_functions(luaState_);
    FasterBASIC::registerDataBindings(luaState_);
    FasterBASIC::registerTerminalBindings(luaState_);
    
    // Register modular commands (CLS, etc.)
    registerModularCommandsWithLua(luaState_);
    
    // TODO: Register GUI-specific bindings (graphics, audio, etc.)
}

void ShellAdapter::cleanupLuaState() {
    std::lock_guard<std::mutex> lock(luaStateMutex_);
    
    if (luaState_) {
        lua_close(luaState_);
        luaState_ = nullptr;
    }
    
    FasterBASIC::clear_fileio_state();
}

// =============================================================================
// Output Helpers
// =============================================================================

void ShellAdapter::output(const std::string& message) {
    if (outputStream_) {
        outputStream_->print(message);
    }
    
    if (outputCallback_) {
        outputCallback_(message);
    }
}

void ShellAdapter::outputLine(const std::string& message) {
    if (outputStream_) {
        outputStream_->println(message);
    }
    
    if (outputCallback_) {
        outputCallback_(message + "\n");
    }
}

void ShellAdapter::error(const std::string& message) {
    if (outputStream_) {
        outputStream_->println("Error: " + message);
    }
    
    if (errorCallback_) {
        errorCallback_(message);
    }
}

// =============================================================================
// File I/O Helpers
// =============================================================================

std::string ShellAdapter::resolveFilename(const std::string& filename) const {
    // If absolute path, use as-is
    if (filename[0] == '/' || filename[0] == '~') {
        std::string path = filename;
        if (path[0] == '~') {
            const char* home = getenv("HOME");
            if (home) {
                path = std::string(home) + path.substr(1);
            }
        }
        return path;
    }
    
    // Try scripts directory first
    std::string path = scriptsDirectory_ + filename;
    if (fileExists(path)) {
        return path;
    }
    
    // Try lib directory
    std::string libDir = scriptsDirectory_ + "lib/";
    path = libDir + filename;
    if (fileExists(path)) {
        return path;
    }
    
    // Default to scripts directory
    return scriptsDirectory_ + filename;
}

bool ShellAdapter::fileExists(const std::string& path) const {
    return std::filesystem::exists(path);
}

std::vector<std::string> ShellAdapter::listBasicFiles() const {
    std::vector<std::string> files;
    
    // List scripts directory
    try {
        for (const auto& entry : std::filesystem::directory_iterator(scriptsDirectory_)) {
            if (entry.is_regular_file()) {
                std::string filename = entry.path().filename().string();
                if (filename.length() > 4 && filename.substr(filename.length() - 4) == ".bas") {
                    files.push_back(filename);
                }
            }
        }
    } catch (...) {
        // Ignore errors
    }
    
    // Sort alphabetically
    std::sort(files.begin(), files.end());
    
    return files;
}

// =============================================================================
// Formatting Helpers
// =============================================================================

std::string ShellAdapter::formatProgramListing() const {
    auto lines = programManager_->getLineNumbers();
    
    if (lines.empty()) {
        return "";
    }
    
    std::ostringstream oss;
    
    for (int lineNum : lines) {
        std::string code = programManager_->getLine(lineNum);
        oss << lineNum << " " << code << "\n";
    }
    
    return oss.str();
}

std::string ShellAdapter::formatLineRange(int start, int end) const {
    auto lines = programManager_->getLineNumbers();
    
    std::ostringstream oss;
    
    for (int lineNum : lines) {
        if (lineNum >= start && lineNum <= end) {
            std::string code = programManager_->getLine(lineNum);
            oss << lineNum << " " << code << "\n";
        }
    }
    
    return oss.str();
}