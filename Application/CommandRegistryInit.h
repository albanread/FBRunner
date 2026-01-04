//
// CommandRegistryInit.h
// FasterBASIC - Command Registry Initialization
//
// Initializes the FasterBASIC command registry with core and SuperTerminal commands.
// This must be called before any FasterBASIC compiler operations.
//

#ifndef COMMANDREGISTRYINIT_H
#define COMMANDREGISTRYINIT_H

namespace FasterBASIC {
namespace ModularCommands {
    class CommandRegistry;
}
}

namespace FBRunner3 {
namespace SuperTerminalCommands {

/**
 * @brief Initialize the FBRunner3 command registry
 * 
 * This function initializes the global FasterBASIC command registry with:
 * - Core commands (math, string, file I/O, etc.)
 * - Core functions
 * - SuperTerminal graphical commands
 * - SuperTerminal functions
 * 
 * This MUST be called before creating any FasterBASIC Lexer objects or
 * performing any compilation operations.
 * 
 * @note This function is idempotent - calling it multiple times is safe.
 */
void initializeFBRunner3CommandRegistry();

/**
 * @brief Get the initialized SuperTerminal command registry
 * 
 * Returns a pointer to the global command registry that has been initialized
 * with all core and SuperTerminal commands.
 * 
 * @return Pointer to the global command registry
 * @note Must call initializeFBRunner3CommandRegistry() first
 */
FasterBASIC::ModularCommands::CommandRegistry* getSuperTerminalRegistry();

} // namespace SuperTerminalCommands
} // namespace FBRunner3

#endif // COMMANDREGISTRYINIT_H