//
// BatchInterpreter.cpp
// FasterBASIC - Batch Mode Command Interpreter
//
// Implementation of synchronous command interpreter for batch/scripting mode.
//

#include "BatchInterpreter.h"
#include "../FasterBASICT/shell/command_parser.h"
#include "../FasterBASICT/shell/program_manager_v2.h"
#include "../Framework/Cart/CartManager.h"
#include "../FasterBASICT/src/basic_formatter_lib.h"
#include "fasterbasic_data_preprocessor.h"
#include "fasterbasic_lexer.h"
#include "fasterbasic_parser.h"
#include "fasterbasic_semantic.h"
#include "fasterbasic_optimizer.h"
#include "fasterbasic_peephole.h"
#include "fasterbasic_cfg.h"
#include "fasterbasic_ircode.h"
#include "fasterbasic_lua_codegen.h"
#include "command_registry_core.h"
#include "command_registry_superterminal.h"

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

#include <sstream>
#include <fstream>
#include <algorithm>

using namespace FasterBASIC;
using namespace FasterBASIC::ModularCommands;

namespace FBRunner3 {
namespace BatchMode {

BatchInterpreter::BatchInterpreter()
    : _initialized(false)
    , _outputStream(&std::cout)
    , _errorStream(&std::cerr)
    , _luaState(nullptr)
{
}

BatchInterpreter::~BatchInterpreter() {
    shutdown();
}

bool BatchInterpreter::initialize() {
    if (_initialized) {
        return true;
    }
    
    try {
        // Initialize command registry (must be done first)
        CommandRegistry& registry = getGlobalCommandRegistry();
        registry.clear();
        CoreCommandRegistry::registerCoreCommands(registry);
        CoreCommandRegistry::registerCoreFunctions(registry);
        SuperTerminalCommandRegistry::registerSuperTerminalCommands(registry);
        SuperTerminalCommandRegistry::registerSuperTerminalFunctions(registry);
        markGlobalRegistryInitialized();
        
        // Create command parser and program manager
        _commandParser = std::make_unique<FasterBASIC::CommandParser>();
        _programManager = std::make_unique<FasterBASIC::ProgramManagerV2>();
        
        // Initialize Lua state
        initializeLua();
        
        // Initialize cart manager
        _cartManager = std::make_unique<SuperTerminal::CartManager>();
        
        _initialized = true;
        return true;
        
    } catch (const std::exception& e) {
        writeError(std::string("Initialization failed: ") + e.what());
        return false;
    }
}

void BatchInterpreter::shutdown() {
    if (!_initialized) {
        return;
    }
    
    // Close any open cart
    if (_cartManager) {
        _cartManager.reset();
    }
    
    // Shutdown Lua
    shutdownLua();
    
    // Clean up managers
    _programManager.reset();
    _commandParser.reset();
    
    _initialized = false;
}

BatchCommandResult BatchInterpreter::executeLine(const std::string& line) {
    if (!_initialized) {
        return BatchCommandResult(false, "", "Interpreter not initialized");
    }
    
    // Skip empty lines and comments
    std::string trimmed = line;
    trimmed.erase(0, trimmed.find_first_not_of(" \t\r\n"));
    trimmed.erase(trimmed.find_last_not_of(" \t\r\n") + 1);
    
    if (trimmed.empty() || trimmed[0] == '#') {
        return BatchCommandResult(true, "", "");
    }
    
    // Parse the command
    auto parseResult = _commandParser->parse(trimmed);
    
    // Handle numbered line
    if (parseResult.type == FasterBASIC::CommandType::NUMBERED_LINE) {
        return handleNumberedLine(parseResult.lineNumber, parseResult.content);
    }
    
    // Handle interactive commands
    std::string cmd = parseResult.command;
    std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper);
    
