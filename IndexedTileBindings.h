//
// IndexedTileBindings.h
// FBRunner3 - Lua bindings for indexed tile rendering
//
// Header for Lua bindings for PaletteBank, TilesetIndexed, TilemapEx
// Copyright © 2024 SuperTerminal. All rights reserved.
//

#ifndef INDEXEDTILEBINDINGS_H
#define INDEXEDTILEBINDINGS_H

struct lua_State;

namespace SuperTerminal {
namespace IndexedTileBindings {

/// Register indexed tile rendering bindings in Lua state
/// @param L Lua state
void registerBindings(lua_State* L);

} // namespace IndexedTileBindings
} // namespace SuperTerminal

#endif // INDEXEDTILEBINDINGS_H