//
// register_voice.h
// Shared Voice Registration - Constants and Commands
//
// Provides shared registration functions for voice synthesis constants
// and commands. Used by both FBRunner3 (IDE) and fbsh_voices (terminal).
//

#ifndef FBRUNNER3_REGISTER_VOICE_H
#define FBRUNNER3_REGISTER_VOICE_H

#include "../FasterBASICT/src/modular_commands.h"

// Forward declare lua_State
struct lua_State;

namespace FasterBASIC {
    // Forward declare ConstantsManager
    class ConstantsManager;
}

namespace FBRunner3 {
namespace VoiceRegistration {

// Register all voice-related constants (waveforms, models, filters, LFO types)
void registerVoiceConstants(FasterBASIC::ConstantsManager& constants);

// Register all voice-related commands (VOICE_*, LFO_*, etc.)
void registerVoiceCommands(FasterBASIC::ModularCommands::CommandRegistry& registry);

// Register all voice-related functions (VOICE_GET_*, etc.)
void registerVoiceFunctions(FasterBASIC::ModularCommands::CommandRegistry& registry);

// Convenience: register both constants, commands, and functions
void registerVoiceSystem(
    FasterBASIC::ConstantsManager& constants,
    FasterBASIC::ModularCommands::CommandRegistry& registry);

// Register Lua bindings for voice system (runtime - called during program execution)
void registerVoiceLuaBindings(lua_State* L);

} // namespace VoiceRegistration
} // namespace FBRunner3

#endif // FBRUNNER3_REGISTER_VOICE_H