    if (cmd == "LIST") {
        return handleList();
    }
    else if (cmd == "NEW") {
        return handleNew();
    }
    else if (cmd == "RUN") {
        return handleRun();
    }
    else if (cmd == "SAVE") {
        if (parseResult.args.empty()) {
            return BatchCommandResult(false, "", "SAVE requires a filename");
        }
        return handleSave(parseResult.args[0]);
    }
    else if (cmd == "LOAD") {
        if (parseResult.args.empty()) {
            return BatchCommandResult(false, "", "LOAD requires a filename");
        }
        return handleLoad(parseResult.args[0]);
    }
    else if (cmd == "CREATECART") {
        if (parseResult.args.empty()) {
            return BatchCommandResult(false, "", "CREATECART requires a path");
        }
        return handleCreateCart(parseResult.args[0]);
    }
    else if (cmd == "USECART") {
        if (parseResult.args.empty()) {
            return BatchCommandResult(false, "", "USECART requires a path");
        }
        return handleUseCart(parseResult.args[0]);
    }
    else if (cmd == "SAVECART") {
        return handleSaveCart();
    }
    else if (cmd == "CLOSECART") {
        return handleCloseCart();
    }
    else if (cmd == "COPY") {
        if (parseResult.args.size() < 3) {
            return BatchCommandResult(false, "", "COPY requires: COPY <type> <src> <dest>");
        }
        std::string type = parseResult.args[0];
        std::transform(type.begin(), type.end(), type.begin(), ::toupper);
        
        if (type == "SCRIPT") {
            return handleCopyScript(parseResult.args[1], parseResult.args[2]);
        } else {
            return handleCopyAsset(type, parseResult.args[1], parseResult.args[2]);
        }
    }
    else {
        return BatchCommandResult(false, "", "Unknown command: " + cmd);
    }
}

BatchCommandResult BatchInterpreter::executeLines(const std::string& lines) {
    std::istringstream stream(lines);
    std::string line;
    std::ostringstream output;
    std::ostringstream errors;
    bool allSuccess = true;
    
    while (std::getline(stream, line)) {
        auto result = executeLine(line);
        
        if (!result.output.empty()) {
            output << result.output;
            if (result.output.back() != '\n') {
                output << '\n';
            }
        }
        
        if (!result.success) {
            allSuccess = false;
            if (!result.error.empty()) {
                errors << result.error;
                if (result.error.back() != '\n') {
                    errors << '\n';
                }
            }
        }
    }
    
    return BatchCommandResult(allSuccess, output.str(), errors.str());
}

std::vector<std::string> BatchInterpreter::getProgramListing() const {
    if (!_programManager) {
        return {};
    }
    
    std::vector<std::string> lines;
    auto programLines = _programManager->getLines();
    
    for (const auto& line : programLines) {
        std::ostringstream oss;
        oss << line.lineNumber << " " << line.content;
        lines.push_back(oss.str());
    }
    
    return lines;
}

void BatchInterpreter::clearProgram() {
    if (_programManager) {
        _programManager->clear();
    }
}

// ============================================================================
// Command Handlers
// ============================================================================

BatchCommandResult BatchInterpreter::handleNumberedLine(int lineNumber, const std::string& content) {
    if (content.empty()) {
        // Delete line
        _programManager->deleteLine(lineNumber);
        return BatchCommandResult(true, "", "");
    } else {
        // Add/replace line
        _programManager->addLine(lineNumber, content);
        return BatchCommandResult(true, "", "");
    }
}

BatchCommandResult BatchInterpreter::handleList() {
    auto lines = getProgramListing();
    std::ostringstream output;
    
    for (const auto& line : lines) {
        output << line << "\n";
    }
    
    std::string result = output.str();
    writeOutput(result);
    return BatchCommandResult(true, result, "");
}

BatchCommandResult BatchInterpreter::handleNew() {
    clearProgram();
    std::string msg = "Program cleared.\n";
    writeOutput(msg);
    return BatchCommandResult(true, msg, "");
}

BatchCommandResult BatchInterpreter::handleRun() {
    // Compile the program
    std::string luaCode = compileProgram();
    if (luaCode.empty()) {
        std::string err = "Compilation failed.\n";
        writeError(err);
        return BatchCommandResult(false, "", err);
    }
    
    // Execute the Lua code
    if (!executeCompiledLua(luaCode)) {
        std::string err = "Execution failed.\n";
        writeError(err);
        return BatchCommandResult(false, "", err);
    }
    
    return BatchCommandResult(true, "", "");
}

BatchCommandResult BatchInterpreter::handleSave(const std::string& filename) {
    std::ofstream file(filename);
    if (!file) {
        std::string err = "Failed to open file: " + filename + "\n";
        writeError(err);
        return BatchCommandResult(false, "", err);
    }
    
    auto lines = getProgramListing();
    for (const auto& line : lines) {
        file << line << "\n";
    }
    
    file.close();
    
    std::string msg = "Saved to: " + filename + "\n";
    writeOutput(msg);
    return BatchCommandResult(true, msg, "");
}

BatchCommandResult BatchInterpreter::handleLoad(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        std::string err = "Failed to open file: " + filename + "\n";
        writeError(err);
        return BatchCommandResult(false, "", err);
    }
    
    // Clear current program
    clearProgram();
    
    // Load lines
    std::string line;
    while (std::getline(file, line)) {
        // Try to parse as numbered line
        auto result = _commandParser->parse(line);
        if (result.type == FasterBASIC::CommandType::NUMBERED_LINE) {
            _programManager->addLine(result.lineNumber, result.content);
        }
    }
    
    file.close();
    
    std::string msg = "Loaded from: " + filename + "\n";
    writeOutput(msg);
    return BatchCommandResult(true, msg, "");
}

BatchCommandResult BatchInterpreter::handleCreateCart(const std::string& path) {
    if (!_cartManager) {
        return BatchCommandResult(false, "", "Cart manager not initialized");
    }
    
    // TODO: Implement synchronous cart creation
    // This requires CartManager API modifications to support synchronous operations
    
    std::string msg = "CREATECART not yet implemented in batch mode\n";
    writeError(msg);
    return BatchCommandResult(false, "", msg);
}

