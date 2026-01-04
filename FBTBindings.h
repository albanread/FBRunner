//
// FBTBindings.h
// FBRunner3 - Lua bindings for SuperTerminal API
//
// This file declares the Lua binding registration function for use with
// FasterBASICT-generated Lua code. These bindings provide the SuperTerminal
// API functions to compiled BASIC programs.
//

#ifndef FBTBINDINGS_H
#define FBTBINDINGS_H

#include <vector>
#include <string>
#include <map>

struct lua_State;

namespace SuperTerminal {
namespace FBTBindings {

// Register all SuperTerminal API functions in the Lua state (full IDE version)
void registerBindings(lua_State* L);

// Register ONLY voice/audio bindings (for terminal tools - no GUI)
void registerVoiceBindings(lua_State* L);

// Initialize DATA manager with compiled DATA values (parsed into typed variants)
void initializeDataManager(const std::vector<std::string>& values);

// Add restore point by line number
void addDataRestorePoint(int lineNumber, size_t index);

// Add restore point by label name
void addDataRestorePointByLabel(const std::string& labelName, size_t index);

// Clear DATA manager (call at end of script or before new script)
void clearDataManager();

// Initialize FileManager (call at start of script)
void initializeFileManager();

// Clear FileManager - close all files (call at end of script or before new script)
void clearFileManager();

} // namespace FBTBindings
} // namespace SuperTerminal

#endif // FBTBINDINGS_H