//
// command_registry_superterminal.h
// FBRunner3 - SuperTerminal BASIC Commands Registry
//
// Defines SuperTerminal-specific BASIC commands and functions for FBRunner3.
// These commands provide access to SuperTerminal's graphics, audio, input,
// and other multimedia features.
//

#ifndef FBRUNNER3_COMMAND_REGISTRY_SUPERTERMINAL_H
#define FBRUNNER3_COMMAND_REGISTRY_SUPERTERMINAL_H

#include "../FasterBASICT/src/modular_commands.h"

namespace FBRunner3 {
namespace SuperTerminalCommands {

// =============================================================================
// SuperTerminal Command Registration
// =============================================================================

class SuperTerminalCommandRegistry {
public:
    // Register all SuperTerminal-specific commands and functions
    static void registerSuperTerminalCommands(FasterBASIC::ModularCommands::CommandRegistry& registry);
    static void registerSuperTerminalFunctions(FasterBASIC::ModularCommands::CommandRegistry& registry);
    
    // Register individual command categories
    static void registerTextCommands(FasterBASIC::ModularCommands::CommandRegistry& registry);
    static void registerGraphicsCommands(FasterBASIC::ModularCommands::CommandRegistry& registry);
    static void registerAudioCommands(FasterBASIC::ModularCommands::CommandRegistry& registry);
    static void registerInputCommands(FasterBASIC::ModularCommands::CommandRegistry& registry);
    static void registerUtilityCommands(FasterBASIC::ModularCommands::CommandRegistry& registry);
    static void registerSpriteCommands(FasterBASIC::ModularCommands::CommandRegistry& registry);
    static void registerParticleCommands(FasterBASIC::ModularCommands::CommandRegistry& registry);
    static void registerChunkyGraphicsCommands(FasterBASIC::ModularCommands::CommandRegistry& registry);
    static void registerSixelCommands(FasterBASIC::ModularCommands::CommandRegistry& registry);
    static void registerTilemapCommands(FasterBASIC::ModularCommands::CommandRegistry& registry);
    static void registerRectangleCommands(FasterBASIC::ModularCommands::CommandRegistry& registry);
    static void registerCircleCommands(FasterBASIC::ModularCommands::CommandRegistry& registry);
    static void registerLineCommands(FasterBASIC::ModularCommands::CommandRegistry& registry);
    static void registerVideoModeCommands(FasterBASIC::ModularCommands::CommandRegistry& registry);
    
    // Register voice/audio synthesis commands (used by fbsh_voices)
    static void registerVoiceCommands(FasterBASIC::ModularCommands::CommandRegistry& registry);
    static void registerVoiceFunctions(FasterBASIC::ModularCommands::CommandRegistry& registry);
    
    // Register SuperTerminal function categories
    static void registerTilemapFunctions(FasterBASIC::ModularCommands::CommandRegistry& registry);
    static void registerSystemFunctions(FasterBASIC::ModularCommands::CommandRegistry& registry);
    static void registerRectangleFunctions(FasterBASIC::ModularCommands::CommandRegistry& registry);
    static void registerCircleFunctions(FasterBASIC::ModularCommands::CommandRegistry& registry);
    static void registerLineFunctions(FasterBASIC::ModularCommands::CommandRegistry& registry);
    static void registerVideoModeFunctions(FasterBASIC::ModularCommands::CommandRegistry& registry);
    
private:
    // Helper methods for text commands
    static void registerPrintAtCommands(FasterBASIC::ModularCommands::CommandRegistry& registry);
    static void registerTextPositioningCommands(FasterBASIC::ModularCommands::CommandRegistry& registry);
    static void registerTextManipulationCommands(FasterBASIC::ModularCommands::CommandRegistry& registry);
    
    // Helper methods for graphics commands  
    static void registerBasicGraphicsCommands(FasterBASIC::ModularCommands::CommandRegistry& registry);
    static void registerAdvancedGraphicsCommands(FasterBASIC::ModularCommands::CommandRegistry& registry);
    
    // Helper methods for audio commands
    static void registerMusicCommands(FasterBASIC::ModularCommands::CommandRegistry& registry);
    static void registerSIDCommands(FasterBASIC::ModularCommands::CommandRegistry& registry);
    static void registerSynthCommands(FasterBASIC::ModularCommands::CommandRegistry& registry);
    
    // Helper methods for sprite commands
    static void registerSpriteManagementCommands(FasterBASIC::ModularCommands::CommandRegistry& registry);
    static void registerSpriteEffectsCommands(FasterBASIC::ModularCommands::CommandRegistry& registry);
    static void registerSpriteFunctions(FasterBASIC::ModularCommands::CommandRegistry& registry);
    
    // Helper methods for sound bank functions
    static void registerSoundBankFunctions(FasterBASIC::ModularCommands::CommandRegistry& registry);
};

// =============================================================================
// Convenience Functions
// =============================================================================

// Initialize a registry with core + SuperTerminal commands
void initializeSuperTerminalRegistry(FasterBASIC::ModularCommands::CommandRegistry& registry);

// Create a pre-initialized SuperTerminal registry
FasterBASIC::ModularCommands::CommandRegistry createSuperTerminalRegistry();

} // namespace SuperTerminalCommands  
} // namespace FBRunner3

#endif // FBRUNNER3_COMMAND_REGISTRY_SUPERTERMINAL_H