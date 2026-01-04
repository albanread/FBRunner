//
// FBTBindings.cpp
// FBRunner3 - Lua bindings for SuperTerminal API
//
// This file provides Lua bindings for all SuperTerminal C API functions.
// Functions are registered in the global Lua namespace for use with
// FasterBASICT-generated Lua code.
//

#include "FBTBindings.h"
#include "../Framework/Debug/Logger.h"
#include "../FasterBASICT/runtime/data_lua_bindings.h"
#include "../FasterBASICT/runtime/fileio_lua_bindings.h"


#include "../Framework/API/superterminal_api.h"
#include "../Framework/API/st_api_context.h"
#include "../Framework/API/st_api_rectangles.h"
#include "../Framework/API/st_api_lines.h"
#include "../Framework/API/st_api_circles.h"
#include "../Framework/API/st_api_video_mode.h"
#include "../Framework/Particles/ParticleSystem.h"
#include "../Framework/Input/SimpleLineEditor.h"
#include <lua.hpp>
#include <string>
#include <cstring>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <thread>

// Video mode constants
constexpr int VIDEO_MODE_TEXT = 0;
constexpr int VIDEO_MODE_LORES = 1;
constexpr int VIDEO_MODE_MIDRES = 2;
constexpr int VIDEO_MODE_HIRES = 3;
constexpr int VIDEO_MODE_URES = 4;
constexpr int VIDEO_MODE_XRES = 5;
constexpr int VIDEO_MODE_WRES = 6;
constexpr int VIDEO_MODE_PRES = 7;
#include <iostream>

// =============================================================================
// GPU Batch State (for VGPUBEGIN/VGPUEND)
// =============================================================================
static bool g_gpuBatchActive = false;
static int g_gpuBatchBuffer = 0;

// Forward declarations
class FBRunner3App;
extern void* g_runnerInstance;

// C wrapper function declarations
extern "C" {
    void fbrunner3_runtime_print_text(const char* text);
    void fbrunner3_runtime_set_cursor(int x, int y);
    void fbrunner3_runtime_print_newline();
    bool fbrunner3_should_stop_script();
}

using SuperTerminal::ParticleMode;

