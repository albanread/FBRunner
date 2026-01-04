//
// CommandRegistryInit.cpp
// FasterBASIC - Command Registry Initialization
//
// Implementation of command registry initialization for FBRunner3.
//

#include "CommandRegistryInit.h"
#include "Debug/Logger.h"
#include <iostream>
#include "../FasterBASICT/src/modular_commands.h"
#include "../FasterBASICT/src/command_registry_core.h"
#include "../FasterBASICT/src/fasterbasic_semantic.h"
#include "../command_registry_superterminal.h"
#include "../register_voice.h"

using namespace FasterBASIC::ModularCommands;
using namespace FBRunner3::VoiceRegistration;

namespace FBRunner3 {
namespace SuperTerminalCommands {

void initializeFBRunner3CommandRegistry() {
    LOG_INFO("Starting registry initialization...");
    
    // Get the FasterBASIC global registry that the compiler uses
    CommandRegistry& registry = getGlobalCommandRegistry();
    LOG_INFO("Got global registry reference");

    // Clear it to ensure clean state
    registry.clear();
    LOG_INFO("Registry cleared");

    // Register core commands first (math, string, file I/O, etc.)
    CoreCommandRegistry::registerCoreCommands(registry);
    LOG_INFO("Core commands registered");
    
    CoreCommandRegistry::registerCoreFunctions(registry);
    LOG_INFO("Core functions registered");

    // Add all SuperTerminal graphical commands
    SuperTerminalCommandRegistry::registerSuperTerminalCommands(registry);
    LOG_INFO("SuperTerminal commands registered");
    
    SuperTerminalCommandRegistry::registerSuperTerminalFunctions(registry);
    LOG_INFO("SuperTerminal functions registered");
    
    // Register voice constants (WAVE_SINE, WAVE_SQUARE, etc.)
    // Note: We need to get the ConstantsManager from a SemanticAnalyzer instance
    // For now, this will be handled at compile time when the semantic analyzer is created
    LOG_INFO("Voice constants will be registered during semantic analysis");

    // Check registry state
    auto categories = registry.getCategories();
    auto allNames = registry.getAllNames();
    LOG_INFOF("Registry now has %zu categories, %zu commands", 
              categories.size(), allNames.size());
    
    if (!categories.empty()) {
        LOG_INFO("First 5 categories:");
        for (size_t i = 0; i < std::min(categories.size(), size_t(5)); i++) {
            LOG_INFOF("  - %s", categories[i].c_str());
        }
    }
    
    // CRITICAL: Mark the registry as initialized to prevent the lexer from clearing it later
    markGlobalRegistryInitialized();
    LOG_INFO("Registry marked as initialized");
    LOG_INFO("✓ Registry initialization COMPLETE");
}

FasterBASIC::ModularCommands::CommandRegistry* getSuperTerminalRegistry() {
    return &getGlobalCommandRegistry();
}

} // namespace SuperTerminalCommands
} // namespace FBRunner3