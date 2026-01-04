//
// IndexedTileBindings.cpp
// FBRunner3 - Lua bindings for indexed tile rendering
//
// Lua bindings for PaletteBank, TilesetIndexed, TilemapEx, and indexed rendering
// Copyright © 2024 SuperTerminal. All rights reserved.
//

#include "IndexedTileBindings.h"
#include "../Framework/Tilemap/TilemapIndexedAPI.h"
#include <lua.hpp>
#include <cstring>

namespace SuperTerminal {
namespace IndexedTileBindings {

// =============================================================================
// PaletteBank Lua Bindings
// =============================================================================

static int lua_palettebank_create(lua_State* L) {
    int32_t paletteCount = luaL_optinteger(L, 1, 32);
    int32_t colorsPerPalette = luaL_optinteger(L, 2, 16);
    
    PaletteBankHandle bank = palettebank_create(paletteCount, colorsPerPalette, nullptr);
    if (!bank) {
        return luaL_error(L, "Failed to create palette bank");
    }
    
    // Initialize GPU resources
    if (!palettebank_initialize(bank, nullptr)) {
        palettebank_destroy(bank);
        return luaL_error(L, "Failed to initialize palette bank GPU resources");
    }
    
    lua_pushlightuserdata(L, bank);
    return 1;
}

static int lua_palettebank_destroy(lua_State* L) {
    PaletteBankHandle bank = (PaletteBankHandle)lua_touserdata(L, 1);
    if (bank) {
        palettebank_destroy(bank);
    }
    return 0;
}

static int lua_palettebank_set_color(lua_State* L) {
    PaletteBankHandle bank = (PaletteBankHandle)lua_touserdata(L, 1);
    int32_t paletteIndex = luaL_checkinteger(L, 2);
    int32_t colorIndex = luaL_checkinteger(L, 3);
    uint8_t r = (uint8_t)luaL_checkinteger(L, 4);
    uint8_t g = (uint8_t)luaL_checkinteger(L, 5);
    uint8_t b = (uint8_t)luaL_checkinteger(L, 6);
    uint8_t a = (uint8_t)luaL_optinteger(L, 7, 255);
    
    bool success = palettebank_set_color(bank, paletteIndex, colorIndex, r, g, b, a);
    lua_pushboolean(L, success);
    return 1;
}

static int lua_palettebank_get_color(lua_State* L) {
    PaletteBankHandle bank = (PaletteBankHandle)lua_touserdata(L, 1);
    int32_t paletteIndex = luaL_checkinteger(L, 2);
    int32_t colorIndex = luaL_checkinteger(L, 3);
    
    uint8_t r, g, b, a;
    if (palettebank_get_color(bank, paletteIndex, colorIndex, &r, &g, &b, &a)) {
        lua_pushinteger(L, r);
        lua_pushinteger(L, g);
        lua_pushinteger(L, b);
        lua_pushinteger(L, a);
        return 4;
    }
    
    return 0;
}

static int lua_palettebank_load_preset(lua_State* L) {
    PaletteBankHandle bank = (PaletteBankHandle)lua_touserdata(L, 1);
    int32_t paletteIndex = luaL_checkinteger(L, 2);
    const char* presetName = luaL_checkstring(L, 3);
    
    bool success = palettebank_load_preset(bank, paletteIndex, presetName);
    lua_pushboolean(L, success);
    return 1;
}

static int lua_palettebank_copy_palette(lua_State* L) {
    PaletteBankHandle bank = (PaletteBankHandle)lua_touserdata(L, 1);
    int32_t srcIndex = luaL_checkinteger(L, 2);
    int32_t dstIndex = luaL_checkinteger(L, 3);
    
    bool success = palettebank_copy_palette(bank, srcIndex, dstIndex);
    lua_pushboolean(L, success);
    return 1;
}

static int lua_palettebank_fill_palette(lua_State* L) {
    PaletteBankHandle bank = (PaletteBankHandle)lua_touserdata(L, 1);
    int32_t paletteIndex = luaL_checkinteger(L, 2);
    uint8_t r = (uint8_t)luaL_checkinteger(L, 3);
    uint8_t g = (uint8_t)luaL_checkinteger(L, 4);
    uint8_t b = (uint8_t)luaL_checkinteger(L, 5);
    uint8_t a = (uint8_t)luaL_optinteger(L, 6, 255);
    
    palettebank_fill_palette(bank, paletteIndex, r, g, b, a);
    return 0;
}

static int lua_palettebank_clear_palette(lua_State* L) {
    PaletteBankHandle bank = (PaletteBankHandle)lua_touserdata(L, 1);
    int32_t paletteIndex = luaL_checkinteger(L, 2);
    
    palettebank_clear_palette(bank, paletteIndex);
    return 0;
}

static int lua_palettebank_enforce_convention(lua_State* L) {
    PaletteBankHandle bank = (PaletteBankHandle)lua_touserdata(L, 1);
    int32_t paletteIndex = luaL_optinteger(L, 2, -1);
    
    palettebank_enforce_convention(bank, paletteIndex);
    return 0;
}

static int lua_palettebank_upload(lua_State* L) {
    PaletteBankHandle bank = (PaletteBankHandle)lua_touserdata(L, 1);
    int32_t paletteIndex = luaL_optinteger(L, 2, -1);
    
    palettebank_upload(bank, paletteIndex);
    return 0;
}

static int lua_palettebank_lerp(lua_State* L) {
    PaletteBankHandle bank = (PaletteBankHandle)lua_touserdata(L, 1);
    int32_t paletteA = luaL_checkinteger(L, 2);
    int32_t paletteB = luaL_checkinteger(L, 3);
    float t = (float)luaL_checknumber(L, 4);
    int32_t outPalette = luaL_checkinteger(L, 5);
    
    palettebank_lerp(bank, paletteA, paletteB, t, outPalette);
    return 0;
}

static int lua_palettebank_rotate(lua_State* L) {
    PaletteBankHandle bank = (PaletteBankHandle)lua_touserdata(L, 1);
    int32_t paletteIndex = luaL_checkinteger(L, 2);
    int32_t startIndex = luaL_checkinteger(L, 3);
    int32_t endIndex = luaL_checkinteger(L, 4);
    int32_t amount = luaL_checkinteger(L, 5);
    
    palettebank_rotate(bank, paletteIndex, startIndex, endIndex, amount);
    return 0;
}

static int lua_palettebank_adjust_brightness(lua_State* L) {
    PaletteBankHandle bank = (PaletteBankHandle)lua_touserdata(L, 1);
    int32_t paletteIndex = luaL_checkinteger(L, 2);
    float brightness = (float)luaL_checknumber(L, 3);
    
    palettebank_adjust_brightness(bank, paletteIndex, brightness);
    return 0;
}

static int lua_palettebank_adjust_saturation(lua_State* L) {
    PaletteBankHandle bank = (PaletteBankHandle)lua_touserdata(L, 1);
    int32_t paletteIndex = luaL_checkinteger(L, 2);
    float saturation = (float)luaL_checknumber(L, 3);
    
    palettebank_adjust_saturation(bank, paletteIndex, saturation);
    return 0;
}

static int lua_palettebank_get_info(lua_State* L) {
    PaletteBankHandle bank = (PaletteBankHandle)lua_touserdata(L, 1);
    
    int32_t paletteCount = palettebank_get_palette_count(bank);
    int32_t colorsPerPalette = palettebank_get_colors_per_palette(bank);
    
    lua_pushinteger(L, paletteCount);
    lua_pushinteger(L, colorsPerPalette);
    return 2;
}

// =============================================================================
// TilesetIndexed Lua Bindings
// =============================================================================

static int lua_tilesetindexed_create(lua_State* L) {
    int32_t tileWidth = luaL_checkinteger(L, 1);
    int32_t tileHeight = luaL_checkinteger(L, 2);
    int32_t tileCount = luaL_checkinteger(L, 3);
    
    TilesetIndexedHandle tileset = tilesetindexed_create(nullptr, tileWidth, tileHeight, tileCount);
    if (!tileset) {
        return luaL_error(L, "Failed to create indexed tileset");
    }
    
    lua_pushlightuserdata(L, tileset);
    return 1;
}

static int lua_tilesetindexed_destroy(lua_State* L) {
    TilesetIndexedHandle tileset = (TilesetIndexedHandle)lua_touserdata(L, 1);
    if (tileset) {
        tilesetindexed_destroy(tileset);
    }
    return 0;
}

static int lua_tilesetindexed_set_pixel(lua_State* L) {
    TilesetIndexedHandle tileset = (TilesetIndexedHandle)lua_touserdata(L, 1);
    int32_t tileId = luaL_checkinteger(L, 2);
    int32_t x = luaL_checkinteger(L, 3);
    int32_t y = luaL_checkinteger(L, 4);
    uint8_t colorIndex = (uint8_t)luaL_checkinteger(L, 5);
    
    bool success = tilesetindexed_set_pixel(tileset, tileId, x, y, colorIndex);
    lua_pushboolean(L, success);
    return 1;
}

static int lua_tilesetindexed_get_pixel(lua_State* L) {
    TilesetIndexedHandle tileset = (TilesetIndexedHandle)lua_touserdata(L, 1);
    int32_t tileId = luaL_checkinteger(L, 2);
    int32_t x = luaL_checkinteger(L, 3);
    int32_t y = luaL_checkinteger(L, 4);
    
    uint8_t colorIndex = tilesetindexed_get_pixel(tileset, tileId, x, y);
    lua_pushinteger(L, colorIndex);
    return 1;
}

static int lua_tilesetindexed_fill_tile(lua_State* L) {
    TilesetIndexedHandle tileset = (TilesetIndexedHandle)lua_touserdata(L, 1);
    int32_t tileId = luaL_checkinteger(L, 2);
    uint8_t colorIndex = (uint8_t)luaL_checkinteger(L, 3);
    
    tilesetindexed_fill_tile(tileset, tileId, colorIndex);
    return 0;
}

static int lua_tilesetindexed_clear_tile(lua_State* L) {
    TilesetIndexedHandle tileset = (TilesetIndexedHandle)lua_touserdata(L, 1);
    int32_t tileId = luaL_checkinteger(L, 2);
    
    tilesetindexed_clear_tile(tileset, tileId);
    return 0;
}

static int lua_tilesetindexed_copy_tile(lua_State* L) {
    TilesetIndexedHandle tileset = (TilesetIndexedHandle)lua_touserdata(L, 1);
    int32_t srcTileId = luaL_checkinteger(L, 2);
    int32_t dstTileId = luaL_checkinteger(L, 3);
    
    bool success = tilesetindexed_copy_tile(tileset, srcTileId, dstTileId);
    lua_pushboolean(L, success);
    return 1;
}

static int lua_tilesetindexed_upload(lua_State* L) {
    TilesetIndexedHandle tileset = (TilesetIndexedHandle)lua_touserdata(L, 1);
    
    tilesetindexed_upload(tileset);
    return 0;
}

static int lua_tilesetindexed_get_info(lua_State* L) {
    TilesetIndexedHandle tileset = (TilesetIndexedHandle)lua_touserdata(L, 1);
    
    int32_t tileWidth = tilesetindexed_get_tile_width(tileset);
    int32_t tileHeight = tilesetindexed_get_tile_height(tileset);
    int32_t tileCount = tilesetindexed_get_tile_count(tileset);
    
    lua_pushinteger(L, tileWidth);
    lua_pushinteger(L, tileHeight);
    lua_pushinteger(L, tileCount);
    return 3;
}

// =============================================================================
// TilemapEx Lua Bindings
// =============================================================================

static int lua_tilemapex_create(lua_State* L) {
    int32_t width = luaL_checkinteger(L, 1);
    int32_t height = luaL_checkinteger(L, 2);
    int32_t tileWidth = luaL_checkinteger(L, 3);
    int32_t tileHeight = luaL_checkinteger(L, 4);
    
    TilemapExHandle tilemap = tilemapex_create(width, height, tileWidth, tileHeight);
    if (!tilemap) {
        return luaL_error(L, "Failed to create extended tilemap");
    }
    
    lua_pushlightuserdata(L, tilemap);
    return 1;
}

static int lua_tilemapex_destroy(lua_State* L) {
    TilemapExHandle tilemap = (TilemapExHandle)lua_touserdata(L, 1);
    if (tilemap) {
        tilemapex_destroy(tilemap);
    }
    return 0;
}

static int lua_tilemapex_set_tile(lua_State* L) {
    TilemapExHandle tilemap = (TilemapExHandle)lua_touserdata(L, 1);
    int32_t x = luaL_checkinteger(L, 2);
    int32_t y = luaL_checkinteger(L, 3);
    uint16_t tileId = (uint16_t)luaL_checkinteger(L, 4);
    uint8_t paletteIndex = (uint8_t)luaL_optinteger(L, 5, 0);
    uint8_t zOrder = (uint8_t)luaL_optinteger(L, 6, 3); // TILEEX_ZORDER_NORMAL
    bool flipX = lua_toboolean(L, 7);
    bool flipY = lua_toboolean(L, 8);
    uint8_t rotation = (uint8_t)luaL_optinteger(L, 9, 0);
    
    tilemapex_set_tile_indexed(tilemap, x, y, tileId, paletteIndex, zOrder, flipX, flipY, rotation);
    return 0;
}

static int lua_tilemapex_get_tile(lua_State* L) {
    TilemapExHandle tilemap = (TilemapExHandle)lua_touserdata(L, 1);
    int32_t x = luaL_checkinteger(L, 2);
    int32_t y = luaL_checkinteger(L, 3);
    
    uint16_t tileId;
    uint8_t paletteIndex, zOrder, rotation;
    bool flipX, flipY;
    
    if (tilemapex_get_tile_indexed(tilemap, x, y, &tileId, &paletteIndex, &zOrder, &flipX, &flipY, &rotation)) {
        lua_pushinteger(L, tileId);
        lua_pushinteger(L, paletteIndex);
        lua_pushinteger(L, zOrder);
        lua_pushboolean(L, flipX);
        lua_pushboolean(L, flipY);
        lua_pushinteger(L, rotation);
        return 6;
    }
    
    return 0;
}

static int lua_tilemapex_set_palette(lua_State* L) {
    TilemapExHandle tilemap = (TilemapExHandle)lua_touserdata(L, 1);
    int32_t x = luaL_checkinteger(L, 2);
    int32_t y = luaL_checkinteger(L, 3);
    uint8_t paletteIndex = (uint8_t)luaL_checkinteger(L, 4);
    
    tilemapex_set_palette(tilemap, x, y, paletteIndex);
    return 0;
}

static int lua_tilemapex_get_palette(lua_State* L) {
    TilemapExHandle tilemap = (TilemapExHandle)lua_touserdata(L, 1);
    int32_t x = luaL_checkinteger(L, 2);
    int32_t y = luaL_checkinteger(L, 3);
    
    uint8_t paletteIndex = tilemapex_get_palette(tilemap, x, y);
    lua_pushinteger(L, paletteIndex);
    return 1;
}

static int lua_tilemapex_fill(lua_State* L) {
    TilemapExHandle tilemap = (TilemapExHandle)lua_touserdata(L, 1);
    uint16_t tileId = (uint16_t)luaL_checkinteger(L, 2);
    uint8_t paletteIndex = (uint8_t)luaL_optinteger(L, 3, 0);
    
    tilemapex_fill_indexed(tilemap, tileId, paletteIndex);
    return 0;
}

static int lua_tilemapex_fill_rect(lua_State* L) {
    TilemapExHandle tilemap = (TilemapExHandle)lua_touserdata(L, 1);
    int32_t x = luaL_checkinteger(L, 2);
    int32_t y = luaL_checkinteger(L, 3);
    int32_t width = luaL_checkinteger(L, 4);
    int32_t height = luaL_checkinteger(L, 5);
    uint16_t tileId = (uint16_t)luaL_checkinteger(L, 6);
    uint8_t paletteIndex = (uint8_t)luaL_optinteger(L, 7, 0);
    
    tilemapex_fill_rect_indexed(tilemap, x, y, width, height, tileId, paletteIndex);
    return 0;
}

static int lua_tilemapex_clear(lua_State* L) {
    TilemapExHandle tilemap = (TilemapExHandle)lua_touserdata(L, 1);
    
    tilemapex_clear(tilemap);
    return 0;
}

static int lua_tilemapex_get_info(lua_State* L) {
    TilemapExHandle tilemap = (TilemapExHandle)lua_touserdata(L, 1);
    
    int32_t width = tilemapex_get_width(tilemap);
    int32_t height = tilemapex_get_height(tilemap);
    int32_t tileWidth = tilemapex_get_tile_width(tilemap);
    int32_t tileHeight = tilemapex_get_tile_height(tilemap);
    
    lua_pushinteger(L, width);
    lua_pushinteger(L, height);
    lua_pushinteger(L, tileWidth);
    lua_pushinteger(L, tileHeight);
    return 4;
}

// =============================================================================
// Registration
// =============================================================================

void registerBindings(lua_State* L) {
    // PaletteBank functions
    lua_register(L, "palettebank_create", lua_palettebank_create);
    lua_register(L, "palettebank_destroy", lua_palettebank_destroy);
    lua_register(L, "palettebank_set_color", lua_palettebank_set_color);
    lua_register(L, "palettebank_get_color", lua_palettebank_get_color);
    lua_register(L, "palettebank_load_preset", lua_palettebank_load_preset);
    lua_register(L, "palettebank_copy_palette", lua_palettebank_copy_palette);
    lua_register(L, "palettebank_fill_palette", lua_palettebank_fill_palette);
    lua_register(L, "palettebank_clear_palette", lua_palettebank_clear_palette);
    lua_register(L, "palettebank_enforce_convention", lua_palettebank_enforce_convention);
    lua_register(L, "palettebank_upload", lua_palettebank_upload);
    lua_register(L, "palettebank_lerp", lua_palettebank_lerp);
    lua_register(L, "palettebank_rotate", lua_palettebank_rotate);
    lua_register(L, "palettebank_adjust_brightness", lua_palettebank_adjust_brightness);
    lua_register(L, "palettebank_adjust_saturation", lua_palettebank_adjust_saturation);
    lua_register(L, "palettebank_get_info", lua_palettebank_get_info);
    
    // TilesetIndexed functions
    lua_register(L, "tilesetindexed_create", lua_tilesetindexed_create);
    lua_register(L, "tilesetindexed_destroy", lua_tilesetindexed_destroy);
    lua_register(L, "tilesetindexed_set_pixel", lua_tilesetindexed_set_pixel);
    lua_register(L, "tilesetindexed_get_pixel", lua_tilesetindexed_get_pixel);
    lua_register(L, "tilesetindexed_fill_tile", lua_tilesetindexed_fill_tile);
    lua_register(L, "tilesetindexed_clear_tile", lua_tilesetindexed_clear_tile);
    lua_register(L, "tilesetindexed_copy_tile", lua_tilesetindexed_copy_tile);
    lua_register(L, "tilesetindexed_upload", lua_tilesetindexed_upload);
    lua_register(L, "tilesetindexed_get_info", lua_tilesetindexed_get_info);
    
    // TilemapEx functions
    lua_register(L, "tilemapex_create", lua_tilemapex_create);
    lua_register(L, "tilemapex_destroy", lua_tilemapex_destroy);
    lua_register(L, "tilemapex_set_tile", lua_tilemapex_set_tile);
    lua_register(L, "tilemapex_get_tile", lua_tilemapex_get_tile);
    lua_register(L, "tilemapex_set_palette", lua_tilemapex_set_palette);
    lua_register(L, "tilemapex_get_palette", lua_tilemapex_get_palette);
    lua_register(L, "tilemapex_fill", lua_tilemapex_fill);
    lua_register(L, "tilemapex_fill_rect", lua_tilemapex_fill_rect);
    lua_register(L, "tilemapex_clear", lua_tilemapex_clear);
    lua_register(L, "tilemapex_get_info", lua_tilemapex_get_info);
}

} // namespace IndexedTileBindings
} // namespace SuperTerminal