BatchCommandResult BatchInterpreter::handleUseCart(const std::string& path) {
    if (!_cartManager) {
        return BatchCommandResult(false, "", "Cart manager not initialized");
    }
    
    // TODO: Implement synchronous cart opening
    
    std::string msg = "USECART not yet implemented in batch mode\n";
    writeError(msg);
    return BatchCommandResult(false, "", msg);
}

BatchCommandResult BatchInterpreter::handleSaveCart() {
    if (!_cartManager) {
        return BatchCommandResult(false, "", "Cart manager not initialized");
    }
    
    // TODO: Implement synchronous cart saving
    
    std::string msg = "SAVECART not yet implemented in batch mode\n";
    writeError(msg);
    return BatchCommandResult(false, "", msg);
}

BatchCommandResult BatchInterpreter::handleCloseCart() {
    if (!_cartManager) {
        return BatchCommandResult(false, "", "Cart manager not initialized");
    }
    
    // TODO: Implement synchronous cart closing
    
    std::string msg = "CLOSECART not yet implemented in batch mode\n";
    writeError(msg);
    return BatchCommandResult(false, "", msg);
}

BatchCommandResult BatchInterpreter::handleCopyScript(const std::string& src, const std::string& dest) {
    // TODO: Implement synchronous script copying
    
    std::string msg = "COPY SCRIPT not yet implemented in batch mode\n";
    writeError(msg);
    return BatchCommandResult(false, "", msg);
}

BatchCommandResult BatchInterpreter::handleCopyAsset(const std::string& type, const std::string& src, const std::string& dest) {
    // TODO: Implement synchronous asset copying
    
    std::string msg = "COPY " + type + " not yet implemented in batch mode\n";
    writeError(msg);
    return BatchCommandResult(false, "", msg);
}

// ============================================================================
// Lua Management
// ============================================================================

void BatchInterpreter::initializeLua() {
    if (_luaState) {
        return;
    }
    
    _luaState = luaL_newstate();
    luaL_openlibs(_luaState);
    
    // NOTE: Batch mode does NOT override os.exit() - we want it to actually exit
    // when the script ends or calls END. This is different from interactive mode.
    
    // TODO: Register SuperTerminal API functions for batch mode
    // This may require special handling for headless mode
}

void BatchInterpreter::shutdownLua() {
    if (_luaState) {
        lua_close(_luaState);
        _luaState = nullptr;
    }
}

std::string BatchInterpreter::compileProgram() {
    // Get program source
    auto lines = getProgramListing();
    std::ostringstream sourceStream;
    for (const auto& line : lines) {
        sourceStream << line << "\n";
    }
    std::string source = sourceStream.str();
    
    if (source.empty()) {
        writeError("No program to compile.\n");
        return "";
    }
    
    try {
        // Preprocess DATA statements
        DataPreprocessorResult dataResult = preprocessDataStatements(source);
        
        // Lex
        Lexer lexer(dataResult.processedSource);
        auto tokens = lexer.tokenize();
        
        // Parse
        Parser parser(tokens);
        auto ast = parser.parse();
        
        // Semantic analysis
        SemanticAnalyzer analyzer;
        analyzer.analyze(ast.get());
        
        // Optimize
        Optimizer optimizer;
        optimizer.optimize(ast.get());
        
        // Peephole optimization
        PeepholeOptimizer peepholeOpt;
        peepholeOpt.optimize(ast.get());
        
        // Build CFG
        CFGBuilder cfgBuilder;
        auto cfg = cfgBuilder.build(ast.get());
        
        // Generate IR
        IRGenerator irGen;
        auto irCode = irGen.generate(cfg.get());
        
        // Generate Lua code
        LuaCodeGenerator luaGen;
        std::string luaCode = luaGen.generate(irCode.get());
        
        return luaCode;
        
    } catch (const std::exception& e) {
        writeError(std::string("Compilation error: ") + e.what() + "\n");
        return "";
    }
}

bool BatchInterpreter::executeCompiledLua(const std::string& luaCode) {
    if (!_luaState) {
        writeError("Lua state not initialized.\n");
        return false;
    }
    
    // Load and execute the Lua code
    int loadResult = luaL_loadstring(_luaState, luaCode.c_str());
    if (loadResult != LUA_OK) {
        std::string err = "Lua load error: ";
        if (lua_isstring(_luaState, -1)) {
            err += lua_tostring(_luaState, -1);
        }
        err += "\n";
        writeError(err);
        lua_pop(_luaState, 1);
        return false;
    }
    
    int execResult = lua_pcall(_luaState, 0, 0, 0);
    if (execResult != LUA_OK) {
        std::string err = "Lua execution error: ";
        if (lua_isstring(_luaState, -1)) {
            err += lua_tostring(_luaState, -1);
        }
        err += "\n";
        writeError(err);
        lua_pop(_luaState, 1);
        return false;
    }
    
    return true;
}

void BatchInterpreter::writeOutput(const std::string& message) {
    if (_outputStream) {
        *_outputStream << message;
        _outputStream->flush();
    }
}

void BatchInterpreter::writeError(const std::string& message) {
    if (_errorStream) {
        *_errorStream << message;
        _errorStream->flush();
    }
}

} // namespace BatchMode
} // namespace FBRunner3