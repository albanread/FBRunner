//
// register_voice.cpp
// Shared Voice Registration - Constants, Commands, and Lua Functions
//
// Provides shared registration functions for voice synthesis constants,
// commands, and Lua bindings. Used by both FBRunner3 (IDE) and fbsh_voices (terminal).
//

#include "register_voice.h"
#include "command_registry_superterminal.h"
#include "../FasterBASICT/src/fasterbasic_semantic.h"

#ifndef VOICE_ONLY
#include "FBTBindings.h"
#endif

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

// Include the voice API for Lua bindings
#include "../Framework/API/superterminal_api.h"

namespace FBRunner3 {
namespace VoiceRegistration {

// Register all voice-related constants
void registerVoiceConstants(FasterBASIC::ConstantsManager& constants) {
    // Waveform types
    constants.addConstant("WAVE_SILENCE", (int64_t)0);
    constants.addConstant("WAVE_SINE", (int64_t)1);
    constants.addConstant("WAVE_SQUARE", (int64_t)2);
    constants.addConstant("WAVE_SAWTOOTH", (int64_t)3);
    constants.addConstant("WAVE_TRIANGLE", (int64_t)4);
    constants.addConstant("WAVE_NOISE", (int64_t)5);
    constants.addConstant("WAVE_PULSE", (int64_t)6);
    constants.addConstant("WAVE_PHYSICAL", (int64_t)7);

    // Physical model types
    constants.addConstant("MODEL_PLUCKED_STRING", (int64_t)0);
    constants.addConstant("MODEL_STRING", (int64_t)0);  // Alias
    constants.addConstant("MODEL_BOWED_STRING", (int64_t)1);
    constants.addConstant("MODEL_BAR", (int64_t)1);  // Alias
    constants.addConstant("MODEL_BELL", (int64_t)1);  // Alias
    constants.addConstant("MODEL_BLOWN_TUBE", (int64_t)2);
    constants.addConstant("MODEL_TUBE", (int64_t)2);  // Alias
    constants.addConstant("MODEL_FLUTE", (int64_t)2);  // Alias
    constants.addConstant("MODEL_DRUMHEAD", (int64_t)3);
    constants.addConstant("MODEL_DRUM", (int64_t)3);  // Alias
    constants.addConstant("MODEL_GLASS", (int64_t)4);

    // Filter types
    constants.addConstant("FILTER_NONE", (int64_t)0);
    constants.addConstant("FILTER_LOWPASS", (int64_t)1);
    constants.addConstant("FILTER_HIGHPASS", (int64_t)2);
    constants.addConstant("FILTER_BANDPASS", (int64_t)3);

    // LFO waveforms
    constants.addConstant("LFO_SINE", (int64_t)0);
    constants.addConstant("LFO_SQUARE", (int64_t)1);
    constants.addConstant("LFO_SAW", (int64_t)2);
    constants.addConstant("LFO_SAWTOOTH", (int64_t)2);  // Alias
    constants.addConstant("LFO_TRIANGLE", (int64_t)3);
    constants.addConstant("LFO_RANDOM", (int64_t)4);

    // LFO targets
    constants.addConstant("LFO_TARGET_PITCH", (int64_t)0);
    constants.addConstant("LFO_TARGET_VOLUME", (int64_t)1);
    constants.addConstant("LFO_TARGET_PAN", (int64_t)2);
    constants.addConstant("LFO_TARGET_CUTOFF", (int64_t)3);
    constants.addConstant("LFO_TARGET_PULSE_WIDTH", (int64_t)4);
}

// Register all voice-related commands
void registerVoiceCommands(FasterBASIC::ModularCommands::CommandRegistry& registry) {
    // Delegate to SuperTerminalCommandRegistry which has all the voice commands
    SuperTerminalCommands::SuperTerminalCommandRegistry::registerVoiceCommands(registry);
}

// Register all voice-related functions
void registerVoiceFunctions(FasterBASIC::ModularCommands::CommandRegistry& registry) {
    // Delegate to SuperTerminalCommandRegistry which has all the voice functions
    SuperTerminalCommands::SuperTerminalCommandRegistry::registerVoiceFunctions(registry);
}

// Convenience: register both constants, commands, and functions
void registerVoiceSystem(
    FasterBASIC::ConstantsManager& constants,
    FasterBASIC::ModularCommands::CommandRegistry& registry)
{
    registerVoiceConstants(constants);
    registerVoiceCommands(registry);
    registerVoiceFunctions(registry);
}

// Voice-only mode Lua wrapper functions
#ifdef VOICE_ONLY
static int lua_voice_set_waveform(lua_State* L) {
    st_voice_set_waveform(lua_tointeger(L, 1), lua_tointeger(L, 2));
    return 0;
}
static int lua_voice_set_frequency(lua_State* L) {
    st_voice_set_frequency(lua_tointeger(L, 1), lua_tonumber(L, 2));
    return 0;
}
static int lua_voice_set_note(lua_State* L) {
    st_voice_set_note(lua_tointeger(L, 1), lua_tointeger(L, 2));
    return 0;
}
static int lua_voice_set_volume(lua_State* L) {
    st_voice_set_volume(lua_tointeger(L, 1), lua_tonumber(L, 2));
    return 0;
}
static int lua_voice_set_envelope(lua_State* L) {
    st_voice_set_envelope(lua_tointeger(L, 1), lua_tonumber(L, 2),
                         lua_tonumber(L, 3), lua_tonumber(L, 4), lua_tonumber(L, 5));
    return 0;
}
static int lua_voice_set_gate(lua_State* L) {
    st_voice_set_gate(lua_tointeger(L, 1), lua_tointeger(L, 2));
    return 0;
}
static int lua_voice_set_pulse_width(lua_State* L) {
    st_voice_set_pulse_width(lua_tointeger(L, 1), lua_tonumber(L, 2));
    return 0;
}
static int lua_voice_set_filter_routing(lua_State* L) {
    st_voice_set_filter_routing(lua_tointeger(L, 1), lua_tointeger(L, 2));
    return 0;
}
static int lua_voice_set_filter_type(lua_State* L) {
    st_voice_set_filter_type(lua_tointeger(L, 1));
    return 0;
}
static int lua_voice_set_filter_cutoff(lua_State* L) {
    st_voice_set_filter_cutoff(lua_tonumber(L, 1));
    return 0;
}
static int lua_voice_set_filter_resonance(lua_State* L) {
    st_voice_set_filter_resonance(lua_tonumber(L, 1));
    return 0;
}
static int lua_voice_set_filter_enabled(lua_State* L) {
    st_voice_set_filter_enabled(lua_tointeger(L, 1));
    return 0;
}
static int lua_voice_set_detune(lua_State* L) {
    st_voice_set_detune(lua_tointeger(L, 1), lua_tonumber(L, 2));
    return 0;
}
static int lua_voice_set_portamento(lua_State* L) {
    st_voice_set_portamento(lua_tointeger(L, 1), lua_tonumber(L, 2));
    return 0;
}
static int lua_voice_set_ring_mod(lua_State* L) {
    st_voice_set_ring_mod(lua_tointeger(L, 1), lua_tointeger(L, 2));
    return 0;
}
static int lua_voice_set_sync(lua_State* L) {
    st_voice_set_sync(lua_tointeger(L, 1), lua_tointeger(L, 2));
    return 0;
}
static int lua_voice_set_master_volume(lua_State* L) {
    st_voice_set_master_volume(lua_tonumber(L, 1));
    return 0;
}
static int lua_voice_set_delay_enable(lua_State* L) {
    st_voice_set_delay_enable(lua_tointeger(L, 1), lua_tointeger(L, 2));
    return 0;
}
static int lua_voice_set_delay_time(lua_State* L) {
    st_voice_set_delay_time(lua_tointeger(L, 1), lua_tonumber(L, 2));
    return 0;
}
static int lua_voice_set_delay_feedback(lua_State* L) {
    st_voice_set_delay_feedback(lua_tointeger(L, 1), lua_tonumber(L, 2));
    return 0;
}
static int lua_voice_set_delay_mix(lua_State* L) {
    st_voice_set_delay_mix(lua_tointeger(L, 1), lua_tonumber(L, 2));
    return 0;
}
static int lua_voice_reset_all(lua_State* L) {
    st_voice_reset_all();
    return 0;
}
static int lua_voice_set_pan(lua_State* L) {
    st_voice_set_pan(lua_tointeger(L, 1), lua_tonumber(L, 2));
    return 0;
}
static int lua_voice_set_physical_model(lua_State* L) {
    st_voice_set_physical_model(lua_tointeger(L, 1), lua_tointeger(L, 2));
    return 0;
}
static int lua_voice_set_physical_damping(lua_State* L) {
    st_voice_set_physical_damping(lua_tointeger(L, 1), lua_tonumber(L, 2));
    return 0;
}
static int lua_voice_set_physical_brightness(lua_State* L) {
    st_voice_set_physical_brightness(lua_tointeger(L, 1), lua_tonumber(L, 2));
    return 0;
}
static int lua_voice_set_physical_excitation(lua_State* L) {
    st_voice_set_physical_excitation(lua_tointeger(L, 1), lua_tonumber(L, 2));
    return 0;
}
static int lua_voice_set_physical_resonance(lua_State* L) {
    st_voice_set_physical_resonance(lua_tointeger(L, 1), lua_tonumber(L, 2));
    return 0;
}
static int lua_voice_set_physical_pressure(lua_State* L) {
    st_voice_set_physical_pressure(lua_tointeger(L, 1), lua_tonumber(L, 2));
    return 0;
}
static int lua_voice_physical_trigger(lua_State* L) {
    st_voice_physical_trigger(lua_tointeger(L, 1));
    return 0;
}
static int lua_voice_filter(lua_State* L) {
    // voice_filter voice, cutoff, resonance, type
    int voice = lua_tointeger(L, 1);
    float cutoff = lua_tonumber(L, 2);
    float resonance = lua_tonumber(L, 3);
    int type = lua_tointeger(L, 4);
    
    // Set all filter parameters at once (convenience command)
    st_voice_set_filter_cutoff(cutoff);
    st_voice_set_filter_resonance(resonance);
    st_voice_set_filter_type(type);
    st_voice_set_filter_routing(voice, 1);  // Enable filter for this voice
    
    return 0;
}
static int lua_voice_waveform_at(lua_State* L) {
    // voice_waveform_at voice, beat, waveform
    int voice = lua_tointeger(L, 1);
    float beat = lua_tonumber(L, 2);
    int waveform = lua_tointeger(L, 3);
    
    st_voice_waveform_at(voice, beat, waveform);
    return 0;
}
static int lua_voice_envelope_at(lua_State* L) {
    // voice_envelope_at voice, beat, attack, decay, sustain, release
    int voice = lua_tointeger(L, 1);
    float beat = lua_tonumber(L, 2);
    float attack = lua_tonumber(L, 3);
    float decay = lua_tonumber(L, 4);
    float sustain = lua_tonumber(L, 5);
    float release = lua_tonumber(L, 6);
    
    st_voice_envelope_at(voice, beat, attack, decay, sustain, release);
    return 0;
}
static int lua_voice_frequency_at(lua_State* L) {
    // voice_frequency_at voice, beat, frequency
    int voice = lua_tointeger(L, 1);
    float beat = lua_tonumber(L, 2);
    float frequency = lua_tonumber(L, 3);
    
    st_voice_frequency_at(voice, beat, frequency);
    return 0;
}
static int lua_voice_volume_at(lua_State* L) {
    // voice_volume_at voice, beat, volume
    int voice = lua_tointeger(L, 1);
    float beat = lua_tonumber(L, 2);
    float volume = lua_tonumber(L, 3);
    
    st_voice_volume_at(voice, beat, volume);
    return 0;
}
static int lua_voice_pan_at(lua_State* L) {
    // voice_pan_at voice, beat, pan
    int voice = lua_tointeger(L, 1);
    float beat = lua_tonumber(L, 2);
    float pan = lua_tonumber(L, 3);
    
    st_voice_pan_at(voice, beat, pan);
    return 0;
}
static int lua_voice_gate_at(lua_State* L) {
    // voice_gate_at voice, beat, gate
    int voice = lua_tointeger(L, 1);
    float beat = lua_tonumber(L, 2);
    int gate = lua_tointeger(L, 3);
    
    st_voice_gate_at(voice, beat, gate);
    return 0;
}
static int lua_voice_filter_at(lua_State* L) {
    // voice_filter_at voice, beat, cutoff, resonance, type
    int voice = lua_tointeger(L, 1);
    float beat = lua_tonumber(L, 2);
    float cutoff = lua_tonumber(L, 3);
    float resonance = lua_tonumber(L, 4);
    int type = lua_tointeger(L, 5);
    
    st_voice_filter_at(voice, beat, cutoff, resonance, type);
    
    return 0;
}
static int lua_lfo_set_waveform(lua_State* L) {
    st_lfo_set_waveform(lua_tointeger(L, 1), lua_tointeger(L, 2));
    return 0;
}
static int lua_lfo_set_rate(lua_State* L) {
    st_lfo_set_rate(lua_tointeger(L, 1), lua_tonumber(L, 2));
    return 0;
}
static int lua_lfo_to_pitch(lua_State* L) {
    st_lfo_to_pitch(lua_tointeger(L, 1), lua_tointeger(L, 2), lua_tonumber(L, 3));
    return 0;
}
static int lua_lfo_to_volume(lua_State* L) {
    st_lfo_to_volume(lua_tointeger(L, 1), lua_tointeger(L, 2), lua_tonumber(L, 3));
    return 0;
}
static int lua_lfo_to_filter(lua_State* L) {
    st_lfo_to_filter(lua_tointeger(L, 1), lua_tointeger(L, 2), lua_tonumber(L, 3));
    return 0;
}
static int lua_lfo_to_pulsewidth(lua_State* L) {
    st_lfo_to_pulsewidth(lua_tointeger(L, 1), lua_tointeger(L, 2), lua_tonumber(L, 3));
    return 0;
}
static int lua_lfo_reset(lua_State* L) {
    st_lfo_reset(lua_tointeger(L, 1));
    return 0;
}
static int lua_voice_wait(lua_State* L) {
    st_voice_wait(lua_tonumber(L, 1));
    return 0;
}
static int lua_voices_start(lua_State* L) {
    st_voices_start();
    return 0;
}
static int lua_voices_set_tempo(lua_State* L) {
    st_voices_set_tempo(lua_tonumber(L, 1));
    return 0;
}
static int lua_voices_end_slot(lua_State* L) {
    st_voices_end_slot(lua_tointeger(L, 1), lua_tonumber(L, 2));
    return 0;
}
static int lua_voices_next_slot(lua_State* L) {
    uint32_t soundId = st_voices_next_slot(lua_tonumber(L, 1));
    lua_pushinteger(L, soundId);
    return 1;
}
static int lua_voices_end_play(lua_State* L) {
    st_voices_end_play();
    return 0;
}
static int lua_voices_end_save(lua_State* L) {
    st_voices_end_save(lua_tostring(L, 1));
    return 0;
}

#endif

// Register Lua bindings for voice system (called at runtime during program execution)
void registerVoiceLuaBindings(lua_State* L) {
#ifndef VOICE_ONLY
    SuperTerminal::FBTBindings::registerVoiceBindings(L);
#else
    // Voice-only mode: Register Lua bindings directly to C API functions
    lua_register(L, "voice_set_waveform", lua_voice_set_waveform);
    lua_register(L, "voice_set_frequency", lua_voice_set_frequency);
    lua_register(L, "voice_set_note", lua_voice_set_note);
    lua_register(L, "voice_set_volume", lua_voice_set_volume);
    lua_register(L, "voice_set_envelope", lua_voice_set_envelope);
    lua_register(L, "voice_set_gate", lua_voice_set_gate);
    lua_register(L, "voice_set_pulse_width", lua_voice_set_pulse_width);
    lua_register(L, "voice_set_filter_routing", lua_voice_set_filter_routing);
    lua_register(L, "voice_set_filter_type", lua_voice_set_filter_type);
    lua_register(L, "voice_set_filter_cutoff", lua_voice_set_filter_cutoff);
    lua_register(L, "voice_set_filter_resonance", lua_voice_set_filter_resonance);
    lua_register(L, "voice_set_filter_enabled", lua_voice_set_filter_enabled);
    lua_register(L, "voice_set_detune", lua_voice_set_detune);
    lua_register(L, "voice_set_portamento", lua_voice_set_portamento);
    lua_register(L, "voice_set_ring_mod", lua_voice_set_ring_mod);
    lua_register(L, "voice_set_sync", lua_voice_set_sync);
    lua_register(L, "voice_set_master_volume", lua_voice_set_master_volume);
    lua_register(L, "voice_set_delay_enable", lua_voice_set_delay_enable);
    lua_register(L, "voice_set_delay_time", lua_voice_set_delay_time);
    lua_register(L, "voice_set_delay_feedback", lua_voice_set_delay_feedback);
    lua_register(L, "voice_set_delay_mix", lua_voice_set_delay_mix);
    lua_register(L, "voice_reset_all", lua_voice_reset_all);
    lua_register(L, "voice_set_pan", lua_voice_set_pan);
    lua_register(L, "voice_set_physical_model", lua_voice_set_physical_model);
    lua_register(L, "voice_set_physical_damping", lua_voice_set_physical_damping);
    lua_register(L, "voice_set_physical_brightness", lua_voice_set_physical_brightness);
    lua_register(L, "voice_set_physical_excitation", lua_voice_set_physical_excitation);
    lua_register(L, "voice_set_physical_resonance", lua_voice_set_physical_resonance);
    lua_register(L, "voice_set_physical_pressure", lua_voice_set_physical_pressure);
    lua_register(L, "voice_physical_trigger", lua_voice_physical_trigger);
    lua_register(L, "voice_filter", lua_voice_filter);
    lua_register(L, "voice_filter_at", lua_voice_filter_at);
    lua_register(L, "voice_waveform_at", lua_voice_waveform_at);
    lua_register(L, "voice_envelope_at", lua_voice_envelope_at);
    lua_register(L, "voice_frequency_at", lua_voice_frequency_at);
    lua_register(L, "voice_volume_at", lua_voice_volume_at);
    lua_register(L, "voice_pan_at", lua_voice_pan_at);
    lua_register(L, "voice_gate_at", lua_voice_gate_at);
    lua_register(L, "lfo_set_waveform", lua_lfo_set_waveform);
    lua_register(L, "lfo_set_rate", lua_lfo_set_rate);
    lua_register(L, "lfo_to_pitch", lua_lfo_to_pitch);
    lua_register(L, "lfo_to_volume", lua_lfo_to_volume);
    lua_register(L, "lfo_to_filter", lua_lfo_to_filter);
    lua_register(L, "lfo_to_pulsewidth", lua_lfo_to_pulsewidth);
    lua_register(L, "lfo_reset", lua_lfo_reset);
    lua_register(L, "voice_wait", lua_voice_wait);
    lua_register(L, "voices_start", lua_voices_start);
    lua_register(L, "voices_set_tempo", lua_voices_set_tempo);
    lua_register(L, "voices_end_slot", lua_voices_end_slot);
    lua_register(L, "voices_next_slot", lua_voices_next_slot);
    lua_register(L, "voices_end_play", lua_voices_end_play);
    lua_register(L, "voices_end_save", lua_voices_end_save);
#endif
}

} // namespace VoiceRegistration
} // namespace FBRunner3