namespace SuperTerminal {
namespace FBTBindings {

// =============================================================================
// DATA/READ/RESTORE Management
// =============================================================================

void initializeDataManager(const std::vector<std::string>& values) {
    FasterBASIC::initializeDataManager(values);
}

void addDataRestorePoint(int lineNumber, size_t index) {
    FasterBASIC::addDataRestorePoint(lineNumber, index);
}

void addDataRestorePointByLabel(const std::string& labelName, size_t index) {
    FasterBASIC::addDataRestorePointByLabel(labelName, index);
}

void clearDataManager() {
    FasterBASIC::clearDataManager();
}

// =============================================================================
// File I/O Management
// =============================================================================

void initializeFileManager() {
    // FileManager is ready to use on construction
}

void clearFileManager() {
    FasterBASIC::clear_fileio_state();
}

// =============================================================================
// Helper Functions
// =============================================================================

static void luaL_setglobalfunction(lua_State* L, const char* name, lua_CFunction func) {
    lua_pushcfunction(L, func);
    lua_setglobal(L, name);
}

static void luaL_setglobalnumber(lua_State* L, const char* name, lua_Number value) {
    lua_pushnumber(L, value);
    lua_setglobal(L, name);
}

// =============================================================================
// DATA/READ/RESTORE API Bindings
// =============================================================================

// DATA/READ/RESTORE functions are now provided by FasterBASICT runtime

// =============================================================================
// File I/O API functions are now provided by FasterBASICT runtime
// =============================================================================

// =============================================================================
// Text API Bindings
// =============================================================================

static int lua_st_text_putchar(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    const char* str = luaL_checkstring(L, 3);
    uint32_t fg = luaL_optinteger(L, 4, 0xFFFFFFFF);
    uint32_t bg = luaL_optinteger(L, 5, 0x000000FF);

    if (str && str[0]) {
        st_text_putchar(x, y, str[0], fg, bg);
    }
    
    // Add small delay to prevent UI flooding when outputting characters in tight loops
    // This allows the UI thread to process events (like Stop button clicks)
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    
    // Check if script should be stopped during character output operations
    if (fbrunner3_should_stop_script()) {
        luaL_error(L, "Script interrupted during TEXT_PUTCHAR operation");
    }
    
    return 0;
}

// poke_text: directly write a 32-bit character code to text grid
static int lua_poke_text(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    uint32_t character = luaL_checkinteger(L, 3);
    uint32_t fg = luaL_optinteger(L, 4, 0xFFFFFFFF);
    uint32_t bg = luaL_optinteger(L, 5, 0x000000FF);

    st_text_putchar(x, y, character, fg, bg);
    return 0;
}

static int lua_st_text_put(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    const char* text = luaL_checkstring(L, 3);
    uint32_t fg = luaL_optinteger(L, 4, 0xFFFFFFFF);
    uint32_t bg = luaL_optinteger(L, 5, 0x000000FF);

    st_text_put(x, y, text, fg, bg);
    
    // Add small delay to prevent UI flooding when outputting in tight loops
    // This allows the UI thread to process events (like Stop button clicks)
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    
    // Check if script should be stopped during text output operations
    if (fbrunner3_should_stop_script()) {
        luaL_error(L, "Script interrupted during TEXT_PUT operation");
    }
    
    return 0;
}

static int lua_st_text_clear(lua_State* L) {
    (void)L;
    st_text_clear();
    return 0;
}

static int lua_st_text_clear_region(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int width = luaL_checkinteger(L, 3);
    int height = luaL_checkinteger(L, 4);

    st_text_clear_region(x, y, width, height);
    return 0;
}

static int lua_st_text_set_size(lua_State* L) {
    int width = luaL_checkinteger(L, 1);
    int height = luaL_checkinteger(L, 2);

    st_text_set_size(width, height);
    return 0;
}

static int lua_st_text_get_size(lua_State* L) {
    int width, height;
    st_text_get_size(&width, &height);

    lua_pushinteger(L, width);
    lua_pushinteger(L, height);
    return 2;
}

static int lua_st_text_scroll(lua_State* L) {
    int lines = luaL_checkinteger(L, 1);
    st_text_scroll(lines);
    return 0;
}

// =============================================================================
// Sixel Graphics API Bindings
// =============================================================================

static int lua_st_text_putsixel(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    uint32_t sixel_char = luaL_checkinteger(L, 3);
    uint32_t bg = luaL_optinteger(L, 5, 0x000000FF);

    // Get the colors table (6 elements)
    luaL_checktype(L, 4, LUA_TTABLE);
    uint8_t colors[6];
    for (int i = 0; i < 6; i++) {
        lua_rawgeti(L, 4, i + 1); // Lua arrays are 1-indexed
        colors[i] = (uint8_t)luaL_checkinteger(L, -1);
        lua_pop(L, 1);
    }

    st_text_putsixel(x, y, sixel_char, colors, bg);
    return 0;
}

static int lua_st_sixel_pack_colors(lua_State* L) {
    // Get the colors table (6 elements)
    luaL_checktype(L, 1, LUA_TTABLE);
    uint8_t colors[6];
    for (int i = 0; i < 6; i++) {
        lua_rawgeti(L, 1, i + 1); // Lua arrays are 1-indexed
        colors[i] = (uint8_t)luaL_checkinteger(L, -1);
        lua_pop(L, 1);
    }

    uint32_t packed = st_sixel_pack_colors(colors);
    lua_pushinteger(L, packed);
    return 1;
}

static int lua_st_text_putsixel_packed(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    uint32_t sixel_char = luaL_checkinteger(L, 3);
    uint32_t packed_colors = luaL_checkinteger(L, 4);
    uint32_t bg = luaL_optinteger(L, 5, 0x000000FF);

    st_text_putsixel_packed(x, y, sixel_char, packed_colors, bg);
    return 0;
}

static int lua_st_sixel_set_stripe(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int stripe_index = luaL_checkinteger(L, 3);
    uint8_t color_index = (uint8_t)luaL_checkinteger(L, 4);

    st_sixel_set_stripe(x, y, stripe_index, color_index);
    return 0;
}

static int lua_st_sixel_get_stripe(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int stripe_index = luaL_checkinteger(L, 3);

    uint8_t color = st_sixel_get_stripe(x, y, stripe_index);
    lua_pushinteger(L, color);
    return 1;
}

static int lua_st_sixel_gradient(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    uint8_t top_color = (uint8_t)luaL_checkinteger(L, 3);
    uint8_t bottom_color = (uint8_t)luaL_checkinteger(L, 4);
    uint32_t bg = luaL_optinteger(L, 5, 0x000000FF);

    st_sixel_gradient(x, y, top_color, bottom_color, bg);
    return 0;
}

static int lua_st_sixel_hline(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int width = luaL_checkinteger(L, 3);
    uint32_t bg = luaL_optinteger(L, 5, 0x000000FF);

    // Get the colors table (6 elements)
    luaL_checktype(L, 4, LUA_TTABLE);
    uint8_t colors[6];
    for (int i = 0; i < 6; i++) {
        lua_rawgeti(L, 4, i + 1);
        colors[i] = (uint8_t)luaL_checkinteger(L, -1);
        lua_pop(L, 1);
    }

    st_sixel_hline(x, y, width, colors, bg);
    return 0;
}

static int lua_st_sixel_fill_rect(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int width = luaL_checkinteger(L, 3);
    int height = luaL_checkinteger(L, 4);
    uint32_t bg = luaL_optinteger(L, 6, 0x000000FF);

    // Get the colors table (6 elements)
    luaL_checktype(L, 5, LUA_TTABLE);
    uint8_t colors[6];
    for (int i = 0; i < 6; i++) {
        lua_rawgeti(L, 5, i + 1);
        colors[i] = (uint8_t)luaL_checkinteger(L, -1);
        lua_pop(L, 1);
    }

    st_sixel_fill_rect(x, y, width, height, colors, bg);
    return 0;
}

// =============================================================================
// Graphics API Bindings
// =============================================================================

static int lua_st_gfx_clear(lua_State* L) {
    (void)L;
    st_gfx_clear();
    return 0;
}

static int lua_st_clear_all_layers(lua_State* L) {
    (void)L;
    st_clear_all_layers();
    return 0;
}

static int lua_st_gfx_rect(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int width = luaL_checkinteger(L, 3);
    int height = luaL_checkinteger(L, 4);
    uint32_t color = luaL_checkinteger(L, 5);

    printf("[lua_st_gfx_rect] Called with x=%d y=%d w=%d h=%d color=0x%08X\n", x, y, width, height, color);
    fflush(stdout);
    st_gfx_rect(x, y, width, height, color);
    printf("[lua_st_gfx_rect] st_gfx_rect returned\n");
    fflush(stdout);
    return 0;
}

static int lua_st_gfx_rect_outline(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int width = luaL_checkinteger(L, 3);
    int height = luaL_checkinteger(L, 4);
    uint32_t color = luaL_checkinteger(L, 5);
    int thickness = luaL_optinteger(L, 6, 1);

    st_gfx_rect_outline(x, y, width, height, color, thickness);
    return 0;
}

static int lua_st_gfx_circle(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int radius = luaL_checkinteger(L, 3);
    uint32_t color = luaL_checkinteger(L, 4);

    st_gfx_circle(x, y, radius, color);
    return 0;
}

static int lua_st_gfx_circle_outline(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int radius = luaL_checkinteger(L, 3);
    uint32_t color = luaL_checkinteger(L, 4);
    int thickness = luaL_optinteger(L, 5, 1);

    st_gfx_circle_outline(x, y, radius, color, thickness);
    return 0;
}

static int lua_st_gfx_line(lua_State* L) {
    int x1 = luaL_checkinteger(L, 1);
    int y1 = luaL_checkinteger(L, 2);
    int x2 = luaL_checkinteger(L, 3);
    int y2 = luaL_checkinteger(L, 4);
    uint32_t color = luaL_checkinteger(L, 5);
    int thickness = luaL_optinteger(L, 6, 1);

    st_gfx_line(x1, y1, x2, y2, color, thickness);
    return 0;
}

static int lua_st_gfx_point(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    uint32_t color = luaL_checkinteger(L, 3);

    st_gfx_point(x, y, color);
    return 0;
}

static int lua_st_gfx_swap(lua_State* L) {
    (void)L;
    st_gfx_swap();
    return 0;
}

static int lua_st_gfx_arc(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int radius = luaL_checkinteger(L, 3);
    float start_angle = luaL_checknumber(L, 4);
    float end_angle = luaL_checknumber(L, 5);
    uint32_t color = luaL_checkinteger(L, 6);

    st_gfx_arc(x, y, radius, start_angle, end_angle, color);
    return 0;
}

static int lua_st_gfx_arc_filled(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int radius = luaL_checkinteger(L, 3);
    float start_angle = luaL_checknumber(L, 4);
    float end_angle = luaL_checknumber(L, 5);
    uint32_t color = luaL_checkinteger(L, 6);

    st_gfx_arc_filled(x, y, radius, start_angle, end_angle, color);
    return 0;
}

// =============================================================================
// Audio API Bindings
// =============================================================================

static int lua_st_music_play(lua_State* L) {
    const char* abc = luaL_checkstring(L, 1);
    printf("[FBTBindings] music_play called with ABC notation: %s\n", abc);
    st_music_play(abc);
    printf("[FBTBindings] st_music_play returned\n");
    return 0;
}

static int lua_st_play_abc(lua_State* L) {
    const char* abc_text = luaL_checkstring(L, 1);
    st_play_abc(abc_text);
    return 0;
}

static int lua_st_music_play_file(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    st_music_play_file(path);
    return 0;
}

static int lua_st_music_play_file_with_format(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    const char* format = luaL_checkstring(L, 2);
    st_music_play_file_with_format(path, format);
    return 0;
}

static int lua_st_music_render_to_wav(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    const char* outputPath = luaL_checkstring(L, 2);
    const char* format = luaL_optstring(L, 3, nullptr);
    // In Lua, 0 is truthy! Check numeric value instead
    bool fastRender = (lua_isnumber(L, 4) && lua_tonumber(L, 4) != 0.0);
    bool success = st_music_render_to_wav(path, outputPath, format, fastRender);
    lua_pushboolean(L, success);
    return 1;
}

static int lua_st_music_render_to_slot(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    uint32_t slotNumber = luaL_checkinteger(L, 2);
    const char* format = luaL_optstring(L, 3, nullptr);
    // In Lua, 0 is truthy! Check numeric value instead
    bool fastRender = (lua_isnumber(L, 4) && lua_tonumber(L, 4) != 0.0);
    uint32_t resultSlot = st_music_render_to_slot(path, slotNumber, format, fastRender);
    lua_pushinteger(L, resultSlot);
    return 1;
}

static int lua_st_music_stop(lua_State* L) {
    (void)L;
    st_music_stop();
    return 0;
}

static int lua_st_music_pause(lua_State* L) {
    (void)L;
    st_music_pause();
    return 0;
}

static int lua_st_music_resume(lua_State* L) {
    (void)L;
    st_music_resume();
    return 0;
}

static int lua_st_music_is_playing(lua_State* L) {
    lua_pushboolean(L, st_music_is_playing());
    return 1;
}

static int lua_st_music_set_volume(lua_State* L) {
    float volume = luaL_checknumber(L, 1);
    st_music_set_volume(volume);
    return 0;
}

// =============================================================================
// Music Bank Bindings
// =============================================================================

static int lua_st_music_load_string(lua_State* L) {
    const char* abc = luaL_checkstring(L, 1);
    uint32_t music_id = st_music_load_string(abc);
    lua_pushinteger(L, music_id);
    return 1;
}

static int lua_st_music_load_file(lua_State* L) {
    const char* filename = luaL_checkstring(L, 1);
    uint32_t music_id = st_music_load_file(filename);
    lua_pushinteger(L, music_id);
    return 1;
}

static int lua_st_music_play_id(lua_State* L) {
    uint32_t music_id = luaL_checkinteger(L, 1);
    float volume = luaL_optnumber(L, 2, 1.0);
    st_music_play_id(music_id, volume);
    return 0;
}

static int lua_st_music_exists(lua_State* L) {
    uint32_t music_id = luaL_checkinteger(L, 1);
    lua_pushboolean(L, st_music_exists(music_id));
    return 1;
}

static int lua_st_music_get_title(lua_State* L) {
    uint32_t music_id = luaL_checkinteger(L, 1);
    const char* title = st_music_get_title(music_id);
    lua_pushstring(L, title);
    return 1;
}

static int lua_st_music_get_composer(lua_State* L) {
    uint32_t music_id = luaL_checkinteger(L, 1);
    const char* composer = st_music_get_composer(music_id);
    lua_pushstring(L, composer);
    return 1;
}

static int lua_st_music_get_key(lua_State* L) {
    uint32_t music_id = luaL_checkinteger(L, 1);
    const char* key = st_music_get_key(music_id);
    lua_pushstring(L, key);
    return 1;
}

static int lua_st_music_get_tempo(lua_State* L) {
    uint32_t music_id = luaL_checkinteger(L, 1);
    float tempo = st_music_get_tempo(music_id);
    lua_pushnumber(L, tempo);
    return 1;
}

static int lua_st_music_free(lua_State* L) {
    uint32_t music_id = luaL_checkinteger(L, 1);
    bool success = st_music_free(music_id);
    lua_pushboolean(L, success);
    return 1;
}

static int lua_st_music_free_all(lua_State* L) {
    (void)L;
    st_music_free_all();
    return 0;
}

static int lua_st_music_get_count(lua_State* L) {
    uint32_t count = st_music_get_count();
    lua_pushinteger(L, count);
    return 1;
}

static int lua_st_music_get_memory(lua_State* L) {
    uint32_t memory = st_music_get_memory();
    lua_pushinteger(L, memory);
    return 1;
}

// =============================================================================
// SID Player API Bindings
// =============================================================================

static int lua_st_sid_load_file(lua_State* L) {
    const char* filename = luaL_checkstring(L, 1);
    uint32_t sid_id = st_sid_load_file(filename);
    lua_pushinteger(L, sid_id);
    return 1;
}

static int lua_st_sid_load_memory(lua_State* L) {
    size_t size;
    const char* data = luaL_checklstring(L, 1, &size);
    uint32_t sid_id = st_sid_load_memory(reinterpret_cast<const uint8_t*>(data), size);
    lua_pushinteger(L, sid_id);
    return 1;
}

static int lua_st_sid_play(lua_State* L) {
    uint32_t sid_id = luaL_checkinteger(L, 1);
    int subtune = luaL_optinteger(L, 2, 0);
    float volume = luaL_optnumber(L, 3, 1.0);
    st_sid_play(sid_id, subtune, volume);
    return 0;
}

static int lua_st_sid_stop(lua_State* L) {
    (void)L;
    st_sid_stop();
    return 0;
}

static int lua_st_sid_pause(lua_State* L) {
    (void)L;
    st_sid_pause();
    return 0;
}

static int lua_st_sid_resume(lua_State* L) {
    (void)L;
    st_sid_resume();
    return 0;
}

static int lua_st_sid_is_playing(lua_State* L) {
    lua_pushboolean(L, st_sid_is_playing());
    return 1;
}

static int lua_st_sid_set_volume(lua_State* L) {
    float volume = luaL_checknumber(L, 1);
    st_sid_set_volume(volume);
    return 0;
}

static int lua_st_sid_get_title(lua_State* L) {
    uint32_t sid_id = luaL_checkinteger(L, 1);
    const char* title = st_sid_get_title(sid_id);
    lua_pushstring(L, title);
    return 1;
}

static int lua_st_sid_get_author(lua_State* L) {
    uint32_t sid_id = luaL_checkinteger(L, 1);
    const char* author = st_sid_get_author(sid_id);
    lua_pushstring(L, author);
    return 1;
}

static int lua_st_sid_get_copyright(lua_State* L) {
    uint32_t sid_id = luaL_checkinteger(L, 1);
    const char* copyright = st_sid_get_copyright(sid_id);
    lua_pushstring(L, copyright);
    return 1;
}

static int lua_st_sid_get_subtune_count(lua_State* L) {
    uint32_t sid_id = luaL_checkinteger(L, 1);
    int count = st_sid_get_subtune_count(sid_id);
    lua_pushinteger(L, count);
    return 1;
}

static int lua_st_sid_get_default_subtune(lua_State* L) {
    uint32_t sid_id = luaL_checkinteger(L, 1);
    int subtune = st_sid_get_default_subtune(sid_id);
    lua_pushinteger(L, subtune);
    return 1;
}

static int lua_st_sid_set_quality(lua_State* L) {
    int quality = luaL_checkinteger(L, 1);
    st_sid_set_quality(quality);
    return 0;
}

static int lua_st_sid_set_chip_model(lua_State* L) {
    int model = luaL_checkinteger(L, 1);
    st_sid_set_chip_model(model);
    return 0;
}

static int lua_st_sid_set_speed(lua_State* L) {
    float speed = luaL_checknumber(L, 1);
    st_sid_set_speed(speed);
    return 0;
}

static int lua_st_sid_get_time(lua_State* L) {
    float time = st_sid_get_time();
    lua_pushnumber(L, time);
    return 1;
}

static int lua_st_sid_set_max_sids(lua_State* L) {
    int maxSids = luaL_checkinteger(L, 1);
    st_sid_set_max_sids(maxSids);
    return 0;
}

static int lua_st_sid_get_max_sids(lua_State* L) {
    int maxSids = st_sid_get_max_sids();
    lua_pushinteger(L, maxSids);
    return 1;
}

static int lua_st_sid_free(lua_State* L) {
    uint32_t sid_id = luaL_checkinteger(L, 1);
    bool success = st_sid_free(sid_id);
    lua_pushboolean(L, success);
    return 1;
}

static int lua_st_sid_free_all(lua_State* L) {
    (void)L;
    st_sid_free_all();
    return 0;
}

static int lua_st_sid_exists(lua_State* L) {
    uint32_t sid_id = luaL_checkinteger(L, 1);
    lua_pushboolean(L, st_sid_exists(sid_id));
    return 1;
}

static int lua_st_sid_get_count(lua_State* L) {
    uint32_t count = st_sid_get_count();
    lua_pushinteger(L, count);
    return 1;
}

static int lua_st_sid_get_memory(lua_State* L) {
    uint32_t memory = st_sid_get_memory();
    lua_pushinteger(L, memory);
    return 1;
}

static int lua_st_synth_note(lua_State* L) {
    int note = luaL_checkinteger(L, 1);
    float duration = luaL_checknumber(L, 2);
    float volume = luaL_optnumber(L, 3, 0.5f);

    st_synth_note(note, duration, volume);
    return 0;
}

static int lua_st_synth_set_instrument(lua_State* L) {
    int instrument = luaL_checkinteger(L, 1);
    st_synth_set_instrument(instrument);
    return 0;
}

static int lua_st_synth_frequency(lua_State* L) {
    float frequency = luaL_checknumber(L, 1);
    float duration = luaL_checknumber(L, 2);
    float volume = luaL_optnumber(L, 3, 0.5f);

    st_synth_frequency(frequency, duration, volume);
    return 0;
}

// =============================================================================
// Sound Bank API Bindings
// =============================================================================

static int lua_st_sound_create_beep(lua_State* L) {
    float frequency = luaL_checknumber(L, 1);
    float duration = luaL_checknumber(L, 2);

    uint32_t soundId = st_sound_create_beep(frequency, duration);
    lua_pushinteger(L, soundId);
    return 1;
}

static int lua_st_sound_create_zap(lua_State* L) {
    float frequency = luaL_checknumber(L, 1);
    float duration = luaL_checknumber(L, 2);

    uint32_t soundId = st_sound_create_zap(frequency, duration);
    lua_pushinteger(L, soundId);
    return 1;
}

static int lua_st_sound_create_explode(lua_State* L) {
    float size = luaL_checknumber(L, 1);
    float duration = luaL_checknumber(L, 2);

    uint32_t soundId = st_sound_create_explode(size, duration);
    lua_pushinteger(L, soundId);
    return 1;
}

static int lua_st_sound_create_coin(lua_State* L) {
    float pitch = luaL_checknumber(L, 1);
    float duration = luaL_checknumber(L, 2);

    uint32_t soundId = st_sound_create_coin(pitch, duration);
    lua_pushinteger(L, soundId);
    return 1;
}

static int lua_st_sound_create_jump(lua_State* L) {
    float power = luaL_checknumber(L, 1);
    float duration = luaL_checknumber(L, 2);

    uint32_t soundId = st_sound_create_jump(power, duration);
    lua_pushinteger(L, soundId);
    return 1;
}

static int lua_st_sound_create_shoot(lua_State* L) {
    float power = luaL_checknumber(L, 1);
    float duration = luaL_checknumber(L, 2);

    uint32_t soundId = st_sound_create_shoot(power, duration);
    lua_pushinteger(L, soundId);
    return 1;
}

static int lua_st_sound_create_click(lua_State* L) {
    float sharpness = luaL_checknumber(L, 1);
    float duration = luaL_checknumber(L, 2);

    uint32_t soundId = st_sound_create_click(sharpness, duration);
    lua_pushinteger(L, soundId);
    return 1;
}

static int lua_st_sound_create_blip(lua_State* L) {
    float pitch = luaL_checknumber(L, 1);
    float duration = luaL_checknumber(L, 2);

    uint32_t soundId = st_sound_create_blip(pitch, duration);
    lua_pushinteger(L, soundId);
    return 1;
}

static int lua_st_sound_create_pickup(lua_State* L) {
    float brightness = luaL_checknumber(L, 1);
    float duration = luaL_checknumber(L, 2);

    uint32_t soundId = st_sound_create_pickup(brightness, duration);
    lua_pushinteger(L, soundId);
    return 1;
}

static int lua_st_sound_create_powerup(lua_State* L) {
    float intensity = luaL_checknumber(L, 1);
    float duration = luaL_checknumber(L, 2);

    uint32_t soundId = st_sound_create_powerup(intensity, duration);
    lua_pushinteger(L, soundId);
    return 1;
}

static int lua_st_sound_create_hurt(lua_State* L) {
    float severity = luaL_checknumber(L, 1);
    float duration = luaL_checknumber(L, 2);

    uint32_t soundId = st_sound_create_hurt(severity, duration);
    lua_pushinteger(L, soundId);
    return 1;
}

static int lua_st_sound_create_sweep_up(lua_State* L) {
    float start_freq = luaL_checknumber(L, 1);
    float end_freq = luaL_checknumber(L, 2);
    float duration = luaL_checknumber(L, 3);

    uint32_t soundId = st_sound_create_sweep_up(start_freq, end_freq, duration);
    lua_pushinteger(L, soundId);
    return 1;
}

static int lua_st_sound_create_sweep_down(lua_State* L) {
    float start_freq = luaL_checknumber(L, 1);
    float end_freq = luaL_checknumber(L, 2);
    float duration = luaL_checknumber(L, 3);

    uint32_t soundId = st_sound_create_sweep_down(start_freq, end_freq, duration);
    lua_pushinteger(L, soundId);
    return 1;
}

static int lua_st_sound_create_big_explosion(lua_State* L) {
    float size = luaL_checknumber(L, 1);
    float duration = luaL_checknumber(L, 2);

    uint32_t soundId = st_sound_create_big_explosion(size, duration);
    lua_pushinteger(L, soundId);
    return 1;
}

static int lua_st_sound_create_small_explosion(lua_State* L) {
    float intensity = luaL_checknumber(L, 1);
    float duration = luaL_checknumber(L, 2);

    uint32_t soundId = st_sound_create_small_explosion(intensity, duration);
    lua_pushinteger(L, soundId);
    return 1;
}

static int lua_st_sound_create_distant_explosion(lua_State* L) {
    float distance = luaL_checknumber(L, 1);
    float duration = luaL_checknumber(L, 2);

    uint32_t soundId = st_sound_create_distant_explosion(distance, duration);
    lua_pushinteger(L, soundId);
    return 1;
}

static int lua_st_sound_create_metal_explosion(lua_State* L) {
    float shrapnel = luaL_checknumber(L, 1);
    float duration = luaL_checknumber(L, 2);

    uint32_t soundId = st_sound_create_metal_explosion(shrapnel, duration);
    lua_pushinteger(L, soundId);
    return 1;
}

static int lua_st_sound_create_bang(lua_State* L) {
    float intensity = luaL_checknumber(L, 1);
    float duration = luaL_checknumber(L, 2);

    uint32_t soundId = st_sound_create_bang(intensity, duration);
    lua_pushinteger(L, soundId);
    return 1;
}

static int lua_st_sound_create_random_beep(lua_State* L) {
    uint32_t seed = luaL_checkinteger(L, 1);
    float duration = luaL_checknumber(L, 2);

    uint32_t soundId = st_sound_create_random_beep(seed, duration);
    lua_pushinteger(L, soundId);
    return 1;
}

// Phase 3: Custom Synthesis
static int lua_st_sound_create_tone(lua_State* L) {
    float frequency = luaL_checknumber(L, 1);
    float duration = luaL_checknumber(L, 2);
    int waveform = luaL_checkinteger(L, 3);

    uint32_t soundId = st_sound_create_tone(frequency, duration, waveform);
    lua_pushinteger(L, soundId);
    return 1;
}

static int lua_st_sound_create_note(lua_State* L) {
    int note = luaL_checkinteger(L, 1);
    float duration = luaL_checknumber(L, 2);
    int waveform = luaL_checkinteger(L, 3);
    float attack = luaL_checknumber(L, 4);
    float decay = luaL_checknumber(L, 5);
    float sustainLevel = luaL_checknumber(L, 6);
    float release = luaL_checknumber(L, 7);

    uint32_t soundId = st_sound_create_note(note, duration, waveform, attack, decay, sustainLevel, release);
    lua_pushinteger(L, soundId);
    return 1;
}

static int lua_st_sound_create_noise(lua_State* L) {
    int noiseType = luaL_checkinteger(L, 1);
    float duration = luaL_checknumber(L, 2);

    uint32_t soundId = st_sound_create_noise(noiseType, duration);
    lua_pushinteger(L, soundId);
    return 1;
}

// Phase 4: Advanced Synthesis
static int lua_st_sound_create_fm(lua_State* L) {
    float carrier_freq = luaL_checknumber(L, 1);
    float modulator_freq = luaL_checknumber(L, 2);
    float mod_index = luaL_checknumber(L, 3);
    float duration = luaL_checknumber(L, 4);

    uint32_t soundId = st_sound_create_fm(carrier_freq, modulator_freq, mod_index, duration);
    lua_pushinteger(L, soundId);
    return 1;
}

static int lua_st_sound_create_filtered_tone(lua_State* L) {
    float frequency = luaL_checknumber(L, 1);
    float duration = luaL_checknumber(L, 2);
    int waveform = luaL_checkinteger(L, 3);
    int filter_type = luaL_checkinteger(L, 4);
    float cutoff = luaL_checknumber(L, 5);
    float resonance = luaL_checknumber(L, 6);

    uint32_t soundId = st_sound_create_filtered_tone(frequency, duration, waveform,
                                                      filter_type, cutoff, resonance);
    lua_pushinteger(L, soundId);
    return 1;
}

static int lua_st_sound_create_filtered_note(lua_State* L) {
    int note = luaL_checkinteger(L, 1);
    float duration = luaL_checknumber(L, 2);
    int waveform = luaL_checkinteger(L, 3);
    float attack = luaL_checknumber(L, 4);
    float decay = luaL_checknumber(L, 5);
    float sustain_level = luaL_checknumber(L, 6);
    float release = luaL_checknumber(L, 7);
    int filter_type = luaL_checkinteger(L, 8);
    float cutoff = luaL_checknumber(L, 9);
    float resonance = luaL_checknumber(L, 10);

    uint32_t soundId = st_sound_create_filtered_note(note, duration, waveform,
                                                      attack, decay, sustain_level, release,
                                                      filter_type, cutoff, resonance);
    lua_pushinteger(L, soundId);
    return 1;
}

// Phase 5: Effects Chain
static int lua_st_sound_create_with_reverb(lua_State* L) {
    float frequency = luaL_checknumber(L, 1);
    float duration = luaL_checknumber(L, 2);
    int waveform = luaL_checkinteger(L, 3);
    float room_size = luaL_checknumber(L, 4);
    float damping = luaL_checknumber(L, 5);
    float wet = luaL_checknumber(L, 6);

    uint32_t soundId = st_sound_create_with_reverb(frequency, duration, waveform,
                                                    room_size, damping, wet);
    lua_pushinteger(L, soundId);
    return 1;
}

static int lua_st_sound_create_with_delay(lua_State* L) {
    float frequency = luaL_checknumber(L, 1);
    float duration = luaL_checknumber(L, 2);
    int waveform = luaL_checkinteger(L, 3);
    float delay_time = luaL_checknumber(L, 4);
    float feedback = luaL_checknumber(L, 5);
    float mix = luaL_checknumber(L, 6);

    uint32_t soundId = st_sound_create_with_delay(frequency, duration, waveform,
                                                   delay_time, feedback, mix);
    lua_pushinteger(L, soundId);
    return 1;
}

static int lua_st_sound_create_with_distortion(lua_State* L) {
    float frequency = luaL_checknumber(L, 1);
    float duration = luaL_checknumber(L, 2);
    int waveform = luaL_checkinteger(L, 3);
    float drive = luaL_checknumber(L, 4);
    float tone = luaL_checknumber(L, 5);
    float level = luaL_checknumber(L, 6);

    uint32_t soundId = st_sound_create_with_distortion(frequency, duration, waveform,
                                                        drive, tone, level);
    lua_pushinteger(L, soundId);
    return 1;
}

static int lua_st_sound_play_id(lua_State* L) {
    uint32_t soundId = luaL_checkinteger(L, 1);
    float volume = luaL_optnumber(L, 2, 1.0f);
    float pan = luaL_optnumber(L, 3, 0.0f);

    st_sound_play_id(soundId, volume, pan);
    return 0;
}

static int lua_st_sound_play_with_fade(lua_State* L) {
    uint32_t soundId = luaL_checkinteger(L, 1);
    float volume = luaL_checknumber(L, 2);
    float capDuration = luaL_checknumber(L, 3);

    st_sound_play_with_fade(soundId, volume, capDuration);
    return 0;
}

static int lua_st_sound_free_id(lua_State* L) {
    uint32_t soundId = luaL_checkinteger(L, 1);

    bool freed = st_sound_free_id(soundId);
    lua_pushboolean(L, freed);
    return 1;
}

static int lua_st_sound_free_all(lua_State* L) {
    st_sound_free_all();
    return 0;
}

static int lua_st_sound_exists(lua_State* L) {
    uint32_t soundId = luaL_checkinteger(L, 1);

    bool exists = st_sound_exists(soundId);
    lua_pushboolean(L, exists);
    return 1;
}

static int lua_st_sound_get_count(lua_State* L) {
    size_t count = st_sound_get_count();
    lua_pushinteger(L, count);
    return 1;
}

static int lua_st_sound_get_memory_usage(lua_State* L) {
    size_t usage = st_sound_get_memory_usage();
    lua_pushinteger(L, usage);
    return 1;
}

// =============================================================================
// Voice Controller API Bindings
// =============================================================================

static int lua_st_voice_set_waveform(lua_State* L) {
    int voiceNum = luaL_checkinteger(L, 1);
    int waveform = luaL_checkinteger(L, 2);
    st_voice_set_waveform(voiceNum, waveform);
    return 0;
}

static int lua_st_voice_set_frequency(lua_State* L) {
    int voiceNum = luaL_checkinteger(L, 1);
    float frequencyHz = luaL_checknumber(L, 2);
    st_voice_set_frequency(voiceNum, frequencyHz);
    return 0;
}

static int lua_st_voice_set_note(lua_State* L) {
    int voiceNum = luaL_checkinteger(L, 1);
    int midiNote = luaL_checkinteger(L, 2);
    st_voice_set_note(voiceNum, midiNote);
    return 0;
}

static int lua_st_voice_set_note_name(lua_State* L) {
    int voiceNum = luaL_checkinteger(L, 1);
    const char* noteName = luaL_checkstring(L, 2);
    st_voice_set_note_name(voiceNum, noteName);
    return 0;
}

static int lua_st_voice_set_envelope(lua_State* L) {
    int voiceNum = luaL_checkinteger(L, 1);
    float attackMs = luaL_checknumber(L, 2);
    float decayMs = luaL_checknumber(L, 3);
    float sustainLevel = luaL_checknumber(L, 4);
    float releaseMs = luaL_checknumber(L, 5);
    st_voice_set_envelope(voiceNum, attackMs, decayMs, sustainLevel, releaseMs);
    return 0;
}

static int lua_st_voice_set_gate(lua_State* L) {
    int voiceNum = luaL_checkinteger(L, 1);
    int gateOn = luaL_checkinteger(L, 2);
    st_voice_set_gate(voiceNum, gateOn);
    return 0;
}

static int lua_st_voice_set_volume(lua_State* L) {
    int voiceNum = luaL_checkinteger(L, 1);
    float volume = luaL_checknumber(L, 2);
    st_voice_set_volume(voiceNum, volume);
    return 0;
}

static int lua_st_voice_set_pulse_width(lua_State* L) {
    int voiceNum = luaL_checkinteger(L, 1);
    float pulseWidth = luaL_checknumber(L, 2);
    st_voice_set_pulse_width(voiceNum, pulseWidth);
    return 0;
}

static int lua_st_voice_set_filter_routing(lua_State* L) {
    int voiceNum = luaL_checkinteger(L, 1);
    int enabled = luaL_checkinteger(L, 2);
    st_voice_set_filter_routing(voiceNum, enabled);
    return 0;
}

static int lua_st_voice_set_filter_type(lua_State* L) {
    int filterType = luaL_checkinteger(L, 1);
    st_voice_set_filter_type(filterType);
    return 0;
}

static int lua_st_voice_set_filter_cutoff(lua_State* L) {
    float cutoffHz = luaL_checknumber(L, 1);
    st_voice_set_filter_cutoff(cutoffHz);
    return 0;
}

static int lua_st_voice_set_filter_resonance(lua_State* L) {
    float resonance = luaL_checknumber(L, 1);
    st_voice_set_filter_resonance(resonance);
    return 0;
}

static int lua_st_voice_set_filter_enabled(lua_State* L) {
    int enabled = luaL_checkinteger(L, 1);
    st_voice_set_filter_enabled(enabled);
    return 0;
}

static int lua_st_voice_set_master_volume(lua_State* L) {
    float volume = luaL_checknumber(L, 1);
    st_voice_set_master_volume(volume);
    return 0;
}

static int lua_st_voice_get_master_volume(lua_State* L) {
    float volume = st_voice_get_master_volume();
    lua_pushnumber(L, volume);
    return 1;
}

static int lua_st_voice_reset_all(lua_State* L) {
    (void)L;
    st_voice_reset_all();
    return 0;
}

static int lua_st_voice_get_active_count(lua_State* L) {
    int count = st_voice_get_active_count();
    lua_pushinteger(L, count);
    return 1;
}

static int lua_st_voices_are_playing(lua_State* L) {
    int playing = st_voices_are_playing();
    lua_pushboolean(L, playing);
    return 1;
}

static int lua_st_voice_direct(lua_State* L) {
    const char* destination = luaL_checkstring(L, 1);
    st_voice_direct(destination);
    return 0;
}

static int lua_st_voice_direct_slot(lua_State* L) {
    int slotNum = luaL_checkinteger(L, 1);
    float volume = luaL_checknumber(L, 2);
    float duration = luaL_optnumber(L, 3, 0.0f);
    uint32_t soundId = st_voice_direct_slot(slotNum, volume, duration);
    lua_pushinteger(L, soundId);
    return 1;
}

static int lua_st_vscript_save_to_bank(lua_State* L) {
    const char* scriptName = luaL_checkstring(L, 1);
    float duration = luaL_optnumber(L, 2, 0.0f);
    uint32_t soundId = st_vscript_save_to_bank(scriptName, duration);
    lua_pushinteger(L, soundId);
    return 1;
}

// =============================================================================
// Voice Controller Extended API - Stereo & Spatial
// =============================================================================

static int lua_st_voice_set_pan(lua_State* L) {
    int voiceNum = luaL_checkinteger(L, 1);
    float pan = luaL_checknumber(L, 2);
    st_voice_set_pan(voiceNum, pan);
    return 0;
}

// =============================================================================
// Voice Controller Extended API - SID-Style Modulation
// =============================================================================

static int lua_st_voice_set_ring_mod(lua_State* L) {
    int voiceNum = luaL_checkinteger(L, 1);
    int sourceVoice = luaL_checkinteger(L, 2);
    st_voice_set_ring_mod(voiceNum, sourceVoice);
    return 0;
}

static int lua_st_voice_set_sync(lua_State* L) {
    int voiceNum = luaL_checkinteger(L, 1);
    int sourceVoice = luaL_checkinteger(L, 2);
    st_voice_set_sync(voiceNum, sourceVoice);
    return 0;
}

static int lua_st_voice_set_portamento(lua_State* L) {
    int voiceNum = luaL_checkinteger(L, 1);
    float time = luaL_checknumber(L, 2);
    st_voice_set_portamento(voiceNum, time);
    return 0;
}

static int lua_st_voice_set_detune(lua_State* L) {
    int voiceNum = luaL_checkinteger(L, 1);
    float cents = luaL_checknumber(L, 2);
    st_voice_set_detune(voiceNum, cents);
    return 0;
}

// =============================================================================
// Voice Controller Extended API - Delay Effects
// =============================================================================

static int lua_st_voice_set_delay_enable(lua_State* L) {
    int voiceNum = luaL_checkinteger(L, 1);
    int enabled = luaL_checkinteger(L, 2);
    st_voice_set_delay_enable(voiceNum, enabled);
    return 0;
}

static int lua_st_voice_set_delay_time(lua_State* L) {
    int voiceNum = luaL_checkinteger(L, 1);
    float time = luaL_checknumber(L, 2);
    st_voice_set_delay_time(voiceNum, time);
    return 0;
}

static int lua_st_voice_set_delay_feedback(lua_State* L) {
    int voiceNum = luaL_checkinteger(L, 1);
    float feedback = luaL_checknumber(L, 2);
    st_voice_set_delay_feedback(voiceNum, feedback);
    return 0;
}

static int lua_st_voice_set_delay_mix(lua_State* L) {
    int voiceNum = luaL_checkinteger(L, 1);
    float mix = luaL_checknumber(L, 2);
    st_voice_set_delay_mix(voiceNum, mix);
    return 0;
}

// =============================================================================
// Voice Controller Extended API - LFO Controls
// =============================================================================

static int lua_st_lfo_set_waveform(lua_State* L) {
    int lfoNum = luaL_checkinteger(L, 1);
    int waveform = luaL_checkinteger(L, 2);
    st_lfo_set_waveform(lfoNum, waveform);
    return 0;
}

static int lua_st_lfo_set_rate(lua_State* L) {
    int lfoNum = luaL_checkinteger(L, 1);
    float rateHz = luaL_checknumber(L, 2);
    st_lfo_set_rate(lfoNum, rateHz);
    return 0;
}

static int lua_st_lfo_reset(lua_State* L) {
    int lfoNum = luaL_checkinteger(L, 1);
    st_lfo_reset(lfoNum);
    return 0;
}

static int lua_st_lfo_to_pitch(lua_State* L) {
    int voiceNum = luaL_checkinteger(L, 1);
    int lfoNum = luaL_checkinteger(L, 2);
    float depthCents = luaL_checknumber(L, 3);
    st_lfo_to_pitch(voiceNum, lfoNum, depthCents);
    return 0;
}

static int lua_st_lfo_to_volume(lua_State* L) {
    int voiceNum = luaL_checkinteger(L, 1);
    int lfoNum = luaL_checkinteger(L, 2);
    float depth = luaL_checknumber(L, 3);
    st_lfo_to_volume(voiceNum, lfoNum, depth);
    return 0;
}

static int lua_st_lfo_to_filter(lua_State* L) {
    int voiceNum = luaL_checkinteger(L, 1);
    int lfoNum = luaL_checkinteger(L, 2);
    float depthHz = luaL_checknumber(L, 3);
    st_lfo_to_filter(voiceNum, lfoNum, depthHz);
    return 0;
}

static int lua_st_lfo_to_pulsewidth(lua_State* L) {
    int voiceNum = luaL_checkinteger(L, 1);
    int lfoNum = luaL_checkinteger(L, 2);
    float depth = luaL_checknumber(L, 3);
    st_lfo_to_pulsewidth(voiceNum, lfoNum, depth);
    return 0;
}

// =============================================================================
// Voice Controller Extended API - Physical Modeling
// =============================================================================

static int lua_st_voice_set_physical_model(lua_State* L) {
    int voiceNum = luaL_checkinteger(L, 1);
    int modelType = luaL_checkinteger(L, 2);
    st_voice_set_physical_model(voiceNum, modelType);
    return 0;
}

static int lua_st_voice_set_physical_damping(lua_State* L) {
    int voiceNum = luaL_checkinteger(L, 1);
    float damping = luaL_checknumber(L, 2);
    st_voice_set_physical_damping(voiceNum, damping);
    return 0;
}

static int lua_st_voice_set_physical_brightness(lua_State* L) {
    int voiceNum = luaL_checkinteger(L, 1);
    float brightness = luaL_checknumber(L, 2);
    st_voice_set_physical_brightness(voiceNum, brightness);
    return 0;
}

static int lua_st_voice_set_physical_excitation(lua_State* L) {
    int voiceNum = luaL_checkinteger(L, 1);
    float excitation = luaL_checknumber(L, 2);
    st_voice_set_physical_excitation(voiceNum, excitation);
    return 0;
}

static int lua_st_voice_set_physical_resonance(lua_State* L) {
    int voiceNum = luaL_checkinteger(L, 1);
    float resonance = luaL_checknumber(L, 2);
    st_voice_set_physical_resonance(voiceNum, resonance);
    return 0;
}

static int lua_st_voice_set_physical_tension(lua_State* L) {
    int voiceNum = luaL_checkinteger(L, 1);
    float tension = luaL_checknumber(L, 2);
    st_voice_set_physical_tension(voiceNum, tension);
    return 0;
}

static int lua_st_voice_set_physical_pressure(lua_State* L) {
    int voiceNum = luaL_checkinteger(L, 1);
    float pressure = luaL_checknumber(L, 2);
    st_voice_set_physical_pressure(voiceNum, pressure);
    return 0;
}

static int lua_st_voice_physical_trigger(lua_State* L) {
    int voiceNum = luaL_checkinteger(L, 1);
    st_voice_physical_trigger(voiceNum);
    return 0;
}

// =============================================================================
// VOICES Timeline System
// =============================================================================

static int lua_st_voices_start(lua_State* L) {
    LOG_DEBUG("lua_st_voices_start: called");
    st_voices_start();
    LOG_DEBUG("lua_st_voices_start: returned from C function");
    return 0;
}

static int lua_st_voice_wait(lua_State* L) {
    float beats = luaL_checknumber(L, 1);
    st_voice_wait(beats);
    return 0;
}

static int lua_st_voice_wait_beats(lua_State* L) {
    float beats = luaL_checknumber(L, 1);
    st_voice_wait(beats);
    return 0;
}

static int lua_st_voices_set_tempo(lua_State* L) {
    LOG_DEBUG("lua_st_voices_set_tempo: called");
    float bpm = luaL_checknumber(L, 1);
    LOG_DEBUGF("lua_st_voices_set_tempo: got bpm=%.1f", bpm);
    st_voices_set_tempo(bpm);
    LOG_DEBUG("lua_st_voices_set_tempo: returned from C function");
    return 0;
}

static int lua_st_voices_end_slot(lua_State* L) {
    int slot = luaL_checkinteger(L, 1);
    float volume = luaL_checknumber(L, 2);
    st_voices_end_slot(slot, volume);
    return 0;
}

static int lua_st_voices_next_slot(lua_State* L) {
    float volume = luaL_checknumber(L, 1);
    uint32_t soundId = st_voices_next_slot(volume);
    lua_pushinteger(L, soundId);
    return 1;
}

static int lua_st_voices_end_play(lua_State* L) {
    st_voices_end_play();
    return 0;
}

static int lua_st_voices_end_save(lua_State* L) {
    const char* filename = luaL_checkstring(L, 1);
    st_voices_end_save(filename);
    return 0;
}

// =============================================================================
// Input API Bindings
// =============================================================================

static int lua_st_key_pressed(lua_State* L) {
    int key = luaL_checkinteger(L, 1);
    lua_pushboolean(L, st_key_pressed((STKeyCode)key));
    return 1;
}

static int lua_st_key_just_pressed(lua_State* L) {
    int key = luaL_checkinteger(L, 1);
    lua_pushboolean(L, st_key_just_pressed((STKeyCode)key));
    return 1;
}

static int lua_st_key_just_released(lua_State* L) {
    int key = luaL_checkinteger(L, 1);
    lua_pushboolean(L, st_key_just_released((STKeyCode)key));
    return 1;
}

static int lua_st_key_get_char(lua_State* L) {
    uint32_t ch = st_key_get_char();
    if (ch) {
        char str[2] = { (char)ch, 0 };
        lua_pushstring(L, str);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static int lua_st_key_clear_buffer(lua_State* L) {
    (void)L;
    st_key_clear_buffer();
    return 0;
}

// INPUT_AT function - interactive input at specific coordinates using SimpleLineEditor
static int lua_st_basic_input_at(lua_State* L) {
    // Safety check: ensure we have a valid Lua state
    if (!L) {
        return 0;
    }

    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    std::string prompt = "";
    if (lua_gettop(L) > 2 && !lua_isnil(L, 3)) {
        prompt = luaL_checkstring(L, 3);
    }

    // Optional: fg/bg colors (for future BASIC INPUT AT x, y, prompt; fgcolor, bgcolor)
    int fgColor = -1;  // -1 means use default
    int bgColor = -1;
    if (lua_gettop(L) > 3 && !lua_isnil(L, 4)) {
        fgColor = luaL_checkinteger(L, 4);
    }
    if (lua_gettop(L) > 4 && !lua_isnil(L, 5)) {
        bgColor = luaL_checkinteger(L, 5);
    }

    // Safety check: validate coordinates
    if (x < 0 || y < 0 || x > 1000 || y > 1000) {
        lua_pushstring(L, "");
        return 1;
    }

    // Clear input buffer first
    st_key_clear_buffer();

    // Request line input mode from Context (render thread will drive the editor)
    // This call blocks until the render thread completes the input
    ST_CONTEXT.requestLineInput(x, y, prompt, fgColor, bgColor);

    // Get the final result from Context
    std::string result = ST_CONTEXT.getLineInputResult();


    lua_pushstring(L, result.c_str());
    return 1;
}

// BASIC PRINT function - print to runtime text buffer at cursor position
static int lua_st_basic_print(lua_State* L) {
    int n = lua_gettop(L);
    std::string output;
    
    for (int i = 1; i <= n; i++) {
        if (i > 1) {
            output += " ";  // Space between arguments
        }
        
        if (lua_isstring(L, i)) {
            output += lua_tostring(L, i);
        } else if (lua_istable(L, i)) {
            // Handle Unicode codepoint arrays (OPTION UNICODE mode)
            // Call unicode.to_utf8() on the table
            lua_getglobal(L, "unicode");
            if (lua_istable(L, -1)) {
                lua_getfield(L, -1, "to_utf8");
                if (lua_isfunction(L, -1)) {
                    lua_pushvalue(L, i);  // Push the codepoint table
                    if (lua_pcall(L, 1, 1, 0) == 0) {
                        // Success - get the UTF-8 string result
                        if (lua_isstring(L, -1)) {
                            output += lua_tostring(L, -1);
                        }
                        lua_pop(L, 1);  // Pop result
                    } else {
                        // Error in conversion - pop error message
                        lua_pop(L, 1);
                        output += "[unicode error]";
                    }
                } else {
                    lua_pop(L, 1);  // Pop non-function
                    output += "[table]";
                }
                lua_pop(L, 1);  // Pop unicode module
            } else {
                lua_pop(L, 1);  // Pop non-table
                output += "[table]";
            }
        } else if (lua_isnumber(L, i)) {
            double num = lua_tonumber(L, i);
            // Check if it's an integer
            if (num == floor(num)) {
                output += std::to_string((long long)num);
            } else {
                output += std::to_string(num);
            }
        } else if (lua_isboolean(L, i)) {
            output += lua_toboolean(L, i) ? "true" : "false";
        } else if (lua_isnil(L, i)) {
            // Skip nil values
        } else {
            output += "[object]";
        }
    }
    
    // Print to runtime text buffer using C wrapper
    fbrunner3_runtime_print_text(output.c_str());
    
    // Add small delay to prevent UI flooding when printing in tight loops
    // This allows the UI thread to process events (like Stop button clicks)
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    
    // Check if script should be stopped during print-heavy operations
    if (fbrunner3_should_stop_script()) {
        luaL_error(L, "Script interrupted during PRINT operation");
    }
    
    return 0;
}

// BASIC CONSOLE function - print to console (stdout)
static int lua_st_basic_console(lua_State* L) {
    int n = lua_gettop(L);
    std::string output;
    
    for (int i = 1; i <= n; i++) {
        if (i > 1) {
            output += " ";  // Space between arguments
        }
        
        if (lua_isstring(L, i)) {
            output += lua_tostring(L, i);
        } else if (lua_isnumber(L, i)) {
            double num = lua_tonumber(L, i);
            // Check if it's an integer
            if (num == floor(num)) {
                output += std::to_string((long long)num);
            } else {
                output += std::to_string(num);
            }
        } else if (lua_isboolean(L, i)) {
            output += lua_toboolean(L, i) ? "true" : "false";
        } else if (lua_isnil(L, i)) {
            // Skip nil values
        } else {
            output += "[object]";
        }
    }
    
    // Print to console (stdout)  
    printf("%s", output.c_str());
    fflush(stdout);
    return 0;
}

// BASIC PRINT NEWLINE function - move cursor to next line
static int lua_st_basic_print_newline(lua_State* L) {
    // Print newline to runtime text buffer using C wrapper
    fbrunner3_runtime_print_newline();
    
    // Add small delay to prevent UI flooding when printing in tight loops
    // This allows the UI thread to process events (like Stop button clicks)
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    
    // Check if script should be stopped during print-heavy operations
    if (fbrunner3_should_stop_script()) {
        luaL_error(L, "Script interrupted during PRINT operation");
    }
    
    return 0;
}

// BASIC LOCATE function - set cursor position in runtime mode
static int lua_st_basic_locate(lua_State* L) {
    int row = luaL_checkinteger(L, 1);
    int col = luaL_optinteger(L, 2, 1);
    
    // Set cursor position in runtime text state using C wrapper
    fbrunner3_runtime_set_cursor(col, row);
    return 0;
}

static int lua_st_mouse_position(lua_State* L) {
    int x, y;
    st_mouse_position(&x, &y);
    lua_pushinteger(L, x);
    lua_pushinteger(L, y);
    return 2;
}

static int lua_st_mouse_grid_position(lua_State* L) {
    int x, y;
    st_mouse_grid_position(&x, &y);
    lua_pushinteger(L, x);
    lua_pushinteger(L, y);
    return 2;
}

static int lua_st_mouse_button(lua_State* L) {
    int button = luaL_checkinteger(L, 1);
    lua_pushboolean(L, st_mouse_button((STMouseButton)button));
    return 1;
}

static int lua_st_mouse_button_just_pressed(lua_State* L) {
    int button = luaL_checkinteger(L, 1);
    lua_pushboolean(L, st_mouse_button_just_pressed((STMouseButton)button));
    return 1;
}

static int lua_st_mouse_button_just_released(lua_State* L) {
    int button = luaL_checkinteger(L, 1);
    lua_pushboolean(L, st_mouse_button_just_released((STMouseButton)button));
    return 1;
}

// =============================================================================
// Random Number Generation
// =============================================================================

static int lua_st_rand(lua_State* L) {
    int n = luaL_checkinteger(L, 1);
    if (n < 0) {
        lua_pushinteger(L, 0);
        return 1;
    }
    // Generate random integer in range 0..n (inclusive)
    int result = rand() % (n + 1);
    lua_pushinteger(L, result);
    return 1;
}

// =============================================================================
// Frame Control API Bindings
// =============================================================================

static int lua_st_wait_frame(lua_State* L) {
    (void)L;
    st_wait_frame();
    return 0;
}

static int lua_st_wait_frames(lua_State* L) {
    int count = luaL_checkinteger(L, 1);
    st_wait_frames(count);
    return 0;
}

static int lua_st_wait_ms(lua_State* L) {
    int milliseconds = luaL_checkinteger(L, 1);
    st_wait_ms(milliseconds);
    return 0;
}

static int lua_st_wait(lua_State* L) {
    float seconds = luaL_checknumber(L, 1);
    int frames = (int)(seconds * 60.0f);  // Assume 60 FPS
    for (int i = 0; i < frames; i++) {
        st_wait_frame();
    }
    return 0;
}

static int lua_st_frame_count(lua_State* L) {
    lua_pushinteger(L, st_frame_count());
    return 1;
}

static int lua_shouldStopScript(lua_State* L) {
    // Check if the script should be cancelled
    bool shouldStop = STApi::Context::instance().shouldStopScript();
    lua_pushboolean(L, shouldStop);
    return 1;
}

static int lua_st_time(lua_State* L) {
    lua_pushnumber(L, st_time());
    return 1;
}

static int lua_st_delta_time(lua_State* L) {
    lua_pushnumber(L, st_delta_time());
    return 1;
}

// =============================================================================
// Utility API Bindings
// =============================================================================

static int lua_st_rgb(lua_State* L) {
    int r = luaL_checkinteger(L, 1);
    int g = luaL_checkinteger(L, 2);
    int b = luaL_checkinteger(L, 3);

    lua_pushinteger(L, st_rgb(r, g, b));
    return 1;
}

static int lua_st_rgba(lua_State* L) {
    int r = luaL_checkinteger(L, 1);
    int g = luaL_checkinteger(L, 2);
    int b = luaL_checkinteger(L, 3);
    int a = luaL_checkinteger(L, 4);

    lua_pushinteger(L, st_rgba(r, g, b, a));
    return 1;
}

static int lua_st_hsv(lua_State* L) {
    float h = luaL_checknumber(L, 1);
    float s = luaL_checknumber(L, 2);
    float v = luaL_checknumber(L, 3);

    lua_pushinteger(L, st_hsv(h, s, v));
    return 1;
}

static int lua_st_debug_print(lua_State* L) {
    const char* msg = luaL_checkstring(L, 1);
    st_debug_print(msg);
    return 0;
}

// =============================================================================
// Display API Bindings
// =============================================================================

static int lua_st_display_size(lua_State* L) {
    int width, height;
    st_display_size(&width, &height);
    lua_pushinteger(L, width);
    lua_pushinteger(L, height);
    return 2;
}

static int lua_st_cell_size(lua_State* L) {
    int width, height;
    st_cell_size(&width, &height);
    lua_pushinteger(L, width);
    lua_pushinteger(L, height);
    return 2;
}

// =============================================================================
// Registration Function
// =============================================================================
// Chunky Pixel Graphics API Bindings
// =============================================================================

static int lua_st_lores_pset(lua_State* L) {
    int pixel_x = luaL_checkinteger(L, 1);
    int pixel_y = luaL_checkinteger(L, 2);
    uint8_t color_index = (uint8_t)luaL_checkinteger(L, 3);
    uint32_t bg = luaL_optinteger(L, 4, 0x000000FF);

    st_lores_pset(pixel_x, pixel_y, color_index, bg);
    return 0;
}

static int lua_st_lores_line(lua_State* L) {
    int x1 = luaL_checkinteger(L, 1);
    int y1 = luaL_checkinteger(L, 2);
    int x2 = luaL_checkinteger(L, 3);
    int y2 = luaL_checkinteger(L, 4);
    uint8_t color_index = (uint8_t)luaL_checkinteger(L, 5);
    uint32_t bg = luaL_optinteger(L, 6, 0x000000FF);

    st_lores_line(x1, y1, x2, y2, color_index, bg);
    return 0;
}

static int lua_st_lores_rect(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int width = luaL_checkinteger(L, 3);
    int height = luaL_checkinteger(L, 4);
    uint8_t color_index = (uint8_t)luaL_checkinteger(L, 5);
    uint32_t bg = luaL_optinteger(L, 6, 0x000000FF);

    st_lores_rect(x, y, width, height, color_index, bg);
    return 0;
}

static int lua_st_lores_fillrect(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int width = luaL_checkinteger(L, 3);
    int height = luaL_checkinteger(L, 4);
    uint8_t color_index = (uint8_t)luaL_checkinteger(L, 5);
    uint32_t bg = luaL_optinteger(L, 6, 0x000000FF);

    st_lores_fillrect(x, y, width, height, color_index, bg);
    return 0;
}

static int lua_st_lores_hline(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int width = luaL_checkinteger(L, 3);
    uint8_t color_index = (uint8_t)luaL_checkinteger(L, 4);
    uint32_t bg = luaL_optinteger(L, 5, 0x000000FF);

    st_lores_hline(x, y, width, color_index, bg);
    return 0;
}

static int lua_st_lores_vline(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int height = luaL_checkinteger(L, 3);
    uint8_t color_index = (uint8_t)luaL_checkinteger(L, 4);
    uint32_t bg = luaL_optinteger(L, 5, 0x000000FF);

    st_lores_vline(x, y, height, color_index, bg);
    return 0;
}

static int lua_st_lores_clear(lua_State* L) {
    uint32_t bg = luaL_optinteger(L, 1, 0x000000FF);
    st_lores_clear(bg);
    return 0;
}

static int lua_st_lores_resolution(lua_State* L) {
    int width, height;
    st_lores_resolution(&width, &height);

    lua_pushinteger(L, width);
    lua_pushinteger(L, height);
    return 2;
}

// =============================================================================
// LORES Buffer Management API Bindings
// =============================================================================

static int lua_st_lores_buffer(lua_State* L) {
    int bufferID = luaL_checkinteger(L, 1);
    st_lores_buffer(bufferID);
    return 0;
}

static int lua_st_lores_buffer_get(lua_State* L) {
    int bufferID = st_lores_buffer_get();
    lua_pushinteger(L, bufferID);
    return 1;
}

static int lua_st_lores_flip(lua_State* L) {
    st_lores_flip();
    return 0;
}

// =============================================================================
// LORES Blitter API Bindings
// =============================================================================

static int lua_st_lores_blit(lua_State* L) {
    int src_x = luaL_checkinteger(L, 1);
    int src_y = luaL_checkinteger(L, 2);
    int width = luaL_checkinteger(L, 3);
    int height = luaL_checkinteger(L, 4);
    int dst_x = luaL_checkinteger(L, 5);
    int dst_y = luaL_checkinteger(L, 6);
    
    st_lores_blit(src_x, src_y, width, height, dst_x, dst_y);
    return 0;
}

static int lua_st_lores_blit_trans(lua_State* L) {
    int src_x = luaL_checkinteger(L, 1);
    int src_y = luaL_checkinteger(L, 2);
    int width = luaL_checkinteger(L, 3);
    int height = luaL_checkinteger(L, 4);
    int dst_x = luaL_checkinteger(L, 5);
    int dst_y = luaL_checkinteger(L, 6);
    uint8_t transparent_color = (uint8_t)luaL_checkinteger(L, 7);
    
    st_lores_blit_trans(src_x, src_y, width, height, dst_x, dst_y, transparent_color);
    return 0;
}

static int lua_st_lores_blit_buffer(lua_State* L) {
    int src_buffer = luaL_checkinteger(L, 1);
    int dst_buffer = luaL_checkinteger(L, 2);
    int src_x = luaL_checkinteger(L, 3);
    int src_y = luaL_checkinteger(L, 4);
    int width = luaL_checkinteger(L, 5);
    int height = luaL_checkinteger(L, 6);
    int dst_x = luaL_checkinteger(L, 7);
    int dst_y = luaL_checkinteger(L, 8);
    
    st_lores_blit_buffer(src_buffer, dst_buffer, src_x, src_y, width, height, dst_x, dst_y);
    return 0;
}

static int lua_st_lores_blit_buffer_trans(lua_State* L) {
    int src_buffer = luaL_checkinteger(L, 1);
    int dst_buffer = luaL_checkinteger(L, 2);
    int src_x = luaL_checkinteger(L, 3);
    int src_y = luaL_checkinteger(L, 4);
    int width = luaL_checkinteger(L, 5);
    int height = luaL_checkinteger(L, 6);
    int dst_x = luaL_checkinteger(L, 7);
    int dst_y = luaL_checkinteger(L, 8);
    uint8_t transparent_color = (uint8_t)luaL_checkinteger(L, 9);
    
    st_lores_blit_buffer_trans(src_buffer, dst_buffer, src_x, src_y, width, height, dst_x, dst_y, transparent_color);
    return 0;
}

// =============================================================================
// LORES Mode Management API Bindings
// =============================================================================

static int lua_st_mode(lua_State* L) {
    int mode = luaL_checkinteger(L, 1);
    st_mode(mode);
    return 0;
}

// =============================================================================
// LORES Palette Management API Bindings
// =============================================================================

static int lua_st_lores_palette_set(lua_State* L) {
    const char* mode = luaL_checkstring(L, 1);
    st_lores_palette_set(mode);
    return 0;
}

static int lua_st_lores_palette_poke(lua_State* L) {
    int row = luaL_checkinteger(L, 1);
    int index = luaL_checkinteger(L, 2);
    uint32_t rgba = (uint32_t)luaL_checkinteger(L, 3);
    st_lores_palette_poke(row, index, rgba);
    return 0;
}

static int lua_st_lores_palette_peek(lua_State* L) {
    int row = luaL_checkinteger(L, 1);
    int index = luaL_checkinteger(L, 2);
    uint32_t rgba = st_lores_palette_peek(row, index);
    lua_pushinteger(L, rgba);
    return 1;
}

// =============================================================================
// XRES Palette API Bindings
// =============================================================================

static int lua_st_xres_palette_row(lua_State* L) {
    int row = luaL_checkinteger(L, 1);
    int index = luaL_checkinteger(L, 2);
    int r = luaL_checkinteger(L, 3);
    int g = luaL_checkinteger(L, 4);
    int b = luaL_checkinteger(L, 5);
    st_xres_palette_row(row, index, r, g, b);
    return 0;
}

static int lua_st_xres_palette_global(lua_State* L) {
    int index = luaL_checkinteger(L, 1);
    int r = luaL_checkinteger(L, 2);
    int g = luaL_checkinteger(L, 3);
    int b = luaL_checkinteger(L, 4);
    st_xres_palette_global(index, r, g, b);
    return 0;
}

static int lua_st_xres_palette_reset(lua_State* L) {
    st_xres_palette_reset();
    return 0;
}

// =============================================================================
// WRES Palette API Bindings
// =============================================================================

static int lua_st_wres_palette_row(lua_State* L) {
    int row = luaL_checkinteger(L, 1);
    int index = luaL_checkinteger(L, 2);
    int r = luaL_checkinteger(L, 3);
    int g = luaL_checkinteger(L, 4);
    int b = luaL_checkinteger(L, 5);
    st_wres_palette_row(row, index, r, g, b);
    return 0;
}

static int lua_st_wres_palette_global(lua_State* L) {
    int index = luaL_checkinteger(L, 1);
    int r = luaL_checkinteger(L, 2);
    int g = luaL_checkinteger(L, 3);
    int b = luaL_checkinteger(L, 4);
    st_wres_palette_global(index, r, g, b);
    return 0;
}

static int lua_st_wres_palette_reset(lua_State* L) {
    st_wres_palette_reset();
    return 0;
}

// =============================================================================
// PRES Palette API Bindings
// =============================================================================

static int lua_st_pres_palette_row(lua_State* L) {
    int row = luaL_checkinteger(L, 1);
    int index = luaL_checkinteger(L, 2);
    int r = luaL_checkinteger(L, 3);
    int g = luaL_checkinteger(L, 4);
    int b = luaL_checkinteger(L, 5);
    st_pres_palette_row(row, index, r, g, b);
    return 0;
}

static int lua_st_pres_palette_global(lua_State* L) {
    int index = luaL_checkinteger(L, 1);
    int r = luaL_checkinteger(L, 2);
    int g = luaL_checkinteger(L, 3);
    int b = luaL_checkinteger(L, 4);
    st_pres_palette_global(index, r, g, b);
    return 0;
}

static int lua_st_pres_palette_reset(lua_State* L) {
    st_pres_palette_reset();
    return 0;
}

// =============================================================================
// XRES Palette Automation API Bindings
// =============================================================================

static int lua_st_xres_palette_auto_gradient(lua_State* L) {
    int paletteIndex = luaL_checkinteger(L, 1);
    int startRow = luaL_checkinteger(L, 2);
    int endRow = luaL_checkinteger(L, 3);
    int startR = luaL_checkinteger(L, 4);
    int startG = luaL_checkinteger(L, 5);
    int startB = luaL_checkinteger(L, 6);
    int endR = luaL_checkinteger(L, 7);
    int endG = luaL_checkinteger(L, 8);
    int endB = luaL_checkinteger(L, 9);
    float speed = (float)luaL_checknumber(L, 10);
    
    st_xres_palette_auto_gradient(paletteIndex, startRow, endRow,
                                   startR, startG, startB,
                                   endR, endG, endB, speed);
    return 0;
}

static int lua_st_xres_palette_auto_bars(lua_State* L) {
    int paletteIndex = luaL_checkinteger(L, 1);
    int startRow = luaL_checkinteger(L, 2);
    int endRow = luaL_checkinteger(L, 3);
    int barHeight = luaL_checkinteger(L, 4);
    int numColors = luaL_checkinteger(L, 5);
    int r1 = luaL_checkinteger(L, 6);
    int g1 = luaL_checkinteger(L, 7);
    int b1 = luaL_checkinteger(L, 8);
    int r2 = luaL_checkinteger(L, 9);
    int g2 = luaL_checkinteger(L, 10);
    int b2 = luaL_checkinteger(L, 11);
    int r3 = luaL_checkinteger(L, 12);
    int g3 = luaL_checkinteger(L, 13);
    int b3 = luaL_checkinteger(L, 14);
    int r4 = luaL_checkinteger(L, 15);
    int g4 = luaL_checkinteger(L, 16);
    int b4 = luaL_checkinteger(L, 17);
    float speed = (float)luaL_checknumber(L, 18);
    
    st_xres_palette_auto_bars(paletteIndex, startRow, endRow,
                               barHeight, numColors,
                               r1, g1, b1, r2, g2, b2,
                               r3, g3, b3, r4, g4, b4,
                               speed);
    return 0;
}

static int lua_st_xres_palette_auto_stop(lua_State* L) {
    st_xres_palette_auto_stop();
    return 0;
}

static int lua_st_xres_palette_auto_update(lua_State* L) {
    float deltaTime = (float)luaL_checknumber(L, 1);
    st_xres_palette_auto_update(deltaTime);
    return 0;
}

// =============================================================================
// WRES Palette Automation API Bindings
// =============================================================================

static int lua_st_wres_palette_auto_gradient(lua_State* L) {
    int paletteIndex = luaL_checkinteger(L, 1);
    int startRow = luaL_checkinteger(L, 2);
    int endRow = luaL_checkinteger(L, 3);
    int startR = luaL_checkinteger(L, 4);
    int startG = luaL_checkinteger(L, 5);
    int startB = luaL_checkinteger(L, 6);
    int endR = luaL_checkinteger(L, 7);
    int endG = luaL_checkinteger(L, 8);
    int endB = luaL_checkinteger(L, 9);
    float speed = (float)luaL_checknumber(L, 10);
    
    st_wres_palette_auto_gradient(paletteIndex, startRow, endRow,
                                   startR, startG, startB,
                                   endR, endG, endB, speed);
    return 0;
}

static int lua_st_wres_palette_auto_bars(lua_State* L) {
    int paletteIndex = luaL_checkinteger(L, 1);
    int startRow = luaL_checkinteger(L, 2);
    int endRow = luaL_checkinteger(L, 3);
    int barHeight = luaL_checkinteger(L, 4);
    int numColors = luaL_checkinteger(L, 5);
    int r1 = luaL_checkinteger(L, 6);
    int g1 = luaL_checkinteger(L, 7);
    int b1 = luaL_checkinteger(L, 8);
    int r2 = luaL_checkinteger(L, 9);
    int g2 = luaL_checkinteger(L, 10);
    int b2 = luaL_checkinteger(L, 11);
    int r3 = luaL_checkinteger(L, 12);
    int g3 = luaL_checkinteger(L, 13);
    int b3 = luaL_checkinteger(L, 14);
    int r4 = luaL_checkinteger(L, 15);
    int g4 = luaL_checkinteger(L, 16);
    int b4 = luaL_checkinteger(L, 17);
    float speed = (float)luaL_checknumber(L, 18);
    
    st_wres_palette_auto_bars(paletteIndex, startRow, endRow,
                               barHeight, numColors,
                               r1, g1, b1, r2, g2, b2,
                               r3, g3, b3, r4, g4, b4,
                               speed);
    return 0;
}

static int lua_st_wres_palette_auto_stop(lua_State* L) {
    st_wres_palette_auto_stop();
    return 0;
}

static int lua_st_wres_palette_auto_update(lua_State* L) {
    float deltaTime = (float)luaL_checknumber(L, 1);
    st_wres_palette_auto_update(deltaTime);
    return 0;
}

// =============================================================================
// PRES Palette Automation API Bindings
// =============================================================================

static int lua_st_pres_palette_auto_gradient(lua_State* L) {
    int paletteIndex = luaL_checkinteger(L, 1);
    int startRow = luaL_checkinteger(L, 2);
    int endRow = luaL_checkinteger(L, 3);
    int startR = luaL_checkinteger(L, 4);
    int startG = luaL_checkinteger(L, 5);
    int startB = luaL_checkinteger(L, 6);
    int endR = luaL_checkinteger(L, 7);
    int endG = luaL_checkinteger(L, 8);
    int endB = luaL_checkinteger(L, 9);
    float speed = (float)luaL_checknumber(L, 10);
    
    st_pres_palette_auto_gradient(paletteIndex, startRow, endRow,
                                   startR, startG, startB,
                                   endR, endG, endB, speed);
    return 0;
}

static int lua_st_pres_palette_auto_bars(lua_State* L) {
    int paletteIndex = luaL_checkinteger(L, 1);
    int startRow = luaL_checkinteger(L, 2);
    int endRow = luaL_checkinteger(L, 3);
    int barHeight = luaL_checkinteger(L, 4);
    int numColors = luaL_checkinteger(L, 5);
    int r1 = luaL_checkinteger(L, 6);
    int g1 = luaL_checkinteger(L, 7);
    int b1 = luaL_checkinteger(L, 8);
    int r2 = luaL_checkinteger(L, 9);
    int g2 = luaL_checkinteger(L, 10);
    int b2 = luaL_checkinteger(L, 11);
    int r3 = luaL_checkinteger(L, 12);
    int g3 = luaL_checkinteger(L, 13);
    int b3 = luaL_checkinteger(L, 14);
    int r4 = luaL_checkinteger(L, 15);
    int g4 = luaL_checkinteger(L, 16);
    int b4 = luaL_checkinteger(L, 17);
    float speed = (float)luaL_checknumber(L, 18);
    
    st_pres_palette_auto_bars(paletteIndex, startRow, endRow,
                               barHeight, numColors,
                               r1, g1, b1, r2, g2, b2,
                               r3, g3, b3, r4, g4, b4,
                               speed);
    return 0;
}

static int lua_st_pres_palette_auto_stop(lua_State* L) {
    st_pres_palette_auto_stop();
    return 0;
}

static int lua_st_pres_palette_auto_update(lua_State* L) {
    float deltaTime = (float)luaL_checknumber(L, 1);
    st_pres_palette_auto_update(deltaTime);
    return 0;
}

// =============================================================================
// Unified Palette Automation API Bindings (V commands - mode-aware)
// =============================================================================

static int lua_vpalette_auto_gradient(lua_State* L) {
    int paletteIndex = luaL_checkinteger(L, 1);
    int startRow = luaL_checkinteger(L, 2);
    int endRow = luaL_checkinteger(L, 3);
    int startR = luaL_checkinteger(L, 4);
    int startG = luaL_checkinteger(L, 5);
    int startB = luaL_checkinteger(L, 6);
    int endR = luaL_checkinteger(L, 7);
    int endG = luaL_checkinteger(L, 8);
    int endB = luaL_checkinteger(L, 9);
    float speed = (float)luaL_checknumber(L, 10);
    
    int mode = st_mode_get();
    if (mode == VIDEO_MODE_XRES) {
        st_xres_palette_auto_gradient(paletteIndex, startRow, endRow,
                                       startR, startG, startB,
                                       endR, endG, endB, speed);
    } else if (mode == VIDEO_MODE_WRES) {
        st_wres_palette_auto_gradient(paletteIndex, startRow, endRow,
                                       startR, startG, startB,
                                       endR, endG, endB, speed);
    } else if (mode == VIDEO_MODE_PRES) {
        st_pres_palette_auto_gradient(paletteIndex, startRow, endRow,
                                       startR, startG, startB,
                                       endR, endG, endB, speed);
    }
    // LORES and URES don't support per-row palette automation
    return 0;
}

static int lua_vpalette_auto_bars(lua_State* L) {
    int paletteIndex = luaL_checkinteger(L, 1);
    int startRow = luaL_checkinteger(L, 2);
    int endRow = luaL_checkinteger(L, 3);
    int barHeight = luaL_checkinteger(L, 4);
    int numColors = luaL_checkinteger(L, 5);
    int r1 = luaL_checkinteger(L, 6);
    int g1 = luaL_checkinteger(L, 7);
    int b1 = luaL_checkinteger(L, 8);
    int r2 = luaL_checkinteger(L, 9);
    int g2 = luaL_checkinteger(L, 10);
    int b2 = luaL_checkinteger(L, 11);
    int r3 = luaL_checkinteger(L, 12);
    int g3 = luaL_checkinteger(L, 13);
    int b3 = luaL_checkinteger(L, 14);
    int r4 = luaL_checkinteger(L, 15);
    int g4 = luaL_checkinteger(L, 16);
    int b4 = luaL_checkinteger(L, 17);
    float speed = (float)luaL_checknumber(L, 18);
    
    int mode = st_mode_get();
    if (mode == VIDEO_MODE_XRES) {
        st_xres_palette_auto_bars(paletteIndex, startRow, endRow,
                                   barHeight, numColors,
                                   r1, g1, b1, r2, g2, b2,
                                   r3, g3, b3, r4, g4, b4,
                                   speed);
    } else if (mode == VIDEO_MODE_WRES) {
        st_wres_palette_auto_bars(paletteIndex, startRow, endRow,
                                   barHeight, numColors,
                                   r1, g1, b1, r2, g2, b2,
                                   r3, g3, b3, r4, g4, b4,
                                   speed);
    } else if (mode == VIDEO_MODE_PRES) {
        st_pres_palette_auto_bars(paletteIndex, startRow, endRow,
                                   barHeight, numColors,
                                   r1, g1, b1, r2, g2, b2,
                                   r3, g3, b3, r4, g4, b4,
                                   speed);
    }
    return 0;
}

static int lua_vpalette_auto_stop(lua_State* L) {
    int mode = st_mode_get();
    if (mode == VIDEO_MODE_XRES) {
        st_xres_palette_auto_stop();
    } else if (mode == VIDEO_MODE_WRES) {
        st_wres_palette_auto_stop();
    } else if (mode == VIDEO_MODE_PRES) {
        st_pres_palette_auto_stop();
    }
    return 0;
}

static int lua_vpalette_auto_update(lua_State* L) {
    float deltaTime = (float)luaL_checknumber(L, 1);
    
    int mode = st_mode_get();
    if (mode == VIDEO_MODE_XRES) {
        st_xres_palette_auto_update(deltaTime);
    } else if (mode == VIDEO_MODE_WRES) {
        st_wres_palette_auto_update(deltaTime);
    } else if (mode == VIDEO_MODE_PRES) {
        st_pres_palette_auto_update(deltaTime);
    }
    return 0;
}

// =============================================================================
// URES Mode API Bindings (Ultra Resolution 1280x720 Direct Color)
// =============================================================================

static int lua_st_ures_pset(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int color = luaL_checkinteger(L, 3);
    st_ures_pset(x, y, color);
    return 0;
}

static int lua_st_ures_pget(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int color = st_ures_pget(x, y);
    lua_pushinteger(L, color);
    return 1;
}

static int lua_st_ures_clear(lua_State* L) {
    int color = luaL_checkinteger(L, 1);
    st_ures_clear(color);
    return 0;
}

static int lua_st_ures_fillrect(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int width = luaL_checkinteger(L, 3);
    int height = luaL_checkinteger(L, 4);
    int color = luaL_checkinteger(L, 5);
    st_ures_fillrect(x, y, width, height, color);
    return 0;
}

static int lua_st_ures_hline(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int width = luaL_checkinteger(L, 3);
    int color = luaL_checkinteger(L, 4);
    st_ures_hline(x, y, width, color);
    return 0;
}

static int lua_st_ures_vline(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int height = luaL_checkinteger(L, 3);
    int color = luaL_checkinteger(L, 4);
    st_ures_vline(x, y, height, color);
    return 0;
}

static int lua_st_urgb(lua_State* L) {
    int r = luaL_checkinteger(L, 1);
    int g = luaL_checkinteger(L, 2);
    int b = luaL_checkinteger(L, 3);
    int color = st_urgb(r, g, b);
    lua_pushinteger(L, color);
    return 1;
}

static int lua_st_urgba(lua_State* L) {
    int r = luaL_checkinteger(L, 1);
    int g = luaL_checkinteger(L, 2);
    int b = luaL_checkinteger(L, 3);
    int a = luaL_checkinteger(L, 4);
    int color = st_urgba(r, g, b, a);
    lua_pushinteger(L, color);
    return 1;
}

static int lua_st_xrgb(lua_State* L) {
    int r = luaL_checkinteger(L, 1);
    int g = luaL_checkinteger(L, 2);
    int b = luaL_checkinteger(L, 3);
    int index = st_xrgb(r, g, b);
    lua_pushinteger(L, index);
    return 1;
}

static int lua_st_wrgb(lua_State* L) {
    int r = luaL_checkinteger(L, 1);
    int g = luaL_checkinteger(L, 2);
    int b = luaL_checkinteger(L, 3);
    int index = st_wrgb(r, g, b);
    lua_pushinteger(L, index);
    return 1;
}

static int lua_st_prgb(lua_State* L) {
    int r = luaL_checkinteger(L, 1);
    int g = luaL_checkinteger(L, 2);
    int b = luaL_checkinteger(L, 3);
    int index = st_prgb(r, g, b);
    lua_pushinteger(L, index);
    return 1;
}

// =============================================================================
// Rectangle Rendering API Bindings
// =============================================================================



static int lua_st_rect_count(lua_State* L) {
    lua_pushinteger(L, st_rect_count());
    return 1;
}

static int lua_st_rect_is_empty(lua_State* L) {
    lua_pushboolean(L, st_rect_is_empty());
    return 1;
}

static int lua_st_rect_set_max(lua_State* L) {
    size_t max = luaL_checkinteger(L, 1);
    st_rect_set_max(max);
    return 0;
}

static int lua_st_rect_get_max(lua_State* L) {
    lua_pushinteger(L, st_rect_get_max());
    return 1;
}

// ID-Based Rectangle Management API Bindings

static int lua_st_rect_create(lua_State* L) {
    float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
    float width = luaL_checknumber(L, 3);
    float height = luaL_checknumber(L, 4);
    uint32_t color = luaL_checkinteger(L, 5);

    int id = st_rect_create(x, y, width, height, color);
    lua_pushinteger(L, id);
    return 1;
}

static int lua_st_rect_create_gradient(lua_State* L) {
    float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
    float width = luaL_checknumber(L, 3);
    float height = luaL_checknumber(L, 4);
    uint32_t color1 = luaL_checkinteger(L, 5);
    uint32_t color2 = luaL_checkinteger(L, 6);
    int mode = luaL_checkinteger(L, 7);

    int id = st_rect_create_gradient(x, y, width, height, color1, color2, (STRectangleGradientMode)mode);
    lua_pushinteger(L, id);
    return 1;
}

static int lua_st_rect_create_three_point(lua_State* L) {
    float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
    float width = luaL_checknumber(L, 3);
    float height = luaL_checknumber(L, 4);
    uint32_t color1 = luaL_checkinteger(L, 5);
    uint32_t color2 = luaL_checkinteger(L, 6);
    uint32_t color3 = luaL_checkinteger(L, 7);
    int mode = luaL_checkinteger(L, 8);

    int id = st_rect_create_three_point(x, y, width, height, color1, color2, color3, (STRectangleGradientMode)mode);
    lua_pushinteger(L, id);
    return 1;
}

static int lua_st_rect_create_four_corner(lua_State* L) {
    float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
    float width = luaL_checknumber(L, 3);
    float height = luaL_checknumber(L, 4);
    uint32_t topLeft = luaL_checkinteger(L, 5);
    uint32_t topRight = luaL_checkinteger(L, 6);
    uint32_t bottomRight = luaL_checkinteger(L, 7);
    uint32_t bottomLeft = luaL_checkinteger(L, 8);

    int id = st_rect_create_four_corner(x, y, width, height, topLeft, topRight, bottomRight, bottomLeft);
    lua_pushinteger(L, id);
    return 1;
}

static int lua_st_rect_set_position(lua_State* L) {
    int id = luaL_checkinteger(L, 1);
    float x = luaL_checknumber(L, 2);
    float y = luaL_checknumber(L, 3);

    bool result = st_rect_set_position(id, x, y);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_rect_set_size(lua_State* L) {
    int id = luaL_checkinteger(L, 1);
    float width = luaL_checknumber(L, 2);
    float height = luaL_checknumber(L, 3);

    bool result = st_rect_set_size(id, width, height);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_rect_set_color(lua_State* L) {
    int id = luaL_checkinteger(L, 1);
    uint32_t color = luaL_checkinteger(L, 2);

    bool result = st_rect_set_color(id, color);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_rect_set_colors(lua_State* L) {
    int id = luaL_checkinteger(L, 1);
    uint32_t color1 = luaL_checkinteger(L, 2);
    uint32_t color2 = luaL_checkinteger(L, 3);
    uint32_t color3 = luaL_checkinteger(L, 4);
    uint32_t color4 = luaL_checkinteger(L, 5);

    bool result = st_rect_set_colors(id, color1, color2, color3, color4);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_rect_set_mode(lua_State* L) {
    int id = luaL_checkinteger(L, 1);
    int mode = luaL_checkinteger(L, 2);

    bool result = st_rect_set_mode(id, (STRectangleGradientMode)mode);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_rect_set_visible(lua_State* L) {
    int id = luaL_checkinteger(L, 1);
    bool visible = lua_toboolean(L, 2);

    bool result = st_rect_set_visible(id, visible);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_rect_exists(lua_State* L) {
    int id = luaL_checkinteger(L, 1);

    bool result = st_rect_exists(id);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_rect_is_visible(lua_State* L) {
    int id = luaL_checkinteger(L, 1);

    bool result = st_rect_is_visible(id);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_rect_delete(lua_State* L) {
    int id = luaL_checkinteger(L, 1);

    bool result = st_rect_delete(id);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_rect_delete_all(lua_State* L) {
    st_rect_delete_all();
    return 0;
}

// =============================================================================
// Rectangle Procedural Pattern API Bindings
// =============================================================================

static int lua_st_rect_create_outline(lua_State* L) {
    float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
    float width = luaL_checknumber(L, 3);
    float height = luaL_checknumber(L, 4);
    uint32_t fillColor = luaL_checkinteger(L, 5);
    uint32_t outlineColor = luaL_checkinteger(L, 6);
    float lineWidth = luaL_optnumber(L, 7, 2.0);

    int id = st_rect_create_outline(x, y, width, height, fillColor, outlineColor, lineWidth);
    lua_pushinteger(L, id);
    return 1;
}

static int lua_st_rect_create_dashed_outline(lua_State* L) {
    float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
    float width = luaL_checknumber(L, 3);
    float height = luaL_checknumber(L, 4);
    uint32_t fillColor = luaL_checkinteger(L, 5);
    uint32_t outlineColor = luaL_checkinteger(L, 6);
    float lineWidth = luaL_optnumber(L, 7, 2.0);
    float dashLength = luaL_optnumber(L, 8, 10.0);

    int id = st_rect_create_dashed_outline(x, y, width, height, fillColor, outlineColor, lineWidth, dashLength);
    lua_pushinteger(L, id);
    return 1;
}

static int lua_st_rect_create_horizontal_stripes(lua_State* L) {
    float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
    float width = luaL_checknumber(L, 3);
    float height = luaL_checknumber(L, 4);
    uint32_t color1 = luaL_checkinteger(L, 5);
    uint32_t color2 = luaL_checkinteger(L, 6);
    float stripeHeight = luaL_optnumber(L, 7, 10.0);

    int id = st_rect_create_horizontal_stripes(x, y, width, height, color1, color2, stripeHeight);
    lua_pushinteger(L, id);
    return 1;
}

static int lua_st_rect_create_vertical_stripes(lua_State* L) {
    float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
    float width = luaL_checknumber(L, 3);
    float height = luaL_checknumber(L, 4);
    uint32_t color1 = luaL_checkinteger(L, 5);
    uint32_t color2 = luaL_checkinteger(L, 6);
    float stripeWidth = luaL_optnumber(L, 7, 10.0);

    int id = st_rect_create_vertical_stripes(x, y, width, height, color1, color2, stripeWidth);
    lua_pushinteger(L, id);
    return 1;
}

static int lua_st_rect_create_diagonal_stripes(lua_State* L) {
    float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
    float width = luaL_checknumber(L, 3);
    float height = luaL_checknumber(L, 4);
    uint32_t color1 = luaL_checkinteger(L, 5);
    uint32_t color2 = luaL_checkinteger(L, 6);
    float stripeWidth = luaL_optnumber(L, 7, 10.0);
    float angle = luaL_optnumber(L, 8, 45.0);

    int id = st_rect_create_diagonal_stripes(x, y, width, height, color1, color2, stripeWidth, angle);
    lua_pushinteger(L, id);
    return 1;
}

static int lua_st_rect_create_checkerboard(lua_State* L) {
    float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
    float width = luaL_checknumber(L, 3);
    float height = luaL_checknumber(L, 4);
    uint32_t color1 = luaL_checkinteger(L, 5);
    uint32_t color2 = luaL_checkinteger(L, 6);
    float cellSize = luaL_optnumber(L, 7, 10.0);

    int id = st_rect_create_checkerboard(x, y, width, height, color1, color2, cellSize);
    lua_pushinteger(L, id);
    return 1;
}

static int lua_st_rect_create_dots(lua_State* L) {
    float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
    float width = luaL_checknumber(L, 3);
    float height = luaL_checknumber(L, 4);
    uint32_t dotColor = luaL_checkinteger(L, 5);
    uint32_t backgroundColor = luaL_checkinteger(L, 6);
    float dotRadius = luaL_optnumber(L, 7, 3.0);
    float spacing = luaL_optnumber(L, 8, 10.0);

    int id = st_rect_create_dots(x, y, width, height, dotColor, backgroundColor, dotRadius, spacing);
    lua_pushinteger(L, id);
    return 1;
}

static int lua_st_rect_create_crosshatch(lua_State* L) {
    float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
    float width = luaL_checknumber(L, 3);
    float height = luaL_checknumber(L, 4);
    uint32_t lineColor = luaL_checkinteger(L, 5);
    uint32_t backgroundColor = luaL_checkinteger(L, 6);
    float lineWidth = luaL_optnumber(L, 7, 1.0);
    float spacing = luaL_optnumber(L, 8, 10.0);

    int id = st_rect_create_crosshatch(x, y, width, height, lineColor, backgroundColor, lineWidth, spacing);
    lua_pushinteger(L, id);
    return 1;
}

static int lua_st_rect_create_rounded_corners(lua_State* L) {
    float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
    float width = luaL_checknumber(L, 3);
    float height = luaL_checknumber(L, 4);
    uint32_t color = luaL_checkinteger(L, 5);
    float cornerRadius = luaL_optnumber(L, 6, 10.0);

    int id = st_rect_create_rounded_corners(x, y, width, height, color, cornerRadius);
    lua_pushinteger(L, id);
    return 1;
}

static int lua_st_rect_create_grid(lua_State* L) {
    float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
    float width = luaL_checknumber(L, 3);
    float height = luaL_checknumber(L, 4);
    uint32_t lineColor = luaL_checkinteger(L, 5);
    uint32_t backgroundColor = luaL_checkinteger(L, 6);
    float lineWidth = luaL_optnumber(L, 7, 1.0);
    float cellSize = luaL_optnumber(L, 8, 10.0);

    int id = st_rect_create_grid(x, y, width, height, lineColor, backgroundColor, lineWidth, cellSize);
    lua_pushinteger(L, id);
    return 1;
}

static int lua_st_rect_set_parameters(lua_State* L) {
    int id = luaL_checkinteger(L, 1);
    float param1 = luaL_checknumber(L, 2);
    float param2 = luaL_checknumber(L, 3);
    float param3 = luaL_checknumber(L, 4);

    bool result = st_rect_set_parameters(id, param1, param2, param3);
    lua_pushboolean(L, result);
    return 1;
}

// =============================================================================
// Circle System API Bindings
// =============================================================================

static int lua_st_circle_create(lua_State* L) {
    float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
    float radius = luaL_checknumber(L, 3);
    uint32_t color = luaL_checkinteger(L, 4);

    int id = st_circle_create(x, y, radius, color);
    lua_pushinteger(L, id);
    return 1;
}

static int lua_st_circle_create_radial(lua_State* L) {
    float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
    float radius = luaL_checknumber(L, 3);
    uint32_t centerColor = luaL_checkinteger(L, 4);
    uint32_t edgeColor = luaL_checkinteger(L, 5);

    int id = st_circle_create_radial(x, y, radius, centerColor, edgeColor);
    lua_pushinteger(L, id);
    return 1;
}

static int lua_st_circle_create_radial_3(lua_State* L) {
    float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
    float radius = luaL_checknumber(L, 3);
    uint32_t color1 = luaL_checkinteger(L, 4);
    uint32_t color2 = luaL_checkinteger(L, 5);
    uint32_t color3 = luaL_checkinteger(L, 6);

    int id = st_circle_create_radial_3(x, y, radius, color1, color2, color3);
    lua_pushinteger(L, id);
    return 1;
}

static int lua_st_circle_create_radial_4(lua_State* L) {
    float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
    float radius = luaL_checknumber(L, 3);
    uint32_t color1 = luaL_checkinteger(L, 4);
    uint32_t color2 = luaL_checkinteger(L, 5);
    uint32_t color3 = luaL_checkinteger(L, 6);
    uint32_t color4 = luaL_checkinteger(L, 7);

    int id = st_circle_create_radial_4(x, y, radius, color1, color2, color3, color4);
    lua_pushinteger(L, id);
    return 1;
}

static int lua_st_circle_create_outline(lua_State* L) {
    float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
    float radius = luaL_checknumber(L, 3);
    uint32_t fillColor = luaL_checkinteger(L, 4);
    uint32_t outlineColor = luaL_checkinteger(L, 5);
    float lineWidth = luaL_optnumber(L, 6, 2.0);

    int id = st_circle_create_outline(x, y, radius, fillColor, outlineColor, lineWidth);
    lua_pushinteger(L, id);
    return 1;
}

static int lua_st_circle_create_dashed_outline(lua_State* L) {
    float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
    float radius = luaL_checknumber(L, 3);
    uint32_t fillColor = luaL_checkinteger(L, 4);
    uint32_t outlineColor = luaL_checkinteger(L, 5);
    float lineWidth = luaL_optnumber(L, 6, 2.0);
    float dashLength = luaL_optnumber(L, 7, 10.0);

    int id = st_circle_create_dashed_outline(x, y, radius, fillColor, outlineColor, lineWidth, dashLength);
    lua_pushinteger(L, id);
    return 1;
}

static int lua_st_circle_create_ring(lua_State* L) {
    float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
    float outerRadius = luaL_checknumber(L, 3);
    float innerRadius = luaL_checknumber(L, 4);
    uint32_t color = luaL_checkinteger(L, 5);

    int id = st_circle_create_ring(x, y, outerRadius, innerRadius, color);
    lua_pushinteger(L, id);
    return 1;
}

static int lua_st_circle_create_pie_slice(lua_State* L) {
    float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
    float radius = luaL_checknumber(L, 3);
    float startAngle = luaL_checknumber(L, 4);
    float endAngle = luaL_checknumber(L, 5);
    uint32_t color = luaL_checkinteger(L, 6);

    int id = st_circle_create_pie_slice(x, y, radius, startAngle, endAngle, color);
    lua_pushinteger(L, id);
    return 1;
}

static int lua_st_circle_create_arc(lua_State* L) {
    float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
    float radius = luaL_checknumber(L, 3);
    float startAngle = luaL_checknumber(L, 4);
    float endAngle = luaL_checknumber(L, 5);
    uint32_t color = luaL_checkinteger(L, 6);
    float lineWidth = luaL_optnumber(L, 7, 2.0);

    int id = st_circle_create_arc(x, y, radius, startAngle, endAngle, color, lineWidth);
    lua_pushinteger(L, id);
    return 1;
}

static int lua_st_circle_create_dots_ring(lua_State* L) {
    float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
    float radius = luaL_checknumber(L, 3);
    uint32_t dotColor = luaL_checkinteger(L, 4);
    uint32_t backgroundColor = luaL_checkinteger(L, 5);
    float dotRadius = luaL_optnumber(L, 6, 3.0);
    int numDots = luaL_optinteger(L, 7, 12);

    int id = st_circle_create_dots_ring(x, y, radius, dotColor, backgroundColor, dotRadius, numDots);
    lua_pushinteger(L, id);
    return 1;
}

static int lua_st_circle_create_star_burst(lua_State* L) {
    float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
    float radius = luaL_checknumber(L, 3);
    uint32_t color1 = luaL_checkinteger(L, 4);
    uint32_t color2 = luaL_checkinteger(L, 5);
    int numRays = luaL_optinteger(L, 6, 8);

    int id = st_circle_create_star_burst(x, y, radius, color1, color2, numRays);
    lua_pushinteger(L, id);
    return 1;
}

static int lua_st_circle_set_position(lua_State* L) {
    int id = luaL_checkinteger(L, 1);
    float x = luaL_checknumber(L, 2);
    float y = luaL_checknumber(L, 3);

    bool result = st_circle_set_position(id, x, y);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_circle_set_radius(lua_State* L) {
    int id = luaL_checkinteger(L, 1);
    float radius = luaL_checknumber(L, 2);

    bool result = st_circle_set_radius(id, radius);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_circle_set_color(lua_State* L) {
    int id = luaL_checkinteger(L, 1);
    uint32_t color = luaL_checkinteger(L, 2);

    bool result = st_circle_set_color(id, color);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_circle_set_colors(lua_State* L) {
    int id = luaL_checkinteger(L, 1);
    uint32_t color1 = luaL_checkinteger(L, 2);
    uint32_t color2 = luaL_checkinteger(L, 3);
    uint32_t color3 = luaL_checkinteger(L, 4);
    uint32_t color4 = luaL_checkinteger(L, 5);

    bool result = st_circle_set_colors(id, color1, color2, color3, color4);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_circle_set_parameters(lua_State* L) {
    int id = luaL_checkinteger(L, 1);
    float param1 = luaL_checknumber(L, 2);
    float param2 = luaL_checknumber(L, 3);
    float param3 = luaL_checknumber(L, 4);

    bool result = st_circle_set_parameters(id, param1, param2, param3);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_circle_set_visible(lua_State* L) {
    int id = luaL_checkinteger(L, 1);
    bool visible = lua_toboolean(L, 2);

    bool result = st_circle_set_visible(id, visible);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_circle_exists(lua_State* L) {
    int id = luaL_checkinteger(L, 1);

    bool result = st_circle_exists(id);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_circle_is_visible(lua_State* L) {
    int id = luaL_checkinteger(L, 1);

    bool result = st_circle_is_visible(id);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_circle_delete(lua_State* L) {
    int id = luaL_checkinteger(L, 1);

    bool result = st_circle_delete(id);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_circle_delete_all(lua_State* L) {
    st_circle_delete_all();
    return 0;
}

static int lua_st_circle_count(lua_State* L) {
    lua_pushinteger(L, st_circle_count());
    return 1;
}

static int lua_st_circle_is_empty(lua_State* L) {
    lua_pushboolean(L, st_circle_is_empty());
    return 1;
}

static int lua_st_circle_set_max(lua_State* L) {
    size_t max = luaL_checkinteger(L, 1);
    st_circle_set_max(max);
    return 0;
}

static int lua_st_circle_get_max(lua_State* L) {
    lua_pushinteger(L, st_circle_get_max());
    return 1;
}

// =============================================================================
// Line Management API Bindings
// =============================================================================

static int lua_st_line_create(lua_State* L) {
    float x1 = luaL_checknumber(L, 1);
    float y1 = luaL_checknumber(L, 2);
    float x2 = luaL_checknumber(L, 3);
    float y2 = luaL_checknumber(L, 4);
    uint32_t color = luaL_checkinteger(L, 5);
    float thickness = luaL_optnumber(L, 6, 2.0);

    int id = st_line_create(x1, y1, x2, y2, color, thickness);
    lua_pushinteger(L, id);
    return 1;
}

static int lua_st_line_create_gradient(lua_State* L) {
    float x1 = luaL_checknumber(L, 1);
    float y1 = luaL_checknumber(L, 2);
    float x2 = luaL_checknumber(L, 3);
    float y2 = luaL_checknumber(L, 4);
    uint32_t color1 = luaL_checkinteger(L, 5);
    uint32_t color2 = luaL_checkinteger(L, 6);
    float thickness = luaL_optnumber(L, 7, 2.0);

    int id = st_line_create_gradient(x1, y1, x2, y2, color1, color2, thickness);
    lua_pushinteger(L, id);
    return 1;
}

static int lua_st_line_create_dashed(lua_State* L) {
    float x1 = luaL_checknumber(L, 1);
    float y1 = luaL_checknumber(L, 2);
    float x2 = luaL_checknumber(L, 3);
    float y2 = luaL_checknumber(L, 4);
    uint32_t color = luaL_checkinteger(L, 5);
    float thickness = luaL_optnumber(L, 6, 2.0);
    float dashLength = luaL_optnumber(L, 7, 10.0);
    float gapLength = luaL_optnumber(L, 8, 5.0);

    int id = st_line_create_dashed(x1, y1, x2, y2, color, thickness, dashLength, gapLength);
    lua_pushinteger(L, id);
    return 1;
}

static int lua_st_line_create_dotted(lua_State* L) {
    float x1 = luaL_checknumber(L, 1);
    float y1 = luaL_checknumber(L, 2);
    float x2 = luaL_checknumber(L, 3);
    float y2 = luaL_checknumber(L, 4);
    uint32_t color = luaL_checkinteger(L, 5);
    float thickness = luaL_optnumber(L, 6, 2.0);
    float dotSpacing = luaL_optnumber(L, 7, 10.0);

    int id = st_line_create_dotted(x1, y1, x2, y2, color, thickness, dotSpacing);
    lua_pushinteger(L, id);
    return 1;
}

static int lua_st_line_set_endpoints(lua_State* L) {
    int id = luaL_checkinteger(L, 1);
    float x1 = luaL_checknumber(L, 2);
    float y1 = luaL_checknumber(L, 3);
    float x2 = luaL_checknumber(L, 4);
    float y2 = luaL_checknumber(L, 5);

    bool result = st_line_set_endpoints(id, x1, y1, x2, y2);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_line_set_thickness(lua_State* L) {
    int id = luaL_checkinteger(L, 1);
    float thickness = luaL_checknumber(L, 2);

    bool result = st_line_set_thickness(id, thickness);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_line_set_color(lua_State* L) {
    int id = luaL_checkinteger(L, 1);
    uint32_t color = luaL_checkinteger(L, 2);

    bool result = st_line_set_color(id, color);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_line_set_colors(lua_State* L) {
    int id = luaL_checkinteger(L, 1);
    uint32_t color1 = luaL_checkinteger(L, 2);
    uint32_t color2 = luaL_checkinteger(L, 3);

    bool result = st_line_set_colors(id, color1, color2);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_line_set_dash_pattern(lua_State* L) {
    int id = luaL_checkinteger(L, 1);
    float dashLength = luaL_checknumber(L, 2);
    float gapLength = luaL_checknumber(L, 3);

    bool result = st_line_set_dash_pattern(id, dashLength, gapLength);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_line_set_visible(lua_State* L) {
    int id = luaL_checkinteger(L, 1);
    bool visible = lua_toboolean(L, 2);

    bool result = st_line_set_visible(id, visible);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_line_exists(lua_State* L) {
    int id = luaL_checkinteger(L, 1);
    lua_pushboolean(L, st_line_exists(id));
    return 1;
}

static int lua_st_line_is_visible(lua_State* L) {
    int id = luaL_checkinteger(L, 1);
    lua_pushboolean(L, st_line_is_visible(id));
    return 1;
}

static int lua_st_line_delete(lua_State* L) {
    int id = luaL_checkinteger(L, 1);

    bool result = st_line_delete(id);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_line_delete_all(lua_State* L) {
    st_line_delete_all();
    return 0;
}

static int lua_st_line_count(lua_State* L) {
    lua_pushinteger(L, st_line_count());
    return 1;
}

static int lua_st_line_is_empty(lua_State* L) {
    lua_pushboolean(L, st_line_is_empty());
    return 1;
}

static int lua_st_line_set_max(lua_State* L) {
    size_t max = luaL_checkinteger(L, 1);
    st_line_set_max(max);
    return 0;
}

static int lua_st_line_get_max(lua_State* L) {
    lua_pushinteger(L, st_line_get_max());
    return 1;
}

// =============================================================================
// Particle System API Bindings
// =============================================================================

static int lua_st_sprite_explode(lua_State* L) {
    float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
    int particleCount = luaL_checkinteger(L, 3);
    uint32_t color = luaL_checkinteger(L, 4);

    bool result = st_sprite_explode(x, y, particleCount, color);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_sprite_explode_advanced(lua_State* L) {
    float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
    int particleCount = luaL_checkinteger(L, 3);
    uint32_t color = luaL_checkinteger(L, 4);
    float force = luaL_checknumber(L, 5);
    float gravity = luaL_checknumber(L, 6);
    float fadeTime = luaL_checknumber(L, 7);

    bool result = st_sprite_explode_advanced(x, y, particleCount, color, force, gravity, fadeTime);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_sprite_explode_directional(lua_State* L) {
    float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
    int particleCount = luaL_checkinteger(L, 3);
    uint32_t color = luaL_checkinteger(L, 4);
    float forceX = luaL_checknumber(L, 5);
    float forceY = luaL_checknumber(L, 6);

    bool result = st_sprite_explode_directional(x, y, particleCount, color, forceX, forceY);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_particle_clear(lua_State* L) {
    st_particle_clear();
    return 0;
}

static int lua_st_particle_pause(lua_State* L) {
    st_particle_pause();
    return 0;
}

static int lua_st_particle_resume(lua_State* L) {
    st_particle_resume();
    return 0;
}

static int lua_st_particle_set_time_scale(lua_State* L) {
    float scale = luaL_checknumber(L, 1);
    st_particle_set_time_scale(scale);
    return 0;
}

static int lua_st_particle_set_world_bounds(lua_State* L) {
    float width = luaL_checknumber(L, 1);
    float height = luaL_checknumber(L, 2);
    st_particle_set_world_bounds(width, height);
    return 0;
}

static int lua_st_particle_set_enabled(lua_State* L) {
    bool enabled = lua_toboolean(L, 1);
    st_particle_set_enabled(enabled);
    return 0;
}

static int lua_st_particle_get_active_count(lua_State* L) {
    uint32_t count = st_particle_get_active_count();
    lua_pushinteger(L, count);
    return 1;
}

static int lua_st_particle_get_total_created(lua_State* L) {
    uint64_t total = st_particle_get_total_created();
    lua_pushinteger(L, total);
    return 1;
}

static int lua_st_particle_dump_stats(lua_State* L) {
    st_particle_dump_stats();
    return 0;
}

// =============================================================================
// Sprite Management API
// =============================================================================

static int lua_st_sprite_load(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    int sprite_id = st_sprite_load(path);
    lua_pushinteger(L, sprite_id);
    return 1;
}

static int lua_st_sprite_load_builtin(lua_State* L) {
    const char* name = luaL_checkstring(L, 1);
    int sprite_id = st_sprite_load_builtin(name);
    lua_pushinteger(L, sprite_id);
    return 1;
}

static int lua_st_sprite_begin_draw(lua_State* L) {
    int width = luaL_checkinteger(L, 1);
    int height = luaL_checkinteger(L, 2);
    int sprite_id = st_sprite_begin_draw(width, height);
    lua_pushinteger(L, sprite_id);
    return 1;
}

static int lua_st_sprite_end_draw(lua_State* L) {
    st_sprite_end_draw();
    return 0;
}

static int lua_st_draw_to_file_begin(lua_State* L) {
    const char* filename = luaL_checkstring(L, 1);
    int width = luaL_checkinteger(L, 2);
    int height = luaL_checkinteger(L, 3);
    bool result = st_draw_to_file_begin(filename, width, height);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_draw_to_file_end(lua_State* L) {
    bool result = st_draw_to_file_end();
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_tileset_begin_draw(lua_State* L) {
    int tileWidth = luaL_checkinteger(L, 1);
    int tileHeight = luaL_checkinteger(L, 2);
    int columns = luaL_checkinteger(L, 3);
    int rows = luaL_checkinteger(L, 4);
    int tilesetId = st_tileset_begin_draw(tileWidth, tileHeight, columns, rows);
    lua_pushinteger(L, tilesetId);
    return 1;
}

static int lua_st_tileset_draw_tile(lua_State* L) {
    int tileIndex = luaL_checkinteger(L, 1);
    bool result = st_tileset_draw_tile(tileIndex);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_tileset_end_draw(lua_State* L) {
    bool result = st_tileset_end_draw();
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_sprite_show(lua_State* L) {
    int sprite_id = luaL_checkinteger(L, 1);
    int x = luaL_checkinteger(L, 2);
    int y = luaL_checkinteger(L, 3);
    st_sprite_show(sprite_id, x, y);
    return 0;
}

static int lua_st_sprite_hide(lua_State* L) {
    int sprite_id = luaL_checkinteger(L, 1);
    st_sprite_hide(sprite_id);
    return 0;
}

static int lua_st_sprite_transform(lua_State* L) {
    int sprite_id = luaL_checkinteger(L, 1);
    int x = luaL_checkinteger(L, 2);
    int y = luaL_checkinteger(L, 3);
    float rotation = luaL_checknumber(L, 4);
    float scale_x = luaL_checknumber(L, 5);
    float scale_y = luaL_checknumber(L, 6);
    st_sprite_transform(sprite_id, x, y, rotation, scale_x, scale_y);
    return 0;
}

static int lua_st_sprite_tint(lua_State* L) {
    int sprite_id = luaL_checkinteger(L, 1);
    uint32_t color = luaL_checkinteger(L, 2);
    st_sprite_tint(sprite_id, color);
    return 0;
}

static int lua_st_sprite_unload(lua_State* L) {
    int sprite_id = luaL_checkinteger(L, 1);
    st_sprite_unload(sprite_id);
    return 0;
}

static int lua_st_sprite_unload_all(lua_State* L) {
    st_sprite_unload_all();
    return 0;
}

// =============================================================================
// Indexed Sprite Functions
// =============================================================================

static int lua_st_sprite_load_sprtz(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    int sprite_id = st_sprite_load_sprtz(path);
    lua_pushinteger(L, sprite_id);
    return 1;
}

static int lua_st_sprite_is_indexed(lua_State* L) {
    int sprite_id = luaL_checkinteger(L, 1);
    bool is_indexed = st_sprite_is_indexed(sprite_id);
    lua_pushboolean(L, is_indexed);
    return 1;
}

static int lua_st_sprite_set_standard_palette(lua_State* L) {
    int sprite_id = luaL_checkinteger(L, 1);
    int palette_id = luaL_checkinteger(L, 2);
    bool result = st_sprite_set_standard_palette(sprite_id, (uint8_t)palette_id);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_sprite_set_palette_color(lua_State* L) {
    int sprite_id = luaL_checkinteger(L, 1);
    int color_index = luaL_checkinteger(L, 2);
    int r = luaL_checkinteger(L, 3);
    int g = luaL_checkinteger(L, 4);
    int b = luaL_checkinteger(L, 5);
    int a = luaL_optinteger(L, 6, 255);
    bool result = st_sprite_set_palette_color(sprite_id, color_index, 
                                              (uint8_t)r, (uint8_t)g, (uint8_t)b, (uint8_t)a);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_sprite_rotate_palette(lua_State* L) {
    int sprite_id = luaL_checkinteger(L, 1);
    int start_index = luaL_checkinteger(L, 2);
    int end_index = luaL_checkinteger(L, 3);
    int amount = luaL_checkinteger(L, 4);
    bool result = st_sprite_rotate_palette(sprite_id, start_index, end_index, amount);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_sprite_adjust_brightness(lua_State* L) {
    int sprite_id = luaL_checkinteger(L, 1);
    float brightness = (float)luaL_checknumber(L, 2);
    bool result = st_sprite_adjust_brightness(sprite_id, brightness);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_sprite_copy_palette(lua_State* L) {
    int src_sprite = luaL_checkinteger(L, 1);
    int dst_sprite = luaL_checkinteger(L, 2);
    bool result = st_sprite_copy_palette(src_sprite, dst_sprite);
    lua_pushboolean(L, result);
    return 1;
}

// =============================================================================
// Sprite-based Particle Explosion API (v1 compatible)
// =============================================================================

static int lua_sprite_explode(lua_State* L) {
    int sprite_id = luaL_checkinteger(L, 1);
    int particle_count = 32; // Default

    if (lua_gettop(L) >= 2) {
        particle_count = luaL_checkinteger(L, 2);
    }

    // Optional mode parameter (default to SPRITE_FRAGMENT for v1 compatibility)
    ParticleMode mode = ParticleMode::SPRITE_FRAGMENT;
    if (lua_gettop(L) >= 3) {
        int mode_int = luaL_checkinteger(L, 3);
        mode = (mode_int == 1) ? ParticleMode::SPRITE_FRAGMENT : ParticleMode::POINT_SPRITE;
    }

    // Validate parameters
    if (sprite_id < 1 || sprite_id > 1024) {
        return luaL_error(L, "sprite_explode: sprite_id must be between 1 and 1024");
    }

    if (particle_count < 1 || particle_count > 500) {
        return luaL_error(L, "sprite_explode: particle_count must be between 1 and 500");
    }

    bool result = sprite_explode((uint16_t)sprite_id, (uint16_t)particle_count);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_sprite_explode_advanced(lua_State* L) {
    int sprite_id = luaL_checkinteger(L, 1);
    int particle_count = luaL_checkinteger(L, 2);

    // Get optional parameters with defaults
    float explosion_force = 200.0f;
    float gravity = 100.0f;
    float fade_time = 2.0f;

    if (lua_gettop(L) >= 3) explosion_force = luaL_checknumber(L, 3);
    if (lua_gettop(L) >= 4) gravity = luaL_checknumber(L, 4);
    if (lua_gettop(L) >= 5) fade_time = luaL_checknumber(L, 5);

    // Optional mode parameter (default to SPRITE_FRAGMENT for v1 compatibility)
    ParticleMode mode = ParticleMode::SPRITE_FRAGMENT;
    if (lua_gettop(L) >= 6) {
        int mode_int = luaL_checkinteger(L, 6);
        mode = (mode_int == 1) ? ParticleMode::SPRITE_FRAGMENT : ParticleMode::POINT_SPRITE;
    }

    // Validate parameters
    if (sprite_id < 1 || sprite_id > 1024) {
        return luaL_error(L, "sprite_explode_advanced: sprite_id must be between 1 and 1024");
    }

    if (particle_count < 1 || particle_count > 500) {
        return luaL_error(L, "sprite_explode_advanced: particle_count must be between 1 and 500");
    }

    bool result = sprite_explode_advanced((uint16_t)sprite_id, (uint16_t)particle_count,
                                         explosion_force, gravity, fade_time);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_sprite_explode_size(lua_State* L) {
    int sprite_id = luaL_checkinteger(L, 1);
    int particle_count = luaL_checkinteger(L, 2);
    float size_multiplier = luaL_checknumber(L, 3);

    printf("[lua_sprite_explode_size] Called with sprite_id=%d, count=%d, size=%.1f\n", 
           sprite_id, particle_count, size_multiplier);

    // Validate parameters
    if (sprite_id < 1 || sprite_id > 1024) {
        return luaL_error(L, "sprite_explode_size: sprite_id must be between 1 and 1024");
    }

    if (particle_count < 1 || particle_count > 500) {
        return luaL_error(L, "sprite_explode_size: particle_count must be between 1 and 500");
    }

    if (size_multiplier < 1.0f || size_multiplier > 100.0f) {
        return luaL_error(L, "sprite_explode_size: size_multiplier must be between 1.0 and 100.0");
    }

    printf("[lua_sprite_explode_size] Calling C function sprite_explode_size...\n");
    bool result = sprite_explode_size((uint16_t)sprite_id, (uint16_t)particle_count, size_multiplier);
    printf("[lua_sprite_explode_size] C function returned: %d\n", result);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_sprite_explode_directional(lua_State* L) {
    int sprite_id = luaL_checkinteger(L, 1);
    int particle_count = luaL_checkinteger(L, 2);
    float force_x = luaL_checknumber(L, 3);
    float force_y = luaL_checknumber(L, 4);

    // Validate parameters
    if (sprite_id < 1 || sprite_id > 1024) {
        return luaL_error(L, "sprite_explode_directional: sprite_id must be between 1 and 1024");
    }

    if (particle_count < 1 || particle_count > 500) {
        return luaL_error(L, "sprite_explode_directional: particle_count must be between 1 and 500");
    }

    bool result = sprite_explode_directional((uint16_t)sprite_id, (uint16_t)particle_count,
                                            force_x, force_y);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_sprite_explode_mode(lua_State* L) {
    int sprite_id = luaL_checkinteger(L, 1);
    int explosion_mode = luaL_checkinteger(L, 2);

    // Validate sprite ID
    if (sprite_id < 1 || sprite_id > 1024) {
        return luaL_error(L, "sprite_explode_mode: sprite_id must be between 1 and 1024");
    }

    // Validate explosion mode
    if (explosion_mode < 1 || explosion_mode > 6) {
        return luaL_error(L, "sprite_explode_mode: explosion_mode must be between 1 and 6");
    }

    bool success = false;

    // Apply the appropriate explosion mode
    switch (explosion_mode) {
        case 1: // BASIC_EXPLOSION
            success = sprite_explode_advanced((uint16_t)sprite_id, 48, 200.0f, 100.0f, 2.0f);
            break;
        case 2: // MASSIVE_BLAST
            success = sprite_explode_advanced((uint16_t)sprite_id, 128, 350.0f, 80.0f, 3.0f);
            break;
        case 3: // GENTLE_DISPERSAL
            success = sprite_explode_advanced((uint16_t)sprite_id, 64, 120.0f, 40.0f, 4.0f);
            break;
        case 4: // RIGHTWARD_BLAST
            success = sprite_explode_directional((uint16_t)sprite_id, 80, 180.0f, -30.0f);
            break;
        case 5: // UPWARD_ERUPTION
            success = sprite_explode_directional((uint16_t)sprite_id, 96, 0.0f, -250.0f);
            break;
        case 6: // RAPID_BURST
            success = sprite_explode_advanced((uint16_t)sprite_id, 32, 400.0f, 200.0f, 1.0f);
            break;
        default:
            return luaL_error(L, "sprite_explode_mode: invalid explosion_mode");
    }

    lua_pushboolean(L, success);
    return 1;
}

// =============================================================================
// Asset Management API Bindings
// =============================================================================

// Initialization
static int lua_st_asset_init(lua_State* L) {
    const char* db_path = luaL_checkstring(L, 1);
    size_t max_cache_size = luaL_optinteger(L, 2, 0);

    bool result = st_asset_init(db_path, max_cache_size);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_asset_shutdown(lua_State* L) {
    (void)L;
    st_asset_shutdown();
    return 0;
}

static int lua_st_asset_is_initialized(lua_State* L) {
    (void)L;
    bool result = st_asset_is_initialized();
    lua_pushboolean(L, result);
    return 1;
}

// Loading/Unloading
static int lua_st_asset_load(lua_State* L) {
    const char* name = luaL_checkstring(L, 1);

    STAssetID asset = st_asset_load(name);
    lua_pushinteger(L, asset);
    return 1;
}

static int lua_st_asset_load_file(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    int type = luaL_checkinteger(L, 2);

    STAssetID asset = st_asset_load_file(path, (STAssetType)type);
    lua_pushinteger(L, asset);
    return 1;
}

static int lua_st_asset_unload(lua_State* L) {
    STAssetID asset = luaL_checkinteger(L, 1);
    st_asset_unload(asset);
    return 0;
}

static int lua_st_asset_is_loaded(lua_State* L) {
    const char* name = luaL_checkstring(L, 1);

    bool result = st_asset_is_loaded(name);
    lua_pushboolean(L, result);
    return 1;
}

// Import/Export
static int lua_st_asset_import(lua_State* L) {
    const char* file_path = luaL_checkstring(L, 1);
    const char* asset_name = luaL_checkstring(L, 2);
    int type = luaL_optinteger(L, 3, -1);

    bool result = st_asset_import(file_path, asset_name, type);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_asset_import_directory(lua_State* L) {
    const char* directory = luaL_checkstring(L, 1);
    bool recursive = lua_toboolean(L, 2);

    int count = st_asset_import_directory(directory, recursive);
    lua_pushinteger(L, count);
    return 1;
}

static int lua_st_asset_export(lua_State* L) {
    const char* asset_name = luaL_checkstring(L, 1);
    const char* file_path = luaL_checkstring(L, 2);

    bool result = st_asset_export(asset_name, file_path);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_asset_delete(lua_State* L) {
    const char* asset_name = luaL_checkstring(L, 1);

    bool result = st_asset_delete(asset_name);
    lua_pushboolean(L, result);
    return 1;
}

// Data Access
static int lua_st_asset_get_data(lua_State* L) {
    STAssetID asset = luaL_checkinteger(L, 1);

    const void* data = st_asset_get_data(asset);
    size_t size = st_asset_get_size(asset);

    if (data && size > 0) {
        lua_pushlstring(L, (const char*)data, size);
        return 1;
    }

    lua_pushnil(L);
    return 1;
}

static int lua_st_asset_get_size(lua_State* L) {
    STAssetID asset = luaL_checkinteger(L, 1);

    size_t size = st_asset_get_size(asset);
    lua_pushinteger(L, size);
    return 1;
}

static int lua_st_asset_get_type(lua_State* L) {
    STAssetID asset = luaL_checkinteger(L, 1);

    int type = st_asset_get_type(asset);
    lua_pushinteger(L, type);
    return 1;
}

static int lua_st_asset_get_name(lua_State* L) {
    STAssetID asset = luaL_checkinteger(L, 1);

    const char* name = st_asset_get_name(asset);
    if (name) {
        lua_pushstring(L, name);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

// Queries
static int lua_st_asset_exists(lua_State* L) {
    const char* name = luaL_checkstring(L, 1);

    bool result = st_asset_exists(name);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_asset_list(lua_State* L) {
    int type = luaL_optinteger(L, 1, -1);

    // Get count first
    int count = st_asset_list(type, nullptr, 0);

    if (count <= 0) {
        lua_newtable(L);
        return 1;
    }

    // Allocate array for names
    const char** names = new const char*[count];
    st_asset_list(type, names, count);

    // Create Lua table
    lua_createtable(L, count, 0);
    for (int i = 0; i < count; i++) {
        lua_pushstring(L, names[i]);
        lua_rawseti(L, -2, i + 1);  // Lua arrays are 1-indexed
    }

    delete[] names;
    return 1;
}

static int lua_st_asset_search(lua_State* L) {
    const char* pattern = luaL_checkstring(L, 1);

    // Get count first
    int count = st_asset_search(pattern, nullptr, 0);

    if (count <= 0) {
        lua_newtable(L);
        return 1;
    }

    // Allocate array for names
    const char** names = new const char*[count];
    st_asset_search(pattern, names, count);

    // Create Lua table
    lua_createtable(L, count, 0);
    for (int i = 0; i < count; i++) {
        lua_pushstring(L, names[i]);
        lua_rawseti(L, -2, i + 1);
    }

    delete[] names;
    return 1;
}

static int lua_st_asset_get_count(lua_State* L) {
    int type = luaL_optinteger(L, 1, -1);

    int count = st_asset_get_count(type);
    lua_pushinteger(L, count);
    return 1;
}

// Cache Management
static int lua_st_asset_clear_cache(lua_State* L) {
    (void)L;
    st_asset_clear_cache();
    return 0;
}

static int lua_st_asset_get_cache_size(lua_State* L) {
    (void)L;
    size_t size = st_asset_get_cache_size();
    lua_pushinteger(L, size);
    return 1;
}

static int lua_st_asset_get_cached_count(lua_State* L) {
    (void)L;
    int count = st_asset_get_cached_count();
    lua_pushinteger(L, count);
    return 1;
}

static int lua_st_asset_set_max_cache_size(lua_State* L) {
    size_t max_size = luaL_checkinteger(L, 1);
    st_asset_set_max_cache_size(max_size);
    return 0;
}

// Statistics
static int lua_st_asset_get_hit_rate(lua_State* L) {
    (void)L;
    double rate = st_asset_get_hit_rate();
    lua_pushnumber(L, rate);
    return 1;
}

static int lua_st_asset_get_database_size(lua_State* L) {
    (void)L;
    size_t size = st_asset_get_database_size();
    lua_pushinteger(L, size);
    return 1;
}

// Error Handling
static int lua_st_asset_get_error(lua_State* L) {
    (void)L;
    const char* error = st_asset_get_error();
    if (error) {
        lua_pushstring(L, error);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static int lua_st_asset_clear_error(lua_State* L) {
    (void)L;
    st_asset_clear_error();
    return 0;
}

// =============================================================================
// Tilemap API
// =============================================================================

static int lua_st_tilemap_init(lua_State* L) {
    float width = (float)luaL_checknumber(L, 1);
    float height = (float)luaL_checknumber(L, 2);
    bool result = st_tilemap_init(width, height);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_st_tilemap_shutdown(lua_State* L) {
    (void)L;
    st_tilemap_shutdown();
    return 0;
}

static int lua_st_tilemap_create(lua_State* L) {
    int32_t width = (int32_t)luaL_checkinteger(L, 1);
    int32_t height = (int32_t)luaL_checkinteger(L, 2);
    int32_t tileWidth = (int32_t)luaL_checkinteger(L, 3);
    int32_t tileHeight = (int32_t)luaL_checkinteger(L, 4);
    STTilemapID id = st_tilemap_create(width, height, tileWidth, tileHeight);
    lua_pushinteger(L, id);
    return 1;
}

static int lua_st_tilemap_destroy(lua_State* L) {
    STTilemapID id = (STTilemapID)luaL_checkinteger(L, 1);
    st_tilemap_destroy(id);
    return 0;
}

static int lua_st_tilemap_get_size(lua_State* L) {
    STTilemapID id = (STTilemapID)luaL_checkinteger(L, 1);
    int32_t width, height;
    st_tilemap_get_size(id, &width, &height);
    lua_pushinteger(L, width);
    lua_pushinteger(L, height);
    return 2;
}

static int lua_st_tilemap_create_layer(lua_State* L) {
    const char* name = lua_isstring(L, 1) ? lua_tostring(L, 1) : nullptr;
    STLayerID id = st_tilemap_create_layer(name);
    lua_pushinteger(L, id);
    return 1;
}

static int lua_st_tilemap_destroy_layer(lua_State* L) {
    STLayerID id = (STLayerID)luaL_checkinteger(L, 1);
    st_tilemap_destroy_layer(id);
    return 0;
}

static int lua_st_tilemap_layer_set_tilemap(lua_State* L) {
    STLayerID layer = (STLayerID)luaL_checkinteger(L, 1);
    STTilemapID tilemap = (STTilemapID)luaL_checkinteger(L, 2);
    st_tilemap_layer_set_tilemap(layer, tilemap);
    return 0;
}

static int lua_st_tilemap_layer_set_parallax(lua_State* L) {
    STLayerID layer = (STLayerID)luaL_checkinteger(L, 1);
    float parallaxX = (float)luaL_checknumber(L, 2);
    float parallaxY = (float)luaL_checknumber(L, 3);
    st_tilemap_layer_set_parallax(layer, parallaxX, parallaxY);
    return 0;
}

static int lua_st_tilemap_layer_set_opacity(lua_State* L) {
    STLayerID layer = (STLayerID)luaL_checkinteger(L, 1);
    float opacity = (float)luaL_checknumber(L, 2);
    st_tilemap_layer_set_opacity(layer, opacity);
    return 0;
}

static int lua_st_tilemap_layer_set_visible(lua_State* L) {
    STLayerID layer = (STLayerID)luaL_checkinteger(L, 1);
    bool visible = lua_toboolean(L, 2);
    st_tilemap_layer_set_visible(layer, visible);
    return 0;
}

static int lua_st_tilemap_layer_set_z_order(lua_State* L) {
    STLayerID layer = (STLayerID)luaL_checkinteger(L, 1);
    int32_t zOrder = (int32_t)luaL_checkinteger(L, 2);
    st_tilemap_layer_set_z_order(layer, zOrder);
    return 0;
}

static int lua_st_tilemap_layer_set_auto_scroll(lua_State* L) {
    STLayerID layer = (STLayerID)luaL_checkinteger(L, 1);
    float scrollX = (float)luaL_checknumber(L, 2);
    float scrollY = (float)luaL_checknumber(L, 3);
    st_tilemap_layer_set_auto_scroll(layer, scrollX, scrollY);
    return 0;
}

static int lua_st_tilemap_set_tile(lua_State* L) {
    STLayerID layer = (STLayerID)luaL_checkinteger(L, 1);
    int32_t x = (int32_t)luaL_checkinteger(L, 2);
    int32_t y = (int32_t)luaL_checkinteger(L, 3);
    uint16_t tileID = (uint16_t)luaL_checkinteger(L, 4);
    st_tilemap_set_tile(layer, x, y, tileID);
    return 0;
}

static int lua_st_tilemap_get_tile(lua_State* L) {
    STLayerID layer = (STLayerID)luaL_checkinteger(L, 1);
    int32_t x = (int32_t)luaL_checkinteger(L, 2);
    int32_t y = (int32_t)luaL_checkinteger(L, 3);
    uint16_t tileID = st_tilemap_get_tile(layer, x, y);
    lua_pushinteger(L, tileID);
    return 1;
}

static int lua_st_tilemap_fill_rect(lua_State* L) {
    STLayerID layer = (STLayerID)luaL_checkinteger(L, 1);
    int32_t x = (int32_t)luaL_checkinteger(L, 2);
    int32_t y = (int32_t)luaL_checkinteger(L, 3);
    int32_t width = (int32_t)luaL_checkinteger(L, 4);
    int32_t height = (int32_t)luaL_checkinteger(L, 5);
    uint16_t tileID = (uint16_t)luaL_checkinteger(L, 6);
    st_tilemap_fill_rect(layer, x, y, width, height, tileID);
    return 0;
}

static int lua_st_tilemap_clear(lua_State* L) {
    STLayerID layer = (STLayerID)luaL_checkinteger(L, 1);
    st_tilemap_clear(layer);
    return 0;
}

static int lua_st_tilemap_set_camera(lua_State* L) {
    float x = (float)luaL_checknumber(L, 1);
    float y = (float)luaL_checknumber(L, 2);
    st_tilemap_set_camera(x, y);
    return 0;
}

static int lua_st_tilemap_move_camera(lua_State* L) {
    float dx = (float)luaL_checknumber(L, 1);
    float dy = (float)luaL_checknumber(L, 2);
    st_tilemap_move_camera(dx, dy);
    return 0;
}

static int lua_st_tilemap_get_camera(lua_State* L) {
    float x, y;
    st_tilemap_get_camera(&x, &y);
    lua_pushnumber(L, x);
    lua_pushnumber(L, y);
    return 2;
}

static int lua_st_tilemap_set_zoom(lua_State* L) {
    float zoom = (float)luaL_checknumber(L, 1);
    st_tilemap_set_zoom(zoom);
    return 0;
}

static int lua_st_tilemap_camera_follow(lua_State* L) {
    float targetX = (float)luaL_checknumber(L, 1);
    float targetY = (float)luaL_checknumber(L, 2);
    float smoothness = (float)luaL_checknumber(L, 3);
    st_tilemap_camera_follow(targetX, targetY, smoothness);
    return 0;
}

static int lua_st_tilemap_set_camera_bounds(lua_State* L) {
    float x = (float)luaL_checknumber(L, 1);
    float y = (float)luaL_checknumber(L, 2);
    float width = (float)luaL_checknumber(L, 3);
    float height = (float)luaL_checknumber(L, 4);
    st_tilemap_set_camera_bounds(x, y, width, height);
    return 0;
}

static int lua_st_tilemap_camera_shake(lua_State* L) {
    float magnitude = (float)luaL_checknumber(L, 1);
    float duration = (float)luaL_checknumber(L, 2);
    st_tilemap_camera_shake(magnitude, duration);
    return 0;
}

static int lua_st_tilemap_update(lua_State* L) {
    float dt = (float)luaL_checknumber(L, 1);
    st_tilemap_update(dt);
    return 0;
}

static int lua_st_tilemap_world_to_tile(lua_State* L) {
    STLayerID layer = (STLayerID)luaL_checkinteger(L, 1);
    float worldX = (float)luaL_checknumber(L, 2);
    float worldY = (float)luaL_checknumber(L, 3);
    int32_t tileX, tileY;
    st_tilemap_world_to_tile(layer, worldX, worldY, &tileX, &tileY);
    lua_pushinteger(L, tileX);
    lua_pushinteger(L, tileY);
    return 2;
}

static int lua_st_tilemap_tile_to_world(lua_State* L) {
    STLayerID layer = (STLayerID)luaL_checkinteger(L, 1);
    int32_t tileX = (int32_t)luaL_checkinteger(L, 2);
    int32_t tileY = (int32_t)luaL_checkinteger(L, 3);
    float worldX, worldY;
    st_tilemap_tile_to_world(layer, tileX, tileY, &worldX, &worldY);
    lua_pushnumber(L, worldX);
    lua_pushnumber(L, worldY);
    return 2;
}

// Error handling API
static int lua_st_get_error(lua_State* L) {
    const char* error = st_get_last_error();
    if (error && error[0] != '\0') {
        lua_pushstring(L, error);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static int lua_st_clear_error(lua_State* L) {
    (void)L;
    st_clear_error();
    return 0;
}

// Tileset API
static int lua_st_tileset_load(lua_State* L) {
    const char* imagePath = luaL_checkstring(L, 1);
    int32_t tileWidth = (int32_t)luaL_checkinteger(L, 2);
    int32_t tileHeight = (int32_t)luaL_checkinteger(L, 3);
    int32_t margin = (int32_t)luaL_optinteger(L, 4, 0);
    int32_t spacing = (int32_t)luaL_optinteger(L, 5, 0);
    STTilesetID id = st_tileset_load(imagePath, tileWidth, tileHeight, margin, spacing);
    lua_pushinteger(L, id);
    return 1;
}

static int lua_st_tileset_load_asset(lua_State* L) {
    const char* assetName = luaL_checkstring(L, 1);
    int32_t tileWidth = (int32_t)luaL_checkinteger(L, 2);
    int32_t tileHeight = (int32_t)luaL_checkinteger(L, 3);
    int32_t margin = (int32_t)luaL_optinteger(L, 4, 0);
    int32_t spacing = (int32_t)luaL_optinteger(L, 5, 0);
    STTilesetID id = st_tileset_load_asset(assetName, tileWidth, tileHeight, margin, spacing);
    lua_pushinteger(L, id);
    return 1;
}

static int lua_st_tileset_destroy(lua_State* L) {
    STTilesetID id = (STTilesetID)luaL_checkinteger(L, 1);
    st_tileset_destroy(id);
    return 0;
}

static int lua_st_tileset_get_tile_count(lua_State* L) {
    STTilesetID id = (STTilesetID)luaL_checkinteger(L, 1);
    int32_t count = st_tileset_get_tile_count(id);
    lua_pushinteger(L, count);
    return 1;
}

static int lua_st_tileset_get_dimensions(lua_State* L) {
    STTilesetID id = (STTilesetID)luaL_checkinteger(L, 1);
    int32_t columns, rows;
    st_tileset_get_dimensions(id, &columns, &rows);
    lua_pushinteger(L, columns);
    lua_pushinteger(L, rows);
    return 2;
}

static int lua_st_tilemap_layer_set_tileset(lua_State* L) {
    STLayerID layer = (STLayerID)luaL_checkinteger(L, 1);
    STTilesetID tileset = (STTilesetID)luaL_checkinteger(L, 2);
    st_tilemap_layer_set_tileset(layer, tileset);
    return 0;
}

// =============================================================================
// BASIC Math Functions
// =============================================================================

static int lua_basic_rnd(lua_State* L) {
    // RND generates random number 0.0 to 1.0
    double r = (double)rand() / (double)RAND_MAX;
    lua_pushnumber(L, r);
    return 1;
}

static int lua_basic_int(lua_State* L) {
    double x = luaL_checknumber(L, 1);
    lua_pushnumber(L, floor(x));
    return 1;
}

static int lua_basic_abs(lua_State* L) {
    double x = luaL_checknumber(L, 1);
    lua_pushnumber(L, fabs(x));
    return 1;
}

static int lua_basic_sqr(lua_State* L) {
    double x = luaL_checknumber(L, 1);
    lua_pushnumber(L, sqrt(x));
    return 1;
}

static int lua_basic_sin(lua_State* L) {
    double x = luaL_checknumber(L, 1);
    lua_pushnumber(L, sin(x));
    return 1;
}

static int lua_basic_cos(lua_State* L) {
    double x = luaL_checknumber(L, 1);
    lua_pushnumber(L, cos(x));
    return 1;
}

static int lua_basic_tan(lua_State* L) {
    double x = luaL_checknumber(L, 1);
    lua_pushnumber(L, tan(x));
    return 1;
}

static int lua_basic_atn(lua_State* L) {
    double x = luaL_checknumber(L, 1);
    lua_pushnumber(L, atan(x));
    return 1;
}

static int lua_basic_log(lua_State* L) {
    double x = luaL_checknumber(L, 1);
    lua_pushnumber(L, log(x));
    return 1;
}

static int lua_basic_exp(lua_State* L) {
    double x = luaL_checknumber(L, 1);
    lua_pushnumber(L, exp(x));
    return 1;
}

static int lua_basic_sgn(lua_State* L) {
    double x = luaL_checknumber(L, 1);
    lua_pushnumber(L, (x > 0) ? 1 : (x < 0) ? -1 : 0);
    return 1;
}

// =============================================================================
// DisplayText API Bindings
// =============================================================================

static int lua_st_text_display_at(lua_State* L) {
    float x = (float)luaL_checknumber(L, 1);
    float y = (float)luaL_checknumber(L, 2);
    const char* text = luaL_checkstring(L, 3);
    float scale_x = (float)luaL_optnumber(L, 4, 1.0);
    float scale_y = (float)luaL_optnumber(L, 5, 1.0);
    float rotation = (float)luaL_optnumber(L, 6, 0.0);
    uint32_t color = (uint32_t)luaL_optinteger(L, 7, 0xFFFFFFFF);
    int alignment = (int)luaL_optinteger(L, 8, 0); // ST_ALIGN_LEFT
    int layer = (int)luaL_optinteger(L, 9, 0);
    
    int item_id = st_text_display_at(x, y, text, scale_x, scale_y, rotation, color, (STTextAlignment)alignment, layer);
    lua_pushinteger(L, item_id);
    return 1;
}

static int lua_st_text_display_shear(lua_State* L) {
    float x = (float)luaL_checknumber(L, 1);
    float y = (float)luaL_checknumber(L, 2);
    const char* text = luaL_checkstring(L, 3);
    float scale_x = (float)luaL_optnumber(L, 4, 1.0);
    float scale_y = (float)luaL_optnumber(L, 5, 1.0);
    float rotation = (float)luaL_optnumber(L, 6, 0.0);
    float shear_x = (float)luaL_optnumber(L, 7, 0.0);
    float shear_y = (float)luaL_optnumber(L, 8, 0.0);
    uint32_t color = (uint32_t)luaL_optinteger(L, 9, 0xFFFFFFFF);
    int alignment = (int)luaL_optinteger(L, 10, 0); // ST_ALIGN_LEFT
    int layer = (int)luaL_optinteger(L, 11, 0);
    
    int item_id = st_text_display_shear(x, y, text, scale_x, scale_y, rotation, shear_x, shear_y, color, (STTextAlignment)alignment, layer);
    lua_pushinteger(L, item_id);
    return 1;
}

static int lua_st_text_display_with_effects(lua_State* L) {
    float x = (float)luaL_checknumber(L, 1);
    float y = (float)luaL_checknumber(L, 2);
    const char* text = luaL_checkstring(L, 3);
    float scale_x = (float)luaL_optnumber(L, 4, 1.0);
    float scale_y = (float)luaL_optnumber(L, 5, 1.0);
    float rotation = (float)luaL_optnumber(L, 6, 0.0);
    uint32_t color = (uint32_t)luaL_optinteger(L, 7, 0xFFFFFFFF);
    int alignment = (int)luaL_optinteger(L, 8, 0); // ST_ALIGN_LEFT
    int layer = (int)luaL_optinteger(L, 9, 0);
    int effect = (int)luaL_optinteger(L, 10, 0); // ST_EFFECT_NONE
    uint32_t effect_color = (uint32_t)luaL_optinteger(L, 11, 0x000000FF);
    float effect_intensity = (float)luaL_optnumber(L, 12, 0.5);
    float effect_size = (float)luaL_optnumber(L, 13, 2.0);
    
    int item_id = st_text_display_with_effects(x, y, text, scale_x, scale_y, rotation, 
                                               color, (STTextAlignment)alignment, layer,
                                               (STTextEffect)effect, effect_color,
                                               effect_intensity, effect_size);
    lua_pushinteger(L, item_id);
    return 1;
}

static int lua_st_text_update_item(lua_State* L) {
    int item_id = (int)luaL_checkinteger(L, 1);
    const char* text = luaL_optstring(L, 2, NULL);
    float x = (float)luaL_optnumber(L, 3, -1.0);
    float y = (float)luaL_optnumber(L, 4, -1.0);
    
    st_text_update_item(item_id, text, x, y);
    return 0;
}

static int lua_st_text_remove_item(lua_State* L) {
    int item_id = (int)luaL_checkinteger(L, 1);
    st_text_remove_item(item_id);
    return 0;
}

static int lua_st_text_clear_displayed(lua_State* L) {
    (void)L;
    st_text_clear_displayed();
    return 0;
}

static int lua_st_text_set_item_visible(lua_State* L) {
    int item_id = (int)luaL_checkinteger(L, 1);
    bool visible = lua_toboolean(L, 2);
    st_text_set_item_visible(item_id, visible);
    return 0;
}

static int lua_st_text_set_item_layer(lua_State* L) {
    int item_id = (int)luaL_checkinteger(L, 1);
    int layer = (int)luaL_checkinteger(L, 2);
    st_text_set_item_layer(item_id, layer);
    return 0;
}

static int lua_st_text_set_item_color(lua_State* L) {
    int item_id = (int)luaL_checkinteger(L, 1);
    uint32_t color = (uint32_t)luaL_checkinteger(L, 2);
    st_text_set_item_color(item_id, color);
    return 0;
}

static int lua_st_text_get_item_count(lua_State* L) {
    int count = st_text_get_item_count();
    lua_pushinteger(L, count);
    return 1;
}

static int lua_st_text_get_visible_count(lua_State* L) {
    int count = st_text_get_visible_count();
    lua_pushinteger(L, count);
    return 1;
}

// =============================================================================
// Unified Video Mode API Functions
// These dispatch to the appropriate mode-specific functions
// =============================================================================

static int lua_video_pset(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    uint32_t color = (uint32_t)luaL_checkinteger(L, 3);
    
    int mode = st_mode_get();
    if (mode == 1) {
        st_lores_pset(x, y, (uint8_t)color, 0xFF000000);
    } else if (mode == 2) {
        st_xres_pset(x, y, color);
    } else if (mode == 3) {
        st_wres_pset(x, y, color);
    } else if (mode == 4) {
        st_ures_pset(x, y, (uint16_t)color);
    } else if (mode == 5) {
        st_pres_pset(x, y, color);
    }
    return 0;
}

static int lua_video_pget(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    uint32_t color = 0;
    
    int mode = st_mode_get();
    if (mode == 1) {
        color = st_lores_palette_peek(y, (uint8_t)color); // Simplified for LORES
    } else if (mode == 2) {
        color = st_xres_pget(x, y);
    } else if (mode == 3) {
        color = st_wres_pget(x, y);
    } else if (mode == 4) {
        color = st_ures_pget(x, y);
    } else if (mode == 5) {
        color = st_pres_pget(x, y);
    }
    
    lua_pushinteger(L, color);
    return 1;
}

static int lua_video_clear(lua_State* L) {
    uint32_t color = (uint32_t)luaL_checkinteger(L, 1);
    
    int mode = st_mode_get();
    
    // AUTO-PROMOTE: Use GPU path if in batch and mode supports it
    if (g_gpuBatchActive && mode >= VIDEO_MODE_XRES && mode <= VIDEO_MODE_PRES) {
        if (mode == VIDEO_MODE_XRES) {
            st_xres_clear_gpu(g_gpuBatchBuffer, color);
        } else if (mode == VIDEO_MODE_WRES) {
            st_wres_clear_gpu(g_gpuBatchBuffer, color);
        } else if (mode == VIDEO_MODE_URES) {
            st_ures_clear_gpu(g_gpuBatchBuffer, (uint16_t)color);
        } else if (mode == VIDEO_MODE_PRES) {
            st_pres_clear_gpu(g_gpuBatchBuffer, color);
        }
        return 0;
    }
    
    // Software path (original implementation)
    if (mode == VIDEO_MODE_LORES) {
        st_lores_clear(color);
    } else if (mode == VIDEO_MODE_XRES) {
        st_xres_clear(color);
    } else if (mode == VIDEO_MODE_WRES) {
        st_wres_clear(color);
    } else if (mode == VIDEO_MODE_URES) {
        st_ures_clear((uint16_t)color);
    } else if (mode == VIDEO_MODE_PRES) {
        st_pres_clear(color);
    }
    return 0;
}

static int lua_video_line(lua_State* L) {
    int x1 = luaL_checkinteger(L, 1);
    int y1 = luaL_checkinteger(L, 2);
    int x2 = luaL_checkinteger(L, 3);
    int y2 = luaL_checkinteger(L, 4);
    uint32_t color = (uint32_t)luaL_checkinteger(L, 5);
    
    int mode = st_mode_get();
    
    // AUTO-PROMOTE: Use GPU path if in batch and mode supports it
    if (g_gpuBatchActive && mode >= VIDEO_MODE_XRES && mode <= VIDEO_MODE_PRES) {
        if (mode == VIDEO_MODE_XRES) {
            st_xres_line_gpu(g_gpuBatchBuffer, x1, y1, x2, y2, color);
        } else if (mode == VIDEO_MODE_WRES) {
            st_wres_line_gpu(g_gpuBatchBuffer, x1, y1, x2, y2, color);
        } else if (mode == VIDEO_MODE_URES) {
            st_ures_line_gpu(g_gpuBatchBuffer, x1, y1, x2, y2, color);
        } else if (mode == VIDEO_MODE_PRES) {
            st_pres_line_gpu(g_gpuBatchBuffer, x1, y1, x2, y2, color);
        }
        return 0;
    }
    
    // Software path (original implementation)
    if (mode == VIDEO_MODE_LORES) {
        st_lores_line(x1, y1, x2, y2, (uint8_t)color, 0xFF000000);
    } else if (mode == VIDEO_MODE_MIDRES || mode == VIDEO_MODE_HIRES) {
        st_gfx_line(x1, y1, x2, y2, color, 1);
    } else if (mode == VIDEO_MODE_XRES) {
        st_xres_line_simple(x1, y1, x2, y2, (uint8_t)color);
    } else if (mode == VIDEO_MODE_WRES) {
        st_wres_line_simple(x1, y1, x2, y2, (uint8_t)color);
    } else if (mode == VIDEO_MODE_PRES) {
        st_pres_line_simple(x1, y1, x2, y2, (uint8_t)color);
    }
    return 0;
}

static int lua_video_rect(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int width = luaL_checkinteger(L, 3);
    int height = luaL_checkinteger(L, 4);
    uint32_t color = (uint32_t)luaL_checkinteger(L, 5);
    bool filled = lua_toboolean(L, 6);
    
    int mode = st_mode_get();
    
    // AUTO-PROMOTE: Use GPU path if in batch and mode supports it
    if (g_gpuBatchActive && mode >= VIDEO_MODE_XRES && mode <= VIDEO_MODE_PRES) {
        if (mode == VIDEO_MODE_XRES) {
            st_xres_rect_fill_gpu(g_gpuBatchBuffer, x, y, width, height, color);
        } else if (mode == VIDEO_MODE_WRES) {
            st_wres_rect_fill_gpu(g_gpuBatchBuffer, x, y, width, height, color);
        } else if (mode == VIDEO_MODE_URES) {
            st_ures_rect_fill_gpu(g_gpuBatchBuffer, x, y, width, height, color);
        } else if (mode == VIDEO_MODE_PRES) {
            st_pres_rect_fill_gpu(g_gpuBatchBuffer, x, y, width, height, color);
        }
        return 0;
    }
    
    // Software path (original implementation)
    if (mode == VIDEO_MODE_LORES) {
        if (filled) {
            st_lores_fillrect(x, y, width, height, (uint8_t)color, 0xFF000000);
        } else {
            st_lores_rect(x, y, width, height, (uint8_t)color, 0xFF000000);
        }
    } else if (mode == VIDEO_MODE_MIDRES || mode == VIDEO_MODE_HIRES) {
        // MIDRES/HIRES use graphics layer
        if (filled) {
            st_gfx_rect(x, y, width, height, color);
        } else {
            st_gfx_rect(x, y, width, height, color);
        }
    } else if (mode == VIDEO_MODE_URES) {
        // URES only has fillrect
        st_ures_fillrect(x, y, width, height, color);
    } else if (mode == VIDEO_MODE_XRES) {
        // XRES only has fillrect
        st_xres_fillrect(x, y, width, height, (uint8_t)color);
    } else if (mode == VIDEO_MODE_WRES) {
        // WRES only has fillrect
        st_wres_fillrect(x, y, width, height, (uint8_t)color);
    } else if (mode == VIDEO_MODE_PRES) {
        // PRES only has fillrect
        st_pres_fillrect(x, y, width, height, (uint8_t)color);
    }
    return 0;
}

static int lua_video_circle(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int radius = luaL_checkinteger(L, 3);
    uint32_t color = (uint32_t)luaL_checkinteger(L, 4);
    bool filled = lua_toboolean(L, 5);
    
    int mode = st_mode_get();
    
    // AUTO-PROMOTE: Use GPU path if in batch and mode supports it
    if (g_gpuBatchActive && mode >= VIDEO_MODE_XRES && mode <= VIDEO_MODE_PRES) {
        if (mode == VIDEO_MODE_XRES) {
            st_xres_circle_fill_gpu(g_gpuBatchBuffer, x, y, radius, color);
        } else if (mode == VIDEO_MODE_WRES) {
            st_wres_circle_fill_gpu(g_gpuBatchBuffer, x, y, radius, color);
        } else if (mode == VIDEO_MODE_URES) {
            st_ures_circle_fill_gpu(g_gpuBatchBuffer, x, y, radius, color);
        } else if (mode == VIDEO_MODE_PRES) {
            st_pres_circle_fill_gpu(g_gpuBatchBuffer, x, y, radius, color);
        }
        return 0;
    }
    
    // Software path (original implementation)
    if (mode == VIDEO_MODE_MIDRES || mode == VIDEO_MODE_HIRES) {
        // MIDRES/HIRES use graphics layer
        st_gfx_circle(x, y, radius, color);
    } else if (mode == VIDEO_MODE_XRES) {
        st_xres_circle_simple(x, y, radius, (uint8_t)color);
    } else if (mode == VIDEO_MODE_WRES) {
        st_wres_circle_simple(x, y, radius, (uint8_t)color);
    } else if (mode == VIDEO_MODE_PRES) {
        st_pres_circle_simple(x, y, radius, (uint8_t)color);
    }
    return 0;
}

static int lua_video_swap(lua_State* L) {
    int mode = st_mode_get();
    if (mode == VIDEO_MODE_LORES) {
        st_lores_flip();
    } else if (mode == VIDEO_MODE_XRES) {
        st_xres_flip();
    } else if (mode == VIDEO_MODE_WRES) {
        st_wres_flip();
    } else if (mode == VIDEO_MODE_URES) {
        st_ures_flip();
    } else if (mode == VIDEO_MODE_PRES) {
        st_pres_flip();
    }
    return 0;
}

static int lua_vpalette_row(lua_State* L) {
    int row = luaL_checkinteger(L, 1);
    int index = luaL_checkinteger(L, 2);
    int r = luaL_checkinteger(L, 3);
    int g = luaL_checkinteger(L, 4);
    int b = luaL_checkinteger(L, 5);
    
    int mode = st_mode_get();
    if (mode == VIDEO_MODE_LORES) {
        // LORES uses RGBA format, construct color
        uint32_t rgba = ((255 & 0xFF) << 24) | ((r & 0xFF) << 16) | ((g & 0xFF) << 8) | (b & 0xFF);
        st_lores_palette_poke(row, index, rgba);
    } else if (mode == VIDEO_MODE_XRES) {
        st_xres_palette_row(row, index, r, g, b);
    } else if (mode == VIDEO_MODE_WRES) {
        st_wres_palette_row(row, index, r, g, b);
    } else if (mode == VIDEO_MODE_PRES) {
        st_pres_palette_row(row, index, r, g, b);
    }
    // URES has no palette (direct color), so it's a no-op
    return 0;
}

static int lua_video_blit(lua_State* L) {
    int src_x = luaL_checkinteger(L, 1);
    int src_y = luaL_checkinteger(L, 2);
    int width = luaL_checkinteger(L, 3);
    int height = luaL_checkinteger(L, 4);
    int dst_x = luaL_checkinteger(L, 5);
    int dst_y = luaL_checkinteger(L, 6);
    
    int mode = st_mode_get();
    if (mode == 1) {
        // LORES basic blit (no buffer parameters in this version)
        st_lores_blit(src_x, src_y, width, height, dst_x, dst_y);
    } else if (mode == 2) {
        st_xres_blit(src_x, src_y, width, height, dst_x, dst_y);
    } else if (mode == 3) {
        st_wres_blit(src_x, src_y, width, height, dst_x, dst_y);
    } else if (mode == 4) {
        // URES uses blit_from with buffer 0
        st_ures_blit_from(0, src_x, src_y, width, height, dst_x, dst_y);
    } else if (mode == 5) {
        st_pres_blit(src_x, src_y, width, height, dst_x, dst_y);
    }
    return 0;
}

static int lua_video_blit_trans(lua_State* L) {
    int src_x = luaL_checkinteger(L, 1);
    int src_y = luaL_checkinteger(L, 2);
    int width = luaL_checkinteger(L, 3);
    int height = luaL_checkinteger(L, 4);
    int dst_x = luaL_checkinteger(L, 5);
    int dst_y = luaL_checkinteger(L, 6);
    
    int mode = st_mode_get();
    if (mode == 1) {
        // LORES transparent blit with color key
        uint32_t trans_color = (uint32_t)luaL_optinteger(L, 7, 0);
        st_lores_blit_trans(src_x, src_y, width, height, dst_x, dst_y, trans_color);
    } else if (mode == 2) {
        st_xres_blit_trans(src_x, src_y, width, height, dst_x, dst_y);
    } else if (mode == 3) {
        st_wres_blit_trans(src_x, src_y, width, height, dst_x, dst_y);
    } else if (mode == 4) {
        // URES uses blit_from_trans with buffer 0
        st_ures_blit_from_trans(0, src_x, src_y, width, height, dst_x, dst_y);
    } else if (mode == 5) {
        st_pres_blit_trans(src_x, src_y, width, height, dst_x, dst_y);
    }
    return 0;
}

static int lua_video_buffer(lua_State* L) {
    int buffer = luaL_checkinteger(L, 1);
    
    int mode = st_mode_get();
    if (mode == VIDEO_MODE_LORES) {
        st_lores_buffer(buffer);
    } else if (mode == VIDEO_MODE_XRES) {
        st_xres_buffer(buffer);
    } else if (mode == VIDEO_MODE_WRES) {
        st_wres_buffer(buffer);
    } else if (mode == VIDEO_MODE_URES) {
        st_ures_buffer(buffer);
    } else if (mode == VIDEO_MODE_PRES) {
        st_pres_buffer(buffer);
    }
    return 0;
}

static int lua_video_buffer_get(lua_State* L) {
    // Use st_video_buffer_get which properly dispatches to all modes via VideoModeManager
    int buffer = st_video_buffer_get();
    lua_pushinteger(L, buffer);
    return 1;
}

static int lua_video_get_active_buffer(lua_State* L) {
    // Get current active drawing buffer (same as buffer_get)
    int buffer = st_video_buffer_get();
    lua_pushinteger(L, buffer);
    return 1;
}

static int lua_video_get_display_buffer(lua_State* L) {
    // Get current display/front buffer
    int buffer = st_video_get_front_buffer();
    lua_pushinteger(L, buffer);
    return 1;
}

static int lua_video_flip(lua_State* L) {
    int mode = st_mode_get();
    printf("[VFLIP DEBUG] VFLIP called, mode=%d\n", mode);
    if (mode == VIDEO_MODE_LORES) {
        printf("[VFLIP DEBUG] Calling st_lores_flip()\n");
        st_lores_flip();
    } else if (mode == VIDEO_MODE_XRES) {
        printf("[VFLIP DEBUG] Calling st_xres_flip()\n");
        st_xres_flip();
    } else if (mode == VIDEO_MODE_WRES) {
        printf("[VFLIP DEBUG] Calling st_wres_flip()\n");
        st_wres_flip();
    } else if (mode == VIDEO_MODE_URES) {
        printf("[VFLIP DEBUG] Calling st_ures_flip()\n");
        st_ures_flip();
    } else if (mode == VIDEO_MODE_PRES) {
        printf("[VFLIP DEBUG] Calling st_pres_flip()\n");
        st_pres_flip();
    } else {
        printf("[VFLIP DEBUG] WARNING: Mode %d not handled!\n", mode);
    }
    printf("[VFLIP DEBUG] VFLIP complete\n");
    return 0;
}

static int lua_video_mode_get(lua_State* L) {
    int mode = st_mode_get();
    lua_pushinteger(L, mode);
    return 1;
}

static int lua_video_mode_name(lua_State* L) {
    int mode = st_mode_get();
    const char* name = "UNKNOWN";
    switch (mode) {
        case 0: name = "TEXT"; break;
        case 1: name = "LORES"; break;
        case 2: name = "XRES"; break;
        case 3: name = "WRES"; break;
        case 4: name = "URES"; break;
        case 5: name = "PRES"; break;
    }
    lua_pushstring(L, name);
    return 1;
}

static int lua_video_get_color_depth(lua_State* L) {
    int mode = st_mode_get();
    int depth = 0;
    if (mode == 1) depth = 8;
    else if (mode == 2 || mode == 3 || mode == 5) depth = 32;
    else if (mode == 4) depth = 16;
    lua_pushinteger(L, depth);
    return 1;
}

static int lua_video_has_palette(lua_State* L) {
    int mode = st_mode_get();
    bool has_pal = (mode == 1 || mode == 4);
    lua_pushboolean(L, has_pal);
    return 1;
}

static int lua_video_has_gpu(lua_State* L) {
    int mode = st_mode_get();
    bool has_gpu = (mode == 2 || mode == 3 || mode == 4 || mode == 5);
    lua_pushboolean(L, has_gpu);
    return 1;
}

static int lua_video_max_buffers(lua_State* L) {
    int mode = st_mode_get();
    int max_buffers = 0;
    if (mode == 1) max_buffers = 8;  // LORES
    else if (mode == 2) max_buffers = 2;  // XRES
    else if (mode == 3) max_buffers = 2;  // WRES
    else if (mode == 4) max_buffers = 4;  // URES
    else if (mode == 5) max_buffers = 2;  // PRES
    lua_pushinteger(L, max_buffers);
    return 1;
}

// =============================================================================
// GPU-Accelerated Video Commands
// =============================================================================

static int lua_video_clear_gpu(lua_State* L) {
    int buffer_id = luaL_checkinteger(L, 1);
    uint32_t color = (uint32_t)luaL_checkinteger(L, 2);
    
    int mode = st_mode_get();
    if (mode == VIDEO_MODE_LORES) {
        st_lores_clear_gpu(buffer_id, color);
    } else if (mode == VIDEO_MODE_XRES) {
        st_xres_clear_gpu(buffer_id, color);
    } else if (mode == VIDEO_MODE_WRES) {
        st_wres_clear_gpu(buffer_id, color);
    } else if (mode == VIDEO_MODE_URES) {
        st_ures_clear_gpu(buffer_id, color);
    } else if (mode == VIDEO_MODE_PRES) {
        st_pres_clear_gpu(buffer_id, color);
    }
    return 0;
}

static int lua_video_line_gpu(lua_State* L) {
    int buffer_id = luaL_checkinteger(L, 1);
    int x1 = luaL_checkinteger(L, 2);
    int y1 = luaL_checkinteger(L, 3);
    int x2 = luaL_checkinteger(L, 4);
    int y2 = luaL_checkinteger(L, 5);
    uint32_t color = (uint32_t)luaL_checkinteger(L, 6);
    
    int mode = st_mode_get();
    if (mode == 1) {
        st_lores_line_gpu(buffer_id, x1, y1, x2, y2, (uint8_t)color);
    } else if (mode == 2) {
        st_xres_line_gpu(buffer_id, x1, y1, x2, y2, color);
    } else if (mode == 3) {
        st_wres_line_gpu(buffer_id, x1, y1, x2, y2, color);
    } else if (mode == 4) {
        st_ures_line_gpu(buffer_id, x1, y1, x2, y2, color);
    } else if (mode == 5) {
        st_pres_line_gpu(buffer_id, x1, y1, x2, y2, color);
    }
    return 0;
}

static int lua_video_rect_gpu(lua_State* L) {
    int buffer_id = luaL_checkinteger(L, 1);
    int x = luaL_checkinteger(L, 2);
    int y = luaL_checkinteger(L, 3);
    int width = luaL_checkinteger(L, 4);
    int height = luaL_checkinteger(L, 5);
    uint32_t color = (uint32_t)luaL_checkinteger(L, 6);
    
    int mode = st_mode_get();
    if (mode == 1) {
        st_lores_rect_fill_gpu(buffer_id, x, y, width, height, (uint8_t)color);
    } else if (mode == 2) {
        st_xres_rect_fill_gpu(buffer_id, x, y, width, height, color);
    } else if (mode == 3) {
        st_wres_rect_fill_gpu(buffer_id, x, y, width, height, color);
    } else if (mode == 4) {
        st_ures_rect_fill_gpu(buffer_id, x, y, width, height, color);
    } else if (mode == 5) {
        st_pres_rect_fill_gpu(buffer_id, x, y, width, height, color);
    }
    return 0;
}

static int lua_video_circle_gpu(lua_State* L) {
    int buffer_id = luaL_checkinteger(L, 1);
    int x = luaL_checkinteger(L, 2);
    int y = luaL_checkinteger(L, 3);
    int radius = luaL_checkinteger(L, 4);
    uint32_t color = (uint32_t)luaL_checkinteger(L, 5);
    
    int mode = st_mode_get();
    if (mode == 1) {
        st_lores_circle_fill_gpu(buffer_id, x, y, radius, (uint8_t)color);
    } else if (mode == 2) {
        st_xres_circle_fill_gpu(buffer_id, x, y, radius, color);
    } else if (mode == 3) {
        st_wres_circle_fill_gpu(buffer_id, x, y, radius, color);
    } else if (mode == 4) {
        st_ures_circle_fill_gpu(buffer_id, x, y, radius, color);
    } else if (mode == 5) {
        st_pres_circle_fill_gpu(buffer_id, x, y, radius, color);
    }
    return 0;
}

static int lua_video_blit_gpu(lua_State* L) {
    int src_buffer = luaL_checkinteger(L, 1);
    int dst_buffer = luaL_checkinteger(L, 2);
    int src_x = luaL_checkinteger(L, 3);
    int src_y = luaL_checkinteger(L, 4);
    int width = luaL_checkinteger(L, 5);
    int height = luaL_checkinteger(L, 6);
    int dst_x = luaL_checkinteger(L, 7);
    int dst_y = luaL_checkinteger(L, 8);
    
    int mode = st_mode_get();
    if (mode == 1) {
        st_lores_blit_gpu(src_buffer, dst_buffer, src_x, src_y, width, height, dst_x, dst_y);
    } else if (mode == 2) {
        st_xres_blit_gpu(src_buffer, dst_buffer, src_x, src_y, width, height, dst_x, dst_y);
    } else if (mode == 3) {
        st_wres_blit_gpu(src_buffer, dst_buffer, src_x, src_y, width, height, dst_x, dst_y);
    } else if (mode == 4) {
        st_ures_blit_copy_gpu(src_buffer, dst_buffer, src_x, src_y, width, height, dst_x, dst_y);
    } else if (mode == 5) {
        st_pres_blit_gpu(src_buffer, dst_buffer, src_x, src_y, width, height, dst_x, dst_y);
    }
    return 0;
}

// =============================================================================
// Palette Commands
// =============================================================================

static int lua_video_palette_set(lua_State* L) {
    int row = luaL_optinteger(L, 1, 0);
    int index = luaL_checkinteger(L, 2);
    uint32_t color = (uint32_t)luaL_checkinteger(L, 3);
    
    int mode = st_mode_get();
    if (mode == 1) {
        // LORES - use poke with RGBA color
        st_lores_palette_poke(row, index, color);
    }
    // Other modes don't have settable palettes in the same way
    return 0;
}

static int lua_video_palette_set_row(lua_State* L) {
    int row = luaL_checkinteger(L, 1);
    int index = luaL_checkinteger(L, 2);
    uint32_t color = (uint32_t)luaL_checkinteger(L, 3);
    
    int mode = st_mode_get();
    if (mode == 1) {
        // LORES - use poke with RGBA color
        st_lores_palette_poke(row, index, color);
    }
    return 0;
}

static int lua_video_palette_get(lua_State* L) {
    int row = luaL_optinteger(L, 1, 0);
    int index = luaL_checkinteger(L, 2);
    uint32_t color = 0;
    
    int mode = st_mode_get();
    if (mode == 1) {
        color = st_lores_palette_peek(row, index);
    }
    
    lua_pushinteger(L, color);
    return 1;
}

static int lua_video_palette_reset(lua_State* L) {
    int mode = st_mode_get();
    if (mode == 1) {
        // LORES - reset to default palette (e.g., "c64")
        const char* palette_name = luaL_optstring(L, 1, "c64");
        st_lores_palette_set(palette_name);
    }
    return 0;
}

// =============================================================================
// Batch Rendering Commands
// =============================================================================

static int lua_video_begin_batch(lua_State* L) {
    int mode = st_mode_get();
    if (mode == 4) {
        st_begin_blit_batch();
    }
    return 0;
}

static int lua_video_end_batch(lua_State* L) {
    int mode = st_mode_get();
    if (mode == 4) {
        st_end_blit_batch();
    }
    return 0;
}

// =============================================================================
// GPU Batch with Auto-Promotion (VGPUBEGIN/VGPUEND)
// =============================================================================

static int lua_video_gpu_begin(lua_State* L) {
    // Optional buffer parameter (defaults to 0)
    int buffer_id = luaL_optinteger(L, 1, 0);
    
    // Check if already in batch
    if (g_gpuBatchActive) {
        fprintf(stderr, "WARNING: VGPUBEGIN called while already in GPU batch. Ignoring nested call.\n");
        return 0;
    }
    
    int mode = st_mode_get();
    
    // Validate buffer ID
    if (buffer_id < 0 || buffer_id >= 8) {
        fprintf(stderr, "ERROR: Invalid buffer ID %d in VGPUBEGIN (must be 0-7)\n", buffer_id);
        return 0;
    }
    
    // Set batch state
    g_gpuBatchActive = true;
    g_gpuBatchBuffer = buffer_id;
    
    // Start Metal command buffer batching for supported modes
    if (mode >= VIDEO_MODE_XRES && mode <= VIDEO_MODE_PRES) {
        st_begin_blit_batch();
    }
    
    return 0;
}

static int lua_video_gpu_end(lua_State* L) {
    // Check if batch was started
    if (!g_gpuBatchActive) {
        fprintf(stderr, "WARNING: VGPUEND called without matching VGPUBEGIN. Ignoring.\n");
        return 0;
    }
    
    int mode = st_mode_get();
    
    // End Metal command buffer batching
    if (mode >= VIDEO_MODE_XRES && mode <= VIDEO_MODE_PRES) {
        st_end_blit_batch();
    }
    
    // Clear batch state
    g_gpuBatchActive = false;
    g_gpuBatchBuffer = 0;
    
    return 0;
}

// =============================================================================
// Anti-Aliased Primitives
// =============================================================================

static int lua_video_line_aa(lua_State* L) {
    int buffer_id = luaL_checkinteger(L, 1);
    int x1 = luaL_checkinteger(L, 2);
    int y1 = luaL_checkinteger(L, 3);
    int x2 = luaL_checkinteger(L, 4);
    int y2 = luaL_checkinteger(L, 5);
    uint32_t color = (uint32_t)luaL_checkinteger(L, 6);
    float line_width = (float)luaL_optnumber(L, 7, 1.0);
    
    int mode = st_mode_get();
    if (mode == 2) {
        st_xres_line_aa(buffer_id, x1, y1, x2, y2, color, line_width);
    } else if (mode == 3) {
        st_wres_line_aa(buffer_id, x1, y1, x2, y2, color, line_width);
    } else if (mode == 4) {
        st_ures_line_aa(buffer_id, x1, y1, x2, y2, color, line_width);
    } else if (mode == 5) {
        st_pres_line_aa(buffer_id, x1, y1, x2, y2, color, line_width);
    }
    return 0;
}

static int lua_video_circle_aa(lua_State* L) {
    int buffer_id = luaL_checkinteger(L, 1);
    int x = luaL_checkinteger(L, 2);
    int y = luaL_checkinteger(L, 3);
    int radius = luaL_checkinteger(L, 4);
    uint32_t color = (uint32_t)luaL_checkinteger(L, 5);
    
    int mode = st_mode_get();
    if (mode == 2) {
        st_xres_circle_fill_aa(buffer_id, x, y, radius, color);
    } else if (mode == 3) {
        st_wres_circle_fill_aa(buffer_id, x, y, radius, color);
    } else if (mode == 4) {
        st_ures_circle_fill_aa(buffer_id, x, y, radius, color);
    } else if (mode == 5) {
        st_pres_circle_fill_aa(buffer_id, x, y, radius, color);
    }
    return 0;
}

// =============================================================================
// Gradient Primitives
// =============================================================================

static int lua_video_rect_gradient(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int width = luaL_checkinteger(L, 3);
    int height = luaL_checkinteger(L, 4);
    uint32_t c1 = (uint32_t)luaL_checkinteger(L, 5);
    uint32_t c2 = (uint32_t)luaL_checkinteger(L, 6);
    uint32_t c3 = (uint32_t)luaL_checkinteger(L, 7);
    uint32_t c4 = (uint32_t)luaL_checkinteger(L, 8);
    
    int mode = st_mode_get();
    if (mode == 2) {
        st_xres_gradient_corners(x, y, width, height, c1, c2, c3, c4);
    } else if (mode == 3) {
        st_wres_gradient_corners(x, y, width, height, c1, c2, c3, c4);
    } else if (mode == 5) {
        // PRES needs buffer ID
        int buffer_id = luaL_optinteger(L, 9, 0);
        st_pres_gradient_corners(buffer_id, x, y, width, height, c1, c2, c3, c4);
    }
    // URES uses GPU variant, see below
    return 0;
}

static int lua_video_rect_gradient_gpu(lua_State* L) {
    int buffer_id = luaL_checkinteger(L, 1);
    int x = luaL_checkinteger(L, 2);
    int y = luaL_checkinteger(L, 3);
    int width = luaL_checkinteger(L, 4);
    int height = luaL_checkinteger(L, 5);
    uint32_t c1 = (uint32_t)luaL_checkinteger(L, 6);
    uint32_t c2 = (uint32_t)luaL_checkinteger(L, 7);
    uint32_t c3 = (uint32_t)luaL_checkinteger(L, 8);
    uint32_t c4 = (uint32_t)luaL_checkinteger(L, 9);
    
    int mode = st_mode_get();
    if (mode == 4) {
        st_ures_rect_fill_gradient_gpu(buffer_id, x, y, width, height, c1, c2, c3, c4);
    }
    return 0;
}

static int lua_video_rect_gradient_h(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int width = luaL_checkinteger(L, 3);
    int height = luaL_checkinteger(L, 4);
    uint32_t c1 = (uint32_t)luaL_checkinteger(L, 5);
    uint32_t c2 = (uint32_t)luaL_checkinteger(L, 6);
    
    int mode = st_mode_get();
    if (mode == 2) {
        st_xres_gradient_h(x, y, width, height, c1, c2);
    } else if (mode == 3) {
        st_wres_gradient_h(x, y, width, height, c1, c2);
    } else if (mode == 5) {
        int buffer_id = luaL_optinteger(L, 7, 0);
        st_pres_gradient_h(buffer_id, x, y, width, height, c1, c2);
    }
    return 0;
}

static int lua_video_rect_gradient_v(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int width = luaL_checkinteger(L, 3);
    int height = luaL_checkinteger(L, 4);
    uint32_t c1 = (uint32_t)luaL_checkinteger(L, 5);
    uint32_t c2 = (uint32_t)luaL_checkinteger(L, 6);
    
    int mode = st_mode_get();
    if (mode == 2) {
        st_xres_gradient_v(x, y, width, height, c1, c2);
    } else if (mode == 3) {
        st_wres_gradient_v(x, y, width, height, c1, c2);
    } else if (mode == 5) {
        int buffer_id = luaL_optinteger(L, 7, 0);
        st_pres_gradient_v(buffer_id, x, y, width, height, c1, c2);
    }
    return 0;
}

static int lua_video_circle_gradient(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int radius = luaL_checkinteger(L, 3);
    uint32_t center_color = (uint32_t)luaL_checkinteger(L, 4);
    uint32_t edge_color = (uint32_t)luaL_checkinteger(L, 5);
    
    int mode = st_mode_get();
    if (mode == 2) {
        st_xres_gradient_radial(x, y, radius, center_color, edge_color);
    } else if (mode == 3) {
        st_wres_gradient_radial(x, y, radius, center_color, edge_color);
    } else if (mode == 5) {
        int buffer_id = luaL_optinteger(L, 6, 0);
        st_pres_gradient_radial(buffer_id, x, y, radius, center_color, edge_color);
    }
    return 0;
}

static int lua_video_circle_gradient_gpu(lua_State* L) {
    int buffer_id = luaL_checkinteger(L, 1);
    int x = luaL_checkinteger(L, 2);
    int y = luaL_checkinteger(L, 3);
    int radius = luaL_checkinteger(L, 4);
    uint32_t center_color = (uint32_t)luaL_checkinteger(L, 5);
    uint32_t edge_color = (uint32_t)luaL_checkinteger(L, 6);
    
    int mode = st_mode_get();
    if (mode == 4) {
        st_ures_circle_fill_gradient_gpu(buffer_id, x, y, radius, center_color, edge_color);
    }
    return 0;
}

static int lua_video_circle_gradient_aa(lua_State* L) {
    int buffer_id = luaL_checkinteger(L, 1);
    int x = luaL_checkinteger(L, 2);
    int y = luaL_checkinteger(L, 3);
    int radius = luaL_checkinteger(L, 4);
    uint32_t center_color = (uint32_t)luaL_checkinteger(L, 5);
    uint32_t edge_color = (uint32_t)luaL_checkinteger(L, 6);
    
    int mode = st_mode_get();
    if (mode == 4) {
        st_ures_circle_fill_gradient_aa(buffer_id, x, y, radius, center_color, edge_color);
    }
    return 0;
}

// =============================================================================

void registerBindings(lua_State* L) {
    // Seed random number generator
    srand((unsigned int)time(NULL));

    // BASIC Math Functions
    luaL_setglobalfunction(L, "basic_rnd", lua_basic_rnd);
    luaL_setglobalfunction(L, "basic_int", lua_basic_int);
    luaL_setglobalfunction(L, "basic_abs", lua_basic_abs);
    luaL_setglobalfunction(L, "basic_sqr", lua_basic_sqr);
    luaL_setglobalfunction(L, "basic_sin", lua_basic_sin);
    luaL_setglobalfunction(L, "basic_cos", lua_basic_cos);
    luaL_setglobalfunction(L, "basic_tan", lua_basic_tan);
    luaL_setglobalfunction(L, "basic_atn", lua_basic_atn);
    luaL_setglobalfunction(L, "basic_log", lua_basic_log);
    luaL_setglobalfunction(L, "basic_exp", lua_basic_exp);
    luaL_setglobalfunction(L, "basic_sgn", lua_basic_sgn);

    // DATA/READ/RESTORE API (from FasterBASICT runtime)
    FasterBASIC::registerDataBindings(L);

    // File I/O API (from FasterBASICT runtime)
    FasterBASIC::register_fileio_functions(L);

    // Timer function
    luaL_setglobalfunction(L, "system_timer", [](lua_State* L) -> int {
        double time = st_timer();
        lua_pushnumber(L, time);
        return 1;
    });

    // Minimal basic_cls function (calls SuperTerminal text_clear)
    luaL_setglobalfunction(L, "basic_cls", [](lua_State* L) -> int {
        st_text_clear();
        return 0;
    });

    // CLS function (uppercase) for modular command system compatibility
    luaL_setglobalfunction(L, "CLS", [](lua_State* L) -> int {
        st_text_clear();
        return 0;
    });

    // cls function (lowercase) for additional compatibility
    luaL_setglobalfunction(L, "cls", [](lua_State* L) -> int {
        st_text_clear();
        return 0;
    });

    // WAITKEY - Wait for single keypress with optional timeout
    luaL_setglobalfunction(L, "basic_waitkey", [](lua_State* L) -> int {
        float timeout_seconds = (float)luaL_optnumber(L, 1, -1.0);
        int timeout_frames = (timeout_seconds > 0) ? (int)(timeout_seconds * 60.0f) : -1;
        int frames_waited = 0;
        
        // Clear input buffer first
        st_key_clear_buffer();
        
        // Wait for NEW keypress using st_key_just_pressed
        while (true) {
            st_wait_frame();
            frames_waited++;
            
            // Check timeout
            if (timeout_frames > 0 && frames_waited >= timeout_frames) {
                lua_pushstring(L, "");
                return 1;
            }
            
            // Check if any key was just pressed this frame
            for (int key = 0; key < 512; key++) {
                if (st_key_just_pressed(static_cast<STKeyCode>(key))) {
                    // Key was just pressed - get character from buffer
                    uint32_t ch = st_key_get_char();
                    if (ch != 0) {
                        // Printable character
                        char keyStr[2] = {(char)ch, '\0'};
                        lua_pushstring(L, keyStr);
                    } else {
                        // Non-printable key (arrow, function key, etc)
                        // Return special escape sequence
                        char keyStr[16];
                        snprintf(keyStr, sizeof(keyStr), "\x1B[%d~", key);
                        lua_pushstring(L, keyStr);
                    }
                    return 1;
                }
            }
        }
        return 0;
    });

    // Basic input functions for FasterBASIC compatibility
    luaL_setglobalfunction(L, "basic_input", [](lua_State* L) -> int {
        // Use SuperTerminal's INPUT_AT at 0,0 for basic input
        std::string result = "";
        if (lua_gettop(L) > 0 && !lua_isnil(L, 1)) {
            std::string prompt = luaL_checkstring(L, 1);
            lua_getglobal(L, "basic_input_at");
            lua_pushinteger(L, 0);  // x = 0
            lua_pushinteger(L, 0);  // y = 0
            lua_pushstring(L, prompt.c_str());
            lua_call(L, 3, 1);
        } else {
            lua_getglobal(L, "basic_input_at");
            lua_pushinteger(L, 0);  // x = 0
            lua_pushinteger(L, 0);  // y = 0
            lua_pushstring(L, "");  // empty prompt
            lua_call(L, 3, 1);
        }
        return 1;  // Return the result from basic_input_at
    });

    luaL_setglobalfunction(L, "basic_input_string", [](lua_State* L) -> int {
        // Same as basic_input for strings
        if (lua_gettop(L) > 0 && !lua_isnil(L, 1)) {
            std::string prompt = luaL_checkstring(L, 1);
            lua_getglobal(L, "basic_input_at");
            lua_pushinteger(L, 0);  // x = 0
            lua_pushinteger(L, 0);  // y = 0
            lua_pushstring(L, prompt.c_str());
            lua_call(L, 3, 1);
        } else {
            lua_getglobal(L, "basic_input_at");
            lua_pushinteger(L, 0);  // x = 0
            lua_pushinteger(L, 0);  // y = 0
            lua_pushstring(L, "");  // empty prompt
            lua_call(L, 3, 1);
        }
        return 1;  // Return the result from basic_input_at
    });

    // Text API
    luaL_setglobalfunction(L, "text_putchar", lua_st_text_putchar);
    luaL_setglobalfunction(L, "poke_text", lua_poke_text);
    luaL_setglobalfunction(L, "text_put", lua_st_text_put);
    luaL_setglobalfunction(L, "text_clear", lua_st_text_clear);
    luaL_setglobalfunction(L, "text_clear_region", lua_st_text_clear_region);
    luaL_setglobalfunction(L, "text_set_size", lua_st_text_set_size);
    luaL_setglobalfunction(L, "text_get_size", lua_st_text_get_size);
    luaL_setglobalfunction(L, "text_scroll", lua_st_text_scroll);
    
    // DisplayText API
    luaL_setglobalfunction(L, "text_display_at", lua_st_text_display_at);
    luaL_setglobalfunction(L, "text_display_shear", lua_st_text_display_shear);
    luaL_setglobalfunction(L, "text_display_with_effects", lua_st_text_display_with_effects);
    luaL_setglobalfunction(L, "text_update_item", lua_st_text_update_item);
    luaL_setglobalfunction(L, "text_remove_item", lua_st_text_remove_item);
    luaL_setglobalfunction(L, "text_clear_displayed", lua_st_text_clear_displayed);
    luaL_setglobalfunction(L, "text_set_item_visible", lua_st_text_set_item_visible);
    luaL_setglobalfunction(L, "text_set_item_layer", lua_st_text_set_item_layer);
    luaL_setglobalfunction(L, "text_set_item_color", lua_st_text_set_item_color);
    luaL_setglobalfunction(L, "text_get_item_count", lua_st_text_get_item_count);
    luaL_setglobalfunction(L, "text_get_visible_count", lua_st_text_get_visible_count);

    // Sixel API
    luaL_setglobalfunction(L, "text_putsixel", lua_st_text_putsixel);
    luaL_setglobalfunction(L, "text_putsixel_packed", lua_st_text_putsixel_packed);
    luaL_setglobalfunction(L, "sixel_pack_colors", lua_st_sixel_pack_colors);
    luaL_setglobalfunction(L, "sixel_set_stripe", lua_st_sixel_set_stripe);
    luaL_setglobalfunction(L, "sixel_get_stripe", lua_st_sixel_get_stripe);
    luaL_setglobalfunction(L, "sixel_gradient", lua_st_sixel_gradient);
    luaL_setglobalfunction(L, "sixel_hline", lua_st_sixel_hline);
    luaL_setglobalfunction(L, "sixel_fill_rect", lua_st_sixel_fill_rect);

    // LORES Pixel Graphics API
    luaL_setglobalfunction(L, "pset", lua_st_lores_pset);
    luaL_setglobalfunction(L, "line", lua_st_lores_line);
    luaL_setglobalfunction(L, "rect", lua_st_lores_rect);
    luaL_setglobalfunction(L, "fillrect", lua_st_lores_fillrect);
    luaL_setglobalfunction(L, "hline", lua_st_lores_hline);
    luaL_setglobalfunction(L, "vline", lua_st_lores_vline);
    luaL_setglobalfunction(L, "lores_clear", lua_st_lores_clear);
    luaL_setglobalfunction(L, "lores_resolution", lua_st_lores_resolution);
    
    // LORES Buffer Management API
    luaL_setglobalfunction(L, "lores_buffer", lua_st_lores_buffer);
    luaL_setglobalfunction(L, "lores_buffer_get", lua_st_lores_buffer_get);
    luaL_setglobalfunction(L, "lores_flip", lua_st_lores_flip);
    
    // LORES Blitter API
    luaL_setglobalfunction(L, "lores_blit", lua_st_lores_blit);
    luaL_setglobalfunction(L, "lores_blit_trans", lua_st_lores_blit_trans);
    luaL_setglobalfunction(L, "lores_blit_buffer", lua_st_lores_blit_buffer);
    luaL_setglobalfunction(L, "lores_blit_buffer_trans", lua_st_lores_blit_buffer_trans);
    
    // LORES Mode Management API
    luaL_setglobalfunction(L, "mode", lua_st_mode);
    
    // LORES Palette Management API
    luaL_setglobalfunction(L, "lores_palette_set", lua_st_lores_palette_set);
    luaL_setglobalfunction(L, "lores_palette_poke", lua_st_lores_palette_poke);
    luaL_setglobalfunction(L, "lores_palette_peek", lua_st_lores_palette_peek);

    // XRES Palette Management API
    luaL_setglobalfunction(L, "xres_palette_row", lua_st_xres_palette_row);
    luaL_setglobalfunction(L, "xres_palette_global", lua_st_xres_palette_global);
    luaL_setglobalfunction(L, "xres_palette_reset", lua_st_xres_palette_reset);
    luaL_setglobalfunction(L, "XRES_PALETTE_ROW", lua_st_xres_palette_row);
    luaL_setglobalfunction(L, "XRES_PALETTE_GLOBAL", lua_st_xres_palette_global);
    luaL_setglobalfunction(L, "XRES_PALETTE_RESET", lua_st_xres_palette_reset);

    // WRES Palette Management API
    luaL_setglobalfunction(L, "wres_palette_row", lua_st_wres_palette_row);
    luaL_setglobalfunction(L, "wres_palette_global", lua_st_wres_palette_global);
    luaL_setglobalfunction(L, "wres_palette_reset", lua_st_wres_palette_reset);
    luaL_setglobalfunction(L, "WRES_PALETTE_ROW", lua_st_wres_palette_row);
    luaL_setglobalfunction(L, "WRES_PALETTE_GLOBAL", lua_st_wres_palette_global);
    luaL_setglobalfunction(L, "WRES_PALETTE_RESET", lua_st_wres_palette_reset);

    // PRES Palette Management API
    luaL_setglobalfunction(L, "pres_palette_row", lua_st_pres_palette_row);
    luaL_setglobalfunction(L, "pres_palette_global", lua_st_pres_palette_global);
    luaL_setglobalfunction(L, "pres_palette_reset", lua_st_pres_palette_reset);
    luaL_setglobalfunction(L, "PRES_PALETTE_ROW", lua_st_pres_palette_row);
    luaL_setglobalfunction(L, "PRES_PALETTE_GLOBAL", lua_st_pres_palette_global);
    luaL_setglobalfunction(L, "PRES_PALETTE_RESET", lua_st_pres_palette_reset);

    // XRES Palette Automation API
    luaL_setglobalfunction(L, "st_xres_palette_auto_gradient", lua_st_xres_palette_auto_gradient);
    luaL_setglobalfunction(L, "st_xres_palette_auto_bars", lua_st_xres_palette_auto_bars);
    luaL_setglobalfunction(L, "st_xres_palette_auto_stop", lua_st_xres_palette_auto_stop);
    luaL_setglobalfunction(L, "st_xres_palette_auto_update", lua_st_xres_palette_auto_update);
    luaL_setglobalfunction(L, "XRES_PALETTE_AUTO_GRADIENT", lua_st_xres_palette_auto_gradient);
    luaL_setglobalfunction(L, "XRES_PALETTE_AUTO_BARS", lua_st_xres_palette_auto_bars);
    luaL_setglobalfunction(L, "XRES_PALETTE_AUTO_STOP", lua_st_xres_palette_auto_stop);
    luaL_setglobalfunction(L, "XRES_PALETTE_AUTO_UPDATE", lua_st_xres_palette_auto_update);

    // WRES Palette Automation API
    luaL_setglobalfunction(L, "st_wres_palette_auto_gradient", lua_st_wres_palette_auto_gradient);
    luaL_setglobalfunction(L, "st_wres_palette_auto_bars", lua_st_wres_palette_auto_bars);
    luaL_setglobalfunction(L, "st_wres_palette_auto_stop", lua_st_wres_palette_auto_stop);
    luaL_setglobalfunction(L, "st_wres_palette_auto_update", lua_st_wres_palette_auto_update);
    luaL_setglobalfunction(L, "WRES_PALETTE_AUTO_GRADIENT", lua_st_wres_palette_auto_gradient);
    luaL_setglobalfunction(L, "WRES_PALETTE_AUTO_BARS", lua_st_wres_palette_auto_bars);
    luaL_setglobalfunction(L, "WRES_PALETTE_AUTO_STOP", lua_st_wres_palette_auto_stop);
    luaL_setglobalfunction(L, "WRES_PALETTE_AUTO_UPDATE", lua_st_wres_palette_auto_update);

    // PRES Palette Automation API
    luaL_setglobalfunction(L, "st_pres_palette_auto_gradient", lua_st_pres_palette_auto_gradient);
    luaL_setglobalfunction(L, "st_pres_palette_auto_bars", lua_st_pres_palette_auto_bars);
    luaL_setglobalfunction(L, "st_pres_palette_auto_stop", lua_st_pres_palette_auto_stop);
    luaL_setglobalfunction(L, "st_pres_palette_auto_update", lua_st_pres_palette_auto_update);
    luaL_setglobalfunction(L, "PRES_PALETTE_AUTO_GRADIENT", lua_st_pres_palette_auto_gradient);
    luaL_setglobalfunction(L, "PRES_PALETTE_AUTO_BARS", lua_st_pres_palette_auto_bars);
    luaL_setglobalfunction(L, "PRES_PALETTE_AUTO_STOP", lua_st_pres_palette_auto_stop);
    luaL_setglobalfunction(L, "PRES_PALETTE_AUTO_UPDATE", lua_st_pres_palette_auto_update);

    // Unified Palette Automation API (V commands - mode-aware)
    luaL_setglobalfunction(L, "vpalette_auto_gradient", lua_vpalette_auto_gradient);
    luaL_setglobalfunction(L, "VPALETTE_AUTO_GRADIENT", lua_vpalette_auto_gradient);
    luaL_setglobalfunction(L, "vpalette_auto_bars", lua_vpalette_auto_bars);
    luaL_setglobalfunction(L, "VPALETTE_AUTO_BARS", lua_vpalette_auto_bars);
    luaL_setglobalfunction(L, "vpalette_auto_stop", lua_vpalette_auto_stop);
    luaL_setglobalfunction(L, "VPALETTE_AUTO_STOP", lua_vpalette_auto_stop);
    luaL_setglobalfunction(L, "vpalette_auto_update", lua_vpalette_auto_update);
    luaL_setglobalfunction(L, "VPALETTE_AUTO_UPDATE", lua_vpalette_auto_update);

    // URES Mode API (Ultra Resolution 1280x720 Direct Color)
    luaL_setglobalfunction(L, "ures_pset", lua_st_ures_pset);
    luaL_setglobalfunction(L, "ures_pget", lua_st_ures_pget);
    luaL_setglobalfunction(L, "ures_clear", lua_st_ures_clear);
    luaL_setglobalfunction(L, "ures_fillrect", lua_st_ures_fillrect);
    luaL_setglobalfunction(L, "ures_hline", lua_st_ures_hline);
    luaL_setglobalfunction(L, "ures_vline", lua_st_ures_vline);
    luaL_setglobalfunction(L, "urgb", lua_st_urgb);
    luaL_setglobalfunction(L, "urgba", lua_st_urgba);

    // Color generator functions
    luaL_setglobalfunction(L, "xrgb", lua_st_xrgb);
    luaL_setglobalfunction(L, "wrgb", lua_st_wrgb);
    luaL_setglobalfunction(L, "prgb", lua_st_prgb);
    
    // Uppercase aliases to avoid BASIC parser treating them as arrays
    luaL_setglobalfunction(L, "XRGB", lua_st_xrgb);
    luaL_setglobalfunction(L, "WRGB", lua_st_wrgb);
    luaL_setglobalfunction(L, "PRGB", lua_st_prgb);
    luaL_setglobalfunction(L, "URGB", lua_st_urgb);
    luaL_setglobalfunction(L, "URGBA", lua_st_urgba);
    luaL_setglobalfunction(L, "RGB", lua_st_rgb);
    luaL_setglobalfunction(L, "RGBA", lua_st_rgba);

    // Graphics API
    luaL_setglobalfunction(L, "gfx_clear", lua_st_gfx_clear);
    luaL_setglobalfunction(L, "st_clear_all_layers", lua_st_clear_all_layers);
    luaL_setglobalfunction(L, "gfx_rect", lua_st_gfx_rect);
    luaL_setglobalfunction(L, "gfx_rect_outline", lua_st_gfx_rect_outline);
    luaL_setglobalfunction(L, "gfx_circle", lua_st_gfx_circle);
    luaL_setglobalfunction(L, "gfx_circle_outline", lua_st_gfx_circle_outline);
    luaL_setglobalfunction(L, "gfx_arc", lua_st_gfx_arc);
    luaL_setglobalfunction(L, "gfx_arc_filled", lua_st_gfx_arc_filled);
    luaL_setglobalfunction(L, "gfx_line", lua_st_gfx_line);
    luaL_setglobalfunction(L, "gfx_point", lua_st_gfx_point);
    luaL_setglobalfunction(L, "gfx_swap", lua_st_gfx_swap);

    // BASIC command aliases for graphics functions
    luaL_setglobalfunction(L, "CLRG", lua_st_gfx_clear);
    luaL_setglobalfunction(L, "SWAPGR", lua_st_gfx_swap);
    luaL_setglobalfunction(L, "LINE", lua_st_gfx_line);
    luaL_setglobalfunction(L, "RECT", lua_st_gfx_rect_outline);
    luaL_setglobalfunction(L, "RECTF", lua_st_gfx_rect);
    luaL_setglobalfunction(L, "CIRCLE", lua_st_gfx_circle_outline);
    luaL_setglobalfunction(L, "CIRCLEF", lua_st_gfx_circle);
    luaL_setglobalfunction(L, "ARC", lua_st_gfx_arc);
    luaL_setglobalfunction(L, "ARCF", lua_st_gfx_arc_filled);
    luaL_setglobalfunction(L, "PSET", lua_st_gfx_point);

    // Audio API
    luaL_setglobalfunction(L, "music_play", lua_st_music_play);
    luaL_setglobalfunction(L, "play_abc", lua_st_play_abc);
    luaL_setglobalfunction(L, "music_play_file", lua_st_music_play_file);
    luaL_setglobalfunction(L, "st_music_play_file_with_format", lua_st_music_play_file_with_format);
    luaL_setglobalfunction(L, "st_music_render_to_wav", lua_st_music_render_to_wav);
    luaL_setglobalfunction(L, "st_music_render_to_slot", lua_st_music_render_to_slot);
    luaL_setglobalfunction(L, "music_stop", lua_st_music_stop);
    luaL_setglobalfunction(L, "music_pause", lua_st_music_pause);
    luaL_setglobalfunction(L, "music_resume", lua_st_music_resume);
    luaL_setglobalfunction(L, "music_is_playing", lua_st_music_is_playing);
    luaL_setglobalfunction(L, "music_set_volume", lua_st_music_set_volume);

    // Music Bank API
    luaL_setglobalfunction(L, "music_load_string", lua_st_music_load_string);
    luaL_setglobalfunction(L, "music_load_file", lua_st_music_load_file);
    luaL_setglobalfunction(L, "music_play_id", lua_st_music_play_id);
    luaL_setglobalfunction(L, "music_exists", lua_st_music_exists);
    luaL_setglobalfunction(L, "music_get_title", lua_st_music_get_title);
    luaL_setglobalfunction(L, "music_get_composer", lua_st_music_get_composer);
    luaL_setglobalfunction(L, "music_get_key", lua_st_music_get_key);
    luaL_setglobalfunction(L, "music_get_tempo", lua_st_music_get_tempo);
    luaL_setglobalfunction(L, "music_free", lua_st_music_free);
    luaL_setglobalfunction(L, "music_free_all", lua_st_music_free_all);
    luaL_setglobalfunction(L, "music_get_count", lua_st_music_get_count);
    luaL_setglobalfunction(L, "music_get_memory", lua_st_music_get_memory);
    
    // SID Player API
    luaL_setglobalfunction(L, "sid_load_file", lua_st_sid_load_file);
    luaL_setglobalfunction(L, "sid_load_memory", lua_st_sid_load_memory);
    luaL_setglobalfunction(L, "sid_play", lua_st_sid_play);
    luaL_setglobalfunction(L, "sid_stop", lua_st_sid_stop);
    luaL_setglobalfunction(L, "sid_pause", lua_st_sid_pause);
    luaL_setglobalfunction(L, "sid_resume", lua_st_sid_resume);
    luaL_setglobalfunction(L, "sid_is_playing", lua_st_sid_is_playing);
    luaL_setglobalfunction(L, "sid_set_volume", lua_st_sid_set_volume);
    luaL_setglobalfunction(L, "sid_get_title", lua_st_sid_get_title);
    luaL_setglobalfunction(L, "sid_get_author", lua_st_sid_get_author);
    luaL_setglobalfunction(L, "sid_get_copyright", lua_st_sid_get_copyright);
    luaL_setglobalfunction(L, "sid_get_subtune_count", lua_st_sid_get_subtune_count);
    luaL_setglobalfunction(L, "sid_get_default_subtune", lua_st_sid_get_default_subtune);
    luaL_setglobalfunction(L, "sid_set_quality", lua_st_sid_set_quality);
    luaL_setglobalfunction(L, "sid_set_chip_model", lua_st_sid_set_chip_model);
    luaL_setglobalfunction(L, "sid_set_speed", lua_st_sid_set_speed);
    luaL_setglobalfunction(L, "sid_set_max_sids", lua_st_sid_set_max_sids);
    luaL_setglobalfunction(L, "sid_get_max_sids", lua_st_sid_get_max_sids);
    luaL_setglobalfunction(L, "sid_get_time", lua_st_sid_get_time);
    luaL_setglobalfunction(L, "sid_free", lua_st_sid_free);
    luaL_setglobalfunction(L, "sid_free_all", lua_st_sid_free_all);
    luaL_setglobalfunction(L, "sid_exists", lua_st_sid_exists);
    luaL_setglobalfunction(L, "sid_get_count", lua_st_sid_get_count);
    luaL_setglobalfunction(L, "sid_get_memory", lua_st_sid_get_memory);
    
    luaL_setglobalfunction(L, "synth_note", lua_st_synth_note);
    luaL_setglobalfunction(L, "synth_set_instrument", lua_st_synth_set_instrument);
    luaL_setglobalfunction(L, "synth_frequency", lua_st_synth_frequency);

    // Sound Bank API
    luaL_setglobalfunction(L, "sound_create_beep", lua_st_sound_create_beep);
    luaL_setglobalfunction(L, "sound_create_zap", lua_st_sound_create_zap);
    luaL_setglobalfunction(L, "sound_create_explode", lua_st_sound_create_explode);
    luaL_setglobalfunction(L, "sound_create_coin", lua_st_sound_create_coin);
    luaL_setglobalfunction(L, "sound_create_jump", lua_st_sound_create_jump);
    luaL_setglobalfunction(L, "sound_create_shoot", lua_st_sound_create_shoot);
    luaL_setglobalfunction(L, "sound_create_click", lua_st_sound_create_click);
    luaL_setglobalfunction(L, "sound_create_blip", lua_st_sound_create_blip);
    luaL_setglobalfunction(L, "sound_create_pickup", lua_st_sound_create_pickup);
    luaL_setglobalfunction(L, "sound_create_powerup", lua_st_sound_create_powerup);
    luaL_setglobalfunction(L, "sound_create_hurt", lua_st_sound_create_hurt);
    luaL_setglobalfunction(L, "sound_create_sweep_up", lua_st_sound_create_sweep_up);
    luaL_setglobalfunction(L, "sound_create_sweep_down", lua_st_sound_create_sweep_down);
    luaL_setglobalfunction(L, "sound_create_big_explosion", lua_st_sound_create_big_explosion);
    luaL_setglobalfunction(L, "sound_create_small_explosion", lua_st_sound_create_small_explosion);
    luaL_setglobalfunction(L, "sound_create_distant_explosion", lua_st_sound_create_distant_explosion);
    luaL_setglobalfunction(L, "sound_create_metal_explosion", lua_st_sound_create_metal_explosion);
    luaL_setglobalfunction(L, "sound_create_bang", lua_st_sound_create_bang);
    luaL_setglobalfunction(L, "sound_create_random_beep", lua_st_sound_create_random_beep);

    // Phase 3: Custom Synthesis
    luaL_setglobalfunction(L, "sound_create_tone", lua_st_sound_create_tone);
    luaL_setglobalfunction(L, "sound_create_note", lua_st_sound_create_note);
    luaL_setglobalfunction(L, "sound_create_noise", lua_st_sound_create_noise);

    // Phase 4: Advanced Synthesis
    luaL_setglobalfunction(L, "sound_create_fm", lua_st_sound_create_fm);
    luaL_setglobalfunction(L, "sound_create_filtered_tone", lua_st_sound_create_filtered_tone);
    luaL_setglobalfunction(L, "sound_create_filtered_note", lua_st_sound_create_filtered_note);

    // Phase 5: Effects Chain
    luaL_setglobalfunction(L, "sound_create_with_reverb", lua_st_sound_create_with_reverb);
    luaL_setglobalfunction(L, "sound_create_with_delay", lua_st_sound_create_with_delay);
    luaL_setglobalfunction(L, "sound_create_with_distortion", lua_st_sound_create_with_distortion);

    luaL_setglobalfunction(L, "sound_play_id", lua_st_sound_play_id);
    luaL_setglobalfunction(L, "sound_play", lua_st_sound_play_id);  // Alias for convenience
    luaL_setglobalfunction(L, "st_sound_play_with_fade", lua_st_sound_play_with_fade);
    luaL_setglobalfunction(L, "sound_free_id", lua_st_sound_free_id);
    luaL_setglobalfunction(L, "sound_free_all", lua_st_sound_free_all);
    luaL_setglobalfunction(L, "sound_exists", lua_st_sound_exists);
    luaL_setglobalfunction(L, "sound_get_count", lua_st_sound_get_count);
    luaL_setglobalfunction(L, "sound_get_memory_usage", lua_st_sound_get_memory_usage);

    // Voice Controller API
    // Waveform constants
    luaL_setglobalnumber(L, "WAVE_SILENCE", 0);
    luaL_setglobalnumber(L, "WAVE_SINE", 1);
    luaL_setglobalnumber(L, "WAVE_SQUARE", 2);
    luaL_setglobalnumber(L, "WAVE_SAW", 3);
    luaL_setglobalnumber(L, "WAVE_SAWTOOTH", 3);
    luaL_setglobalnumber(L, "WAVE_TRIANGLE", 4);
    luaL_setglobalnumber(L, "WAVE_NOISE", 5);
    luaL_setglobalnumber(L, "WAVE_PULSE", 6);
    luaL_setglobalnumber(L, "WAVE_PHYSICAL", 7);

    luaL_setglobalfunction(L, "voice_set_waveform", lua_st_voice_set_waveform);
    luaL_setglobalfunction(L, "voice_set_frequency", lua_st_voice_set_frequency);
    luaL_setglobalfunction(L, "voice_set_note", lua_st_voice_set_note);
    luaL_setglobalfunction(L, "voice_set_note_name", lua_st_voice_set_note_name);
    luaL_setglobalfunction(L, "voice_set_envelope", lua_st_voice_set_envelope);
    luaL_setglobalfunction(L, "voice_set_gate", lua_st_voice_set_gate);
    luaL_setglobalfunction(L, "voice_set_volume", lua_st_voice_set_volume);
    luaL_setglobalfunction(L, "voice_set_pulse_width", lua_st_voice_set_pulse_width);
    luaL_setglobalfunction(L, "voice_set_filter_routing", lua_st_voice_set_filter_routing);
    luaL_setglobalfunction(L, "voice_set_filter_type", lua_st_voice_set_filter_type);
    luaL_setglobalfunction(L, "voice_set_filter_cutoff", lua_st_voice_set_filter_cutoff);
    luaL_setglobalfunction(L, "voice_set_filter_resonance", lua_st_voice_set_filter_resonance);
    luaL_setglobalfunction(L, "voice_set_filter_enabled", lua_st_voice_set_filter_enabled);
    luaL_setglobalfunction(L, "voice_set_master_volume", lua_st_voice_set_master_volume);
    luaL_setglobalfunction(L, "voice_get_master_volume", lua_st_voice_get_master_volume);
    luaL_setglobalfunction(L, "voice_reset_all", lua_st_voice_reset_all);
    luaL_setglobalfunction(L, "voice_get_active_count", lua_st_voice_get_active_count);
    luaL_setglobalfunction(L, "voices_are_playing", lua_st_voices_are_playing);
    luaL_setglobalfunction(L, "voice_direct", lua_st_voice_direct);
    luaL_setglobalfunction(L, "voice_direct_slot", lua_st_voice_direct_slot);
    luaL_setglobalfunction(L, "vscript_save_to_bank", lua_st_vscript_save_to_bank);

    // Voice Controller Extended API - Stereo & Spatial
    luaL_setglobalfunction(L, "voice_set_pan", lua_st_voice_set_pan);

    // Voice Controller Extended API - SID-Style Modulation
    luaL_setglobalfunction(L, "voice_set_ring_mod", lua_st_voice_set_ring_mod);
    luaL_setglobalfunction(L, "voice_set_sync", lua_st_voice_set_sync);
    luaL_setglobalfunction(L, "voice_set_portamento", lua_st_voice_set_portamento);
    luaL_setglobalfunction(L, "voice_set_detune", lua_st_voice_set_detune);

    // Voice Controller Extended API - Delay Effects
    luaL_setglobalfunction(L, "voice_set_delay_enable", lua_st_voice_set_delay_enable);
    luaL_setglobalfunction(L, "voice_set_delay_time", lua_st_voice_set_delay_time);
    luaL_setglobalfunction(L, "voice_set_delay_feedback", lua_st_voice_set_delay_feedback);
    luaL_setglobalfunction(L, "voice_set_delay_mix", lua_st_voice_set_delay_mix);

    // Voice Controller Extended API - LFO Controls
    luaL_setglobalfunction(L, "lfo_set_waveform", lua_st_lfo_set_waveform);
    luaL_setglobalfunction(L, "lfo_set_rate", lua_st_lfo_set_rate);
    luaL_setglobalfunction(L, "lfo_reset", lua_st_lfo_reset);
    luaL_setglobalfunction(L, "lfo_to_pitch", lua_st_lfo_to_pitch);
    luaL_setglobalfunction(L, "lfo_to_volume", lua_st_lfo_to_volume);
    luaL_setglobalfunction(L, "lfo_to_filter", lua_st_lfo_to_filter);
    luaL_setglobalfunction(L, "lfo_to_pulsewidth", lua_st_lfo_to_pulsewidth);

    // Voice Controller Extended API - Physical Modeling
    luaL_setglobalfunction(L, "voice_set_physical_model", lua_st_voice_set_physical_model);
    luaL_setglobalfunction(L, "voice_set_physical_damping", lua_st_voice_set_physical_damping);
    luaL_setglobalfunction(L, "voice_set_physical_brightness", lua_st_voice_set_physical_brightness);
    luaL_setglobalfunction(L, "voice_set_physical_excitation", lua_st_voice_set_physical_excitation);
    luaL_setglobalfunction(L, "voice_set_physical_resonance", lua_st_voice_set_physical_resonance);
    luaL_setglobalfunction(L, "voice_set_physical_tension", lua_st_voice_set_physical_tension);
    luaL_setglobalfunction(L, "voice_set_physical_pressure", lua_st_voice_set_physical_pressure);
    luaL_setglobalfunction(L, "voice_physical_trigger", lua_st_voice_physical_trigger);

    // VOICES Timeline System
    luaL_setglobalfunction(L, "voices_start", lua_st_voices_start);
    luaL_setglobalfunction(L, "voice_wait", lua_st_voice_wait);
    luaL_setglobalfunction(L, "voice_wait_beats", lua_st_voice_wait_beats);
    luaL_setglobalfunction(L, "voices_set_tempo", lua_st_voices_set_tempo);
    luaL_setglobalfunction(L, "voices_end_slot", lua_st_voices_end_slot);
    luaL_setglobalfunction(L, "voices_next_slot", lua_st_voices_next_slot);
    luaL_setglobalfunction(L, "voices_end_play", lua_st_voices_end_play);
    luaL_setglobalfunction(L, "voices_end_save", lua_st_voices_end_save);

    // Input API
    luaL_setglobalfunction(L, "key_pressed", lua_st_key_pressed);
    luaL_setglobalfunction(L, "key_just_pressed", lua_st_key_just_pressed);
    luaL_setglobalfunction(L, "key_just_released", lua_st_key_just_released);
    luaL_setglobalfunction(L, "key_get_char", lua_st_key_get_char);
    luaL_setglobalfunction(L, "key_clear_buffer", lua_st_key_clear_buffer);
    luaL_setglobalfunction(L, "mouse_position", lua_st_mouse_position);
    luaL_setglobalfunction(L, "mouse_grid_position", lua_st_mouse_grid_position);
    luaL_setglobalfunction(L, "mouse_button", lua_st_mouse_button);
    luaL_setglobalfunction(L, "mouse_button_just_pressed", lua_st_mouse_button_just_pressed);
    luaL_setglobalfunction(L, "mouse_button_just_released", lua_st_mouse_button_just_released);

    // Random number generation
    luaL_setglobalfunction(L, "st_rand", lua_st_rand);

    // BASIC Input Functions
    luaL_setglobalfunction(L, "basic_input_at", lua_st_basic_input_at);

    // BASIC Print Functions
    luaL_setglobalfunction(L, "basic_print", lua_st_basic_print);
    luaL_setglobalfunction(L, "basic_console", lua_st_basic_console);
    luaL_setglobalfunction(L, "basic_print_newline", lua_st_basic_print_newline);
    
    // BASIC Cursor Functions
    luaL_setglobalfunction(L, "basic_locate", lua_st_basic_locate);

    // Frame Control API
    luaL_setglobalfunction(L, "wait_frame", lua_st_wait_frame);
    luaL_setglobalfunction(L, "wait_frames", lua_st_wait_frames);
    luaL_setglobalfunction(L, "wait_ms", lua_st_wait_ms);
    luaL_setglobalfunction(L, "wait", lua_st_wait);  // Wait for N seconds
    luaL_setglobalfunction(L, "frame_count", lua_st_frame_count);
    luaL_setglobalfunction(L, "time", lua_st_time);
    luaL_setglobalfunction(L, "delta_time", lua_st_delta_time);
    
    // Script Control API
    luaL_setglobalfunction(L, "shouldStopScript", lua_shouldStopScript);

    // Utility API
    luaL_setglobalfunction(L, "rgb", lua_st_rgb);
    luaL_setglobalfunction(L, "rgba", lua_st_rgba);
    luaL_setglobalfunction(L, "hsv", lua_st_hsv);
    luaL_setglobalfunction(L, "debug_print", lua_st_debug_print);

    // Display API
    luaL_setglobalfunction(L, "display_size", lua_st_display_size);
    luaL_setglobalfunction(L, "cell_size", lua_st_cell_size);

    // Error handling API
    luaL_setglobalfunction(L, "st_get_error", lua_st_get_error);
    luaL_setglobalfunction(L, "st_clear_error", lua_st_clear_error);

    // Key codes as constants
    luaL_setglobalnumber(L, "KEY_ESCAPE", ST_KEY_ESCAPE);
    luaL_setglobalnumber(L, "KEY_ENTER", ST_KEY_ENTER);
    luaL_setglobalnumber(L, "KEY_SPACE", ST_KEY_SPACE);
    luaL_setglobalnumber(L, "KEY_BACKSPACE", ST_KEY_BACKSPACE);
    luaL_setglobalnumber(L, "KEY_TAB", ST_KEY_TAB);
    luaL_setglobalnumber(L, "KEY_UP", ST_KEY_UP);
    luaL_setglobalnumber(L, "KEY_DOWN", ST_KEY_DOWN);
    luaL_setglobalnumber(L, "KEY_LEFT", ST_KEY_LEFT);
    luaL_setglobalnumber(L, "KEY_RIGHT", ST_KEY_RIGHT);

    // Navigation keys
    luaL_setglobalnumber(L, "KEY_INSERT", ST_KEY_INSERT);
    luaL_setglobalnumber(L, "KEY_DELETE", ST_KEY_DELETE);
    luaL_setglobalnumber(L, "KEY_HOME", ST_KEY_HOME);
    luaL_setglobalnumber(L, "KEY_END", ST_KEY_END);

    // Letters
    luaL_setglobalnumber(L, "KEY_A", ST_KEY_A);
    luaL_setglobalnumber(L, "KEY_B", ST_KEY_B);
    luaL_setglobalnumber(L, "KEY_C", ST_KEY_C);
    luaL_setglobalnumber(L, "KEY_D", ST_KEY_D);
    luaL_setglobalnumber(L, "KEY_E", ST_KEY_E);
    luaL_setglobalnumber(L, "KEY_F", ST_KEY_F);
    luaL_setglobalnumber(L, "KEY_G", ST_KEY_G);
    luaL_setglobalnumber(L, "KEY_H", ST_KEY_H);
    luaL_setglobalnumber(L, "KEY_I", ST_KEY_I);
    luaL_setglobalnumber(L, "KEY_J", ST_KEY_J);
    luaL_setglobalnumber(L, "KEY_K", ST_KEY_K);
    luaL_setglobalnumber(L, "KEY_L", ST_KEY_L);
    luaL_setglobalnumber(L, "KEY_M", ST_KEY_M);
    luaL_setglobalnumber(L, "KEY_N", ST_KEY_N);
    luaL_setglobalnumber(L, "KEY_O", ST_KEY_O);
    luaL_setglobalnumber(L, "KEY_P", ST_KEY_P);
    luaL_setglobalnumber(L, "KEY_Q", ST_KEY_Q);
    luaL_setglobalnumber(L, "KEY_R", ST_KEY_R);
    luaL_setglobalnumber(L, "KEY_S", ST_KEY_S);
    luaL_setglobalnumber(L, "KEY_T", ST_KEY_T);
    luaL_setglobalnumber(L, "KEY_U", ST_KEY_U);
    luaL_setglobalnumber(L, "KEY_V", ST_KEY_V);
    luaL_setglobalnumber(L, "KEY_W", ST_KEY_W);
    luaL_setglobalnumber(L, "KEY_X", ST_KEY_X);
    luaL_setglobalnumber(L, "KEY_Y", ST_KEY_Y);
    luaL_setglobalnumber(L, "KEY_Z", ST_KEY_Z);

    // Function keys
    luaL_setglobalnumber(L, "KEY_F1", ST_KEY_F1);
    luaL_setglobalnumber(L, "KEY_F2", ST_KEY_F2);
    luaL_setglobalnumber(L, "KEY_F3", ST_KEY_F3);
    luaL_setglobalnumber(L, "KEY_F4", ST_KEY_F4);
    luaL_setglobalnumber(L, "KEY_F5", ST_KEY_F5);
    luaL_setglobalnumber(L, "KEY_F6", ST_KEY_F6);
    luaL_setglobalnumber(L, "KEY_F7", ST_KEY_F7);
    luaL_setglobalnumber(L, "KEY_F8", ST_KEY_F8);
    luaL_setglobalnumber(L, "KEY_F9", ST_KEY_F9);
    luaL_setglobalnumber(L, "KEY_F10", ST_KEY_F10);
    luaL_setglobalnumber(L, "KEY_F11", ST_KEY_F11);
    luaL_setglobalnumber(L, "KEY_F12", ST_KEY_F12);

    // Mouse buttons
    luaL_setglobalnumber(L, "MOUSE_LEFT", ST_MOUSE_LEFT);
    luaL_setglobalnumber(L, "MOUSE_RIGHT", ST_MOUSE_RIGHT);
    luaL_setglobalnumber(L, "MOUSE_MIDDLE", ST_MOUSE_MIDDLE);

    // Rectangle Rendering API (ID-based system only)
    luaL_setglobalfunction(L, "st_rect_count", lua_st_rect_count);
    luaL_setglobalfunction(L, "st_rect_is_empty", lua_st_rect_is_empty);
    luaL_setglobalfunction(L, "st_rect_set_max", lua_st_rect_set_max);
    luaL_setglobalfunction(L, "st_rect_get_max", lua_st_rect_get_max);

    // ID-Based Rectangle Management API
    luaL_setglobalfunction(L, "st_rect_create", lua_st_rect_create);
    luaL_setglobalfunction(L, "st_rect_create_gradient", lua_st_rect_create_gradient);
    luaL_setglobalfunction(L, "st_rect_create_three_point", lua_st_rect_create_three_point);
    luaL_setglobalfunction(L, "st_rect_create_four_corner", lua_st_rect_create_four_corner);
    luaL_setglobalfunction(L, "st_rect_set_position", lua_st_rect_set_position);
    luaL_setglobalfunction(L, "st_rect_set_size", lua_st_rect_set_size);
    luaL_setglobalfunction(L, "st_rect_set_color", lua_st_rect_set_color);
    luaL_setglobalfunction(L, "st_rect_set_colors", lua_st_rect_set_colors);
    luaL_setglobalfunction(L, "st_rect_set_mode", lua_st_rect_set_mode);
    luaL_setglobalfunction(L, "st_rect_set_visible", lua_st_rect_set_visible);
    luaL_setglobalfunction(L, "st_rect_exists", lua_st_rect_exists);
    luaL_setglobalfunction(L, "st_rect_is_visible", lua_st_rect_is_visible);
    luaL_setglobalfunction(L, "st_rect_delete", lua_st_rect_delete);
    luaL_setglobalfunction(L, "st_rect_delete_all", lua_st_rect_delete_all);

    // Procedural Pattern Rectangle API
    luaL_setglobalfunction(L, "st_rect_create_outline", lua_st_rect_create_outline);
    luaL_setglobalfunction(L, "st_rect_create_dashed_outline", lua_st_rect_create_dashed_outline);
    luaL_setglobalfunction(L, "st_rect_create_horizontal_stripes", lua_st_rect_create_horizontal_stripes);
    luaL_setglobalfunction(L, "st_rect_create_vertical_stripes", lua_st_rect_create_vertical_stripes);
    luaL_setglobalfunction(L, "st_rect_create_diagonal_stripes", lua_st_rect_create_diagonal_stripes);
    luaL_setglobalfunction(L, "st_rect_create_checkerboard", lua_st_rect_create_checkerboard);
    luaL_setglobalfunction(L, "st_rect_create_dots", lua_st_rect_create_dots);
    luaL_setglobalfunction(L, "st_rect_create_crosshatch", lua_st_rect_create_crosshatch);
    luaL_setglobalfunction(L, "st_rect_create_rounded_corners", lua_st_rect_create_rounded_corners);
    luaL_setglobalfunction(L, "st_rect_create_grid", lua_st_rect_create_grid);
    luaL_setglobalfunction(L, "st_rect_set_parameters", lua_st_rect_set_parameters);

    // ID-Based Circle Management API
    luaL_setglobalfunction(L, "st_circle_create", lua_st_circle_create);
    luaL_setglobalfunction(L, "st_circle_create_radial", lua_st_circle_create_radial);
    luaL_setglobalfunction(L, "st_circle_create_radial_3", lua_st_circle_create_radial_3);
    luaL_setglobalfunction(L, "st_circle_create_radial_4", lua_st_circle_create_radial_4);
    luaL_setglobalfunction(L, "st_circle_set_position", lua_st_circle_set_position);
    luaL_setglobalfunction(L, "st_circle_set_radius", lua_st_circle_set_radius);
    luaL_setglobalfunction(L, "st_circle_set_color", lua_st_circle_set_color);
    luaL_setglobalfunction(L, "st_circle_set_colors", lua_st_circle_set_colors);
    luaL_setglobalfunction(L, "st_circle_set_parameters", lua_st_circle_set_parameters);
    luaL_setglobalfunction(L, "st_circle_set_visible", lua_st_circle_set_visible);
    luaL_setglobalfunction(L, "st_circle_exists", lua_st_circle_exists);
    luaL_setglobalfunction(L, "st_circle_is_visible", lua_st_circle_is_visible);
    luaL_setglobalfunction(L, "st_circle_delete", lua_st_circle_delete);
    luaL_setglobalfunction(L, "st_circle_delete_all", lua_st_circle_delete_all);
    luaL_setglobalfunction(L, "st_circle_count", lua_st_circle_count);
    luaL_setglobalfunction(L, "st_circle_is_empty", lua_st_circle_is_empty);
    luaL_setglobalfunction(L, "st_circle_set_max", lua_st_circle_set_max);
    luaL_setglobalfunction(L, "st_circle_get_max", lua_st_circle_get_max);

    // Procedural Pattern Circle API
    luaL_setglobalfunction(L, "st_circle_create_outline", lua_st_circle_create_outline);
    luaL_setglobalfunction(L, "st_circle_create_dashed_outline", lua_st_circle_create_dashed_outline);
    luaL_setglobalfunction(L, "st_circle_create_ring", lua_st_circle_create_ring);
    luaL_setglobalfunction(L, "st_circle_create_pie_slice", lua_st_circle_create_pie_slice);
    luaL_setglobalfunction(L, "st_circle_create_arc", lua_st_circle_create_arc);
    luaL_setglobalfunction(L, "st_circle_create_dots_ring", lua_st_circle_create_dots_ring);
    luaL_setglobalfunction(L, "st_circle_create_star_burst", lua_st_circle_create_star_burst);

    // ID-Based Line Management API
    luaL_setglobalfunction(L, "st_line_create", lua_st_line_create);
    luaL_setglobalfunction(L, "st_line_create_gradient", lua_st_line_create_gradient);
    luaL_setglobalfunction(L, "st_line_create_dashed", lua_st_line_create_dashed);
    luaL_setglobalfunction(L, "st_line_create_dotted", lua_st_line_create_dotted);
    luaL_setglobalfunction(L, "st_line_set_endpoints", lua_st_line_set_endpoints);
    luaL_setglobalfunction(L, "st_line_set_thickness", lua_st_line_set_thickness);
    luaL_setglobalfunction(L, "st_line_set_color", lua_st_line_set_color);
    luaL_setglobalfunction(L, "st_line_set_colors", lua_st_line_set_colors);
    luaL_setglobalfunction(L, "st_line_set_dash_pattern", lua_st_line_set_dash_pattern);
    luaL_setglobalfunction(L, "st_line_set_visible", lua_st_line_set_visible);
    luaL_setglobalfunction(L, "st_line_exists", lua_st_line_exists);
    luaL_setglobalfunction(L, "st_line_is_visible", lua_st_line_is_visible);
    luaL_setglobalfunction(L, "st_line_delete", lua_st_line_delete);
    luaL_setglobalfunction(L, "st_line_delete_all", lua_st_line_delete_all);
    luaL_setglobalfunction(L, "st_line_count", lua_st_line_count);
    luaL_setglobalfunction(L, "st_line_is_empty", lua_st_line_is_empty);
    luaL_setglobalfunction(L, "st_line_set_max", lua_st_line_set_max);
    luaL_setglobalfunction(L, "st_line_get_max", lua_st_line_get_max);

    // Particle System API
    luaL_setglobalfunction(L, "st_sprite_explode", lua_st_sprite_explode);
    luaL_setglobalfunction(L, "st_sprite_explode_advanced", lua_st_sprite_explode_advanced);
    luaL_setglobalfunction(L, "st_sprite_explode_directional", lua_st_sprite_explode_directional);
    luaL_setglobalfunction(L, "st_particle_clear", lua_st_particle_clear);
    luaL_setglobalfunction(L, "st_particle_pause", lua_st_particle_pause);
    luaL_setglobalfunction(L, "st_particle_resume", lua_st_particle_resume);
    luaL_setglobalfunction(L, "st_particle_set_time_scale", lua_st_particle_set_time_scale);
    luaL_setglobalfunction(L, "st_particle_set_world_bounds", lua_st_particle_set_world_bounds);
    luaL_setglobalfunction(L, "st_particle_set_enabled", lua_st_particle_set_enabled);
    luaL_setglobalfunction(L, "st_particle_get_active_count", lua_st_particle_get_active_count);
    luaL_setglobalfunction(L, "st_particle_get_total_created", lua_st_particle_get_total_created);
    luaL_setglobalfunction(L, "st_particle_dump_stats", lua_st_particle_dump_stats);

    // BASIC-style particle command aliases
    luaL_setglobalfunction(L, "PARTCLEAR", lua_st_particle_clear);
    luaL_setglobalfunction(L, "PARTPAUSE", lua_st_particle_pause);
    luaL_setglobalfunction(L, "PARTRESUME", lua_st_particle_resume);
    luaL_setglobalfunction(L, "PARTCOUNT", lua_st_particle_get_active_count);

    // Sprite Management API
    luaL_setglobalfunction(L, "sprite_load", lua_st_sprite_load);
    luaL_setglobalfunction(L, "sprite_load_builtin", lua_st_sprite_load_builtin);
    luaL_setglobalfunction(L, "sprite_begin_draw", lua_st_sprite_begin_draw);
    luaL_setglobalfunction(L, "sprite_end_draw", lua_st_sprite_end_draw);
    luaL_setglobalfunction(L, "draw_to_file_begin", lua_st_draw_to_file_begin);
    luaL_setglobalfunction(L, "draw_to_file_end", lua_st_draw_to_file_end);
    luaL_setglobalfunction(L, "tileset_begin_draw", lua_st_tileset_begin_draw);
    luaL_setglobalfunction(L, "tileset_draw_tile", lua_st_tileset_draw_tile);
    luaL_setglobalfunction(L, "tileset_end_draw", lua_st_tileset_end_draw);
    luaL_setglobalfunction(L, "sprite_show", lua_st_sprite_show);
    luaL_setglobalfunction(L, "sprite_hide", lua_st_sprite_hide);
    luaL_setglobalfunction(L, "sprite_transform", lua_st_sprite_transform);
    luaL_setglobalfunction(L, "sprite_tint", lua_st_sprite_tint);
    luaL_setglobalfunction(L, "sprite_unload", lua_st_sprite_unload);
    luaL_setglobalfunction(L, "sprite_unload_all", lua_st_sprite_unload_all);
    
    // Indexed sprite functions
    luaL_setglobalfunction(L, "sprite_load_sprtz", lua_st_sprite_load_sprtz);
    luaL_setglobalfunction(L, "sprite_is_indexed", lua_st_sprite_is_indexed);
    luaL_setglobalfunction(L, "sprite_set_standard_palette", lua_st_sprite_set_standard_palette);
    luaL_setglobalfunction(L, "sprite_set_palette_color", lua_st_sprite_set_palette_color);
    luaL_setglobalfunction(L, "sprite_rotate_palette", lua_st_sprite_rotate_palette);
    luaL_setglobalfunction(L, "sprite_adjust_brightness", lua_st_sprite_adjust_brightness);
    luaL_setglobalfunction(L, "sprite_copy_palette", lua_st_sprite_copy_palette);

    // Sprite-based Particle Explosion API (v1 compatible)
    luaL_setglobalfunction(L, "sprite_explode", lua_sprite_explode);
    luaL_setglobalfunction(L, "sprite_explode_advanced", lua_sprite_explode_advanced);
    luaL_setglobalfunction(L, "sprite_explode_directional", lua_sprite_explode_directional);
    luaL_setglobalfunction(L, "sprite_explode_mode", lua_sprite_explode_mode);
    luaL_setglobalfunction(L, "sprite_explode_size", lua_sprite_explode_size);

    // Explosion mode constants
    luaL_setglobalnumber(L, "BASIC_EXPLOSION", 1);
    luaL_setglobalnumber(L, "MASSIVE_BLAST", 2);
    luaL_setglobalnumber(L, "GENTLE_DISPERSAL", 3);
    luaL_setglobalnumber(L, "RIGHTWARD_BLAST", 4);
    luaL_setglobalnumber(L, "UPWARD_ERUPTION", 5);
    luaL_setglobalnumber(L, "RAPID_BURST", 6);

    // Particle rendering mode constants
    luaL_setglobalnumber(L, "PARTICLE_MODE_POINT_SPRITE", 0);
    luaL_setglobalnumber(L, "PARTICLE_MODE_SPRITE_FRAGMENT", 1);

    // Asset type constants
    luaL_setglobalnumber(L, "ASSET_IMAGE", ST_ASSET_IMAGE);
    luaL_setglobalnumber(L, "ASSET_SOUND", ST_ASSET_SOUND);
    luaL_setglobalnumber(L, "ASSET_MUSIC", ST_ASSET_MUSIC);
    luaL_setglobalnumber(L, "ASSET_FONT", ST_ASSET_FONT);
    luaL_setglobalnumber(L, "ASSET_SPRITE", ST_ASSET_SPRITE);
    luaL_setglobalnumber(L, "ASSET_DATA", ST_ASSET_DATA);

    // Asset Management API - Create 'asset' namespace table
    lua_newtable(L);

    // Initialization functions
    lua_pushcfunction(L, lua_st_asset_init);
    lua_setfield(L, -2, "init");

    lua_pushcfunction(L, lua_st_asset_shutdown);
    lua_setfield(L, -2, "shutdown");

    lua_pushcfunction(L, lua_st_asset_is_initialized);
    lua_setfield(L, -2, "isInitialized");

    // Loading/Unloading functions
    lua_pushcfunction(L, lua_st_asset_load);
    lua_setfield(L, -2, "load");

    lua_pushcfunction(L, lua_st_asset_load_file);
    lua_setfield(L, -2, "loadFile");

    lua_pushcfunction(L, lua_st_asset_unload);
    lua_setfield(L, -2, "unload");

    lua_pushcfunction(L, lua_st_asset_is_loaded);
    lua_setfield(L, -2, "isLoaded");

    // Import/Export functions
    lua_pushcfunction(L, lua_st_asset_import);
    lua_setfield(L, -2, "import");

    lua_pushcfunction(L, lua_st_asset_import_directory);
    lua_setfield(L, -2, "importDirectory");

    lua_pushcfunction(L, lua_st_asset_export);
    lua_setfield(L, -2, "export");

    lua_pushcfunction(L, lua_st_asset_delete);
    lua_setfield(L, -2, "delete");

    // Data access functions
    lua_pushcfunction(L, lua_st_asset_get_data);
    lua_setfield(L, -2, "getData");

    lua_pushcfunction(L, lua_st_asset_get_size);
    lua_setfield(L, -2, "getSize");

    lua_pushcfunction(L, lua_st_asset_get_type);
    lua_setfield(L, -2, "getType");

    lua_pushcfunction(L, lua_st_asset_get_name);
    lua_setfield(L, -2, "getName");

    // Query functions
    lua_pushcfunction(L, lua_st_asset_exists);
    lua_setfield(L, -2, "exists");

    lua_pushcfunction(L, lua_st_asset_list);
    lua_setfield(L, -2, "list");

    lua_pushcfunction(L, lua_st_asset_search);
    lua_setfield(L, -2, "search");

    lua_pushcfunction(L, lua_st_asset_get_count);
    lua_setfield(L, -2, "getCount");

    // Cache management functions
    lua_pushcfunction(L, lua_st_asset_clear_cache);
    lua_setfield(L, -2, "clearCache");

    lua_pushcfunction(L, lua_st_asset_get_cache_size);
    lua_setfield(L, -2, "getCacheSize");

    lua_pushcfunction(L, lua_st_asset_get_cached_count);
    lua_setfield(L, -2, "getCachedCount");

    lua_pushcfunction(L, lua_st_asset_set_max_cache_size);
    lua_setfield(L, -2, "setMaxCacheSize");

    // Statistics functions
    lua_pushcfunction(L, lua_st_asset_get_hit_rate);
    lua_setfield(L, -2, "getHitRate");

    lua_pushcfunction(L, lua_st_asset_get_database_size);
    lua_setfield(L, -2, "getDatabaseSize");

    // Error handling functions
    lua_pushcfunction(L, lua_st_asset_get_error);
    lua_setfield(L, -2, "getError");

    lua_pushcfunction(L, lua_st_asset_clear_error);
    lua_setfield(L, -2, "clearError");

    // Set the 'asset' global table
    lua_setglobal(L, "asset");

    // Tilemap API - Create 'tilemap' namespace table
    lua_newtable(L);

    // System management
    lua_pushcfunction(L, lua_st_tilemap_init);
    lua_setfield(L, -2, "init");

    lua_pushcfunction(L, lua_st_tilemap_shutdown);
    lua_setfield(L, -2, "shutdown");

    // Tilemap management
    lua_pushcfunction(L, lua_st_tilemap_create);
    lua_setfield(L, -2, "create");

    lua_pushcfunction(L, lua_st_tilemap_destroy);
    lua_setfield(L, -2, "destroy");

    lua_pushcfunction(L, lua_st_tilemap_get_size);
    lua_setfield(L, -2, "getSize");

    // Layer management
    lua_pushcfunction(L, lua_st_tilemap_create_layer);
    lua_setfield(L, -2, "createLayer");

    lua_pushcfunction(L, lua_st_tilemap_destroy_layer);
    lua_setfield(L, -2, "destroyLayer");

    lua_pushcfunction(L, lua_st_tilemap_layer_set_tilemap);
    lua_setfield(L, -2, "layerSetTilemap");

    lua_pushcfunction(L, lua_st_tilemap_layer_set_tileset);
    lua_setfield(L, -2, "layerSetTileset");

    lua_pushcfunction(L, lua_st_tilemap_layer_set_parallax);
    lua_setfield(L, -2, "layerSetParallax");

    lua_pushcfunction(L, lua_st_tilemap_layer_set_opacity);
    lua_setfield(L, -2, "layerSetOpacity");

    lua_pushcfunction(L, lua_st_tilemap_layer_set_visible);
    lua_setfield(L, -2, "layerSetVisible");

    lua_pushcfunction(L, lua_st_tilemap_layer_set_z_order);
    lua_setfield(L, -2, "layerSetZOrder");

    lua_pushcfunction(L, lua_st_tilemap_layer_set_auto_scroll);
    lua_setfield(L, -2, "layerSetAutoScroll");

    // Tile manipulation
    lua_pushcfunction(L, lua_st_tilemap_set_tile);
    lua_setfield(L, -2, "setTile");

    lua_pushcfunction(L, lua_st_tilemap_get_tile);
    lua_setfield(L, -2, "getTile");

    lua_pushcfunction(L, lua_st_tilemap_fill_rect);
    lua_setfield(L, -2, "fillRect");

    lua_pushcfunction(L, lua_st_tilemap_clear);
    lua_setfield(L, -2, "clear");

    // Camera control
    lua_pushcfunction(L, lua_st_tilemap_set_camera);
    lua_setfield(L, -2, "setCamera");

    lua_pushcfunction(L, lua_st_tilemap_move_camera);
    lua_setfield(L, -2, "moveCamera");

    lua_pushcfunction(L, lua_st_tilemap_get_camera);
    lua_setfield(L, -2, "getCamera");

    lua_pushcfunction(L, lua_st_tilemap_set_zoom);
    lua_setfield(L, -2, "setZoom");

    lua_pushcfunction(L, lua_st_tilemap_camera_follow);
    lua_setfield(L, -2, "cameraFollow");

    lua_pushcfunction(L, lua_st_tilemap_set_camera_bounds);
    lua_setfield(L, -2, "setCameraBounds");

    lua_pushcfunction(L, lua_st_tilemap_camera_shake);
    lua_setfield(L, -2, "cameraShake");

    // Update
    lua_pushcfunction(L, lua_st_tilemap_update);
    lua_setfield(L, -2, "update");

    // Coordinate conversion
    lua_pushcfunction(L, lua_st_tilemap_world_to_tile);
    lua_setfield(L, -2, "worldToTile");

    lua_pushcfunction(L, lua_st_tilemap_tile_to_world);
    lua_setfield(L, -2, "tileToWorld");

    // Tileset management
    lua_pushcfunction(L, lua_st_tileset_load);
    lua_setfield(L, -2, "loadTileset");

    lua_pushcfunction(L, lua_st_tileset_load_asset);
    lua_setfield(L, -2, "loadTilesetAsset");

    lua_pushcfunction(L, lua_st_tileset_destroy);
    lua_setfield(L, -2, "destroyTileset");

    lua_pushcfunction(L, lua_st_tileset_get_tile_count);
    lua_setfield(L, -2, "getTilesetTileCount");

    lua_pushcfunction(L, lua_st_tileset_get_dimensions);
    lua_setfield(L, -2, "getTilesetDimensions");

    // Set the 'tilemap' global table
    lua_setglobal(L, "tilemap");

    // =============================================================================
    // Unified Video Mode API (V-commands)
    // =============================================================================
    
    luaL_setglobalfunction(L, "video_pset", lua_video_pset);
    luaL_setglobalfunction(L, "video_pget", lua_video_pget);
    luaL_setglobalfunction(L, "video_clear", lua_video_clear);
    luaL_setglobalfunction(L, "video_line", lua_video_line);
    luaL_setglobalfunction(L, "video_rect", lua_video_rect);
    luaL_setglobalfunction(L, "video_circle", lua_video_circle);
    luaL_setglobalfunction(L, "video_swap", lua_video_swap);
    luaL_setglobalfunction(L, "VSWAP", lua_video_swap);
    luaL_setglobalfunction(L, "vpalette_row", lua_vpalette_row);
    luaL_setglobalfunction(L, "VPALETTE_ROW", lua_vpalette_row);
    luaL_setglobalfunction(L, "video_blit", lua_video_blit);
    luaL_setglobalfunction(L, "video_blit_trans", lua_video_blit_trans);
    luaL_setglobalfunction(L, "video_buffer", lua_video_buffer);
    luaL_setglobalfunction(L, "video_buffer_get", lua_video_buffer_get);
    luaL_setglobalfunction(L, "video_get_active_buffer", lua_video_get_active_buffer);
    luaL_setglobalfunction(L, "video_get_display_buffer", lua_video_get_display_buffer);
    luaL_setglobalfunction(L, "video_flip", lua_video_flip);
    luaL_setglobalfunction(L, "video_mode_get", lua_video_mode_get);
    luaL_setglobalfunction(L, "video_mode_name", lua_video_mode_name);
    luaL_setglobalfunction(L, "video_get_color_depth", lua_video_get_color_depth);
    luaL_setglobalfunction(L, "video_has_palette", lua_video_has_palette);
    luaL_setglobalfunction(L, "video_has_gpu", lua_video_has_gpu);
    luaL_setglobalfunction(L, "video_max_buffers", lua_video_max_buffers);
    
    // GPU-accelerated commands
    luaL_setglobalfunction(L, "video_clear_gpu", lua_video_clear_gpu);
    luaL_setglobalfunction(L, "video_line_gpu", lua_video_line_gpu);
    luaL_setglobalfunction(L, "video_rect_gpu", lua_video_rect_gpu);
    luaL_setglobalfunction(L, "video_circle_gpu", lua_video_circle_gpu);
    luaL_setglobalfunction(L, "video_blit_gpu", lua_video_blit_gpu);
    
    // Palette commands
    luaL_setglobalfunction(L, "video_palette_set", lua_video_palette_set);
    luaL_setglobalfunction(L, "video_palette_set_row", lua_video_palette_set_row);
    luaL_setglobalfunction(L, "video_palette_get", lua_video_palette_get);
    luaL_setglobalfunction(L, "video_palette_reset", lua_video_palette_reset);
    
    // Batch rendering
    luaL_setglobalfunction(L, "video_begin_batch", lua_video_begin_batch);
    luaL_setglobalfunction(L, "video_end_batch", lua_video_end_batch);
    
    // GPU batch with auto-promotion
    luaL_setglobalfunction(L, "video_gpu_begin", lua_video_gpu_begin);
    luaL_setglobalfunction(L, "video_gpu_end", lua_video_gpu_end);
    
    // Anti-aliased commands
    luaL_setglobalfunction(L, "video_line_aa", lua_video_line_aa);
    luaL_setglobalfunction(L, "video_circle_aa", lua_video_circle_aa);
    
    // Gradient commands
    luaL_setglobalfunction(L, "video_rect_gradient", lua_video_rect_gradient);
    luaL_setglobalfunction(L, "video_rect_gradient_gpu", lua_video_rect_gradient_gpu);
    luaL_setglobalfunction(L, "video_rect_gradient_h", lua_video_rect_gradient_h);
    luaL_setglobalfunction(L, "video_rect_gradient_v", lua_video_rect_gradient_v);
    luaL_setglobalfunction(L, "video_circle_gradient", lua_video_circle_gradient);
    luaL_setglobalfunction(L, "video_circle_gradient_gpu", lua_video_circle_gradient_gpu);
    luaL_setglobalfunction(L, "video_circle_gradient_aa", lua_video_circle_gradient_aa);
}

// =============================================================================
// Voice-Only Bindings (for terminal tools without GUI)
// =============================================================================

void registerVoiceBindings(lua_State* L) {
    // Waveform constants
    luaL_setglobalnumber(L, "WAVE_SILENCE", 0);
    luaL_setglobalnumber(L, "WAVE_SINE", 1);
    luaL_setglobalnumber(L, "WAVE_SQUARE", 2);
    luaL_setglobalnumber(L, "WAVE_SAW", 3);
    luaL_setglobalnumber(L, "WAVE_SAWTOOTH", 3);
    luaL_setglobalnumber(L, "WAVE_TRIANGLE", 4);
    luaL_setglobalnumber(L, "WAVE_NOISE", 5);
    luaL_setglobalnumber(L, "WAVE_PULSE", 6);
    luaL_setglobalnumber(L, "WAVE_PHYSICAL", 7);

    // Voice Controller API - Basic Controls
    luaL_setglobalfunction(L, "voice_set_waveform", lua_st_voice_set_waveform);
    luaL_setglobalfunction(L, "voice_set_frequency", lua_st_voice_set_frequency);
    luaL_setglobalfunction(L, "voice_set_note", lua_st_voice_set_note);
    luaL_setglobalfunction(L, "voice_set_note_name", lua_st_voice_set_note_name);
    luaL_setglobalfunction(L, "voice_set_envelope", lua_st_voice_set_envelope);
    luaL_setglobalfunction(L, "voice_set_gate", lua_st_voice_set_gate);
    luaL_setglobalfunction(L, "voice_set_volume", lua_st_voice_set_volume);
    luaL_setglobalfunction(L, "voice_set_pulse_width", lua_st_voice_set_pulse_width);
    
    // Voice Controller API - Filter
    luaL_setglobalfunction(L, "voice_set_filter_routing", lua_st_voice_set_filter_routing);
    luaL_setglobalfunction(L, "voice_set_filter_type", lua_st_voice_set_filter_type);
    luaL_setglobalfunction(L, "voice_set_filter_cutoff", lua_st_voice_set_filter_cutoff);
    luaL_setglobalfunction(L, "voice_set_filter_resonance", lua_st_voice_set_filter_resonance);
    luaL_setglobalfunction(L, "voice_set_filter_enabled", lua_st_voice_set_filter_enabled);
    
    // Voice Controller API - Master Controls
    luaL_setglobalfunction(L, "voice_set_master_volume", lua_st_voice_set_master_volume);
    luaL_setglobalfunction(L, "voice_get_master_volume", lua_st_voice_get_master_volume);
    luaL_setglobalfunction(L, "voice_reset_all", lua_st_voice_reset_all);
    luaL_setglobalfunction(L, "voice_get_active_count", lua_st_voice_get_active_count);
    luaL_setglobalfunction(L, "voices_are_playing", lua_st_voices_are_playing);
    
    // Voice Controller API - Rendering
    luaL_setglobalfunction(L, "voice_direct", lua_st_voice_direct);
    luaL_setglobalfunction(L, "voice_direct_slot", lua_st_voice_direct_slot);
    luaL_setglobalfunction(L, "vscript_save_to_bank", lua_st_vscript_save_to_bank);

    // Voice Controller Extended API - Stereo & Spatial
    luaL_setglobalfunction(L, "voice_set_pan", lua_st_voice_set_pan);

    // Voice Controller Extended API - SID-Style Modulation
    luaL_setglobalfunction(L, "voice_set_ring_mod", lua_st_voice_set_ring_mod);
    luaL_setglobalfunction(L, "voice_set_sync", lua_st_voice_set_sync);
    luaL_setglobalfunction(L, "voice_set_portamento", lua_st_voice_set_portamento);
    luaL_setglobalfunction(L, "voice_set_detune", lua_st_voice_set_detune);

    // Voice Controller Extended API - Delay Effects
    luaL_setglobalfunction(L, "voice_set_delay_enable", lua_st_voice_set_delay_enable);
    luaL_setglobalfunction(L, "voice_set_delay_time", lua_st_voice_set_delay_time);
    luaL_setglobalfunction(L, "voice_set_delay_feedback", lua_st_voice_set_delay_feedback);
    luaL_setglobalfunction(L, "voice_set_delay_mix", lua_st_voice_set_delay_mix);

    // Voice Controller Extended API - LFO Controls
    luaL_setglobalfunction(L, "lfo_set_waveform", lua_st_lfo_set_waveform);
    luaL_setglobalfunction(L, "lfo_set_rate", lua_st_lfo_set_rate);
    luaL_setglobalfunction(L, "lfo_reset", lua_st_lfo_reset);
    luaL_setglobalfunction(L, "lfo_to_pitch", lua_st_lfo_to_pitch);
    luaL_setglobalfunction(L, "lfo_to_volume", lua_st_lfo_to_volume);
    luaL_setglobalfunction(L, "lfo_to_filter", lua_st_lfo_to_filter);
    luaL_setglobalfunction(L, "lfo_to_pulsewidth", lua_st_lfo_to_pulsewidth);

    // Voice Controller Extended API - Physical Modeling
    luaL_setglobalfunction(L, "voice_set_physical_model", lua_st_voice_set_physical_model);
    luaL_setglobalfunction(L, "voice_set_physical_damping", lua_st_voice_set_physical_damping);
    luaL_setglobalfunction(L, "voice_set_physical_brightness", lua_st_voice_set_physical_brightness);
    luaL_setglobalfunction(L, "voice_set_physical_excitation", lua_st_voice_set_physical_excitation);
    luaL_setglobalfunction(L, "voice_set_physical_resonance", lua_st_voice_set_physical_resonance);
    luaL_setglobalfunction(L, "voice_set_physical_tension", lua_st_voice_set_physical_tension);
    luaL_setglobalfunction(L, "voice_set_physical_pressure", lua_st_voice_set_physical_pressure);
    luaL_setglobalfunction(L, "voice_physical_trigger", lua_st_voice_physical_trigger);

    // VOICES Timeline System
    luaL_setglobalfunction(L, "voices_start", lua_st_voices_start);
    luaL_setglobalfunction(L, "voice_wait", lua_st_voice_wait);
    luaL_setglobalfunction(L, "voice_wait_beats", lua_st_voice_wait_beats);
    luaL_setglobalfunction(L, "voices_set_tempo", lua_st_voices_set_tempo);
    luaL_setglobalfunction(L, "voices_end_slot", lua_st_voices_end_slot);
    luaL_setglobalfunction(L, "voices_next_slot", lua_st_voices_next_slot);
    luaL_setglobalfunction(L, "voices_end_play", lua_st_voices_end_play);
    luaL_setglobalfunction(L, "voices_end_save", lua_st_voices_end_save);
}

} // namespace FBTBindings
} // namespace SuperTerminal
