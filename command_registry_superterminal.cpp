//
// command_registry_superterminal.cpp
// FBRunner3 - SuperTerminal BASIC Commands Registry Implementation
//
// Implements SuperTerminal-specific BASIC commands and functions for FBRunner3.
// These commands provide access to SuperTerminal's graphics, audio, input,
// and other multimedia features.
//

#include "command_registry_superterminal.h"
#include "../FasterBASICT/src/command_registry_core.h"

namespace FBRunner3 {
namespace SuperTerminalCommands {

using namespace FasterBASIC::ModularCommands;

// =============================================================================
// SuperTerminal Command Registration
// =============================================================================

void SuperTerminalCommandRegistry::registerSuperTerminalCommands(CommandRegistry& registry) {
    registerTextCommands(registry);
    registerGraphicsCommands(registry);
    registerAudioCommands(registry);
    registerInputCommands(registry);
    registerUtilityCommands(registry);
    registerSpriteCommands(registry);
    registerParticleCommands(registry);
    registerChunkyGraphicsCommands(registry);
    registerSixelCommands(registry);
    registerTilemapCommands(registry);
    registerRectangleCommands(registry);
    registerCircleCommands(registry);
    registerLineCommands(registry);
    registerVideoModeCommands(registry);
}

void SuperTerminalCommandRegistry::registerSuperTerminalFunctions(CommandRegistry& registry) {
    registerTilemapFunctions(registry);
    registerSystemFunctions(registry);
    registerSpriteFunctions(registry);
    registerSoundBankFunctions(registry);
    registerRectangleFunctions(registry);
    registerCircleFunctions(registry);
    registerLineFunctions(registry);
    registerVoiceFunctions(registry);
    registerVideoModeFunctions(registry);
}

// =============================================================================
// Text Commands
// =============================================================================

void SuperTerminalCommandRegistry::registerTextCommands(CommandRegistry& registry) {
    registerPrintAtCommands(registry);
    registerTextPositioningCommands(registry);
    registerTextManipulationCommands(registry);
}

void SuperTerminalCommandRegistry::registerPrintAtCommands(CommandRegistry& registry) {
    // PRINT_AT is handled natively by the compiler with complex PRINT-style syntax
    // Do not register it in the modular command system

    // TEXTPUT - Direct text positioning with colors
    CommandDefinition textput("TEXTPUT",
                             "Put text at specific screen coordinates with colors",
                             "text_put", "text");
    textput.addParameter("x", ParameterType::INT, "X coordinate (column)")
           .addParameter("y", ParameterType::INT, "Y coordinate (row)")
           .addParameter("text", ParameterType::STRING, "Text to display")
           .addParameter("fg", ParameterType::COLOR, "Foreground color")
           .addParameter("bg", ParameterType::COLOR, "Background color");
    registry.registerCommand(std::move(textput));
}

void SuperTerminalCommandRegistry::registerTextPositioningCommands(CommandRegistry& registry) {
    // AT - Position text cursor
    CommandDefinition at("AT",
                        "Position text cursor at coordinates",
                        "", "text");  // Special handling - sets cursor vars
    at.addParameter("x", ParameterType::INT, "X coordinate (column)")
      .addParameter("y", ParameterType::INT, "Y coordinate (row)")
      .setCustomCodeGen("_cursor_x = {0}; _cursor_y = {1}");
    registry.registerCommand(std::move(at));

    // LOCATE - Position text cursor (QuickBASIC style)
    CommandDefinition locate("LOCATE",
                           "Position text cursor at coordinates (QuickBASIC style)",
                           "", "text");  // Special handling - sets cursor vars
    locate.addParameter("x", ParameterType::INT, "X coordinate (column)")
          .addParameter("y", ParameterType::INT, "Y coordinate (row)")
          .setCustomCodeGen("_cursor_x = {0}; _cursor_y = {1}");
    registry.registerCommand(std::move(locate));
}

void SuperTerminalCommandRegistry::registerTextManipulationCommands(CommandRegistry& registry) {
    // TCHAR - Put single character with colors
    CommandDefinition tchar("TCHAR",
                           "Put single character at specific coordinates with colors",
                           "text_putchar", "text");
    tchar.addParameter("x", ParameterType::INT, "X coordinate (column)")
         .addParameter("y", ParameterType::INT, "Y coordinate (row)")
         .addParameter("ch", ParameterType::STRING, "Character to display")
         .addParameter("fg", ParameterType::COLOR, "Foreground color")
         .addParameter("bg", ParameterType::COLOR, "Background color");
    registry.registerCommand(std::move(tchar));

    // TGRID - Set text grid size
    CommandDefinition tgrid("TGRID",
                           "Set the size of the text grid",
                           "text_set_size", "text");
    tgrid.addParameter("w", ParameterType::INT, "Grid width in characters")
         .addParameter("h", ParameterType::INT, "Grid height in characters");
    registry.registerCommand(std::move(tgrid));

    // TSCROLL - Scroll text
    CommandDefinition tscroll("TSCROLL",
                             "Scroll text by specified amount",
                             "text_scroll", "text");
    tscroll.addParameter("amount", ParameterType::INT, "Scroll amount");
    registry.registerCommand(std::move(tscroll));

    // TCLEAR - Clear text region
    CommandDefinition tclear("TCLEAR",
                           "Clear a rectangular region of text",
                           "text_clear_region", "text");
    tclear.addParameter("x", ParameterType::INT, "X coordinate (column)")
          .addParameter("y", ParameterType::INT, "Y coordinate (row)")
          .addParameter("w", ParameterType::INT, "Width")
          .addParameter("h", ParameterType::INT, "Height");
    registry.registerCommand(std::move(tclear));

    // DISPLAYTEXT - Display overlay text with transformations
    CommandDefinition displaytext("DISPLAYTEXT",
                                 "Display overlay text at pixel coordinates with scale and rotation",
                                 "text_display_at", "text");
    displaytext.addParameter("x", ParameterType::FLOAT, "X pixel coordinate")
               .addParameter("y", ParameterType::FLOAT, "Y pixel coordinate")
               .addParameter("text", ParameterType::STRING, "Text to display")
               .addParameter("scale_x", ParameterType::FLOAT, "X scale factor", true, "1.0")
               .addParameter("scale_y", ParameterType::FLOAT, "Y scale factor", true, "1.0")
               .addParameter("rotation", ParameterType::FLOAT, "Rotation in degrees", true, "0.0")
               .addParameter("color", ParameterType::COLOR, "Text color", true, "0xFFFFFFFF")
               .addParameter("alignment", ParameterType::INT, "Text alignment (0=left, 1=center, 2=right)", true, "0")
               .addParameter("layer", ParameterType::INT, "Display layer (higher = on top)", true, "0");
    registry.registerCommand(std::move(displaytext));

    // DISPLAYTEXT_SHEAR - Display overlay text with full transformations including shear
    CommandDefinition displaytext_shear("DISPLAYTEXT_SHEAR",
                                       "Display overlay text with scale, rotation, and shear",
                                       "text_display_shear", "text");
    displaytext_shear.addParameter("x", ParameterType::FLOAT, "X pixel coordinate")
                     .addParameter("y", ParameterType::FLOAT, "Y pixel coordinate")
                     .addParameter("text", ParameterType::STRING, "Text to display")
                     .addParameter("scale_x", ParameterType::FLOAT, "X scale factor", true, "1.0")
                     .addParameter("scale_y", ParameterType::FLOAT, "Y scale factor", true, "1.0")
                     .addParameter("rotation", ParameterType::FLOAT, "Rotation in degrees", true, "0.0")
                     .addParameter("shear_x", ParameterType::FLOAT, "X shear factor", true, "0.0")
                     .addParameter("shear_y", ParameterType::FLOAT, "Y shear factor", true, "0.0")
                     .addParameter("color", ParameterType::COLOR, "Text color", true, "0xFFFFFFFF")
                     .addParameter("alignment", ParameterType::INT, "Text alignment (0=left, 1=center, 2=right)", true, "0")
                     .addParameter("layer", ParameterType::INT, "Display layer (higher = on top)", true, "0");
    registry.registerCommand(std::move(displaytext_shear));

    // DISPLAYTEXT_EFFECTS - Display overlay text with visual effects
    CommandDefinition displaytext_effects("DISPLAYTEXT_EFFECTS",
                                         "Display overlay text with visual effects (shadow, outline, glow, etc.)",
                                         "text_display_with_effects", "text");
    displaytext_effects.addParameter("x", ParameterType::FLOAT, "X pixel coordinate")
                       .addParameter("y", ParameterType::FLOAT, "Y pixel coordinate")
                       .addParameter("text", ParameterType::STRING, "Text to display")
                       .addParameter("scale_x", ParameterType::FLOAT, "Horizontal scale factor", true, "1.0")
                       .addParameter("scale_y", ParameterType::FLOAT, "Vertical scale factor", true, "1.0")
                       .addParameter("rotation", ParameterType::FLOAT, "Rotation in degrees", true, "0.0")
                       .addParameter("color", ParameterType::INT, "Text color (RGBA)", true, "0xFFFFFFFF")
                       .addParameter("alignment", ParameterType::INT, "Text alignment (0=left, 1=center, 2=right)", true, "0")
                       .addParameter("layer", ParameterType::INT, "Display layer (higher = on top)", true, "0")
                       .addParameter("effect", ParameterType::INT, "Effect type (0=none, 1=shadow, 2=outline, 3=glow, 4=gradient, 5=wave, 6=neon)", true, "0")
                       .addParameter("effect_color", ParameterType::INT, "Effect color (RGBA)", true, "0x000000FF")
                       .addParameter("effect_intensity", ParameterType::FLOAT, "Effect intensity (0.0-1.0)", true, "0.5")
                       .addParameter("effect_size", ParameterType::FLOAT, "Effect size (pixels)", true, "2.0");
    registry.registerCommand(std::move(displaytext_effects));

    // REMOVE_DISPLAYTEXT - Remove a displayed text item
    CommandDefinition remove_displaytext("REMOVE_DISPLAYTEXT",
                                       "Remove a displayed text item by ID",
                                       "text_remove_item", "text");
    remove_displaytext.addParameter("item_id", ParameterType::INT, "Text item ID to remove");
    registry.registerCommand(std::move(remove_displaytext));

    // CLEAR_DISPLAYTEXT - Clear all displayed text items
    CommandDefinition clear_displaytext("CLEAR_DISPLAYTEXT",
                                      "Clear all displayed text items",
                                      "text_clear_displayed", "text");
    registry.registerCommand(std::move(clear_displaytext));

    // UPDATE_DISPLAYTEXT - Update an existing displayed text item
    CommandDefinition update_displaytext("UPDATE_DISPLAYTEXT",
                                       "Update text and/or position of displayed text item",
                                       "text_update_item", "text");
    update_displaytext.addParameter("item_id", ParameterType::INT, "Text item ID to update")
                      .addParameter("text", ParameterType::STRING, "New text (empty string = no change)", true, "")
                      .addParameter("x", ParameterType::FLOAT, "New X coordinate (-1 = no change)", true, "-1")
                      .addParameter("y", ParameterType::FLOAT, "New Y coordinate (-1 = no change)", true, "-1");
    registry.registerCommand(std::move(update_displaytext));

    // SET_DISPLAYTEXT_VISIBLE - Set visibility of a displayed text item
    CommandDefinition set_displaytext_visible("SET_DISPLAYTEXT_VISIBLE",
                                             "Set visibility of a displayed text item",
                                             "text_set_item_visible", "text");
    set_displaytext_visible.addParameter("item_id", ParameterType::INT, "Text item ID")
                           .addParameter("visible", ParameterType::BOOL, "Visibility flag");
    registry.registerCommand(std::move(set_displaytext_visible));

    // SET_DISPLAYTEXT_LAYER - Set layer of a displayed text item
    CommandDefinition set_displaytext_layer("SET_DISPLAYTEXT_LAYER",
                                           "Set display layer of a text item",
                                           "text_set_item_layer", "text");
    set_displaytext_layer.addParameter("item_id", ParameterType::INT, "Text item ID")
                         .addParameter("layer", ParameterType::INT, "Display layer (higher = on top)");
    registry.registerCommand(std::move(set_displaytext_layer));
}

// =============================================================================
// Graphics Commands
// =============================================================================

void SuperTerminalCommandRegistry::registerGraphicsCommands(CommandRegistry& registry) {
    registerBasicGraphicsCommands(registry);
    registerAdvancedGraphicsCommands(registry);
    registerRectangleCommands(registry);
    registerCircleCommands(registry);
    registerLineCommands(registry);
}

void SuperTerminalCommandRegistry::registerBasicGraphicsCommands(CommandRegistry& registry) {
    // PSET - Set pixel
    CommandDefinition pset("PSET",
                          "Set a pixel at coordinates with optional color",
                          "gfx_point", "graphics");
    pset.addParameter("x", ParameterType::INT, "X coordinate")
        .addParameter("y", ParameterType::INT, "Y coordinate")
        .addParameter("color", ParameterType::COLOR, "Pixel color", true, "0xFFFFFFFF");
    registry.registerCommand(std::move(pset));

    // LINE - Draw line
    CommandDefinition line("LINE",
                          "Draw a line between two points",
                          "gfx_line", "graphics");
    line.addParameter("x1", ParameterType::INT, "Start X coordinate")
        .addParameter("y1", ParameterType::INT, "Start Y coordinate")
        .addParameter("x2", ParameterType::INT, "End X coordinate")
        .addParameter("y2", ParameterType::INT, "End Y coordinate")
        .addParameter("color", ParameterType::COLOR, "Line color", true, "0xFFFFFFFF");
    registry.registerCommand(std::move(line));

    // RECT - Rectangle outline
    CommandDefinition rect("RECT",
                          "Draw a rectangle outline",
                          "gfx_rect_outline", "graphics");
    rect.addParameter("x", ParameterType::INT, "X coordinate")
        .addParameter("y", ParameterType::INT, "Y coordinate")
        .addParameter("w", ParameterType::INT, "Width")
        .addParameter("h", ParameterType::INT, "Height")
        .addParameter("color", ParameterType::COLOR, "Rectangle color", true, "0xFFFFFFFF");
    registry.registerCommand(std::move(rect));

    // RECTF - Filled rectangle
    CommandDefinition rectf("RECTF",
                           "Draw a filled rectangle",
                           "gfx_rect", "graphics");
    rectf.addParameter("x", ParameterType::INT, "X coordinate")
         .addParameter("y", ParameterType::INT, "Y coordinate")
         .addParameter("w", ParameterType::INT, "Width")
         .addParameter("h", ParameterType::INT, "Height")
         .addParameter("color", ParameterType::COLOR, "Fill color", true, "0xFFFFFFFF");
    registry.registerCommand(std::move(rectf));

    // SWAP - Swap graphics buffers (double buffering)
    CommandDefinition swap("SWAP",
                          "Swap front and back graphics buffers for smooth animation",
                          "gfx_swap", "graphics");
    registry.registerCommand(std::move(swap));
}

void SuperTerminalCommandRegistry::registerAdvancedGraphicsCommands(CommandRegistry& registry) {
    // CIRCLE - Circle outline
    CommandDefinition circle("CIRCLE",
                            "Draw a circle outline",
                            "gfx_circle_outline", "graphics");
    circle.addParameter("x", ParameterType::INT, "Center X coordinate")
          .addParameter("y", ParameterType::INT, "Center Y coordinate")
          .addParameter("r", ParameterType::INT, "Radius")
          .addParameter("color", ParameterType::COLOR, "Circle color", true, "0xFFFFFFFF");
    registry.registerCommand(std::move(circle));

    // CIRCLEF - Filled circle
    CommandDefinition circlef("CIRCLEF",
                             "Draw a filled circle",
                             "gfx_circle", "graphics");
    circlef.addParameter("x", ParameterType::INT, "Center X coordinate")
           .addParameter("y", ParameterType::INT, "Center Y coordinate")
           .addParameter("r", ParameterType::INT, "Radius")
           .addParameter("color", ParameterType::COLOR, "Fill color", true, "0xFFFFFFFF");
    registry.registerCommand(std::move(circlef));

    // HLINE - Horizontal line
    CommandDefinition hline("HLINE",
                           "Draw a horizontal line",
                           "hline", "graphics");
    hline.addParameter("x", ParameterType::INT, "Start X coordinate")
         .addParameter("y", ParameterType::INT, "Y coordinate")
         .addParameter("len", ParameterType::INT, "Length")
         .addParameter("c", ParameterType::COLOR, "Color", true, "0xFFFFFFFF");
    registry.registerCommand(std::move(hline));

    // VERTLINE - Vertical line
    CommandDefinition vertline("VERTLINE",
                               "Draw a vertical line",
                               "vline", "graphics");
    vertline.addParameter("x", ParameterType::INT, "X coordinate")
            .addParameter("y", ParameterType::INT, "Start Y coordinate")
            .addParameter("len", ParameterType::INT, "Length")
            .addParameter("c", ParameterType::COLOR, "Color", true, "0xFFFFFFFF");
    registry.registerCommand(std::move(vertline));

    // ARC - Draw an arc outline
    CommandDefinition arc("ARC",
                         "Draw an arc outline",
                         "gfx_arc", "graphics");
    arc.addParameter("x", ParameterType::INT, "Center X coordinate")
       .addParameter("y", ParameterType::INT, "Center Y coordinate")
       .addParameter("r", ParameterType::INT, "Radius")
       .addParameter("start_angle", ParameterType::FLOAT, "Start angle in degrees")
       .addParameter("end_angle", ParameterType::FLOAT, "End angle in degrees")
       .addParameter("color", ParameterType::COLOR, "Arc color", true, "0xFFFFFFFF");
    registry.registerCommand(std::move(arc));

    // ARCF - Draw a filled arc (pie slice)
    CommandDefinition arcf("ARCF",
                          "Draw a filled arc (pie slice)",
                          "gfx_arc_filled", "graphics");
    arcf.addParameter("x", ParameterType::INT, "Center X coordinate")
        .addParameter("y", ParameterType::INT, "Center Y coordinate")
        .addParameter("r", ParameterType::INT, "Radius")
        .addParameter("start_angle", ParameterType::FLOAT, "Start angle in degrees")
        .addParameter("end_angle", ParameterType::FLOAT, "End angle in degrees")
        .addParameter("color", ParameterType::COLOR, "Fill color", true, "0xFFFFFFFF");
    registry.registerCommand(std::move(arcf));
}

// =============================================================================
// Audio Commands
// =============================================================================

void SuperTerminalCommandRegistry::registerAudioCommands(CommandRegistry& registry) {
    registerMusicCommands(registry);
    registerSIDCommands(registry);
    registerSynthCommands(registry);
    registerVoiceCommands(registry);
}

void SuperTerminalCommandRegistry::registerMusicCommands(CommandRegistry& registry) {
    // PLAY_SOUND - Play a sound effect
    CommandDefinition playSound("PLAY_SOUND",
                               "Play a sound or music file",
                               "music_play_file", "audio");
    playSound.addParameter("filename", ParameterType::STRING, "Sound file path");
    registry.registerCommand(std::move(playSound));

    // MUSIC - Play music
    CommandDefinition music("MUSIC",
                           "Play music",
                           "music_play", "audio");
    music.addParameter("music", ParameterType::STRING, "Music to play");
    registry.registerCommand(std::move(music));

    // MUSICFILE - Play music file
    CommandDefinition musicfile("MUSICFILE",
                               "Play music from file",
                               "music_play_file", "audio");
    musicfile.addParameter("filename", ParameterType::STRING, "Music file path");
    registry.registerCommand(std::move(musicfile));

    // MUSIC_PLAY_FILE - Play music file (ABC standard naming)
    CommandDefinition musicPlayFile("MUSIC_PLAY_FILE",
                                   "Play music from file (supports AssetManager, cart, and filesystem)",
                                   "music_play_file", "audio");
    musicPlayFile.addParameter("filename", ParameterType::STRING, "Music file path or asset name");
    registry.registerCommand(std::move(musicPlayFile));

    // MUSICSTOP - Stop music
    CommandDefinition musicstop("MUSICSTOP",
                               "Stop playing music",
                               "music_stop", "audio");
    registry.registerCommand(std::move(musicstop));

    // MUSICPAUSE - Pause music
    CommandDefinition musicpause("MUSICPAUSE",
                                "Pause music playback",
                                "music_pause", "audio");
    registry.registerCommand(std::move(musicpause));

    // MUSICRESUME - Resume music
    CommandDefinition musicresume("MUSICRESUME",
                                 "Resume music playback",
                                 "music_resume", "audio");
    registry.registerCommand(std::move(musicresume));

    // MUSICVOL - Set music volume
    CommandDefinition musicvol("MUSICVOL",
                              "Set music volume",
                              "music_set_volume", "audio");
    musicvol.addParameter("volume", ParameterType::FLOAT, "Volume level (0.0 to 1.0)");
    registry.registerCommand(std::move(musicvol));

    // SET_VOLUME - Set audio volume
    CommandDefinition setVolume("SET_VOLUME",
                               "Set the audio volume",
                               "music_set_volume", "audio");
    setVolume.addParameter("volume", ParameterType::FLOAT, "Volume level (0.0 to 1.0)");
    registry.registerCommand(std::move(setVolume));

    // PLAY - Play music
    CommandDefinition play("PLAY",
                          "Play music",
                          "music_play", "audio");
    play.addParameter("music", ParameterType::STRING, "Music to play");
    registry.registerCommand(std::move(play));

    // PLAY_ABC - Play ABC notation with escape sequence processing
    CommandDefinition playAbc("PLAY_ABC",
                             "Play ABC notation string with escape sequences (\\n becomes newline)",
                             "play_abc", "audio");
    playAbc.addParameter("abc_text", ParameterType::STRING, "ABC notation text with escape sequences");
    registry.registerCommand(std::move(playAbc));

    // =========================================================================
    // Music Bank Functions (ID-based Music Management)
    // =========================================================================

    // MUSIC_LOAD - Load music from ABC string
    CommandDefinition musicLoad("MUSIC_LOAD",
                               "Load music from ABC notation string",
                               "music_load_string", "audio");
    musicLoad.addParameter("abc_notation", ParameterType::STRING, "ABC notation string");
    musicLoad.setReturnType(ReturnType::INT);
    registry.registerFunction(std::move(musicLoad));

    // MUSIC_LOAD_FILE - Load music from file
    CommandDefinition musicLoadFile("MUSIC_LOAD_FILE",
                                   "Load music from ABC file",
                                   "music_load_file", "audio");
    musicLoadFile.addParameter("filename", ParameterType::STRING, "ABC file path");
    musicLoadFile.setReturnType(ReturnType::INT);
    registry.registerFunction(std::move(musicLoadFile));

    // MUSIC_PLAY_ID - Play music by ID
    CommandDefinition musicPlayId("MUSIC_PLAY_ID",
                                 "Play music by ID",
                                 "music_play_id", "audio");
    musicPlayId.addParameter("music_id", ParameterType::INT, "Music ID");
    musicPlayId.addParameter("volume", ParameterType::FLOAT, "Volume (0.0-1.0)", true, "1.0");
    registry.registerCommand(std::move(musicPlayId));

    // MUSIC_EXISTS - Check if music exists
    CommandDefinition musicExists("MUSIC_EXISTS",
                                 "Check if music ID exists in bank",
                                 "music_exists", "audio");
    musicExists.addParameter("music_id", ParameterType::INT, "Music ID");
    musicExists.setReturnType(ReturnType::INT);
    registry.registerFunction(std::move(musicExists));

    // MUSIC_GET_TITLE - Get music title
    CommandDefinition musicGetTitle("MUSIC_GET_TITLE",
                                   "Get music title by ID",
                                   "music_get_title", "audio");
    musicGetTitle.addParameter("music_id", ParameterType::INT, "Music ID");
    musicGetTitle.setReturnType(ReturnType::STRING);
    registry.registerFunction(std::move(musicGetTitle));

    // MUSIC_GET_COMPOSER - Get music composer
    CommandDefinition musicGetComposer("MUSIC_GET_COMPOSER",
                                      "Get music composer by ID",
                                      "music_get_composer", "audio");
    musicGetComposer.addParameter("music_id", ParameterType::INT, "Music ID");
    musicGetComposer.setReturnType(ReturnType::STRING);
    registry.registerFunction(std::move(musicGetComposer));

    // MUSIC_GET_KEY - Get music key signature
    CommandDefinition musicGetKey("MUSIC_GET_KEY",
                                 "Get music key signature by ID",
                                 "music_get_key", "audio");
    musicGetKey.addParameter("music_id", ParameterType::INT, "Music ID");
    musicGetKey.setReturnType(ReturnType::STRING);
    registry.registerFunction(std::move(musicGetKey));

    // MUSIC_GET_TEMPO - Get music tempo
    CommandDefinition musicGetTempo("MUSIC_GET_TEMPO",
                                   "Get music tempo by ID",
                                   "music_get_tempo", "audio");
    musicGetTempo.addParameter("music_id", ParameterType::INT, "Music ID");
    musicGetTempo.setReturnType(ReturnType::FLOAT);
    registry.registerFunction(std::move(musicGetTempo));

    // MUSIC_FREE - Free music by ID
    CommandDefinition musicFree("MUSIC_FREE",
                               "Free music from bank by ID",
                               "music_free", "audio");
    musicFree.addParameter("music_id", ParameterType::INT, "Music ID");
    musicFree.setReturnType(ReturnType::INT);
    registry.registerFunction(std::move(musicFree));

    // MUSIC_FREE_ALL - Free all music
    CommandDefinition musicFreeAll("MUSIC_FREE_ALL",
                                  "Free all music from bank",
                                  "music_free_all", "audio");
    registry.registerCommand(std::move(musicFreeAll));

    // MUSIC_COUNT - Get music count
    CommandDefinition musicCount("MUSIC_COUNT",
                                "Get number of music pieces in bank",
                                "music_get_count", "audio");
    musicCount.setReturnType(ReturnType::INT);
    registry.registerFunction(std::move(musicCount));

    // MUSIC_MEMORY - Get music bank memory usage
    CommandDefinition musicMemory("MUSIC_MEMORY",
                                 "Get music bank memory usage in bytes",
                                 "music_get_memory", "audio");
    musicMemory.setReturnType(ReturnType::INT);
    registry.registerFunction(std::move(musicMemory));

    // MUSIC_SAVE_TO_WAV - Render VoiceScript to WAV file in cart
    CommandDefinition musicSaveToWav("MUSIC_SAVE_TO_WAV",
                                     "Render VoiceScript to WAV file and save to cart",
                                     "music_save_to_wav", "audio");
    musicSaveToWav.addParameter("script_name", ParameterType::STRING, "VoiceScript name");
    musicSaveToWav.addParameter("asset_name", ParameterType::STRING, "Output WAV asset name (no extension)");
    musicSaveToWav.addParameter("duration", ParameterType::FLOAT, "Duration in seconds (0 = auto)", true);
    registry.registerCommand(std::move(musicSaveToWav));

    // VSCRIPT_SAVE_TO_BANK - Render VoiceScript to sound bank
    CommandDefinition vscriptSaveToBank("VSCRIPT_SAVE_TO_BANK",
                                        "Render VoiceScript and save to sound bank",
                                        "vscript_save_to_bank", "audio");
    vscriptSaveToBank.addParameter("script_name", ParameterType::STRING, "VoiceScript name");
    vscriptSaveToBank.addParameter("duration", ParameterType::FLOAT, "Duration in seconds (0 = auto)", true, "0.0");
    vscriptSaveToBank.setReturnType(ReturnType::INT);
    registry.registerFunction(std::move(vscriptSaveToBank));
}

void SuperTerminalCommandRegistry::registerSIDCommands(CommandRegistry& registry) {
    // =========================================================================
    // SID Player Commands (Commodore 64 Music)
    // =========================================================================

    // SID_LOAD_FILE - Load SID file
    CommandDefinition sidLoadFile("SID_LOAD_FILE",
                                 "Load Commodore 64 SID music file",
                                 "sid_load_file", "audio");
    sidLoadFile.addParameter("filename", ParameterType::STRING, "SID file path");
    sidLoadFile.setReturnType(ReturnType::INT);
    registry.registerFunction(std::move(sidLoadFile));

    // SID_PLAY - Play SID tune by ID
    CommandDefinition sidPlay("SID_PLAY",
                             "Play SID tune by ID",
                             "sid_play", "audio");
    sidPlay.addParameter("sid_id", ParameterType::INT, "SID ID");
    sidPlay.addParameter("subtune", ParameterType::INT, "Subtune number (0=default)", true, "0");
    sidPlay.addParameter("volume", ParameterType::FLOAT, "Volume (0.0-1.0)", true, "1.0");
    registry.registerCommand(std::move(sidPlay));

    // SID_STOP - Stop SID playback
    CommandDefinition sidStop("SID_STOP",
                             "Stop SID playback",
                             "sid_stop", "audio");
    registry.registerCommand(std::move(sidStop));

    // SID_PAUSE - Pause SID playback
    CommandDefinition sidPause("SID_PAUSE",
                              "Pause SID playback",
                              "sid_pause", "audio");
    registry.registerCommand(std::move(sidPause));

    // SID_RESUME - Resume SID playback
    CommandDefinition sidResume("SID_RESUME",
                               "Resume SID playback",
                               "sid_resume", "audio");
    registry.registerCommand(std::move(sidResume));

    // SID_IS_PLAYING - Check if SID is playing
    CommandDefinition sidIsPlaying("SID_IS_PLAYING",
                                  "Check if SID is currently playing",
                                  "sid_is_playing", "audio");
    sidIsPlaying.setReturnType(ReturnType::INT);
    registry.registerFunction(std::move(sidIsPlaying));

    // SID_SET_VOLUME - Set SID volume
    CommandDefinition sidSetVolume("SID_SET_VOLUME",
                                  "Set SID playback volume",
                                  "sid_set_volume", "audio");
    sidSetVolume.addParameter("volume", ParameterType::FLOAT, "Volume (0.0-1.0)");
    registry.registerCommand(std::move(sidSetVolume));

    // SID_GET_TITLE - Get SID title
    CommandDefinition sidGetTitle("SID_GET_TITLE",
                                 "Get SID title/name by ID",
                                 "sid_get_title", "audio");
    sidGetTitle.addParameter("sid_id", ParameterType::INT, "SID ID");
    sidGetTitle.setReturnType(ReturnType::STRING);
    registry.registerFunction(std::move(sidGetTitle));

    // SID_GET_AUTHOR - Get SID author
    CommandDefinition sidGetAuthor("SID_GET_AUTHOR",
                                  "Get SID author/composer by ID",
                                  "sid_get_author", "audio");
    sidGetAuthor.addParameter("sid_id", ParameterType::INT, "SID ID");
    sidGetAuthor.setReturnType(ReturnType::STRING);
    registry.registerFunction(std::move(sidGetAuthor));

    // SID_GET_COPYRIGHT - Get SID copyright
    CommandDefinition sidGetCopyright("SID_GET_COPYRIGHT",
                                     "Get SID copyright/released info by ID",
                                     "sid_get_copyright", "audio");
    sidGetCopyright.addParameter("sid_id", ParameterType::INT, "SID ID");
    sidGetCopyright.setReturnType(ReturnType::STRING);
    registry.registerFunction(std::move(sidGetCopyright));

    // SID_GET_SUBTUNE_COUNT - Get number of subtunes
    CommandDefinition sidGetSubtuneCount("SID_GET_SUBTUNE_COUNT",
                                        "Get number of subtunes in SID",
                                        "sid_get_subtune_count", "audio");
    sidGetSubtuneCount.addParameter("sid_id", ParameterType::INT, "SID ID");
    sidGetSubtuneCount.setReturnType(ReturnType::INT);
    registry.registerFunction(std::move(sidGetSubtuneCount));

    // SID_GET_DEFAULT_SUBTUNE - Get default subtune number
    CommandDefinition sidGetDefaultSubtune("SID_GET_DEFAULT_SUBTUNE",
                                          "Get default subtune number (1-based)",
                                          "sid_get_default_subtune", "audio");
    sidGetDefaultSubtune.addParameter("sid_id", ParameterType::INT, "SID ID");
    sidGetDefaultSubtune.setReturnType(ReturnType::INT);
    registry.registerFunction(std::move(sidGetDefaultSubtune));

    // SID_SET_QUALITY - Set emulation quality
    CommandDefinition sidSetQuality("SID_SET_QUALITY",
                                   "Set SID emulation quality (0=FAST, 1=GOOD, 2=BEST)",
                                   "sid_set_quality", "audio");
    sidSetQuality.addParameter("quality", ParameterType::INT, "Quality level (0-2)");
    registry.registerCommand(std::move(sidSetQuality));

    // SID_SET_CHIP_MODEL - Set SID chip model
    CommandDefinition sidSetChipModel("SID_SET_CHIP_MODEL",
                                     "Set SID chip model (0=6581, 1=8580, 2=AUTO)",
                                     "sid_set_chip_model", "audio");
    sidSetChipModel.addParameter("model", ParameterType::INT, "Chip model (0-2)");
    registry.registerCommand(std::move(sidSetChipModel));

    // SID_SET_SPEED - Set playback speed
    CommandDefinition sidSetSpeed("SID_SET_SPEED",
                                 "Set SID playback speed multiplier",
                                 "sid_set_speed", "audio");
    sidSetSpeed.addParameter("speed", ParameterType::FLOAT, "Speed multiplier (1.0=normal)");
    registry.registerCommand(std::move(sidSetSpeed));

    // SID_SET_MAX_SIDS - Set maximum SID chips
    CommandDefinition sidSetMaxSids("SID_SET_MAX_SIDS",
                                   "Set maximum number of SID chips to emulate (1-3)",
                                   "sid_set_max_sids", "audio");
    sidSetMaxSids.addParameter("max_sids", ParameterType::INT, "Number of SID chips (1-3)");
    registry.registerCommand(std::move(sidSetMaxSids));

    // SID_GET_MAX_SIDS - Get maximum SID chips
    CommandDefinition sidGetMaxSids("SID_GET_MAX_SIDS",
                                   "Get maximum number of SID chips",
                                   "sid_get_max_sids", "audio");
    sidGetMaxSids.setReturnType(ReturnType::INT);
    registry.registerFunction(std::move(sidGetMaxSids));

    // SID_GET_TIME - Get current playback time
    CommandDefinition sidGetTime("SID_GET_TIME",
                                "Get current SID playback time in seconds",
                                "sid_get_time", "audio");
    sidGetTime.setReturnType(ReturnType::FLOAT);
    registry.registerFunction(std::move(sidGetTime));

    // SID_FREE - Free SID by ID
    CommandDefinition sidFree("SID_FREE",
                             "Free SID from bank by ID",
                             "sid_free", "audio");
    sidFree.addParameter("sid_id", ParameterType::INT, "SID ID");
    sidFree.setReturnType(ReturnType::INT);
    registry.registerFunction(std::move(sidFree));

    // SID_FREE_ALL - Free all SIDs
    CommandDefinition sidFreeAll("SID_FREE_ALL",
                                "Free all SIDs from bank",
                                "sid_free_all", "audio");
    registry.registerCommand(std::move(sidFreeAll));

    // SID_EXISTS - Check if SID exists
    CommandDefinition sidExists("SID_EXISTS",
                               "Check if SID ID exists in bank",
                               "sid_exists", "audio");
    sidExists.addParameter("sid_id", ParameterType::INT, "SID ID");
    sidExists.setReturnType(ReturnType::INT);
    registry.registerFunction(std::move(sidExists));

    // SID_COUNT - Get SID count
    CommandDefinition sidCount("SID_COUNT",
                              "Get number of SIDs in bank",
                              "sid_get_count", "audio");
    sidCount.setReturnType(ReturnType::INT);
    registry.registerFunction(std::move(sidCount));

    // SID_MEMORY - Get SID bank memory usage
    CommandDefinition sidMemory("SID_MEMORY",
                               "Get SID bank memory usage in bytes",
                               "sid_get_memory", "audio");
    sidMemory.setReturnType(ReturnType::INT);
    registry.registerFunction(std::move(sidMemory));
}

void SuperTerminalCommandRegistry::registerSynthCommands(CommandRegistry& registry) {
    // PLAY_NOTE - Play a musical note
    CommandDefinition playNote("PLAY_NOTE",
                              "Play a musical note",
                              "synth_note", "audio");
    playNote.addParameter("note", ParameterType::INT, "MIDI note number (0-127)")
            .addParameter("duration", ParameterType::FLOAT, "Note duration in seconds", true, "0.5")
            .addParameter("velocity", ParameterType::INT, "Note velocity (0-127)", true, "100");
    registry.registerCommand(std::move(playNote));

    // SOUND - Generate sound by frequency
    CommandDefinition sound("SOUND",
                           "Generate sound by frequency",
                           "", "audio");
    sound.addParameter("freq", ParameterType::FLOAT, "Frequency in Hz")
         .addParameter("dur", ParameterType::FLOAT, "Duration in seconds")
         .setCustomCodeGen("synth_frequency({0}, {1})");
    registry.registerCommand(std::move(sound));

    // BEEP - Simple beep sound
    CommandDefinition beep("BEEP",
                          "Generate a simple beep sound",
                          "", "audio");
    beep.setCustomCodeGen("synth_frequency(440, 0.1)");
    registry.registerCommand(std::move(beep));

    // SYNTH - Play synthesized note
    CommandDefinition synth("SYNTH",
                           "Play synthesized note",
                           "", "audio");
    synth.addParameter("note", ParameterType::INT, "MIDI note number")
         .addParameter("dur", ParameterType::FLOAT, "Duration in seconds")
         .setCustomCodeGen("synth_note({0}, {1})");
    registry.registerCommand(std::move(synth));

    // SYNTHFREQ - Play synthesized frequency
    CommandDefinition synthfreq("SYNTHFREQ",
                               "Play synthesized frequency",
                               "synth_frequency", "audio");
    synthfreq.addParameter("freq", ParameterType::FLOAT, "Frequency in Hz")
             .addParameter("dur", ParameterType::FLOAT, "Duration in seconds");
    registry.registerCommand(std::move(synthfreq));

    // SYNTHINST - Set synthesizer instrument
    CommandDefinition synthinst("SYNTHINST",
                               "Set synthesizer instrument",
                               "synth_set_instrument", "audio");
    synthinst.addParameter("instrument", ParameterType::INT, "Instrument number");
    registry.registerCommand(std::move(synthinst));

    // =============================================================================
    // Predefined Sound Effects (Phase 1)
    // =============================================================================

    // SYNTH_ZAP - Laser zap sound effect
    CommandDefinition synthZap("SYNTH_ZAP",
                              "Play laser zap sound effect",
                              "synth_zap", "audio");
    registry.registerCommand(std::move(synthZap));

    // SYNTH_EXPLODE - Explosion sound effect
    CommandDefinition synthExplode("SYNTH_EXPLODE",
                                  "Play explosion sound effect",
                                  "synth_explode", "audio");
    registry.registerCommand(std::move(synthExplode));

    // SYNTH_COIN - Coin pickup sound effect
    CommandDefinition synthCoin("SYNTH_COIN",
                               "Play coin pickup sound effect",
                               "synth_coin", "audio");
    registry.registerCommand(std::move(synthCoin));

    // SYNTH_JUMP - Jump sound effect
    CommandDefinition synthJump("SYNTH_JUMP",
                               "Play jump sound effect",
                               "synth_jump", "audio");
    registry.registerCommand(std::move(synthJump));

    // SYNTH_POWERUP - Power-up sound effect
    CommandDefinition synthPowerup("SYNTH_POWERUP",
                                  "Play power-up sound effect",
                                  "synth_powerup", "audio");
    registry.registerCommand(std::move(synthPowerup));

    // SYNTH_HURT - Hurt/damage sound effect
    CommandDefinition synthHurt("SYNTH_HURT",
                               "Play hurt/damage sound effect",
                               "synth_hurt", "audio");
    registry.registerCommand(std::move(synthHurt));

    // SYNTH_SHOOT - Shooting sound effect
    CommandDefinition synthShoot("SYNTH_SHOOT",
                                "Play shooting sound effect",
                                "synth_shoot", "audio");
    registry.registerCommand(std::move(synthShoot));

    // SYNTH_CLICK - UI click sound effect
    CommandDefinition synthClick("SYNTH_CLICK",
                                "Play UI click sound effect",
                                "synth_click", "audio");
    registry.registerCommand(std::move(synthClick));

    // SYNTH_PICKUP - Item pickup sound effect
    CommandDefinition synthPickup("SYNTH_PICKUP",
                                 "Play item pickup sound effect",
                                 "synth_pickup", "audio");
    registry.registerCommand(std::move(synthPickup));

    // SYNTH_BLIP - Short blip sound effect
    CommandDefinition synthBlip("SYNTH_BLIP",
                               "Play short blip sound effect",
                               "synth_blip", "audio");
    registry.registerCommand(std::move(synthBlip));

    // SYNTH_SWEEP_UP - Rising sweep sound effect
    CommandDefinition synthSweepUp("SYNTH_SWEEP_UP",
                                  "Play rising sweep sound effect",
                                  "synth_sweep_up", "audio");
    registry.registerCommand(std::move(synthSweepUp));

    // SYNTH_SWEEP_DOWN - Falling sweep sound effect
    CommandDefinition synthSweepDown("SYNTH_SWEEP_DOWN",
                                    "Play falling sweep sound effect",
                                    "synth_sweep_down", "audio");
    registry.registerCommand(std::move(synthSweepDown));

    // Advanced explosion variants

    // SYNTH_BIG_EXPLOSION - Large explosion sound effect
    CommandDefinition synthBigExplosion("SYNTH_BIG_EXPLOSION",
                                       "Play large explosion sound effect",
                                       "synth_big_explosion", "audio");
    registry.registerCommand(std::move(synthBigExplosion));

    // SYNTH_SMALL_EXPLOSION - Small explosion sound effect
    CommandDefinition synthSmallExplosion("SYNTH_SMALL_EXPLOSION",
                                         "Play small explosion sound effect",
                                         "synth_small_explosion", "audio");
    registry.registerCommand(std::move(synthSmallExplosion));

    // SYNTH_DISTANT_EXPLOSION - Distant explosion rumble
    CommandDefinition synthDistantExplosion("SYNTH_DISTANT_EXPLOSION",
                                           "Play distant explosion sound effect",
                                           "synth_distant_explosion", "audio");
    registry.registerCommand(std::move(synthDistantExplosion));

    // SYNTH_METAL_EXPLOSION - Metallic explosion sound effect
    CommandDefinition synthMetalExplosion("SYNTH_METAL_EXPLOSION",
                                         "Play metallic explosion sound effect",
                                         "synth_metal_explosion", "audio");
    registry.registerCommand(std::move(synthMetalExplosion));

    // =============================================================================
    // Sound Bank Commands (non-returning) - Phase 1
    // =============================================================================

    // SOUND_PLAY - Play a sound from the sound bank by ID
    CommandDefinition soundPlay("SOUND_PLAY",
                               "Play a sound from the sound bank",
                               "sound_play_id", "audio");
    soundPlay.addParameter("sound_id", ParameterType::INT, "Sound ID")
             .addParameter("volume", ParameterType::FLOAT, "Volume (0.0-1.0)", true, "1.0")
             .addParameter("pan", ParameterType::FLOAT, "Pan (-1.0 to 1.0, 0=center)", true, "0.0");
    registry.registerCommand(std::move(soundPlay));

    // SOUND_FREE_ALL - Free all sounds from the sound bank
    CommandDefinition soundFreeAll("SOUND_FREE_ALL",
                                  "Free all sounds from the sound bank",
                                  "sound_free_all", "audio");
    registry.registerCommand(std::move(soundFreeAll));

}

// =============================================================================
// Voice Controller Commands (Persistent Voice Synthesis)
// =============================================================================

void SuperTerminalCommandRegistry::registerVoiceCommands(CommandRegistry& registry) {
    // VOICE_WAVEFORM - Set voice waveform
    CommandDefinition voiceWaveform("VOICE_WAVEFORM",
                                    "Set voice waveform type",
                                    "voice_set_waveform", "audio");
    voiceWaveform.addParameter("voice", ParameterType::INT, "Voice number (1-8)");
    voiceWaveform.addParameter("waveform", ParameterType::INT, "Waveform (0=Silence, 1=Sine, 2=Square, 3=Sawtooth, 4=Triangle, 5=Noise, 6=Pulse)");
    registry.registerCommand(std::move(voiceWaveform));

    // VOICE_FREQUENCY - Set voice frequency
    CommandDefinition voiceFrequency("VOICE_FREQUENCY",
                                     "Set voice frequency in Hz",
                                     "voice_set_frequency", "audio");
    voiceFrequency.addParameter("voice", ParameterType::INT, "Voice number (1-8)");
    voiceFrequency.addParameter("frequency", ParameterType::FLOAT, "Frequency in Hz");
    registry.registerCommand(std::move(voiceFrequency));

    // VOICE_NOTE - Set voice note by MIDI number
    CommandDefinition voiceNote("VOICE_NOTE",
                                "Set voice note by MIDI note number",
                                "voice_set_note", "audio");
    voiceNote.addParameter("voice", ParameterType::INT, "Voice number (1-8)");
    voiceNote.addParameter("note", ParameterType::INT, "MIDI note (0-127, middle C=60)");
    registry.registerCommand(std::move(voiceNote));

    // VOICE_NOTE_NAME - Set voice note by name
    CommandDefinition voiceNoteName("VOICE_NOTE_NAME",
                                    "Set voice note by note name",
                                    "voice_set_note_name", "audio");
    voiceNoteName.addParameter("voice", ParameterType::INT, "Voice number (1-8)");
    voiceNoteName.addParameter("notename", ParameterType::STRING, "Note name (e.g., 'C-4', 'A#3')");
    registry.registerCommand(std::move(voiceNoteName));

    // VOICE_ENVELOPE - Set voice ADSR envelope
    CommandDefinition voiceEnvelope("VOICE_ENVELOPE",
                                    "Set voice ADSR envelope parameters",
                                    "voice_set_envelope", "audio");
    voiceEnvelope.addParameter("voice", ParameterType::INT, "Voice number (1-8)");
    voiceEnvelope.addParameter("attack", ParameterType::FLOAT, "Attack time in ms");
    voiceEnvelope.addParameter("decay", ParameterType::FLOAT, "Decay time in ms");
    voiceEnvelope.addParameter("sustain", ParameterType::FLOAT, "Sustain level (0.0-1.0)");
    voiceEnvelope.addParameter("release", ParameterType::FLOAT, "Release time in ms");
    registry.registerCommand(std::move(voiceEnvelope));

    // VOICE_GATE - Set voice gate (on/off)
    CommandDefinition voiceGate("VOICE_GATE",
                                "Set voice gate state (on=play, off=release)",
                                "voice_set_gate", "audio");
    voiceGate.addParameter("voice", ParameterType::INT, "Voice number (1-8)");
    voiceGate.addParameter("state", ParameterType::INT, "Gate state (0=off, 1=on)");
    registry.registerCommand(std::move(voiceGate));

    // VOICE_VOLUME - Set voice volume
    CommandDefinition voiceVolume("VOICE_VOLUME",
                                  "Set voice volume level",
                                  "voice_set_volume", "audio");
    voiceVolume.addParameter("voice", ParameterType::INT, "Voice number (1-8)");
    voiceVolume.addParameter("volume", ParameterType::FLOAT, "Volume (0.0-1.0)");
    registry.registerCommand(std::move(voiceVolume));

    // VOICE_PULSE_WIDTH - Set pulse width
    CommandDefinition voicePulseWidth("VOICE_PULSE_WIDTH",
                                      "Set pulse width for pulse waveform",
                                      "voice_set_pulse_width", "audio");
    voicePulseWidth.addParameter("voice", ParameterType::INT, "Voice number (1-8)");
    voicePulseWidth.addParameter("width", ParameterType::FLOAT, "Pulse width (0.0-1.0, 0.5=square)");
    registry.registerCommand(std::move(voicePulseWidth));

    // VOICE_FILTER_ROUTE - Enable/disable filter routing
    CommandDefinition voiceFilterRoute("VOICE_FILTER_ROUTE",
                                       "Enable/disable filter routing for voice",
                                       "voice_set_filter_routing", "audio");
    voiceFilterRoute.addParameter("voice", ParameterType::INT, "Voice number (1-8)");
    voiceFilterRoute.addParameter("enabled", ParameterType::INT, "Route through filter (0=off, 1=on)");
    registry.registerCommand(std::move(voiceFilterRoute));

    // VOICE_FILTER_TYPE - Set global filter type
    CommandDefinition voiceFilterType("VOICE_FILTER_TYPE",
                                      "Set global filter type",
                                      "voice_set_filter_type", "audio");
    voiceFilterType.addParameter("type", ParameterType::INT, "Filter type (0=None, 1=LowPass, 2=HighPass, 3=BandPass)");
    registry.registerCommand(std::move(voiceFilterType));

    // VOICE_FILTER_CUTOFF - Set filter cutoff frequency
    CommandDefinition voiceFilterCutoff("VOICE_FILTER_CUTOFF",
                                        "Set global filter cutoff frequency",
                                        "voice_set_filter_cutoff", "audio");
    voiceFilterCutoff.addParameter("cutoff", ParameterType::FLOAT, "Cutoff frequency in Hz");
    registry.registerCommand(std::move(voiceFilterCutoff));

    // VOICE_FILTER_RESONANCE - Set filter resonance
    CommandDefinition voiceFilterResonance("VOICE_FILTER_RESONANCE",
                                           "Set global filter resonance",
                                           "voice_set_filter_resonance", "audio");
    voiceFilterResonance.addParameter("resonance", ParameterType::FLOAT, "Resonance (1.0=none, higher=more)");
    registry.registerCommand(std::move(voiceFilterResonance));

    // VOICE_FILTER_ENABLE - Enable/disable global filter
    CommandDefinition voiceFilterEnable("VOICE_FILTER_ENABLE",
                                        "Enable/disable global filter",
                                        "voice_set_filter_enabled", "audio");
    voiceFilterEnable.addParameter("enabled", ParameterType::INT, "Filter enabled (0=off, 1=on)");
    registry.registerCommand(std::move(voiceFilterEnable));

    // VOICE_FILTER - Set all filter parameters at once (convenience command, current beat)
    CommandDefinition voiceFilter("VOICE_FILTER",
                                  "Set filter cutoff, resonance and type at current beat",
                                  "voice_filter", "audio");
    voiceFilter.addParameter("voice", ParameterType::INT, "Voice number (1-8)");
    voiceFilter.addParameter("cutoff", ParameterType::FLOAT, "Cutoff frequency in Hz");
    voiceFilter.addParameter("resonance", ParameterType::FLOAT, "Resonance (0.0-1.0)");
    voiceFilter.addParameter("type", ParameterType::INT, "Filter type (0=LowPass, 1=HighPass, 2=BandPass)");
    registry.registerCommand(std::move(voiceFilter));

    // VOICE_FILTER_AT - Set all filter parameters at specific beat (timeline command)
    CommandDefinition voiceFilterAt("VOICE_FILTER_AT",
                                    "Set filter cutoff, resonance and type at specific beat position",
                                    "voice_filter_at", "audio");
    voiceFilterAt.addParameter("voice", ParameterType::INT, "Voice number (1-8)");
    voiceFilterAt.addParameter("beat", ParameterType::FLOAT, "Beat position in timeline");
    voiceFilterAt.addParameter("cutoff", ParameterType::FLOAT, "Cutoff frequency in Hz");
    voiceFilterAt.addParameter("resonance", ParameterType::FLOAT, "Resonance (0.0-1.0)");
    voiceFilterAt.addParameter("type", ParameterType::INT, "Filter type (0=LowPass, 1=HighPass, 2=BandPass)");
    registry.registerCommand(std::move(voiceFilterAt));

    // VOICE_WAVEFORM_AT - Set waveform at specific beat
    CommandDefinition voiceWaveformAt("VOICE_WAVEFORM_AT",
                                      "Set voice waveform at specific beat position",
                                      "voice_waveform_at", "audio");
    voiceWaveformAt.addParameter("voice", ParameterType::INT, "Voice number (1-8)");
    voiceWaveformAt.addParameter("beat", ParameterType::FLOAT, "Beat position in timeline");
    voiceWaveformAt.addParameter("waveform", ParameterType::INT, "Waveform type");
    registry.registerCommand(std::move(voiceWaveformAt));

    // VOICE_ENVELOPE_AT - Set envelope at specific beat
    CommandDefinition voiceEnvelopeAt("VOICE_ENVELOPE_AT",
                                      "Set ADSR envelope at specific beat position",
                                      "voice_envelope_at", "audio");
    voiceEnvelopeAt.addParameter("voice", ParameterType::INT, "Voice number (1-8)");
    voiceEnvelopeAt.addParameter("beat", ParameterType::FLOAT, "Beat position in timeline");
    voiceEnvelopeAt.addParameter("attack", ParameterType::FLOAT, "Attack time in ms");
    voiceEnvelopeAt.addParameter("decay", ParameterType::FLOAT, "Decay time in ms");
    voiceEnvelopeAt.addParameter("sustain", ParameterType::FLOAT, "Sustain level (0.0-1.0)");
    voiceEnvelopeAt.addParameter("release", ParameterType::FLOAT, "Release time in ms");
    registry.registerCommand(std::move(voiceEnvelopeAt));

    // VOICE_FREQUENCY_AT - Set frequency at specific beat
    CommandDefinition voiceFrequencyAt("VOICE_FREQUENCY_AT",
                                       "Set voice frequency at specific beat position",
                                       "voice_frequency_at", "audio");
    voiceFrequencyAt.addParameter("voice", ParameterType::INT, "Voice number (1-8)");
    voiceFrequencyAt.addParameter("beat", ParameterType::FLOAT, "Beat position in timeline");
    voiceFrequencyAt.addParameter("frequency", ParameterType::FLOAT, "Frequency in Hz");
    registry.registerCommand(std::move(voiceFrequencyAt));

    // VOICE_VOLUME_AT - Set volume at specific beat
    CommandDefinition voiceVolumeAt("VOICE_VOLUME_AT",
                                    "Set voice volume at specific beat position",
                                    "voice_volume_at", "audio");
    voiceVolumeAt.addParameter("voice", ParameterType::INT, "Voice number (1-8)");
    voiceVolumeAt.addParameter("beat", ParameterType::FLOAT, "Beat position in timeline");
    voiceVolumeAt.addParameter("volume", ParameterType::FLOAT, "Volume (0.0-1.0)");
    registry.registerCommand(std::move(voiceVolumeAt));

    // VOICE_PAN_AT - Set pan at specific beat
    CommandDefinition voicePanAt("VOICE_PAN_AT",
                                 "Set voice pan at specific beat position",
                                 "voice_pan_at", "audio");
    voicePanAt.addParameter("voice", ParameterType::INT, "Voice number (1-8)");
    voicePanAt.addParameter("beat", ParameterType::FLOAT, "Beat position in timeline");
    voicePanAt.addParameter("pan", ParameterType::FLOAT, "Pan position (-1.0 to 1.0)");
    registry.registerCommand(std::move(voicePanAt));

    // VOICE_GATE_AT - Set gate at specific beat
    CommandDefinition voiceGateAt("VOICE_GATE_AT",
                                  "Set voice gate at specific beat position",
                                  "voice_gate_at", "audio");
    voiceGateAt.addParameter("voice", ParameterType::INT, "Voice number (1-8)");
    voiceGateAt.addParameter("beat", ParameterType::FLOAT, "Beat position in timeline");
    voiceGateAt.addParameter("state", ParameterType::INT, "Gate state (0=off, 1=on)");
    registry.registerCommand(std::move(voiceGateAt));

    // VOICE_MASTER_VOLUME - Set master volume
    CommandDefinition voiceMasterVolume("VOICE_MASTER_VOLUME",
                                        "Set voice master volume",
                                        "voice_set_master_volume", "audio");
    voiceMasterVolume.addParameter("volume", ParameterType::FLOAT, "Master volume (0.0-1.0)");
    registry.registerCommand(std::move(voiceMasterVolume));

    // VOICE_RESET_ALL - Reset all voices
    CommandDefinition voiceResetAll("VOICE_RESET_ALL",
                                    "Reset all voices (gate off, clear state)",
                                    "voice_reset_all", "audio");
    registry.registerCommand(std::move(voiceResetAll));

    // WAIT - Wait for N/60 seconds (N is in 60ths of a second, like 60 FPS frames)
    // Only for batch mode (fbsh_voices) - provides simple timing control
    CommandDefinition wait("WAIT",
                          "Wait for N/60 seconds (N in 60ths of second)",
                          "wait", "timing");
    wait.addParameter("sixtieths", ParameterType::FLOAT, "Number of 60ths of a second to wait");
    registry.registerCommand(std::move(wait));

    // VOICE_WAIT - Wait for N beats (tempo-based timing)
    CommandDefinition voiceWait("VOICE_WAIT",
                                "Wait for specified number of beats (based on tempo)",
                                "voice_wait", "audio");
    voiceWait.addParameter("beats", ParameterType::FLOAT, "Number of beats to wait");
    registry.registerCommand(std::move(voiceWait));

    // VOICE_WAIT_BEATS - Explicit alias for VOICE_WAIT (uses beats at 120 BPM)
    CommandDefinition voiceWaitBeats("VOICE_WAIT_BEATS",
                                     "Wait for specified number of beats (120 BPM = 2 beats/sec)",
                                     "voice_wait_beats", "audio");
    voiceWaitBeats.addParameter("beats", ParameterType::FLOAT, "Number of beats to wait");
    registry.registerCommand(std::move(voiceWaitBeats));

    // VOICES_START - Begin recording voice commands to timeline
    CommandDefinition voicesStart("VOICES_START",
                                  "Begin recording voice commands to timeline for rendering",
                                  "voices_start", "audio");
    registry.registerCommand(std::move(voicesStart));

    // VOICES_SET_TEMPO - Set tempo for VOICES timeline
    CommandDefinition voicesSetTempo("VOICES_SET_TEMPO",
                                     "Set tempo in beats per minute (BPM) for timeline playback",
                                     "voices_set_tempo", "audio");
    voicesSetTempo.addParameter("bpm", ParameterType::FLOAT, "Beats per minute (e.g., 120)");
    registry.registerCommand(std::move(voicesSetTempo));

    // VOICES_END_SLOT - Render timeline to sound bank slot
    CommandDefinition voicesEndSlot("VOICES_END_SLOT",
                                    "Render voice timeline to sound bank slot",
                                    "voices_end_slot", "audio");
    voicesEndSlot.addParameter("slot", ParameterType::INT, "Sound bank slot number");
    voicesEndSlot.addParameter("volume", ParameterType::FLOAT, "Playback volume (0.0-1.0)");
    registry.registerCommand(std::move(voicesEndSlot));

    // VOICES_NEXT_SLOT - Render timeline and save to next available sound bank slot
    CommandDefinition voicesNextSlot("VOICES_NEXT_SLOT",
                                     "Render voice timeline and save to next available sound bank slot, returning the slot ID",
                                     "voices_next_slot", "audio", false, ReturnType::INT);
    voicesNextSlot.addParameter("volume", ParameterType::FLOAT, "Playback volume (0.0-1.0)");
    registry.registerCommand(std::move(voicesNextSlot));

    // VOICES_END_PLAY - Render timeline and play immediately
    CommandDefinition voicesEndPlay("VOICES_END_PLAY",
                                    "Render voice timeline and play immediately",
                                    "voices_end_play", "audio");
    registry.registerCommand(std::move(voicesEndPlay));

    // VOICES_END_SAVE - Render timeline and save to WAV file
    CommandDefinition voicesEndSave("VOICES_END_SAVE",
                                    "Render voice timeline and save to WAV file",
                                    "voices_end_save", "audio");
    voicesEndSave.addParameter("filename", ParameterType::STRING, "Output WAV filename");
    registry.registerCommand(std::move(voicesEndSave));

    // VOICE_DIRECT - Direct audio output to WAV file or live playback
    CommandDefinition voiceDirect("VOICE_DIRECT",
                                  "Direct voice output to WAV file for rendering or empty for live playback",
                                  "voice_direct", "audio");
    voiceDirect.addParameter("destination", ParameterType::STRING, "WAV filename for rendering, or empty string for live playback");
    registry.registerCommand(std::move(voiceDirect));

    // VOICE_DIRECT_SLOT - Render audio to sound slot
    CommandDefinition voiceDirectSlot("VOICE_DIRECT_SLOT",
                                      "Render voice output directly to sound slot",
                                      "voice_direct_slot", "audio");
    voiceDirectSlot.addParameter("slot_num", ParameterType::INT, "Sound slot number (1-based)");
    voiceDirectSlot.addParameter("volume", ParameterType::FLOAT, "Playback volume (0.0 to 1.0)");
    voiceDirectSlot.addParameter("duration", ParameterType::FLOAT, "Duration in seconds (0 = auto-detect)", true);
    registry.registerCommand(std::move(voiceDirectSlot));

    // =========================================================================
    // Stereo & Spatial Commands
    // =========================================================================

    // VOICE_PAN - Set voice stereo pan position
    CommandDefinition voicePan("VOICE_PAN",
                                "Set voice stereo pan position",
                                "voice_set_pan", "audio");
    voicePan.addParameter("voice", ParameterType::INT, "Voice number (1-8)");
    voicePan.addParameter("pan", ParameterType::FLOAT, "Pan position (-1.0=left, 0.0=center, 1.0=right)");
    registry.registerCommand(std::move(voicePan));

    // =========================================================================
    // SID-Style Modulation Commands
    // =========================================================================

    // VOICE_RING_MOD - Enable ring modulation from source voice
    CommandDefinition voiceRingMod("VOICE_RING_MOD",
                                    "Enable ring modulation from source voice",
                                    "voice_set_ring_mod", "audio");
    voiceRingMod.addParameter("voice", ParameterType::INT, "Voice number (1-8)");
    voiceRingMod.addParameter("source_voice", ParameterType::INT, "Source voice for modulation (1-8, 0=off)");
    registry.registerCommand(std::move(voiceRingMod));

    // VOICE_SYNC - Enable hard sync from source voice
    CommandDefinition voiceSync("VOICE_SYNC",
                                 "Enable hard sync from source voice",
                                 "voice_set_sync", "audio");
    voiceSync.addParameter("voice", ParameterType::INT, "Voice number (1-8)");
    voiceSync.addParameter("source_voice", ParameterType::INT, "Source voice for sync (1-8, 0=off)");
    registry.registerCommand(std::move(voiceSync));

    // VOICE_PORTAMENTO - Set portamento (pitch glide) time
    CommandDefinition voicePortamento("VOICE_PORTAMENTO",
                                       "Set portamento (pitch glide) time",
                                       "voice_set_portamento", "audio");
    voicePortamento.addParameter("voice", ParameterType::INT, "Voice number (1-8)");
    voicePortamento.addParameter("time", ParameterType::FLOAT, "Glide time in seconds");
    registry.registerCommand(std::move(voicePortamento));

    // VOICE_DETUNE - Set voice detuning in cents
    CommandDefinition voiceDetune("VOICE_DETUNE",
                                   "Set voice detuning in cents",
                                   "voice_set_detune", "audio");
    voiceDetune.addParameter("voice", ParameterType::INT, "Voice number (1-8)");
    voiceDetune.addParameter("cents", ParameterType::FLOAT, "Detuning in cents (-100 to +100)");
    registry.registerCommand(std::move(voiceDetune));

    // =========================================================================
    // Delay Effects Commands
    // =========================================================================

    // VOICE_DELAY_ENABLE - Enable/disable delay effect for voice
    CommandDefinition voiceDelayEnable("VOICE_DELAY_ENABLE",
                                        "Enable/disable delay effect for voice",
                                        "voice_set_delay_enable", "audio");
    voiceDelayEnable.addParameter("voice", ParameterType::INT, "Voice number (1-8)");
    voiceDelayEnable.addParameter("enabled", ParameterType::INT, "Enable state (0=off, 1=on)");
    registry.registerCommand(std::move(voiceDelayEnable));

    // VOICE_DELAY_TIME - Set delay time
    CommandDefinition voiceDelayTime("VOICE_DELAY_TIME",
                                      "Set delay time",
                                      "voice_set_delay_time", "audio");
    voiceDelayTime.addParameter("voice", ParameterType::INT, "Voice number (1-8)");
    voiceDelayTime.addParameter("time", ParameterType::FLOAT, "Delay time in seconds");
    registry.registerCommand(std::move(voiceDelayTime));

    // VOICE_DELAY_FEEDBACK - Set delay feedback amount
    CommandDefinition voiceDelayFeedback("VOICE_DELAY_FEEDBACK",
                                          "Set delay feedback amount",
                                          "voice_set_delay_feedback", "audio");
    voiceDelayFeedback.addParameter("voice", ParameterType::INT, "Voice number (1-8)");
    voiceDelayFeedback.addParameter("feedback", ParameterType::FLOAT, "Feedback (0.0 to 0.95)");
    registry.registerCommand(std::move(voiceDelayFeedback));

    // VOICE_DELAY_MIX - Set delay wet/dry mix
    CommandDefinition voiceDelayMix("VOICE_DELAY_MIX",
                                     "Set delay wet/dry mix",
                                     "voice_set_delay_mix", "audio");
    voiceDelayMix.addParameter("voice", ParameterType::INT, "Voice number (1-8)");
    voiceDelayMix.addParameter("mix", ParameterType::FLOAT, "Wet mix (0.0=dry, 1.0=wet)");
    registry.registerCommand(std::move(voiceDelayMix));

    // =========================================================================
    // LFO Controls Commands
    // =========================================================================

    // LFO_WAVEFORM - Set LFO waveform type
    CommandDefinition lfoWaveform("LFO_WAVEFORM",
                                   "Set LFO waveform type",
                                   "lfo_set_waveform", "audio");
    lfoWaveform.addParameter("lfo_num", ParameterType::INT, "LFO number (1-4)");
    lfoWaveform.addParameter("waveform", ParameterType::INT, "Waveform (0=sine, 1=tri, 2=square, 3=saw, 4=random)");
    registry.registerCommand(std::move(lfoWaveform));

    // LFO_RATE - Set LFO rate in Hz
    CommandDefinition lfoRate("LFO_RATE",
                               "Set LFO rate in Hz",
                               "lfo_set_rate", "audio");
    lfoRate.addParameter("lfo_num", ParameterType::INT, "LFO number (1-4)");
    lfoRate.addParameter("rate_hz", ParameterType::FLOAT, "Rate in Hz");
    registry.registerCommand(std::move(lfoRate));

    // LFO_RESET - Reset LFO phase to start
    CommandDefinition lfoReset("LFO_RESET",
                                "Reset LFO phase to start",
                                "lfo_reset", "audio");
    lfoReset.addParameter("lfo_num", ParameterType::INT, "LFO number (1-4)");
    registry.registerCommand(std::move(lfoReset));

    // LFO_TO_PITCH - Route LFO to pitch (vibrato)
    CommandDefinition lfoToPitch("LFO_TO_PITCH",
                                  "Route LFO to pitch (vibrato)",
                                  "lfo_to_pitch", "audio");
    lfoToPitch.addParameter("voice", ParameterType::INT, "Voice number (1-8)");
    lfoToPitch.addParameter("lfo_num", ParameterType::INT, "LFO number (1-4, 0=off)");
    lfoToPitch.addParameter("depth_cents", ParameterType::FLOAT, "Modulation depth in cents");
    registry.registerCommand(std::move(lfoToPitch));

    // LFO_TO_VOLUME - Route LFO to volume (tremolo)
    CommandDefinition lfoToVolume("LFO_TO_VOLUME",
                                   "Route LFO to volume (tremolo)",
                                   "lfo_to_volume", "audio");
    lfoToVolume.addParameter("voice", ParameterType::INT, "Voice number (1-8)");
    lfoToVolume.addParameter("lfo_num", ParameterType::INT, "LFO number (1-4, 0=off)");
    lfoToVolume.addParameter("depth", ParameterType::FLOAT, "Modulation depth (0.0 to 1.0)");
    registry.registerCommand(std::move(lfoToVolume));

    // LFO_TO_FILTER - Route LFO to filter cutoff (auto-wah)
    CommandDefinition lfoToFilter("LFO_TO_FILTER",
                                   "Route LFO to filter cutoff (auto-wah)",
                                   "lfo_to_filter", "audio");
    lfoToFilter.addParameter("voice", ParameterType::INT, "Voice number (1-8)");
    lfoToFilter.addParameter("lfo_num", ParameterType::INT, "LFO number (1-4, 0=off)");
    lfoToFilter.addParameter("depth_hz", ParameterType::FLOAT, "Modulation depth in Hz");
    registry.registerCommand(std::move(lfoToFilter));

    // LFO_TO_PULSEWIDTH - Route LFO to pulse width (auto-PWM)
    CommandDefinition lfoToPulseWidth("LFO_TO_PULSEWIDTH",
                                       "Route LFO to pulse width (auto-PWM)",
                                       "lfo_to_pulsewidth", "audio");
    lfoToPulseWidth.addParameter("voice", ParameterType::INT, "Voice number (1-8)");
    lfoToPulseWidth.addParameter("lfo_num", ParameterType::INT, "LFO number (1-4, 0=off)");
    lfoToPulseWidth.addParameter("depth", ParameterType::FLOAT, "Modulation depth (0.0 to 1.0)");
    registry.registerCommand(std::move(lfoToPulseWidth));

    // =========================================================================
    // Physical Modeling Commands
    // =========================================================================

    // VOICE_PHYSICAL_MODEL - Set physical modeling type
    CommandDefinition voicePhysicalModel("VOICE_PHYSICAL_MODEL",
                                          "Set physical modeling type",
                                          "voice_set_physical_model", "audio");
    voicePhysicalModel.addParameter("voice", ParameterType::INT, "Voice number (1-8)");
    voicePhysicalModel.addParameter("model_type", ParameterType::INT, "Model (0=string, 1=bar, 2=tube, 3=drum, 4=glass)");
    registry.registerCommand(std::move(voicePhysicalModel));

    // VOICE_PHYSICAL_DAMPING - Set physical model damping
    CommandDefinition voicePhysicalDamping("VOICE_PHYSICAL_DAMPING",
                                            "Set physical model damping",
                                            "voice_set_physical_damping", "audio");
    voicePhysicalDamping.addParameter("voice", ParameterType::INT, "Voice number (1-8)");
    voicePhysicalDamping.addParameter("damping", ParameterType::FLOAT, "Damping (0.0=none, 1.0=max)");
    registry.registerCommand(std::move(voicePhysicalDamping));

    // VOICE_PHYSICAL_BRIGHTNESS - Set physical model brightness
    CommandDefinition voicePhysicalBrightness("VOICE_PHYSICAL_BRIGHTNESS",
                                               "Set physical model brightness",
                                               "voice_set_physical_brightness", "audio");
    voicePhysicalBrightness.addParameter("voice", ParameterType::INT, "Voice number (1-8)");
    voicePhysicalBrightness.addParameter("brightness", ParameterType::FLOAT, "Brightness (0.0=dark, 1.0=bright)");
    registry.registerCommand(std::move(voicePhysicalBrightness));

    // VOICE_PHYSICAL_EXCITATION - Set physical model excitation strength
    CommandDefinition voicePhysicalExcitation("VOICE_PHYSICAL_EXCITATION",
                                               "Set physical model excitation strength",
                                               "voice_set_physical_excitation", "audio");
    voicePhysicalExcitation.addParameter("voice", ParameterType::INT, "Voice number (1-8)");
    voicePhysicalExcitation.addParameter("excitation", ParameterType::FLOAT, "Excitation (0.0=gentle, 1.0=violent)");
    registry.registerCommand(std::move(voicePhysicalExcitation));

    // VOICE_PHYSICAL_RESONANCE - Set physical model resonance
    CommandDefinition voicePhysicalResonance("VOICE_PHYSICAL_RESONANCE",
                                              "Set physical model resonance",
                                              "voice_set_physical_resonance", "audio");
    voicePhysicalResonance.addParameter("voice", ParameterType::INT, "Voice number (1-8)");
    voicePhysicalResonance.addParameter("resonance", ParameterType::FLOAT, "Resonance (0.0=damped, 1.0=max)");
    registry.registerCommand(std::move(voicePhysicalResonance));

    // VOICE_PHYSICAL_TENSION - Set string tension (string models only)
    CommandDefinition voicePhysicalTension("VOICE_PHYSICAL_TENSION",
                                            "Set string tension (string models only)",
                                            "voice_set_physical_tension", "audio");
    voicePhysicalTension.addParameter("voice", ParameterType::INT, "Voice number (1-8)");
    voicePhysicalTension.addParameter("tension", ParameterType::FLOAT, "Tension (0.0=loose, 1.0=tight)");
    registry.registerCommand(std::move(voicePhysicalTension));

    // VOICE_PHYSICAL_PRESSURE - Set air pressure (wind models only)
    CommandDefinition voicePhysicalPressure("VOICE_PHYSICAL_PRESSURE",
                                             "Set air pressure (wind models only)",
                                             "voice_set_physical_pressure", "audio");
    voicePhysicalPressure.addParameter("voice", ParameterType::INT, "Voice number (1-8)");
    voicePhysicalPressure.addParameter("pressure", ParameterType::FLOAT, "Pressure (0.0=gentle, 1.0=strong)");
    registry.registerCommand(std::move(voicePhysicalPressure));

    // VOICE_PHYSICAL_TRIGGER - Trigger physical model excitation
    CommandDefinition voicePhysicalTrigger("VOICE_PHYSICAL_TRIGGER",
                                            "Trigger physical model excitation",
                                            "voice_physical_trigger", "audio");
    voicePhysicalTrigger.addParameter("voice", ParameterType::INT, "Voice number (1-8)");
    registry.registerCommand(std::move(voicePhysicalTrigger));
}

// =============================================================================
// Voice Controller Functions
// =============================================================================

void SuperTerminalCommandRegistry::registerVoiceFunctions(CommandRegistry& registry) {
    // VOICE_GET_MASTER_VOLUME - Get voice master volume
    CommandDefinition voiceGetMasterVolume("VOICE_GET_MASTER_VOLUME",
                                           "Get voice master volume",
                                           "voice_get_master_volume", "audio", false, ReturnType::FLOAT);
    registry.registerFunction(std::move(voiceGetMasterVolume));

    // VOICE_GET_ACTIVE_COUNT - Get number of active voices
    CommandDefinition voiceGetActiveCount("VOICE_GET_ACTIVE_COUNT",
                                          "Get number of active voices (gate on)",
                                          "voice_get_active_count", "audio", false, ReturnType::INT);
    registry.registerFunction(std::move(voiceGetActiveCount));

    // VOICES_ARE_PLAYING - Check if VOICES_END_PLAY is currently playing
    CommandDefinition voicesArePlaying("VOICES_ARE_PLAYING",
                                       "Check if VOICES_END_PLAY buffer is currently playing",
                                       "voices_are_playing", "audio", false, ReturnType::INT);
    registry.registerFunction(std::move(voicesArePlaying));
}

// =============================================================================
// Synth Commands
// =============================================================================

void SuperTerminalCommandRegistry::registerInputCommands(CommandRegistry& registry) {
    // WAIT_KEY - Wait for key press
    CommandDefinition waitKey("WAIT_KEY",
                             "Wait for a key to be pressed",
                             "", "input");
    waitKey.setCustomCodeGen("repeat wait_frame() until key_get_char()");
    registry.registerCommand(std::move(waitKey));

    // WAITKEY - Wait for single keypress with optional timeout (returns string)
    CommandDefinition waitkey("WAITKEY",
                             "Wait for single keypress with optional timeout",
                             "basic_waitkey", "input");
    waitkey.addParameter("timeout", ParameterType::FLOAT, "Timeout in seconds (0=no timeout)", true, "0")
           .setReturnType(ReturnType::STRING);
    registry.registerFunction(std::move(waitkey));

    // GET_MOUSE - Get mouse position
    CommandDefinition getMouse("GET_MOUSE",
                              "Get current mouse coordinates",
                              "mouse_position", "input");
    registry.registerCommand(std::move(getMouse));

    // INPUT_AT - Removed from registry, uses custom parsing in lexer/parser


    // KEY_CLEAR_BUFFER - Clear keyboard input buffer
    CommandDefinition keyClear("KEY_CLEAR_BUFFER",
                              "Clear the keyboard input buffer",
                              "key_clear_buffer", "input");
    registry.registerCommand(std::move(keyClear));
}

// =============================================================================
// Utility Commands
// =============================================================================

void SuperTerminalCommandRegistry::registerUtilityCommands(CommandRegistry& registry) {
    // CLS - Clear text screen (SuperTerminal GUI implementation)
    CommandDefinition cls("CLS",
                         "Clear the text screen",
                         "text_clear", "utility");
    registry.registerCommand(std::move(cls));

    // CLEAR_SCREEN - Clear the entire screen
    CommandDefinition clearScreen("CLEAR_SCREEN",
                                 "Clear the graphics screen",
                                 "gfx_clear", "utility");
    clearScreen.addParameter("color", ParameterType::COLOR, "Clear color", true, "0xFF000000");
    registry.registerCommand(std::move(clearScreen));

    // CLEAR_ALL_LAYERS - Clear all display layers
    CommandDefinition clearAllLayers("CLEAR_ALL_LAYERS",
                                     "Clear all display layers (text, graphics, rectangles, circles, particles)",
                                     "st_clear_all_layers", "utility");
    registry.registerCommand(std::move(clearAllLayers));

    // VSYNC - Wait for vertical sync
    CommandDefinition vsync("VSYNC",
                           "Wait for vertical sync (frame)",
                           "wait_frame", "utility");
    registry.registerCommand(std::move(vsync));

    // WAIT_FRAMES - Wait for specific number of frames
    CommandDefinition waitFrames("WAIT_FRAMES",
                                "Wait for a specified number of frames",
                                "wait_frames", "utility");
    waitFrames.addParameter("count", ParameterType::INT, "Number of frames to wait");
    registry.registerCommand(std::move(waitFrames));

    // WAIT - Alias for WAIT_FRAMES (common BASIC usage)
    CommandDefinition wait("WAIT",
                          "Wait for specified number of frames",
                          "wait_frames", "utility");
    wait.addParameter("count", ParameterType::INT, "Number of frames to wait");
    registry.registerCommand(std::move(wait));

    // WAIT_MS - Wait for specific number of milliseconds (with cancellation support)
    CommandDefinition waitMs("WAIT_MS",
                             "Wait for specified number of milliseconds",
                             "wait_ms", "utility");
    waitMs.addParameter("milliseconds", ParameterType::INT, "Number of milliseconds to wait");
    registry.registerCommand(std::move(waitMs));

    // SCREEN_SIZE - Get screen dimensions
    CommandDefinition screenSize("SCREEN_SIZE",
                                "Get the screen dimensions",
                                "display_size", "utility");
    registry.registerCommand(std::move(screenSize));

    // DEBUG_PRINT - Print debug message
    CommandDefinition debugPrint("DEBUG_PRINT",
                                "Print a debug message to console",
                                "debug_print", "utility");
    debugPrint.addParameter("message", ParameterType::STRING, "Debug message to print");
    registry.registerCommand(std::move(debugPrint));

    // CLG/GCLS - Clear graphics screen
    CommandDefinition clg("CLG",
                         "Clear the graphics screen (LORES graphics)",
                         "lores_clear", "utility");
    registry.registerCommand(std::move(clg));

    CommandDefinition gcls("GCLS",
                          "Clear the graphics screen (high-resolution graphics)",
                          "gfx_clear", "utility");
    registry.registerCommand(std::move(gcls));

    // COLOR_RGB - Create RGB color value
    CommandDefinition colorRgb("COLOR_RGB",
                               "Create an RGB color value",
                               "rgb", "utility", false, ReturnType::INT);
    colorRgb.addParameter("red", ParameterType::INT, "Red component (0-255)")
            .addParameter("green", ParameterType::INT, "Green component (0-255)")
            .addParameter("blue", ParameterType::INT, "Blue component (0-255)");
    registry.registerFunction(std::move(colorRgb));

    // COLOR_RGBA - Create RGBA color value
    CommandDefinition colorRgba("COLOR_RGBA",
                                "Create an RGBA color value with alpha",
                                "rgba", "utility", false, ReturnType::INT);
    colorRgba.addParameter("red", ParameterType::INT, "Red component (0-255)")
             .addParameter("green", ParameterType::INT, "Green component (0-255)")
             .addParameter("blue", ParameterType::INT, "Blue component (0-255)")
             .addParameter("alpha", ParameterType::INT, "Alpha component (0-255)");
    registry.registerFunction(std::move(colorRgba));

    // COLOR_HSV - Create color from HSV
    CommandDefinition colorHsv("COLOR_HSV",
                               "Create a color from HSV (Hue, Saturation, Value)",
                               "hsv", "utility", false, ReturnType::INT);
    colorHsv.addParameter("hue", ParameterType::FLOAT, "Hue (0-360)")
            .addParameter("saturation", ParameterType::FLOAT, "Saturation (0-255)")
            .addParameter("value", ParameterType::FLOAT, "Value/Brightness (0-255)");
    registry.registerFunction(std::move(colorHsv));

    // XRGB - Create XRES palette index from RGB
    CommandDefinition xrgbFunc("XRGB",
                               "Map RGB (0-255) to XRES palette index (16-255)",
                               "xrgb", "utility", false, ReturnType::INT);
    xrgbFunc.addParameter("red", ParameterType::INT, "Red component (0-255)")
            .addParameter("green", ParameterType::INT, "Green component (0-255)")
            .addParameter("blue", ParameterType::INT, "Blue component (0-255)");
    registry.registerFunction(std::move(xrgbFunc));

    // WRGB - Create WRES palette index from RGB
    CommandDefinition wrgbFunc("WRGB",
                               "Map RGB (0-255) to WRES palette index (16-255)",
                               "wrgb", "utility", false, ReturnType::INT);
    wrgbFunc.addParameter("red", ParameterType::INT, "Red component (0-255)")
            .addParameter("green", ParameterType::INT, "Green component (0-255)")
            .addParameter("blue", ParameterType::INT, "Blue component (0-255)");
    registry.registerFunction(std::move(wrgbFunc));

    // PRGB - Create PRES palette index from RGB
    CommandDefinition prgbFunc("PRGB",
                               "Map RGB (0-255) to PRES palette index (16-255)",
                               "prgb", "utility", false, ReturnType::INT);
    prgbFunc.addParameter("red", ParameterType::INT, "Red component (0-255)")
            .addParameter("green", ParameterType::INT, "Green component (0-255)")
            .addParameter("blue", ParameterType::INT, "Blue component (0-255)");
    registry.registerFunction(std::move(prgbFunc));

    // URGB - Create URES direct color (ARGB4444) from RGB
    CommandDefinition urgbFunc("URGB",
                               "Create URES ARGB4444 color from RGB (0-15 each)",
                               "urgb", "utility", false, ReturnType::INT);
    urgbFunc.addParameter("red", ParameterType::INT, "Red component (0-15)")
            .addParameter("green", ParameterType::INT, "Green component (0-15)")
            .addParameter("blue", ParameterType::INT, "Blue component (0-15)");
    registry.registerFunction(std::move(urgbFunc));

    // URGBA - Create URES direct color (ARGB4444) from RGBA
    CommandDefinition urgbaFunc("URGBA",
                                "Create URES ARGB4444 color from RGBA (0-15 each)",
                                "urgba", "utility", false, ReturnType::INT);
    urgbaFunc.addParameter("red", ParameterType::INT, "Red component (0-15)")
             .addParameter("green", ParameterType::INT, "Green component (0-15)")
             .addParameter("blue", ParameterType::INT, "Blue component (0-15)")
             .addParameter("alpha", ParameterType::INT, "Alpha component (0-15)");
    registry.registerFunction(std::move(urgbaFunc));

    // RGB - Shorthand for COLOR_RGB
    CommandDefinition rgbFunc("RGB",
                             "Create an RGB color value (shorthand)",
                             "rgb", "utility", false, ReturnType::INT);
    rgbFunc.addParameter("red", ParameterType::INT, "Red component (0-255)")
           .addParameter("green", ParameterType::INT, "Green component (0-255)")
           .addParameter("blue", ParameterType::INT, "Blue component (0-255)");
    registry.registerFunction(std::move(rgbFunc));

    // RGBA - Shorthand for COLOR_RGBA
    CommandDefinition rgbaFunc("RGBA",
                              "Create an RGBA color value (shorthand)",
                              "rgba", "utility", false, ReturnType::INT);
    rgbaFunc.addParameter("red", ParameterType::INT, "Red component (0-255)")
            .addParameter("green", ParameterType::INT, "Green component (0-255)")
            .addParameter("blue", ParameterType::INT, "Blue component (0-255)")
            .addParameter("alpha", ParameterType::INT, "Alpha component (0-255)");
    registry.registerFunction(std::move(rgbaFunc));
}

// =============================================================================
// Sprite Commands (Minimal Implementation)
// =============================================================================

void SuperTerminalCommandRegistry::registerSpriteCommands(CommandRegistry& registry) {
    registerSpriteManagementCommands(registry);
    registerSpriteEffectsCommands(registry);
    registerSpriteFunctions(registry);
}

void SuperTerminalCommandRegistry::registerSpriteManagementCommands(CommandRegistry& registry) {
    // SPRLOAD - Load sprite
    CommandDefinition sprload("SPRLOAD",
                             "Load a sprite from file",
                             "", "sprite");
    sprload.addParameter("id", ParameterType::INT, "Sprite ID")
           .addParameter("file", ParameterType::STRING, "Sprite file path")
           .addParameter("builtin", ParameterType::BOOL, "Use builtin sprite", true, "false")
           .setCustomCodeGen("if {2} then sprite_load_builtin({0}, {1}) else sprite_load({0}, {1}) end");
    registry.registerCommand(std::move(sprload));

    // SPRMOVE - Move sprite
    CommandDefinition sprmove("SPRMOVE",
                             "Move a sprite to new position",
                             "", "sprite");
    sprmove.addParameter("id", ParameterType::INT, "Sprite ID")
           .addParameter("x", ParameterType::INT, "X coordinate")
           .addParameter("y", ParameterType::INT, "Y coordinate")
           .setCustomCodeGen("sprite_transform({0}, {1}, {2}, 1, 0)");
    registry.registerCommand(std::move(sprmove));
}

void SuperTerminalCommandRegistry::registerSpriteEffectsCommands(CommandRegistry& registry) {
    // SPRSHOW - Show sprite
    CommandDefinition sprshow("SPRSHOW",
                             "Show a sprite",
                             "sprite_show", "sprite");
    sprshow.addParameter("id", ParameterType::INT, "Sprite ID");
    registry.registerCommand(std::move(sprshow));

    // SPRHIDE - Hide sprite
    CommandDefinition sprhide("SPRHIDE",
                             "Hide a sprite",
                             "sprite_hide", "sprite");
    sprhide.addParameter("id", ParameterType::INT, "Sprite ID");
    registry.registerCommand(std::move(sprhide));

    // SPRFREE - Free sprite
    CommandDefinition sprfree("SPRFREE",
                             "Free/unload a sprite",
                             "sprite_unload", "sprite");
    sprfree.addParameter("id", ParameterType::INT, "Sprite ID");
    registry.registerCommand(std::move(sprfree));

    // SPRPOS - Position/transform sprite
    CommandDefinition sprpos("SPRPOS",
                            "Position and transform a sprite",
                            "sprite_transform", "sprite");
    sprpos.addParameter("id", ParameterType::INT, "Sprite ID")
          .addParameter("x", ParameterType::INT, "X coordinate")
          .addParameter("y", ParameterType::INT, "Y coordinate")
          .addParameter("scale", ParameterType::FLOAT, "Scale factor")
          .addParameter("angle", ParameterType::FLOAT, "Rotation angle");
    registry.registerCommand(std::move(sprpos));

    // SPRTINT - Tint sprite
    CommandDefinition sprtint("SPRTINT",
                             "Tint a sprite with RGBA values",
                             "sprite_tint", "sprite");
    sprtint.addParameter("id", ParameterType::INT, "Sprite ID")
           .addParameter("r", ParameterType::INT, "Red component (0-255)")
           .addParameter("g", ParameterType::INT, "Green component (0-255)")
           .addParameter("b", ParameterType::INT, "Blue component (0-255)")
           .addParameter("a", ParameterType::INT, "Alpha component (0-255)");
    registry.registerCommand(std::move(sprtint));

    // SPREXPLODE - Explode sprite effect
    CommandDefinition sprexplode("SPREXPLODE",
                                "Create sprite explosion effect",
                                "", "sprite");
    sprexplode.addParameter("id", ParameterType::INT, "Sprite ID")
              .addParameter("x", ParameterType::INT, "X coordinate")
              .addParameter("y", ParameterType::INT, "Y coordinate")
              .addParameter("count", ParameterType::INT, "Particle count", true, "50")
              .addParameter("speed", ParameterType::FLOAT, "Explosion speed", true, "100.0")
              .addParameter("spread", ParameterType::FLOAT, "Spread factor", true, "1.0")
              .addParameter("life", ParameterType::FLOAT, "Particle lifetime", true, "2.0")
              .addParameter("fade", ParameterType::FLOAT, "Fade time", true, "1.0")
              .setCustomCodeGen("if {3} and {4} and {5} and {6} and {7} then sprite_explode_advanced({0}, {1}, {2}, {3}, {4}, {5}, {6}, {7}) else sprite_explode({0}, {1}, {2}) end");
    registry.registerCommand(std::move(sprexplode));
}

void SuperTerminalCommandRegistry::registerSpriteFunctions(CommandRegistry& registry) {
    // SPRITE_LOAD - Load sprite function (returns sprite ID)
    CommandDefinition sprite_load("SPRITE_LOAD",
                                   "Load a sprite from file and return sprite ID",
                                   "sprite_load", "sprite", false, ReturnType::INT);
    sprite_load.addParameter("file", ParameterType::STRING, "Sprite file path");
    registry.registerFunction(std::move(sprite_load));

    // SPRITE_LOAD_BUILTIN - Load builtin sprite function (returns sprite ID)
    CommandDefinition sprite_load_builtin("SPRITE_LOAD_BUILTIN",
                                           "Load a builtin sprite and return sprite ID",
                                           "sprite_load_builtin", "sprite", false, ReturnType::INT);
    sprite_load_builtin.addParameter("name", ParameterType::STRING, "Builtin sprite name");
    registry.registerFunction(std::move(sprite_load_builtin));

    // DRAWINTOSPRITE - Begin drawing into a sprite (returns sprite ID)
    CommandDefinition draw_into_sprite("DRAWINTOSPRITE",
                                        "Begin drawing graphics into a sprite and return sprite ID",
                                        "sprite_begin_draw", "sprite", false, ReturnType::INT);
    draw_into_sprite.addParameter("width", ParameterType::INT, "Sprite width in pixels")
                    .addParameter("height", ParameterType::INT, "Sprite height in pixels");
    registry.registerFunction(std::move(draw_into_sprite));

    // ENDDRAWINTOSPRITE - End drawing into a sprite
    CommandDefinition end_draw_into_sprite("ENDDRAWINTOSPRITE",
                                            "Finalize sprite drawing and upload texture",
                                            "sprite_end_draw", "sprite", false, ReturnType::VOID);
    registry.registerCommand(std::move(end_draw_into_sprite));

    // DRAWTOFILE - Begin drawing to a file (returns success boolean)
    CommandDefinition draw_to_file("DRAWTOFILE",
                                    "Begin drawing graphics to a PNG file",
                                    "draw_to_file_begin", "file", false, ReturnType::BOOL);
    draw_to_file.addParameter("filename", ParameterType::STRING, "Output PNG filename")
                .addParameter("width", ParameterType::INT, "Image width in pixels")
                .addParameter("height", ParameterType::INT, "Image height in pixels");
    registry.registerFunction(std::move(draw_to_file));

    // ENDDRAWTOFILE - End drawing to file
    CommandDefinition end_draw_to_file("ENDDRAWTOFILE",
                                        "Finalize file drawing and save PNG",
                                        "draw_to_file_end", "file", false, ReturnType::BOOL);
    registry.registerFunction(std::move(end_draw_to_file));

    // DRAWTOTILESET - Begin drawing a tileset (returns tileset ID)
    CommandDefinition draw_to_tileset("DRAWTOTILESET",
                                       "Begin drawing graphics into a tileset atlas and return tileset ID",
                                       "tileset_begin_draw", "tileset", false, ReturnType::INT);
    draw_to_tileset.addParameter("tile_width", ParameterType::INT, "Tile width in pixels")
                   .addParameter("tile_height", ParameterType::INT, "Tile height in pixels")
                   .addParameter("columns", ParameterType::INT, "Number of tile columns")
                   .addParameter("rows", ParameterType::INT, "Number of tile rows");
    registry.registerFunction(std::move(draw_to_tileset));

    // DRAWTILE - Select which tile to draw into
    CommandDefinition draw_tile("DRAWTILE",
                                 "Select which tile in the tileset to draw into",
                                 "tileset_draw_tile", "tileset", false, ReturnType::BOOL);
    draw_tile.addParameter("tile_index", ParameterType::INT, "Tile index (0-based)");
    registry.registerFunction(std::move(draw_tile));

    // ENDDRAWTOTILESET - End drawing to tileset
    CommandDefinition end_draw_to_tileset("ENDDRAWTOTILESET",
                                           "Finalize tileset drawing and upload texture atlas",
                                           "tileset_end_draw", "tileset", false, ReturnType::BOOL);
    registry.registerFunction(std::move(end_draw_to_tileset));

    // SPRITE_SHOW - Show sprite at position
    CommandDefinition sprite_show("SPRITE_SHOW",
                                   "Show a sprite at specified position",
                                   "sprite_show", "sprite", false, ReturnType::VOID);
    sprite_show.addParameter("id", ParameterType::INT, "Sprite ID")
               .addParameter("x", ParameterType::INT, "X coordinate")
               .addParameter("y", ParameterType::INT, "Y coordinate");
    registry.registerCommand(std::move(sprite_show));

    // SPRITE_HIDE - Hide sprite
    CommandDefinition sprite_hide("SPRITE_HIDE",
                                   "Hide a sprite",
                                   "sprite_hide", "sprite", false, ReturnType::VOID);
    sprite_hide.addParameter("id", ParameterType::INT, "Sprite ID");
    registry.registerCommand(std::move(sprite_hide));

    // SPRITE_TRANSFORM - Transform sprite (position, rotation, scale)
    CommandDefinition sprite_transform("SPRITE_TRANSFORM",
                                        "Transform sprite with position, rotation and scale",
                                        "sprite_transform", "sprite", false, ReturnType::VOID);
    sprite_transform.addParameter("id", ParameterType::INT, "Sprite ID")
                    .addParameter("x", ParameterType::INT, "X coordinate")
                    .addParameter("y", ParameterType::INT, "Y coordinate")
                    .addParameter("rotation", ParameterType::FLOAT, "Rotation in degrees")
                    .addParameter("scaleX", ParameterType::FLOAT, "X scale factor")
                    .addParameter("scaleY", ParameterType::FLOAT, "Y scale factor");
    registry.registerCommand(std::move(sprite_transform));

    // SPRITE_TINT - Tint sprite with color
    CommandDefinition sprite_tint("SPRITE_TINT",
                                   "Tint a sprite with a color",
                                   "sprite_tint", "sprite", false, ReturnType::VOID);
    sprite_tint.addParameter("id", ParameterType::INT, "Sprite ID")
               .addParameter("color", ParameterType::COLOR, "Tint color");
    registry.registerCommand(std::move(sprite_tint));

    // SPRITE_UNLOAD - Unload/free sprite
    CommandDefinition sprite_unload("SPRITE_UNLOAD",
                                     "Unload and free a sprite",
                                     "sprite_unload", "sprite", false, ReturnType::VOID);
    sprite_unload.addParameter("id", ParameterType::INT, "Sprite ID");
    registry.registerCommand(std::move(sprite_unload));

    // SPRITE_UNLOAD_ALL - Unload all sprites
    CommandDefinition sprite_unload_all("SPRITE_UNLOAD_ALL",
                                         "Unload and free all sprites",
                                         "sprite_unload_all", "sprite", false, ReturnType::VOID);
    registry.registerCommand(std::move(sprite_unload_all));

    // =============================================================================
    // Indexed Sprite Commands
    // =============================================================================

    // SPRITE_LOAD_SPRTZ - Load indexed sprite from .sprtz file
    CommandDefinition sprite_load_sprtz("SPRITE_LOAD_SPRTZ",
                                         "Load an indexed sprite from .sprtz file",
                                         "sprite_load_sprtz", "sprite", false, ReturnType::INT);
    sprite_load_sprtz.addParameter("path", ParameterType::STRING, "Path to .sprtz file");
    registry.registerFunction(std::move(sprite_load_sprtz));

    // SPRITE_IS_INDEXED - Check if sprite is indexed
    CommandDefinition sprite_is_indexed("SPRITE_IS_INDEXED",
                                         "Check if a sprite uses indexed colors",
                                         "sprite_is_indexed", "sprite", false, ReturnType::BOOL);
    sprite_is_indexed.addParameter("sprite_id", ParameterType::INT, "Sprite ID");
    registry.registerFunction(std::move(sprite_is_indexed));

    // SPRITE_SET_PALETTE - Set sprite to use a standard palette
    CommandDefinition sprite_set_palette("SPRITE_SET_PALETTE",
                                          "Set an indexed sprite to use a standard palette",
                                          "sprite_set_standard_palette", "sprite", false, ReturnType::BOOL);
    sprite_set_palette.addParameter("sprite_id", ParameterType::INT, "Sprite ID")
                      .addParameter("palette_id", ParameterType::INT, "Standard palette ID (0-31)");
    registry.registerFunction(std::move(sprite_set_palette));

    // SPRITE_SET_PALETTE_COLOR - Set a single color in sprite palette
    CommandDefinition sprite_set_palette_color("SPRITE_SET_PALETTE_COLOR",
                                                "Set a single color in an indexed sprite's palette",
                                                "sprite_set_palette_color", "sprite", false, ReturnType::BOOL);
    sprite_set_palette_color.addParameter("sprite_id", ParameterType::INT, "Sprite ID")
                            .addParameter("color_index", ParameterType::INT, "Color index (0-15)")
                            .addParameter("r", ParameterType::INT, "Red (0-255)")
                            .addParameter("g", ParameterType::INT, "Green (0-255)")
                            .addParameter("b", ParameterType::INT, "Blue (0-255)")
                            .addParameter("a", ParameterType::INT, "Alpha (0-255, default 255)", true);
    registry.registerFunction(std::move(sprite_set_palette_color));

    // SPRITE_ROTATE_PALETTE - Rotate palette colors for animation
    CommandDefinition sprite_rotate_palette("SPRITE_ROTATE_PALETTE",
                                             "Rotate a range of colors in sprite palette",
                                             "sprite_rotate_palette", "sprite", false, ReturnType::BOOL);
    sprite_rotate_palette.addParameter("sprite_id", ParameterType::INT, "Sprite ID")
                         .addParameter("start_index", ParameterType::INT, "Start color index")
                         .addParameter("end_index", ParameterType::INT, "End color index")
                         .addParameter("amount", ParameterType::INT, "Rotation amount");
    registry.registerFunction(std::move(sprite_rotate_palette));

    // SPRITE_ADJUST_BRIGHTNESS - Adjust sprite palette brightness
    CommandDefinition sprite_adjust_brightness("SPRITE_ADJUST_BRIGHTNESS",
                                                "Adjust brightness of sprite palette",
                                                "sprite_adjust_brightness", "sprite", false, ReturnType::BOOL);
    sprite_adjust_brightness.addParameter("sprite_id", ParameterType::INT, "Sprite ID")
                            .addParameter("brightness", ParameterType::FLOAT, "Brightness (1.0=normal, 0.5=darker, 2.0=brighter)");
    registry.registerFunction(std::move(sprite_adjust_brightness));

    // SPRITE_COPY_PALETTE - Copy palette from one sprite to another
    CommandDefinition sprite_copy_palette("SPRITE_COPY_PALETTE",
                                           "Copy palette from one indexed sprite to another",
                                           "sprite_copy_palette", "sprite", false, ReturnType::BOOL);
    sprite_copy_palette.addParameter("src_sprite", ParameterType::INT, "Source sprite ID")
                       .addParameter("dst_sprite", ParameterType::INT, "Destination sprite ID");
    registry.registerFunction(std::move(sprite_copy_palette));

    // SPRITE_EXPLODE - Basic sprite explosion effect (returns bool)
    CommandDefinition sprite_explode("SPRITE_EXPLODE",
                                      "Create a basic sprite explosion effect",
                                      "sprite_explode", "sprite", false, ReturnType::BOOL);
    sprite_explode.addParameter("sprite_id", ParameterType::INT, "Sprite ID")
                  .addParameter("particle_count", ParameterType::INT, "Number of particles");
    registry.registerFunction(std::move(sprite_explode));

    // SPRITE_EXPLODE_ADVANCED - Advanced sprite explosion (returns bool)
    CommandDefinition sprite_explode_advanced("SPRITE_EXPLODE_ADVANCED",
                                               "Create an advanced sprite explosion with custom physics",
                                               "sprite_explode_advanced", "sprite", false, ReturnType::BOOL);
    sprite_explode_advanced.addParameter("sprite_id", ParameterType::INT, "Sprite ID")
                           .addParameter("particle_count", ParameterType::INT, "Number of particles")
                           .addParameter("explosion_force", ParameterType::FLOAT, "Explosion force")
                           .addParameter("gravity", ParameterType::FLOAT, "Gravity strength")
                           .addParameter("fade_time", ParameterType::FLOAT, "Fade time in seconds");
    registry.registerFunction(std::move(sprite_explode_advanced));

    // SPRITE_EXPLODE_DIRECTIONAL - Directional sprite explosion (returns bool)
    CommandDefinition sprite_explode_directional("SPRITE_EXPLODE_DIRECTIONAL",
                                                  "Create a directional sprite explosion",
                                                  "sprite_explode_directional", "sprite", false, ReturnType::BOOL);
    sprite_explode_directional.addParameter("sprite_id", ParameterType::INT, "Sprite ID")
                              .addParameter("particle_count", ParameterType::INT, "Number of particles")
                              .addParameter("force_x", ParameterType::FLOAT, "Horizontal force")
                              .addParameter("force_y", ParameterType::FLOAT, "Vertical force");
    registry.registerFunction(std::move(sprite_explode_directional));

    // SPRITE_EXPLODE_SIZE - Sprite explosion with custom particle size (returns bool)
    CommandDefinition sprite_explode_size("SPRITE_EXPLODE_SIZE",
                                           "Create sprite explosion with custom particle size",
                                           "sprite_explode_size", "sprite", false, ReturnType::BOOL);
    sprite_explode_size.addParameter("sprite_id", ParameterType::INT, "Sprite ID")
                       .addParameter("particle_count", ParameterType::INT, "Number of particles")
                       .addParameter("size_multiplier", ParameterType::FLOAT, "Particle size multiplier (1.0-100.0)");
    registry.registerFunction(std::move(sprite_explode_size));

    // SPRITE_EXPLODE_MODE - Preset sprite explosion modes (returns bool)
    CommandDefinition sprite_explode_mode("SPRITE_EXPLODE_MODE",
                                           "Create sprite explosion using preset mode",
                                           "sprite_explode_mode", "sprite", false, ReturnType::BOOL);
    sprite_explode_mode.addParameter("sprite_id", ParameterType::INT, "Sprite ID")
                       .addParameter("mode", ParameterType::INT, "Explosion mode (1-6)");
    registry.registerFunction(std::move(sprite_explode_mode));
}

// =============================================================================
// Stub Implementations for Complex Systems
// =============================================================================

void SuperTerminalCommandRegistry::registerParticleCommands(CommandRegistry& registry) {
    // PARTCLEAR - Clear all particles
    CommandDefinition partclear("PARTCLEAR",
                               "Clear all particles from the particle system",
                               "st_particle_clear", "particle");
    registry.registerCommand(std::move(partclear));

    // PARTPAUSE - Pause particle system
    CommandDefinition partpause("PARTPAUSE",
                               "Pause the particle system",
                               "st_particle_pause", "particle");
    registry.registerCommand(std::move(partpause));

    // PARTRESUME - Resume particle system
    CommandDefinition partresume("PARTRESUME",
                                "Resume the particle system",
                                "st_particle_resume", "particle");
    registry.registerCommand(std::move(partresume));

    // PARTTIME - Set particle time scale
    CommandDefinition parttime("PARTTIME",
                              "Set particle system time scale",
                              "st_particle_set_time_scale", "particle");
    parttime.addParameter("timescale", ParameterType::FLOAT, "Time scale multiplier");
    registry.registerCommand(std::move(parttime));

    // PARTBOUNDS - Set particle world bounds
    CommandDefinition partbounds("PARTBOUNDS",
                                "Set particle system world bounds",
                                "", "particle");
    partbounds.addParameter("x1", ParameterType::INT, "Left bound")
              .addParameter("y1", ParameterType::INT, "Top bound")
              .addParameter("x2", ParameterType::INT, "Right bound")
              .addParameter("y2", ParameterType::INT, "Bottom bound")
              .setCustomCodeGen("st_particle_set_world_bounds({0}, {1}, {2}, {3})");
    registry.registerCommand(std::move(partbounds));

    // PARTENABLE - Enable/disable particle system
    CommandDefinition partenable("PARTENABLE",
                                "Enable or disable the particle system",
                                "st_particle_set_enabled", "particle");
    partenable.addParameter("enabled", ParameterType::BOOL, "Enable (true) or disable (false)");
    registry.registerCommand(std::move(partenable));

    // PARTICLE_DUMP_STATS - Dump particle system statistics
    CommandDefinition partStats("PARTICLE_DUMP_STATS",
                               "Dump particle system statistics to console",
                               "st_particle_dump_stats", "particle");
    registry.registerCommand(std::move(partStats));
}

void SuperTerminalCommandRegistry::registerChunkyGraphicsCommands(CommandRegistry& registry) {
    // LPSET - Set LORES pixel
    CommandDefinition lpset("LPSET",
                          "Set a LORES graphics pixel",
                          "pset", "lores");
    lpset.addParameter("x", ParameterType::INT, "X coordinate")
        .addParameter("y", ParameterType::INT, "Y coordinate")
        .addParameter("color", ParameterType::INT, "Color index (0-15)")
        .addParameter("bg", ParameterType::COLOR, "Background color", true, "0xFF000000");
    registry.registerCommand(std::move(lpset));

    // LLINE - Draw LORES line
    CommandDefinition lline("LLINE",
                           "Draw a line in LORES graphics",
                           "line", "lores");
    lline.addParameter("x1", ParameterType::INT, "Start X coordinate")
         .addParameter("y1", ParameterType::INT, "Start Y coordinate")
         .addParameter("x2", ParameterType::INT, "End X coordinate")
         .addParameter("y2", ParameterType::INT, "End Y coordinate")
         .addParameter("color", ParameterType::INT, "Color index (0-15)")
         .addParameter("bg", ParameterType::COLOR, "Background color", true, "0xFF000000");
    registry.registerCommand(std::move(lline));

    // LRECT - Draw LORES rectangle outline
    CommandDefinition lrect("LRECT",
                           "Draw a rectangle outline in LORES graphics",
                           "rect", "lores");
    lrect.addParameter("x", ParameterType::INT, "X coordinate")
         .addParameter("y", ParameterType::INT, "Y coordinate")
         .addParameter("w", ParameterType::INT, "Width")
         .addParameter("h", ParameterType::INT, "Height")
         .addParameter("color", ParameterType::INT, "Color index (0-15)")
         .addParameter("bg", ParameterType::COLOR, "Background color", true, "0xFF000000");
    registry.registerCommand(std::move(lrect));

    // LFILLRECT - Draw filled LORES rectangle
    CommandDefinition lfillrect("LFILLRECT",
                            "Draw a filled rectangle in LORES graphics",
                            "fillrect", "lores");
    lfillrect.addParameter("x", ParameterType::INT, "X coordinate")
          .addParameter("y", ParameterType::INT, "Y coordinate")
          .addParameter("w", ParameterType::INT, "Width")
          .addParameter("h", ParameterType::INT, "Height")
          .addParameter("color", ParameterType::INT, "Color index (0-15)")
          .addParameter("bg", ParameterType::COLOR, "Background color", true, "0xFF000000");
    registry.registerCommand(std::move(lfillrect));

    // LHLINE - Draw LORES horizontal line
    CommandDefinition lhline("LHLINE",
                            "Draw a horizontal line in LORES graphics",
                            "hline", "lores");
    lhline.addParameter("x", ParameterType::INT, "Start X coordinate")
          .addParameter("y", ParameterType::INT, "Y coordinate")
          .addParameter("width", ParameterType::INT, "Line width")
          .addParameter("color", ParameterType::INT, "Color index (0-15)")
          .addParameter("bg", ParameterType::COLOR, "Background color", true, "0xFF000000");
    registry.registerCommand(std::move(lhline));

    // LVLINE - Draw LORES vertical line
    CommandDefinition lvline("LVLINE",
                            "Draw a vertical line in LORES graphics",
                            "vline", "lores");
    lvline.addParameter("x", ParameterType::INT, "X coordinate")
          .addParameter("y", ParameterType::INT, "Start Y coordinate")
          .addParameter("height", ParameterType::INT, "Line height")
          .addParameter("color", ParameterType::INT, "Color index (0-15)")
          .addParameter("bg", ParameterType::COLOR, "Background color", true, "0xFF000000");
    registry.registerCommand(std::move(lvline));

    // LCLEAR - Clear LORES graphics
    CommandDefinition lclear("LCLEAR",
                            "Clear all LORES graphics",
                            "lores_clear", "lores");
    lclear.addParameter("bg", ParameterType::COLOR, "Background color", true, "0xFF000000");
    registry.registerCommand(std::move(lclear));

    // LRESOLUTION - Get LORES resolution
    CommandDefinition lresolution("LRESOLUTION",
                                 "Get LORES graphics resolution",
                                 "lores_resolution", "lores");
    registry.registerCommand(std::move(lresolution));

    // LPALETTE_SET - Set all palettes to preset
    CommandDefinition lpaletteSet("LPALETTE_SET",
                                 "Set all LORES palettes to preset (IBM or C64)",
                                 "lores_palette_set", "lores");
    lpaletteSet.addParameter("mode", ParameterType::STRING, "Palette mode: 'IBM' or 'C64'");
    registry.registerCommand(std::move(lpaletteSet));

    // LPALETTE_POKE - Set palette entry
    CommandDefinition lpalettePoke("LPALETTE_POKE",
                                  "Set a LORES palette entry",
                                  "lores_palette_poke", "lores");
    lpalettePoke.addParameter("row", ParameterType::INT, "Palette row (0-299)")
                .addParameter("index", ParameterType::INT, "Color index (0-15)")
                .addParameter("rgba", ParameterType::INT, "ARGB color (0xAARRGGBB)");
    registry.registerCommand(std::move(lpalettePoke));

    // LPALETTE_PEEK - Get palette entry
    CommandDefinition lpalettePeek("LPALETTE_PEEK",
                                  "Get a LORES palette entry",
                                  "lores_palette_peek", "lores");
    lpalettePeek.addParameter("row", ParameterType::INT, "Palette row (0-299)")
                .addParameter("index", ParameterType::INT, "Color index (0-15)");
    registry.registerCommand(std::move(lpalettePeek));

    // MODE - Switch display mode (0=TEXT, 1=LORES, 2=MIDRES, 3=HIRES, 4=URES)
    CommandDefinition mode("MODE",
                          "Switch display mode (0=TEXT, 1=LORES 160x75, 2=MIDRES 320x150, 3=HIRES 640x300, 4=URES 1280x720)",
                          "mode", "display");
    mode.addParameter("mode", ParameterType::INT, "Display mode (0=TEXT, 1=LORES, 2=MIDRES, 3=HIRES, 4=URES)");
    registry.registerCommand(std::move(mode));

    // VMODE - Alias for MODE
    CommandDefinition vmode("VMODE",
                           "Switch display mode (0=TEXT, 1=LORES, 2=MIDRES, 3=HIRES, 4=URES, 5=XRES, 6=WRES, 7=PRES)",
                           "mode", "display");
    vmode.addParameter("mode", ParameterType::INT, "Display mode");
    registry.registerCommand(std::move(vmode));

    // LBUFFER - Select active buffer
    CommandDefinition lbuffer("LBUFFER",
                             "Select active buffer for drawing (0=front, 1=back)",
                             "lores_buffer", "lores");
    lbuffer.addParameter("buffer_id", ParameterType::INT, "Buffer ID (0 or 1)");
    registry.registerCommand(std::move(lbuffer));

    // LBUFFER_GET - Get active buffer ID
    CommandDefinition lbufferGet("LBUFFER_GET",
                                "Get current active buffer ID",
                                "lores_buffer_get", "lores");
    registry.registerCommand(std::move(lbufferGet));

    // LFLIP - Flip front and back buffers
    CommandDefinition lflip("LFLIP",
                           "Swap front and back buffers (instant flip)",
                           "lores_flip", "lores");
    registry.registerCommand(std::move(lflip));

    // LBLIT - Copy rectangular region
    CommandDefinition lblit("LBLIT",
                           "Copy rectangular region within current buffer",
                           "lores_blit", "lores");
    lblit.addParameter("src_x", ParameterType::INT, "Source X coordinate")
         .addParameter("src_y", ParameterType::INT, "Source Y coordinate")
         .addParameter("width", ParameterType::INT, "Width in pixels")
         .addParameter("height", ParameterType::INT, "Height in pixels")
         .addParameter("dst_x", ParameterType::INT, "Destination X coordinate")
         .addParameter("dst_y", ParameterType::INT, "Destination Y coordinate");
    registry.registerCommand(std::move(lblit));

    // LBLIT_TRANS - Copy with transparency
    CommandDefinition lblitTrans("LBLIT_TRANS",
                                "Copy rectangular region with transparency (cookie-cut)",
                                "lores_blit_trans", "lores");
    lblitTrans.addParameter("src_x", ParameterType::INT, "Source X coordinate")
              .addParameter("src_y", ParameterType::INT, "Source Y coordinate")
              .addParameter("width", ParameterType::INT, "Width in pixels")
              .addParameter("height", ParameterType::INT, "Height in pixels")
              .addParameter("dst_x", ParameterType::INT, "Destination X coordinate")
              .addParameter("dst_y", ParameterType::INT, "Destination Y coordinate")
              .addParameter("transparent_color", ParameterType::INT, "Color to skip (0-15)");
    registry.registerCommand(std::move(lblitTrans));

    // LBLIT_BUFFER - Copy between buffers
    CommandDefinition lblitBuffer("LBLIT_BUFFER",
                                 "Copy rectangular region between buffers",
                                 "lores_blit_buffer", "lores");
    lblitBuffer.addParameter("src_buffer", ParameterType::INT, "Source buffer (0 or 1)")
               .addParameter("dst_buffer", ParameterType::INT, "Destination buffer (0 or 1)")
               .addParameter("src_x", ParameterType::INT, "Source X coordinate")
               .addParameter("src_y", ParameterType::INT, "Source Y coordinate")
               .addParameter("width", ParameterType::INT, "Width in pixels")
               .addParameter("height", ParameterType::INT, "Height in pixels")
               .addParameter("dst_x", ParameterType::INT, "Destination X coordinate")
               .addParameter("dst_y", ParameterType::INT, "Destination Y coordinate");
    registry.registerCommand(std::move(lblitBuffer));

    // LBLIT_BUFFER_TRANS - Copy between buffers with transparency
    CommandDefinition lblitBufferTrans("LBLIT_BUFFER_TRANS",
                                      "Copy between buffers with transparency",
                                      "lores_blit_buffer_trans", "lores");
    lblitBufferTrans.addParameter("src_buffer", ParameterType::INT, "Source buffer (0 or 1)")
                    .addParameter("dst_buffer", ParameterType::INT, "Destination buffer (0 or 1)")
                    .addParameter("src_x", ParameterType::INT, "Source X coordinate")
                    .addParameter("src_y", ParameterType::INT, "Source Y coordinate")
                    .addParameter("width", ParameterType::INT, "Width in pixels")
                    .addParameter("height", ParameterType::INT, "Height in pixels")
                    .addParameter("dst_x", ParameterType::INT, "Destination X coordinate")
                    .addParameter("dst_y", ParameterType::INT, "Destination Y coordinate")
                    .addParameter("transparent_color", ParameterType::INT, "Color to skip (0-15)");
    registry.registerCommand(std::move(lblitBufferTrans));

    // =========================================================================
    // URES Mode Commands (Ultra Resolution 1280×720 Direct Color)
    // =========================================================================

    // UPSET - Set URES pixel
    CommandDefinition upset("UPSET",
                          "Set a URES pixel with direct color",
                          "ures_pset", "ures");
    upset.addParameter("x", ParameterType::INT, "X coordinate (0-1279)")
        .addParameter("y", ParameterType::INT, "Y coordinate (0-719)")
        .addParameter("color", ParameterType::INT, "16-bit ARGB4444 color");
    registry.registerCommand(std::move(upset));

    // UPGET - Get URES pixel color
    CommandDefinition upget("UPGET",
                           "Get URES pixel color",
                           "ures_pget", "ures");
    upget.addParameter("x", ParameterType::INT, "X coordinate (0-1279)")
         .addParameter("y", ParameterType::INT, "Y coordinate (0-719)");
    registry.registerCommand(std::move(upget));

    // UCLEAR - Clear URES buffer
    CommandDefinition uclear("UCLEAR",
                            "Clear URES buffer to a color",
                            "ures_clear", "ures");
    uclear.addParameter("color", ParameterType::INT, "16-bit ARGB4444 color");
    registry.registerCommand(std::move(uclear));

    // UFILLRECT - Fill URES rectangle
    CommandDefinition ufillrect("UFILLRECT",
                               "Fill a rectangle in URES mode",
                               "ures_fillrect", "ures");
    ufillrect.addParameter("x", ParameterType::INT, "X coordinate")
            .addParameter("y", ParameterType::INT, "Y coordinate")
            .addParameter("width", ParameterType::INT, "Width in pixels")
            .addParameter("height", ParameterType::INT, "Height in pixels")
            .addParameter("color", ParameterType::INT, "16-bit ARGB4444 color");
    registry.registerCommand(std::move(ufillrect));

    // UHLINE - Draw URES horizontal line
    CommandDefinition uhline("UHLINE",
                            "Draw a horizontal line in URES mode",
                            "ures_hline", "ures");
    uhline.addParameter("x", ParameterType::INT, "X coordinate")
         .addParameter("y", ParameterType::INT, "Y coordinate")
         .addParameter("width", ParameterType::INT, "Length in pixels")
         .addParameter("color", ParameterType::INT, "16-bit ARGB4444 color");
    registry.registerCommand(std::move(uhline));

    // UVLINE - Draw URES vertical line
    CommandDefinition uvline("UVLINE",
                            "Draw a vertical line in URES mode",
                            "ures_vline", "ures");
    uvline.addParameter("x", ParameterType::INT, "X coordinate")
         .addParameter("y", ParameterType::INT, "Y coordinate")
         .addParameter("height", ParameterType::INT, "Length in pixels")
         .addParameter("color", ParameterType::INT, "16-bit ARGB4444 color");
    registry.registerCommand(std::move(uvline));

    // URGB - Create ARGB4444 color from RGB
    CommandDefinition urgb("URGB",
                          "Create ARGB4444 color from RGB (4-bit per channel)",
                          "urgb", "ures");
    urgb.addParameter("r", ParameterType::INT, "Red (0-15)")
        .addParameter("g", ParameterType::INT, "Green (0-15)")
        .addParameter("b", ParameterType::INT, "Blue (0-15)");
    registry.registerCommand(std::move(urgb));

    // URGBA - Create ARGB4444 color from RGBA
    CommandDefinition urgba("URGBA",
                           "Create ARGB4444 color from RGBA (4-bit per channel)",
                           "urgba", "ures");
    urgba.addParameter("r", ParameterType::INT, "Red (0-15)")
         .addParameter("g", ParameterType::INT, "Green (0-15)")
         .addParameter("b", ParameterType::INT, "Blue (0-15)")
         .addParameter("a", ParameterType::INT, "Alpha (0-15)");
    registry.registerCommand(std::move(urgba));
}

void SuperTerminalCommandRegistry::registerSixelCommands(CommandRegistry& registry) {
    // SIXEL_PUT - Put sixel character with colors
    CommandDefinition sixelPut("SIXEL_PUT",
                              "Put a sixel character with 6-stripe colors",
                              "text_putsixel", "sixel");
    sixelPut.addParameter("x", ParameterType::INT, "X coordinate")
            .addParameter("y", ParameterType::INT, "Y coordinate")
            .addParameter("sixel_char", ParameterType::INT, "Sixel character pattern")
            .addParameter("colors", ParameterType::STRING, "6 color indices as string/array")
            .addParameter("bg", ParameterType::COLOR, "Background color", true, "0xFF000000");
    registry.registerCommand(std::move(sixelPut));

    // SIXEL_SET_STRIPE - Set single stripe color
    CommandDefinition sixelStripe("SIXEL_SET_STRIPE",
                                 "Set color of a single sixel stripe",
                                 "sixel_set_stripe", "sixel");
    sixelStripe.addParameter("x", ParameterType::INT, "X coordinate")
               .addParameter("y", ParameterType::INT, "Y coordinate")
               .addParameter("stripe", ParameterType::INT, "Stripe index (0-5)")
               .addParameter("color", ParameterType::INT, "Color index");
    registry.registerCommand(std::move(sixelStripe));

    // SIXEL_GRADIENT - Create vertical gradient in sixel
    CommandDefinition sixelGrad("SIXEL_GRADIENT",
                               "Create a vertical color gradient in sixel",
                               "sixel_gradient", "sixel");
    sixelGrad.addParameter("x", ParameterType::INT, "X coordinate")
             .addParameter("y", ParameterType::INT, "Y coordinate")
             .addParameter("top_color", ParameterType::INT, "Top color index")
             .addParameter("bottom_color", ParameterType::INT, "Bottom color index")
             .addParameter("bg", ParameterType::COLOR, "Background color", true, "0xFF000000");
    registry.registerCommand(std::move(sixelGrad));

    // SIXEL_HLINE - Draw horizontal line in sixel
    CommandDefinition sixelHLine("SIXEL_HLINE",
                                "Draw horizontal line using sixel graphics",
                                "sixel_hline", "sixel");
    sixelHLine.addParameter("x", ParameterType::INT, "Start X coordinate")
              .addParameter("y", ParameterType::INT, "Y coordinate")
              .addParameter("width", ParameterType::INT, "Line width")
              .addParameter("colors", ParameterType::STRING, "6 color indices as string/array")
              .addParameter("bg", ParameterType::COLOR, "Background color", true, "0xFF000000");
    registry.registerCommand(std::move(sixelHLine));

    // SIXEL_FILL_RECT - Fill rectangle using sixel
    CommandDefinition sixelRect("SIXEL_FILL_RECT",
                              "Fill rectangle using sixel graphics",
                              "sixel_fill_rect", "sixel");
    sixelRect.addParameter("x", ParameterType::INT, "X coordinate")
             .addParameter("y", ParameterType::INT, "Y coordinate")
             .addParameter("width", ParameterType::INT, "Rectangle width")
             .addParameter("height", ParameterType::INT, "Rectangle height")
             .addParameter("colors", ParameterType::STRING, "6 color indices as string/array")
             .addParameter("bg", ParameterType::COLOR, "Background color", true, "0xFF000000");
    registry.registerCommand(std::move(sixelRect));
}

void SuperTerminalCommandRegistry::registerTilemapCommands(CommandRegistry& registry) {
    // Tilemap System Initialization
    CommandDefinition tilemapInit("TILEMAP_INIT",
                                 "Initialize the tilemap system with screen dimensions",
                                 "tilemap.init", "tilemap");
    tilemapInit.addParameter("width", ParameterType::FLOAT, "Screen width")
               .addParameter("height", ParameterType::FLOAT, "Screen height");
    registry.registerCommand(std::move(tilemapInit));

    CommandDefinition tilemapShutdown("TILEMAP_SHUTDOWN",
                                     "Shutdown the tilemap system",
                                     "tilemap.shutdown", "tilemap");
    registry.registerCommand(std::move(tilemapShutdown));

    // Tilemap Management
    CommandDefinition tilemapCreate("TILEMAP_CREATE",
                                   "Create a new tilemap with specified dimensions",
                                   "tilemap.create", "tilemap", false, ReturnType::INT);
    tilemapCreate.addParameter("width", ParameterType::INT, "Tilemap width in tiles")
                 .addParameter("height", ParameterType::INT, "Tilemap height in tiles")
                 .addParameter("tile_width", ParameterType::INT, "Width of each tile in pixels")
                 .addParameter("tile_height", ParameterType::INT, "Height of each tile in pixels");
    registry.registerFunction(std::move(tilemapCreate));

    CommandDefinition tilemapDestroy("TILEMAP_DESTROY",
                                    "Destroy a tilemap by ID",
                                    "tilemap.destroy", "tilemap");
    tilemapDestroy.addParameter("id", ParameterType::INT, "Tilemap ID to destroy");
    registry.registerCommand(std::move(tilemapDestroy));

    CommandDefinition tilemapGetSize("TILEMAP_GET_SIZE",
                                    "Get the size of a tilemap",
                                    "tilemap.getSize", "tilemap");
    tilemapGetSize.addParameter("id", ParameterType::INT, "Tilemap ID");
    registry.registerCommand(std::move(tilemapGetSize));

    // Layer Management
    CommandDefinition tilemapCreateLayer("TILEMAP_CREATE_LAYER",
                                        "Create a tilemap layer with optional name",
                                        "tilemap.createLayer", "tilemap", false, ReturnType::INT);
    tilemapCreateLayer.addParameter("name", ParameterType::STRING, "Layer name", true, "\"\"");
    registry.registerFunction(std::move(tilemapCreateLayer));

    CommandDefinition tilemapDestroyLayer("TILEMAP_DESTROY_LAYER",
                                         "Destroy a tilemap layer",
                                         "tilemap.destroyLayer", "tilemap");
    tilemapDestroyLayer.addParameter("layer_id", ParameterType::INT, "Layer ID to destroy");
    registry.registerCommand(std::move(tilemapDestroyLayer));

    CommandDefinition tilemapLayerSetTilemap("TILEMAP_LAYER_SET_TILEMAP",
                                            "Assign a tilemap to a layer",
                                            "tilemap.layerSetTilemap", "tilemap");
    tilemapLayerSetTilemap.addParameter("layer_id", ParameterType::INT, "Layer ID")
                          .addParameter("tilemap_id", ParameterType::INT, "Tilemap ID");
    registry.registerCommand(std::move(tilemapLayerSetTilemap));

    CommandDefinition tilemapLayerSetTileset("TILEMAP_LAYER_SET_TILESET",
                                            "Assign a tileset to a layer",
                                            "tilemap.layerSetTileset", "tilemap");
    tilemapLayerSetTileset.addParameter("layer_id", ParameterType::INT, "Layer ID")
                          .addParameter("tileset_id", ParameterType::INT, "Tileset ID");
    registry.registerCommand(std::move(tilemapLayerSetTileset));

    CommandDefinition tilemapLayerSetParallax("TILEMAP_LAYER_SET_PARALLAX",
                                             "Set parallax scrolling for a layer",
                                             "tilemap.layerSetParallax", "tilemap");
    tilemapLayerSetParallax.addParameter("layer_id", ParameterType::INT, "Layer ID")
                           .addParameter("parallax_x", ParameterType::FLOAT, "Horizontal parallax factor")
                           .addParameter("parallax_y", ParameterType::FLOAT, "Vertical parallax factor");
    registry.registerCommand(std::move(tilemapLayerSetParallax));

    CommandDefinition tilemapLayerSetOpacity("TILEMAP_LAYER_SET_OPACITY",
                                            "Set opacity for a layer",
                                            "tilemap.layerSetOpacity", "tilemap");
    tilemapLayerSetOpacity.addParameter("layer_id", ParameterType::INT, "Layer ID")
                          .addParameter("opacity", ParameterType::FLOAT, "Opacity (0.0 to 1.0)");
    registry.registerCommand(std::move(tilemapLayerSetOpacity));

    CommandDefinition tilemapLayerSetVisible("TILEMAP_LAYER_SET_VISIBLE",
                                            "Set visibility for a layer",
                                            "tilemap.layerSetVisible", "tilemap");
    tilemapLayerSetVisible.addParameter("layer_id", ParameterType::INT, "Layer ID")
                          .addParameter("visible", ParameterType::BOOL, "Visibility flag");
    registry.registerCommand(std::move(tilemapLayerSetVisible));

    CommandDefinition tilemapLayerSetZOrder("TILEMAP_LAYER_SET_Z_ORDER",
                                           "Set Z-order for a layer",
                                           "tilemap.layerSetZOrder", "tilemap");
    tilemapLayerSetZOrder.addParameter("layer_id", ParameterType::INT, "Layer ID")
                         .addParameter("z_order", ParameterType::INT, "Z-order value");
    registry.registerCommand(std::move(tilemapLayerSetZOrder));

    CommandDefinition tilemapLayerSetAutoScroll("TILEMAP_LAYER_SET_AUTO_SCROLL",
                                               "Set auto-scroll for a layer",
                                               "tilemap.layerSetAutoScroll", "tilemap");
    tilemapLayerSetAutoScroll.addParameter("layer_id", ParameterType::INT, "Layer ID")
                             .addParameter("scroll_x", ParameterType::FLOAT, "Horizontal scroll speed")
                             .addParameter("scroll_y", ParameterType::FLOAT, "Vertical scroll speed");
    registry.registerCommand(std::move(tilemapLayerSetAutoScroll));

    // Tile Manipulation
    CommandDefinition tilemapSetTile("TILEMAP_SET_TILE",
                                   "Set a tile at specific coordinates",
                                   "tilemap.setTile", "tilemap");
    tilemapSetTile.addParameter("layer_id", ParameterType::INT, "Layer ID")
                  .addParameter("x", ParameterType::INT, "Tile X coordinate")
                  .addParameter("y", ParameterType::INT, "Tile Y coordinate")
                  .addParameter("tile_id", ParameterType::INT, "Tile ID to place");
    registry.registerCommand(std::move(tilemapSetTile));

    CommandDefinition tilemapGetTile("TILEMAP_GET_TILE",
                                   "Get a tile at specific coordinates",
                                   "tilemap.getTile", "tilemap");
    tilemapGetTile.addParameter("layer_id", ParameterType::INT, "Layer ID")
                  .addParameter("x", ParameterType::INT, "Tile X coordinate")
                  .addParameter("y", ParameterType::INT, "Tile Y coordinate");
    registry.registerCommand(std::move(tilemapGetTile));

    CommandDefinition tilemapFillRect("TILEMAP_FILL_RECT",
                                     "Fill a rectangular area with a tile",
                                     "tilemap.fillRect", "tilemap");
    tilemapFillRect.addParameter("layer_id", ParameterType::INT, "Layer ID")
                   .addParameter("x", ParameterType::INT, "Start X coordinate")
                   .addParameter("y", ParameterType::INT, "Start Y coordinate")
                   .addParameter("width", ParameterType::INT, "Rectangle width in tiles")
                   .addParameter("height", ParameterType::INT, "Rectangle height in tiles")
                   .addParameter("tile_id", ParameterType::INT, "Tile ID to fill with");
    registry.registerCommand(std::move(tilemapFillRect));

    CommandDefinition tilemapClear("TILEMAP_CLEAR",
                                  "Clear all tiles from a layer",
                                  "tilemap.clear", "tilemap");
    tilemapClear.addParameter("layer_id", ParameterType::INT, "Layer ID to clear");
    registry.registerCommand(std::move(tilemapClear));

    // Camera Control
    CommandDefinition tilemapSetCamera("TILEMAP_SET_CAMERA",
                                      "Set camera position",
                                      "tilemap.setCamera", "tilemap");
    tilemapSetCamera.addParameter("x", ParameterType::FLOAT, "Camera X position")
                    .addParameter("y", ParameterType::FLOAT, "Camera Y position");
    registry.registerCommand(std::move(tilemapSetCamera));

    CommandDefinition tilemapMoveCamera("TILEMAP_MOVE_CAMERA",
                                       "Move camera by offset",
                                       "tilemap.moveCamera", "tilemap");
    tilemapMoveCamera.addParameter("dx", ParameterType::FLOAT, "X offset")
                     .addParameter("dy", ParameterType::FLOAT, "Y offset");
    registry.registerCommand(std::move(tilemapMoveCamera));

    CommandDefinition tilemapGetCamera("TILEMAP_GET_CAMERA",
                                      "Get current camera position",
                                      "tilemap.getCamera", "tilemap");
    registry.registerCommand(std::move(tilemapGetCamera));

    CommandDefinition tilemapSetZoom("TILEMAP_SET_ZOOM",
                                    "Set camera zoom level",
                                    "tilemap.setZoom", "tilemap");
    tilemapSetZoom.addParameter("zoom", ParameterType::FLOAT, "Zoom factor");
    registry.registerCommand(std::move(tilemapSetZoom));

    CommandDefinition tilemapCameraFollow("TILEMAP_CAMERA_FOLLOW",
                                         "Make camera follow a target with smoothing",
                                         "tilemap.cameraFollow", "tilemap");
    tilemapCameraFollow.addParameter("target_x", ParameterType::FLOAT, "Target X position")
                       .addParameter("target_y", ParameterType::FLOAT, "Target Y position")
                       .addParameter("smoothness", ParameterType::FLOAT, "Follow smoothness factor");
    registry.registerCommand(std::move(tilemapCameraFollow));

    CommandDefinition tilemapSetCameraBounds("TILEMAP_SET_CAMERA_BOUNDS",
                                            "Set camera movement bounds",
                                            "tilemap.setCameraBounds", "tilemap");
    tilemapSetCameraBounds.addParameter("x", ParameterType::FLOAT, "Bounds X position")
                          .addParameter("y", ParameterType::FLOAT, "Bounds Y position")
                          .addParameter("width", ParameterType::FLOAT, "Bounds width")
                          .addParameter("height", ParameterType::FLOAT, "Bounds height");
    registry.registerCommand(std::move(tilemapSetCameraBounds));

    CommandDefinition tilemapCameraShake("TILEMAP_CAMERA_SHAKE",
                                        "Apply camera shake effect",
                                        "tilemap.cameraShake", "tilemap");
    tilemapCameraShake.addParameter("magnitude", ParameterType::FLOAT, "Shake magnitude")
                      .addParameter("duration", ParameterType::FLOAT, "Shake duration in seconds");
    registry.registerCommand(std::move(tilemapCameraShake));

    // System Update
    CommandDefinition tilemapUpdate("TILEMAP_UPDATE",
                                   "Update tilemap system (call once per frame)",
                                   "tilemap.update", "tilemap");
    tilemapUpdate.addParameter("dt", ParameterType::FLOAT, "Delta time in seconds");
    registry.registerCommand(std::move(tilemapUpdate));

    // Coordinate Conversion
    CommandDefinition tilemapWorldToTile("TILEMAP_WORLD_TO_TILE",
                                        "Convert world coordinates to tile coordinates",
                                        "tilemap.worldToTile", "tilemap");
    tilemapWorldToTile.addParameter("layer_id", ParameterType::INT, "Layer ID")
                      .addParameter("world_x", ParameterType::FLOAT, "World X coordinate")
                      .addParameter("world_y", ParameterType::FLOAT, "World Y coordinate");
    registry.registerCommand(std::move(tilemapWorldToTile));

    CommandDefinition tilemapTileToWorld("TILEMAP_TILE_TO_WORLD",
                                        "Convert tile coordinates to world coordinates",
                                        "tilemap.tileToWorld", "tilemap");
    tilemapTileToWorld.addParameter("layer_id", ParameterType::INT, "Layer ID")
                      .addParameter("tile_x", ParameterType::INT, "Tile X coordinate")
                      .addParameter("tile_y", ParameterType::INT, "Tile Y coordinate");
    registry.registerCommand(std::move(tilemapTileToWorld));
}

// =============================================================================
// SuperTerminal Functions
// =============================================================================

void SuperTerminalCommandRegistry::registerTilemapFunctions(CommandRegistry& registry) {
    // Tilemap functions can be added here if needed in the future
    // Note: TILEMAP_GET_TILE is already registered as a command above
}

void SuperTerminalCommandRegistry::registerSystemFunctions(CommandRegistry& registry) {
    // System-specific functions
    CommandDefinition screenWidth("SCREEN_WIDTH", "Get screen width", "display_width", "system");
    screenWidth.setReturnType(ReturnType::INT);
    registry.registerFunction(std::move(screenWidth));

    CommandDefinition screenHeight("SCREEN_HEIGHT", "Get screen height", "display_height", "system");
    screenHeight.setReturnType(ReturnType::INT);
    registry.registerFunction(std::move(screenHeight));

    // RAND function - random integer in range 0..n
    CommandDefinition randFunc("RAND", "Generate random integer in range 0 to n (inclusive)", "st_rand", "system");
    randFunc.addParameter("n", ParameterType::INT, "Maximum value (inclusive)")
           .setReturnType(ReturnType::INT);
    registry.registerFunction(std::move(randFunc));

    // DisplayText functions that return values
    CommandDefinition displayTextCount("DISPLAYTEXT_COUNT", "Get number of displayed text items", "text_get_item_count", "text");
    displayTextCount.setReturnType(ReturnType::INT);
    registry.registerFunction(std::move(displayTextCount));

    CommandDefinition displayTextVisibleCount("DISPLAYTEXT_VISIBLE_COUNT", "Get number of visible displayed text items", "text_get_visible_count", "text");
    displayTextVisibleCount.setReturnType(ReturnType::INT);
    registry.registerFunction(std::move(displayTextVisibleCount));

    // DISPLAYTEXT_AT function - returns item ID
    CommandDefinition displayTextAt("DISPLAYTEXT_AT", "Display overlay text and return item ID", "text_display_at", "text");
    displayTextAt.addParameter("x", ParameterType::FLOAT, "X pixel coordinate")
                .addParameter("y", ParameterType::FLOAT, "Y pixel coordinate")
                .addParameter("text", ParameterType::STRING, "Text to display")
                .addParameter("scale_x", ParameterType::FLOAT, "X scale factor", true, "1.0")
                .addParameter("scale_y", ParameterType::FLOAT, "Y scale factor", true, "1.0")
                .addParameter("rotation", ParameterType::FLOAT, "Rotation in degrees", true, "0.0")
                .addParameter("color", ParameterType::COLOR, "Text color", true, "0xFFFFFFFF")
                .addParameter("alignment", ParameterType::INT, "Text alignment (0=left, 1=center, 2=right)", true, "0")
                .addParameter("layer", ParameterType::INT, "Display layer (higher = on top)", true, "0")
                .setReturnType(ReturnType::INT);
    registry.registerFunction(std::move(displayTextAt));

    // DISPLAYTEXT_SHEAR_AT function - returns item ID
    CommandDefinition displayTextShearAt("DISPLAYTEXT_SHEAR_AT", "Display overlay text with shear and return item ID", "text_display_shear", "text");
    displayTextShearAt.addParameter("x", ParameterType::FLOAT, "X pixel coordinate")
                      .addParameter("y", ParameterType::FLOAT, "Y pixel coordinate")
                      .addParameter("text", ParameterType::STRING, "Text to display")
                      .addParameter("scale_x", ParameterType::FLOAT, "X scale factor", true, "1.0")
                      .addParameter("scale_y", ParameterType::FLOAT, "Y scale factor", true, "1.0")
                      .addParameter("rotation", ParameterType::FLOAT, "Rotation in degrees", true, "0.0")
                      .addParameter("shear_x", ParameterType::FLOAT, "X shear factor", true, "0.0")
                      .addParameter("shear_y", ParameterType::FLOAT, "Y shear factor", true, "0.0")
                      .addParameter("color", ParameterType::COLOR, "Text color", true, "0xFFFFFFFF")
                      .addParameter("alignment", ParameterType::INT, "Text alignment (0=left, 1=center, 2=right)", true, "0")
                      .addParameter("layer", ParameterType::INT, "Display layer (higher = on top)", true, "0")
                      .setReturnType(ReturnType::INT);
    registry.registerFunction(std::move(displayTextShearAt));

    // DISPLAYTEXT_EFFECTS_AT function - returns item ID
    CommandDefinition displayTextEffectsAt("DISPLAYTEXT_EFFECTS_AT", "Display overlay text with effects and return item ID", "text_display_with_effects", "text");
    displayTextEffectsAt.addParameter("x", ParameterType::FLOAT, "X pixel coordinate")
                        .addParameter("y", ParameterType::FLOAT, "Y pixel coordinate")
                        .addParameter("text", ParameterType::STRING, "Text to display")
                        .addParameter("scale_x", ParameterType::FLOAT, "Horizontal scale factor", true, "1.0")
                        .addParameter("scale_y", ParameterType::FLOAT, "Vertical scale factor", true, "1.0")
                        .addParameter("rotation", ParameterType::FLOAT, "Rotation in degrees", true, "0.0")
                        .addParameter("color", ParameterType::INT, "Text color (RGBA)", true, "0xFFFFFFFF")
                        .addParameter("alignment", ParameterType::INT, "Text alignment (0=left, 1=center, 2=right)", true, "0")
                        .addParameter("layer", ParameterType::INT, "Display layer (higher = on top)", true, "0")
                        .addParameter("effect", ParameterType::INT, "Effect type (0=none, 1=shadow, 2=outline, 3=glow, 4=gradient, 5=wave, 6=neon)", true, "0")
                        .addParameter("effect_color", ParameterType::INT, "Effect color (RGBA)", true, "0x000000FF")
                        .addParameter("effect_intensity", ParameterType::FLOAT, "Effect intensity (0.0-1.0)", true, "0.5")
                        .addParameter("effect_size", ParameterType::FLOAT, "Effect size (pixels)", true, "2.0")
                        .setReturnType(ReturnType::INT);
    registry.registerFunction(std::move(displayTextEffectsAt));
}

// =============================================================================
// Convenience Functions
// =============================================================================

void initializeSuperTerminalRegistry(CommandRegistry& registry) {
    // Start with core commands
    FasterBASIC::ModularCommands::CoreCommandRegistry::registerCoreCommands(registry);
    FasterBASIC::ModularCommands::CoreCommandRegistry::registerCoreFunctions(registry);

    // Add SuperTerminal-specific commands
    SuperTerminalCommandRegistry::registerSuperTerminalCommands(registry);
    SuperTerminalCommandRegistry::registerSuperTerminalFunctions(registry);
}

// =============================================================================
// Sound Bank Functions (ID-based Sound Management) - Phase 1
// =============================================================================

void SuperTerminalCommandRegistry::registerSoundBankFunctions(CommandRegistry& registry) {
    // SOUND_CREATE_BEEP - Create a beep sound and store in sound bank
    CommandDefinition soundCreateBeep("SOUND_CREATE_BEEP",
                                     "Create a beep sound and return its ID",
                                     "sound_create_beep", "audio", false, ReturnType::INT);
    soundCreateBeep.addParameter("frequency", ParameterType::FLOAT, "Frequency in Hz")
                   .addParameter("duration", ParameterType::FLOAT, "Duration in seconds");
    registry.registerFunction(std::move(soundCreateBeep));

    // SOUND_CREATE_ZAP - Create a laser zap sound
    CommandDefinition soundCreateZap("SOUND_CREATE_ZAP",
                                     "Create a laser zap sound and return its ID",
                                     "sound_create_zap", "audio", false, ReturnType::INT);
    soundCreateZap.addParameter("frequency", ParameterType::FLOAT, "Frequency in Hz")
                  .addParameter("duration", ParameterType::FLOAT, "Duration in seconds");
    registry.registerFunction(std::move(soundCreateZap));

    // SOUND_CREATE_EXPLODE - Create an explosion sound
    CommandDefinition soundCreateExplode("SOUND_CREATE_EXPLODE",
                                         "Create an explosion sound and return its ID",
                                         "sound_create_explode", "audio", false, ReturnType::INT);
    soundCreateExplode.addParameter("size", ParameterType::FLOAT, "Explosion size (0.5-2.0)")
                      .addParameter("duration", ParameterType::FLOAT, "Duration in seconds");
    registry.registerFunction(std::move(soundCreateExplode));

    // SOUND_CREATE_COIN - Create a coin pickup sound
    CommandDefinition soundCreateCoin("SOUND_CREATE_COIN",
                                      "Create a coin pickup sound and return its ID",
                                      "sound_create_coin", "audio", false, ReturnType::INT);
    soundCreateCoin.addParameter("pitch", ParameterType::FLOAT, "Pitch multiplier (0.5-2.0)")
                   .addParameter("duration", ParameterType::FLOAT, "Duration in seconds");
    registry.registerFunction(std::move(soundCreateCoin));

    // SOUND_CREATE_JUMP - Create a jump sound
    CommandDefinition soundCreateJump("SOUND_CREATE_JUMP",
                                      "Create a jump sound and return its ID",
                                      "sound_create_jump", "audio", false, ReturnType::INT);
    soundCreateJump.addParameter("power", ParameterType::FLOAT, "Jump power (0.5-2.0)")
                   .addParameter("duration", ParameterType::FLOAT, "Duration in seconds");
    registry.registerFunction(std::move(soundCreateJump));

    // SOUND_CREATE_SHOOT - Create a shooting sound
    CommandDefinition soundCreateShoot("SOUND_CREATE_SHOOT",
                                       "Create a shooting sound and return its ID",
                                       "sound_create_shoot", "audio", false, ReturnType::INT);
    soundCreateShoot.addParameter("power", ParameterType::FLOAT, "Shot power (0.5-2.0)")
                    .addParameter("duration", ParameterType::FLOAT, "Duration in seconds");
    registry.registerFunction(std::move(soundCreateShoot));

    // SOUND_CREATE_CLICK - Create a UI click sound
    CommandDefinition soundCreateClick("SOUND_CREATE_CLICK",
                                       "Create a UI click sound and return its ID",
                                       "sound_create_click", "audio", false, ReturnType::INT);
    soundCreateClick.addParameter("sharpness", ParameterType::FLOAT, "Click sharpness (0.5-2.0)")
                    .addParameter("duration", ParameterType::FLOAT, "Duration in seconds");
    registry.registerFunction(std::move(soundCreateClick));

    // SOUND_CREATE_BLIP - Create a blip sound
    CommandDefinition soundCreateBlip("SOUND_CREATE_BLIP",
                                      "Create a blip sound and return its ID",
                                      "sound_create_blip", "audio", false, ReturnType::INT);
    soundCreateBlip.addParameter("pitch", ParameterType::FLOAT, "Pitch multiplier (0.5-2.0)")
                   .addParameter("duration", ParameterType::FLOAT, "Duration in seconds");
    registry.registerFunction(std::move(soundCreateBlip));

    // SOUND_CREATE_PICKUP - Create an item pickup sound
    CommandDefinition soundCreatePickup("SOUND_CREATE_PICKUP",
                                        "Create an item pickup sound and return its ID",
                                        "sound_create_pickup", "audio", false, ReturnType::INT);
    soundCreatePickup.addParameter("brightness", ParameterType::FLOAT, "Brightness (0.5-2.0)")
                     .addParameter("duration", ParameterType::FLOAT, "Duration in seconds");
    registry.registerFunction(std::move(soundCreatePickup));

    // SOUND_CREATE_POWERUP - Create a power-up sound
    CommandDefinition soundCreatePowerup("SOUND_CREATE_POWERUP",
                                         "Create a power-up sound and return its ID",
                                         "sound_create_powerup", "audio", false, ReturnType::INT);
    soundCreatePowerup.addParameter("intensity", ParameterType::FLOAT, "Power-up intensity (0.5-2.0)")
                      .addParameter("duration", ParameterType::FLOAT, "Duration in seconds");
    registry.registerFunction(std::move(soundCreatePowerup));

    // SOUND_CREATE_HURT - Create a hurt/damage sound
    CommandDefinition soundCreateHurt("SOUND_CREATE_HURT",
                                      "Create a hurt/damage sound and return its ID",
                                      "sound_create_hurt", "audio", false, ReturnType::INT);
    soundCreateHurt.addParameter("severity", ParameterType::FLOAT, "Damage severity (0.5-2.0)")
                   .addParameter("duration", ParameterType::FLOAT, "Duration in seconds");
    registry.registerFunction(std::move(soundCreateHurt));

    // SOUND_CREATE_SWEEP_UP - Create a rising frequency sweep
    CommandDefinition soundCreateSweepUp("SOUND_CREATE_SWEEP_UP",
                                         "Create a rising frequency sweep and return its ID",
                                         "sound_create_sweep_up", "audio", false, ReturnType::INT);
    soundCreateSweepUp.addParameter("start_freq", ParameterType::FLOAT, "Starting frequency in Hz")
                      .addParameter("end_freq", ParameterType::FLOAT, "Ending frequency in Hz")
                      .addParameter("duration", ParameterType::FLOAT, "Duration in seconds");
    registry.registerFunction(std::move(soundCreateSweepUp));

    // SOUND_CREATE_SWEEP_DOWN - Create a falling frequency sweep
    CommandDefinition soundCreateSweepDown("SOUND_CREATE_SWEEP_DOWN",
                                           "Create a falling frequency sweep and return its ID",
                                           "sound_create_sweep_down", "audio", false, ReturnType::INT);
    soundCreateSweepDown.addParameter("start_freq", ParameterType::FLOAT, "Starting frequency in Hz")
                        .addParameter("end_freq", ParameterType::FLOAT, "Ending frequency in Hz")
                        .addParameter("duration", ParameterType::FLOAT, "Duration in seconds");
    registry.registerFunction(std::move(soundCreateSweepDown));

    // SOUND_CREATE_BIG_EXPLOSION - Create a big explosion sound
    CommandDefinition soundCreateBigExplosion("SOUND_CREATE_BIG_EXPLOSION",
                                              "Create a big explosion sound and return its ID",
                                              "sound_create_big_explosion", "audio", false, ReturnType::INT);
    soundCreateBigExplosion.addParameter("size", ParameterType::FLOAT, "Explosion size (0.5-2.0)")
                           .addParameter("duration", ParameterType::FLOAT, "Duration in seconds");
    registry.registerFunction(std::move(soundCreateBigExplosion));

    // SOUND_CREATE_SMALL_EXPLOSION - Create a small explosion sound
    CommandDefinition soundCreateSmallExplosion("SOUND_CREATE_SMALL_EXPLOSION",
                                                "Create a small explosion sound and return its ID",
                                                "sound_create_small_explosion", "audio", false, ReturnType::INT);
    soundCreateSmallExplosion.addParameter("intensity", ParameterType::FLOAT, "Explosion intensity (0.5-2.0)")
                             .addParameter("duration", ParameterType::FLOAT, "Duration in seconds");
    registry.registerFunction(std::move(soundCreateSmallExplosion));

    // SOUND_CREATE_DISTANT_EXPLOSION - Create a distant explosion sound
    CommandDefinition soundCreateDistantExplosion("SOUND_CREATE_DISTANT_EXPLOSION",
                                                  "Create a distant explosion sound and return its ID",
                                                  "sound_create_distant_explosion", "audio", false, ReturnType::INT);
    soundCreateDistantExplosion.addParameter("distance", ParameterType::FLOAT, "Distance factor (0.5-2.0)")
                               .addParameter("duration", ParameterType::FLOAT, "Duration in seconds");
    registry.registerFunction(std::move(soundCreateDistantExplosion));

    // SOUND_CREATE_METAL_EXPLOSION - Create a metallic explosion sound
    CommandDefinition soundCreateMetalExplosion("SOUND_CREATE_METAL_EXPLOSION",
                                                "Create a metallic explosion sound and return its ID",
                                                "sound_create_metal_explosion", "audio", false, ReturnType::INT);
    soundCreateMetalExplosion.addParameter("shrapnel", ParameterType::FLOAT, "Shrapnel amount (0.5-2.0)")
                             .addParameter("duration", ParameterType::FLOAT, "Duration in seconds");
    registry.registerFunction(std::move(soundCreateMetalExplosion));

    // SOUND_CREATE_BANG - Create an impact/bang sound
    CommandDefinition soundCreateBang("SOUND_CREATE_BANG",
                                      "Create an impact/bang sound and return its ID",
                                      "sound_create_bang", "audio", false, ReturnType::INT);
    soundCreateBang.addParameter("intensity", ParameterType::FLOAT, "Impact intensity (0.5-2.0)")
                   .addParameter("duration", ParameterType::FLOAT, "Duration in seconds");
    registry.registerFunction(std::move(soundCreateBang));

    // SOUND_CREATE_RANDOM_BEEP - Create a random procedural beep
    CommandDefinition soundCreateRandomBeep("SOUND_CREATE_RANDOM_BEEP",
                                            "Create a random procedural beep and return its ID",
                                            "sound_create_random_beep", "audio", false, ReturnType::INT);
    soundCreateRandomBeep.addParameter("seed", ParameterType::INT, "Random seed value")
                         .addParameter("duration", ParameterType::FLOAT, "Duration in seconds");
    registry.registerFunction(std::move(soundCreateRandomBeep));

    // SOUND_FREE - Free a sound from the sound bank
    CommandDefinition soundFree("SOUND_FREE",
                               "Free a sound from the sound bank",
                               "sound_free_id", "audio", false, ReturnType::BOOL);
    soundFree.addParameter("sound_id", ParameterType::INT, "Sound ID to free");
    registry.registerFunction(std::move(soundFree));

    // SOUND_EXISTS - Check if a sound exists in the sound bank
    CommandDefinition soundExists("SOUND_EXISTS",
                                 "Check if a sound exists in the sound bank",
                                 "sound_exists", "audio", false, ReturnType::BOOL);
    soundExists.addParameter("sound_id", ParameterType::INT, "Sound ID to check");
    registry.registerFunction(std::move(soundExists));

    // SOUND_COUNT - Get number of sounds in the sound bank
    CommandDefinition soundCount("SOUND_COUNT",
                                "Get number of sounds in the sound bank",
                                "sound_get_count", "audio", false, ReturnType::INT);
    registry.registerFunction(std::move(soundCount));

    // SOUND_MEMORY - Get memory usage of sound bank
    CommandDefinition soundMemory("SOUND_MEMORY",
                                 "Get memory usage of sound bank in bytes",
                                 "sound_get_memory_usage", "audio", false, ReturnType::INT);
    registry.registerFunction(std::move(soundMemory));

    // -------------------------------------------------------------------------
    // Phase 3: Custom Synthesis
    // -------------------------------------------------------------------------

    // SOUND_CREATE_TONE - Create a tone with specified waveform
    CommandDefinition soundCreateTone("SOUND_CREATE_TONE",
                                     "Create a tone with specified frequency, duration, and waveform",
                                     "sound_create_tone", "audio", false, ReturnType::INT);
    soundCreateTone.addParameter("frequency", ParameterType::FLOAT, "Frequency in Hz")
                   .addParameter("duration", ParameterType::FLOAT, "Duration in seconds")
                   .addParameter("waveform", ParameterType::INT, "Waveform: 0=SINE, 1=SQUARE, 2=SAWTOOTH, 3=TRIANGLE, 4=NOISE, 5=PULSE");
    registry.registerFunction(std::move(soundCreateTone));

    // SOUND_CREATE_NOTE - Create a musical note with ADSR envelope
    CommandDefinition soundCreateNote("SOUND_CREATE_NOTE",
                                     "Create a musical note with ADSR envelope",
                                     "sound_create_note", "audio", false, ReturnType::INT);
    soundCreateNote.addParameter("note", ParameterType::INT, "MIDI note number (0-127, middle C = 60)")
                   .addParameter("duration", ParameterType::FLOAT, "Total duration in seconds")
                   .addParameter("waveform", ParameterType::INT, "Waveform: 0=SINE, 1=SQUARE, 2=SAWTOOTH, 3=TRIANGLE, 4=NOISE, 5=PULSE")
                   .addParameter("attack", ParameterType::FLOAT, "Attack time in seconds")
                   .addParameter("decay", ParameterType::FLOAT, "Decay time in seconds")
                   .addParameter("sustain_level", ParameterType::FLOAT, "Sustain level (0.0-1.0)")
                   .addParameter("release", ParameterType::FLOAT, "Release time in seconds");
    registry.registerFunction(std::move(soundCreateNote));

    // SOUND_CREATE_NOISE - Create noise sound
    CommandDefinition soundCreateNoise("SOUND_CREATE_NOISE",
                                      "Create noise sound with specified type and duration",
                                      "sound_create_noise", "audio", false, ReturnType::INT);
    soundCreateNoise.addParameter("noise_type", ParameterType::INT, "Noise type: 0=WHITE, 1=PINK, 2=BROWN/RED")
                    .addParameter("duration", ParameterType::FLOAT, "Duration in seconds");
    registry.registerFunction(std::move(soundCreateNoise));

    // -------------------------------------------------------------------------
    // Advanced Synthesis - Phase 4
    // -------------------------------------------------------------------------

    // SOUND_CREATE_FM - Create FM synthesized sound
    CommandDefinition soundCreateFM("SOUND_CREATE_FM",
                                   "Create FM synthesized sound",
                                   "sound_create_fm", "audio", false, ReturnType::INT);
    soundCreateFM.addParameter("carrier_freq", ParameterType::FLOAT, "Carrier frequency in Hz")
                 .addParameter("modulator_freq", ParameterType::FLOAT, "Modulator frequency in Hz")
                 .addParameter("mod_index", ParameterType::FLOAT, "Modulation index (depth, typically 0.5-10.0)")
                 .addParameter("duration", ParameterType::FLOAT, "Duration in seconds");
    registry.registerFunction(std::move(soundCreateFM));

    // SOUND_CREATE_FILTERED_TONE - Create a tone with filter
    CommandDefinition soundCreateFilteredTone("SOUND_CREATE_FILTERED_TONE",
                                             "Create a tone with filter applied",
                                             "sound_create_filtered_tone", "audio", false, ReturnType::INT);
    soundCreateFilteredTone.addParameter("frequency", ParameterType::FLOAT, "Frequency in Hz")
                          .addParameter("duration", ParameterType::FLOAT, "Duration in seconds")
                          .addParameter("waveform", ParameterType::INT, "Waveform: 0=SINE, 1=SQUARE, 2=SAWTOOTH, 3=TRIANGLE, 4=NOISE, 5=PULSE")
                          .addParameter("filter_type", ParameterType::INT, "Filter: 0=NONE, 1=LOW_PASS, 2=HIGH_PASS, 3=BAND_PASS")
                          .addParameter("cutoff", ParameterType::FLOAT, "Filter cutoff frequency in Hz")
                          .addParameter("resonance", ParameterType::FLOAT, "Filter resonance (0.0-1.0)");
    registry.registerFunction(std::move(soundCreateFilteredTone));

    // SOUND_CREATE_FILTERED_NOTE - Create a musical note with ADSR and filter
    CommandDefinition soundCreateFilteredNote("SOUND_CREATE_FILTERED_NOTE",
                                             "Create a musical note with ADSR envelope and filter",
                                             "sound_create_filtered_note", "audio", false, ReturnType::INT);
    soundCreateFilteredNote.addParameter("note", ParameterType::INT, "MIDI note number (0-127, middle C = 60)")
                          .addParameter("duration", ParameterType::FLOAT, "Total duration in seconds")
                          .addParameter("waveform", ParameterType::INT, "Waveform: 0=SINE, 1=SQUARE, 2=SAWTOOTH, 3=TRIANGLE, 4=NOISE, 5=PULSE")
                          .addParameter("attack", ParameterType::FLOAT, "Attack time in seconds")
                          .addParameter("decay", ParameterType::FLOAT, "Decay time in seconds")
                          .addParameter("sustain_level", ParameterType::FLOAT, "Sustain level (0.0-1.0)")
                          .addParameter("release", ParameterType::FLOAT, "Release time in seconds")
                          .addParameter("filter_type", ParameterType::INT, "Filter: 0=NONE, 1=LOW_PASS, 2=HIGH_PASS, 3=BAND_PASS")
                          .addParameter("cutoff", ParameterType::FLOAT, "Filter cutoff frequency in Hz")
                          .addParameter("resonance", ParameterType::FLOAT, "Filter resonance (0.0-1.0)");
    registry.registerFunction(std::move(soundCreateFilteredNote));

    // -------------------------------------------------------------------------
    // Effects Chain - Phase 5
    // -------------------------------------------------------------------------

    // SOUND_CREATE_WITH_REVERB - Create a tone with reverb effect
    CommandDefinition soundCreateWithReverb("SOUND_CREATE_WITH_REVERB",
                                           "Create a tone with reverb effect",
                                           "sound_create_with_reverb", "audio", false, ReturnType::INT);
    soundCreateWithReverb.addParameter("frequency", ParameterType::FLOAT, "Frequency in Hz")
                        .addParameter("duration", ParameterType::FLOAT, "Duration in seconds")
                        .addParameter("waveform", ParameterType::INT, "Waveform: 0=SINE, 1=SQUARE, 2=SAWTOOTH, 3=TRIANGLE, 4=NOISE, 5=PULSE")
                        .addParameter("room_size", ParameterType::FLOAT, "Reverb room size (0.0-1.0)")
                        .addParameter("damping", ParameterType::FLOAT, "High frequency damping (0.0-1.0)")
                        .addParameter("wet", ParameterType::FLOAT, "Wet signal level (0.0-1.0)");
    registry.registerFunction(std::move(soundCreateWithReverb));

    // SOUND_CREATE_WITH_DELAY - Create a tone with delay/echo effect
    CommandDefinition soundCreateWithDelay("SOUND_CREATE_WITH_DELAY",
                                          "Create a tone with delay/echo effect",
                                          "sound_create_with_delay", "audio", false, ReturnType::INT);
    soundCreateWithDelay.addParameter("frequency", ParameterType::FLOAT, "Frequency in Hz")
                       .addParameter("duration", ParameterType::FLOAT, "Duration in seconds")
                       .addParameter("waveform", ParameterType::INT, "Waveform: 0=SINE, 1=SQUARE, 2=SAWTOOTH, 3=TRIANGLE, 4=NOISE, 5=PULSE")
                       .addParameter("delay_time", ParameterType::FLOAT, "Delay time in seconds")
                       .addParameter("feedback", ParameterType::FLOAT, "Feedback amount (0.0-1.0)")
                       .addParameter("mix", ParameterType::FLOAT, "Dry/wet mix (0.0-1.0)");
    registry.registerFunction(std::move(soundCreateWithDelay));

    // SOUND_CREATE_WITH_DISTORTION - Create a tone with distortion effect
    CommandDefinition soundCreateWithDistortion("SOUND_CREATE_WITH_DISTORTION",
                                               "Create a tone with distortion effect",
                                               "sound_create_with_distortion", "audio", false, ReturnType::INT);
    soundCreateWithDistortion.addParameter("frequency", ParameterType::FLOAT, "Frequency in Hz")
                            .addParameter("duration", ParameterType::FLOAT, "Duration in seconds")
                            .addParameter("waveform", ParameterType::INT, "Waveform: 0=SINE, 1=SQUARE, 2=SAWTOOTH, 3=TRIANGLE, 4=NOISE, 5=PULSE")
                            .addParameter("drive", ParameterType::FLOAT, "Distortion drive amount (0.0-10.0)")
                            .addParameter("tone", ParameterType::FLOAT, "Tone control (0.0-1.0)")
                            .addParameter("level", ParameterType::FLOAT, "Output level (0.0-1.0)");
    registry.registerFunction(std::move(soundCreateWithDistortion));
}

// createSuperTerminalRegistry removed - use initializeSuperTerminalRegistry() directly instead
// (CommandRegistry is no longer copyable/movable due to thread safety mutex)

// =============================================================================
// RECTANGLE COMMANDS
// =============================================================================

void SuperTerminalCommandRegistry::registerRectangleCommands(CommandRegistry& registry) {
    // RECT_CREATE - Create a solid-color rectangle (returns ID)
    CommandDefinition rect_create("RECT_CREATE",
                                   "Create a solid-color rectangle and return its ID",
                                   "st_rect_create", "graphics", false, ReturnType::INT);
    rect_create.addParameter("x", ParameterType::FLOAT, "X coordinate in pixels")
               .addParameter("y", ParameterType::FLOAT, "Y coordinate in pixels")
               .addParameter("width", ParameterType::FLOAT, "Width in pixels")
               .addParameter("height", ParameterType::FLOAT, "Height in pixels")
               .addParameter("color", ParameterType::COLOR, "RGBA color", true, "0xFFFFFFFF");
    registry.registerFunction(std::move(rect_create));

    // RECT_CREATE_GRADIENT - Create a gradient rectangle (returns ID)
    CommandDefinition rect_create_gradient("RECT_CREATE_GRADIENT",
                                            "Create a gradient rectangle and return its ID",
                                            "st_rect_create_gradient", "graphics", false, ReturnType::INT);
    rect_create_gradient.addParameter("x", ParameterType::FLOAT, "X coordinate in pixels")
                        .addParameter("y", ParameterType::FLOAT, "Y coordinate in pixels")
                        .addParameter("width", ParameterType::FLOAT, "Width in pixels")
                        .addParameter("height", ParameterType::FLOAT, "Height in pixels")
                        .addParameter("color1", ParameterType::COLOR, "First gradient color")
                        .addParameter("color2", ParameterType::COLOR, "Second gradient color")
                        .addParameter("mode", ParameterType::INT, "Gradient mode (1=horizontal, 2=vertical, 5=radial)", true, "1");
    registry.registerFunction(std::move(rect_create_gradient));

    // RECT_CREATE_GRADIENT_3 - Create a three-point gradient rectangle (returns ID)
    CommandDefinition rect_create_gradient3("RECT_CREATE_GRADIENT_3",
                                             "Create a three-point gradient rectangle and return its ID",
                                             "st_rect_create_three_point", "graphics", false, ReturnType::INT);
    rect_create_gradient3.addParameter("x", ParameterType::FLOAT, "X coordinate in pixels")
                         .addParameter("y", ParameterType::FLOAT, "Y coordinate in pixels")
                         .addParameter("width", ParameterType::FLOAT, "Width in pixels")
                         .addParameter("height", ParameterType::FLOAT, "Height in pixels")
                         .addParameter("color1", ParameterType::COLOR, "First gradient color")
                         .addParameter("color2", ParameterType::COLOR, "Second gradient color")
                         .addParameter("color3", ParameterType::COLOR, "Third gradient color")
                         .addParameter("mode", ParameterType::INT, "Gradient mode (7=three-point)", true, "7");
    registry.registerFunction(std::move(rect_create_gradient3));

    // RECT_CREATE_GRADIENT_4 - Create a four-corner gradient rectangle (returns ID)
    CommandDefinition rect_create_gradient4("RECT_CREATE_GRADIENT_4",
                                             "Create a four-corner gradient rectangle and return its ID",
                                             "st_rect_create_four_corner", "graphics", false, ReturnType::INT);
    rect_create_gradient4.addParameter("x", ParameterType::FLOAT, "X coordinate in pixels")
                         .addParameter("y", ParameterType::FLOAT, "Y coordinate in pixels")
                         .addParameter("width", ParameterType::FLOAT, "Width in pixels")
                         .addParameter("height", ParameterType::FLOAT, "Height in pixels")
                         .addParameter("topLeft", ParameterType::COLOR, "Top-left corner color")
                         .addParameter("topRight", ParameterType::COLOR, "Top-right corner color")
                         .addParameter("bottomRight", ParameterType::COLOR, "Bottom-right corner color")
                         .addParameter("bottomLeft", ParameterType::COLOR, "Bottom-left corner color");
    registry.registerFunction(std::move(rect_create_gradient4));

    // RECT_CREATE_OUTLINE - Create an outlined rectangle (returns ID)
    CommandDefinition rect_create_outline("RECT_CREATE_OUTLINE",
                                           "Create an outlined rectangle and return its ID",
                                           "st_rect_create_outline", "graphics", false, ReturnType::INT);
    rect_create_outline.addParameter("x", ParameterType::FLOAT, "X coordinate in pixels")
                       .addParameter("y", ParameterType::FLOAT, "Y coordinate in pixels")
                       .addParameter("width", ParameterType::FLOAT, "Width in pixels")
                       .addParameter("height", ParameterType::FLOAT, "Height in pixels")
                       .addParameter("fillColor", ParameterType::COLOR, "Fill color")
                       .addParameter("outlineColor", ParameterType::COLOR, "Outline color")
                       .addParameter("lineWidth", ParameterType::FLOAT, "Outline width in pixels", true, "2.0");
    registry.registerFunction(std::move(rect_create_outline));

    // RECT_CREATE_DASHED_OUTLINE - Create a dashed outline rectangle (returns ID)
    CommandDefinition rect_create_dashed("RECT_CREATE_DASHED_OUTLINE",
                                          "Create a dashed outline rectangle and return its ID",
                                          "st_rect_create_dashed_outline", "graphics", false, ReturnType::INT);
    rect_create_dashed.addParameter("x", ParameterType::FLOAT, "X coordinate in pixels")
                      .addParameter("y", ParameterType::FLOAT, "Y coordinate in pixels")
                      .addParameter("width", ParameterType::FLOAT, "Width in pixels")
                      .addParameter("height", ParameterType::FLOAT, "Height in pixels")
                      .addParameter("fillColor", ParameterType::COLOR, "Fill color")
                      .addParameter("outlineColor", ParameterType::COLOR, "Outline color")
                      .addParameter("lineWidth", ParameterType::FLOAT, "Outline width in pixels", true, "2.0")
                      .addParameter("dashLength", ParameterType::FLOAT, "Dash length in pixels", true, "10.0");
    registry.registerFunction(std::move(rect_create_dashed));

    // RECT_CREATE_HORIZONTAL_STRIPES - Create horizontal striped rectangle (returns ID)
    CommandDefinition rect_create_hstripes("RECT_CREATE_HORIZONTAL_STRIPES",
                                            "Create a horizontal striped rectangle and return its ID",
                                            "st_rect_create_horizontal_stripes", "graphics", false, ReturnType::INT);
    rect_create_hstripes.addParameter("x", ParameterType::FLOAT, "X coordinate in pixels")
                        .addParameter("y", ParameterType::FLOAT, "Y coordinate in pixels")
                        .addParameter("width", ParameterType::FLOAT, "Width in pixels")
                        .addParameter("height", ParameterType::FLOAT, "Height in pixels")
                        .addParameter("color1", ParameterType::COLOR, "First stripe color")
                        .addParameter("color2", ParameterType::COLOR, "Second stripe color")
                        .addParameter("stripeHeight", ParameterType::FLOAT, "Height of each stripe in pixels", true, "10.0");
    registry.registerFunction(std::move(rect_create_hstripes));

    // RECT_CREATE_VERTICAL_STRIPES - Create vertical striped rectangle (returns ID)
    CommandDefinition rect_create_vstripes("RECT_CREATE_VERTICAL_STRIPES",
                                            "Create a vertical striped rectangle and return its ID",
                                            "st_rect_create_vertical_stripes", "graphics", false, ReturnType::INT);
    rect_create_vstripes.addParameter("x", ParameterType::FLOAT, "X coordinate in pixels")
                        .addParameter("y", ParameterType::FLOAT, "Y coordinate in pixels")
                        .addParameter("width", ParameterType::FLOAT, "Width in pixels")
                        .addParameter("height", ParameterType::FLOAT, "Height in pixels")
                        .addParameter("color1", ParameterType::COLOR, "First stripe color")
                        .addParameter("color2", ParameterType::COLOR, "Second stripe color")
                        .addParameter("stripeWidth", ParameterType::FLOAT, "Width of each stripe in pixels", true, "10.0");
    registry.registerFunction(std::move(rect_create_vstripes));

    // RECT_CREATE_DIAGONAL_STRIPES - Create diagonal striped rectangle (returns ID)
    CommandDefinition rect_create_dstripes("RECT_CREATE_DIAGONAL_STRIPES",
                                            "Create a diagonal striped rectangle and return its ID",
                                            "st_rect_create_diagonal_stripes", "graphics", false, ReturnType::INT);
    rect_create_dstripes.addParameter("x", ParameterType::FLOAT, "X coordinate in pixels")
                        .addParameter("y", ParameterType::FLOAT, "Y coordinate in pixels")
                        .addParameter("width", ParameterType::FLOAT, "Width in pixels")
                        .addParameter("height", ParameterType::FLOAT, "Height in pixels")
                        .addParameter("color1", ParameterType::COLOR, "First stripe color")
                        .addParameter("color2", ParameterType::COLOR, "Second stripe color")
                        .addParameter("stripeWidth", ParameterType::FLOAT, "Width of each stripe in pixels", true, "10.0")
                        .addParameter("angle", ParameterType::FLOAT, "Rotation angle in degrees", true, "45.0");
    registry.registerFunction(std::move(rect_create_dstripes));

    // RECT_CREATE_CHECKERBOARD - Create checkerboard pattern rectangle (returns ID)
    CommandDefinition rect_create_checker("RECT_CREATE_CHECKERBOARD",
                                           "Create a checkerboard pattern rectangle and return its ID",
                                           "st_rect_create_checkerboard", "graphics", false, ReturnType::INT);
    rect_create_checker.addParameter("x", ParameterType::FLOAT, "X coordinate in pixels")
                       .addParameter("y", ParameterType::FLOAT, "Y coordinate in pixels")
                       .addParameter("width", ParameterType::FLOAT, "Width in pixels")
                       .addParameter("height", ParameterType::FLOAT, "Height in pixels")
                       .addParameter("color1", ParameterType::COLOR, "First checker color")
                       .addParameter("color2", ParameterType::COLOR, "Second checker color")
                       .addParameter("cellSize", ParameterType::FLOAT, "Size of each checker cell in pixels", true, "10.0");
    registry.registerFunction(std::move(rect_create_checker));

    // RECT_CREATE_DOTS - Create dot pattern rectangle (returns ID)
    CommandDefinition rect_create_dots("RECT_CREATE_DOTS",
                                        "Create a dot pattern rectangle and return its ID",
                                        "st_rect_create_dots", "graphics", false, ReturnType::INT);
    rect_create_dots.addParameter("x", ParameterType::FLOAT, "X coordinate in pixels")
                    .addParameter("y", ParameterType::FLOAT, "Y coordinate in pixels")
                    .addParameter("width", ParameterType::FLOAT, "Width in pixels")
                    .addParameter("height", ParameterType::FLOAT, "Height in pixels")
                    .addParameter("dotColor", ParameterType::COLOR, "Dot color")
                    .addParameter("backgroundColor", ParameterType::COLOR, "Background color")
                    .addParameter("dotRadius", ParameterType::FLOAT, "Radius of each dot in pixels", true, "3.0")
                    .addParameter("spacing", ParameterType::FLOAT, "Spacing between dot centers in pixels", true, "10.0");
    registry.registerFunction(std::move(rect_create_dots));

    // RECT_CREATE_CROSSHATCH - Create crosshatch pattern rectangle (returns ID)
    CommandDefinition rect_create_cross("RECT_CREATE_CROSSHATCH",
                                         "Create a crosshatch pattern rectangle and return its ID",
                                         "st_rect_create_crosshatch", "graphics", false, ReturnType::INT);
    rect_create_cross.addParameter("x", ParameterType::FLOAT, "X coordinate in pixels")
                     .addParameter("y", ParameterType::FLOAT, "Y coordinate in pixels")
                     .addParameter("width", ParameterType::FLOAT, "Width in pixels")
                     .addParameter("height", ParameterType::FLOAT, "Height in pixels")
                     .addParameter("lineColor", ParameterType::COLOR, "Line color")
                     .addParameter("backgroundColor", ParameterType::COLOR, "Background color")
                     .addParameter("lineWidth", ParameterType::FLOAT, "Width of crosshatch lines in pixels", true, "1.0")
                     .addParameter("spacing", ParameterType::FLOAT, "Spacing between lines in pixels", true, "10.0");
    registry.registerFunction(std::move(rect_create_cross));

    // RECT_CREATE_ROUNDED_CORNERS - Create rounded corner rectangle (returns ID)
    CommandDefinition rect_create_rounded("RECT_CREATE_ROUNDED_CORNERS",
                                           "Create a rounded corner rectangle and return its ID",
                                           "st_rect_create_rounded_corners", "graphics", false, ReturnType::INT);
    rect_create_rounded.addParameter("x", ParameterType::FLOAT, "X coordinate in pixels")
                       .addParameter("y", ParameterType::FLOAT, "Y coordinate in pixels")
                       .addParameter("width", ParameterType::FLOAT, "Width in pixels")
                       .addParameter("height", ParameterType::FLOAT, "Height in pixels")
                       .addParameter("color", ParameterType::COLOR, "Fill color")
                       .addParameter("cornerRadius", ParameterType::FLOAT, "Corner radius in pixels", true, "10.0");
    registry.registerFunction(std::move(rect_create_rounded));

    // RECT_CREATE_GRID - Create grid pattern rectangle (returns ID)
    CommandDefinition rect_create_grid("RECT_CREATE_GRID",
                                        "Create a grid pattern rectangle and return its ID",
                                        "st_rect_create_grid", "graphics", false, ReturnType::INT);
    rect_create_grid.addParameter("x", ParameterType::FLOAT, "X coordinate in pixels")
                    .addParameter("y", ParameterType::FLOAT, "Y coordinate in pixels")
                    .addParameter("width", ParameterType::FLOAT, "Width in pixels")
                    .addParameter("height", ParameterType::FLOAT, "Height in pixels")
                    .addParameter("lineColor", ParameterType::COLOR, "Grid line color")
                    .addParameter("backgroundColor", ParameterType::COLOR, "Background color")
                    .addParameter("lineWidth", ParameterType::FLOAT, "Width of grid lines in pixels", true, "1.0")
                    .addParameter("cellSize", ParameterType::FLOAT, "Size of each grid cell in pixels", true, "10.0");
    registry.registerFunction(std::move(rect_create_grid));

    // RECT_SET_POSITION - Update rectangle position
    CommandDefinition rect_set_pos("RECT_SET_POSITION",
                                    "Update rectangle position by ID",
                                    "st_rect_set_position", "graphics");
    rect_set_pos.addParameter("id", ParameterType::INT, "Rectangle ID")
                .addParameter("x", ParameterType::FLOAT, "New X coordinate")
                .addParameter("y", ParameterType::FLOAT, "New Y coordinate");
    registry.registerCommand(std::move(rect_set_pos));

    // RECT_SET_SIZE - Update rectangle size
    CommandDefinition rect_set_size("RECT_SET_SIZE",
                                     "Update rectangle size by ID",
                                     "st_rect_set_size", "graphics");
    rect_set_size.addParameter("id", ParameterType::INT, "Rectangle ID")
                 .addParameter("width", ParameterType::FLOAT, "New width")
                 .addParameter("height", ParameterType::FLOAT, "New height");
    registry.registerCommand(std::move(rect_set_size));

    // RECT_SET_COLOR - Update rectangle color
    CommandDefinition rect_set_color("RECT_SET_COLOR",
                                      "Update rectangle color by ID",
                                      "st_rect_set_color", "graphics");
    rect_set_color.addParameter("id", ParameterType::INT, "Rectangle ID")
                  .addParameter("color", ParameterType::COLOR, "New color");
    registry.registerCommand(std::move(rect_set_color));

    // RECT_SET_COLORS - Update rectangle gradient colors
    CommandDefinition rect_set_colors("RECT_SET_COLORS",
                                       "Update rectangle gradient colors by ID",
                                       "st_rect_set_colors", "graphics");
    rect_set_colors.addParameter("id", ParameterType::INT, "Rectangle ID")
                   .addParameter("color1", ParameterType::COLOR, "First color")
                   .addParameter("color2", ParameterType::COLOR, "Second color")
                   .addParameter("color3", ParameterType::COLOR, "Third color")
                   .addParameter("color4", ParameterType::COLOR, "Fourth color");
    registry.registerCommand(std::move(rect_set_colors));

    // RECT_SET_MODE - Update rectangle gradient mode
    CommandDefinition rect_set_mode("RECT_SET_MODE",
                                     "Update rectangle gradient mode by ID",
                                     "st_rect_set_mode", "graphics");
    rect_set_mode.addParameter("id", ParameterType::INT, "Rectangle ID")
                 .addParameter("mode", ParameterType::INT, "Gradient mode");
    registry.registerCommand(std::move(rect_set_mode));

    // RECT_SET_PARAMETERS - Update rectangle pattern parameters
    CommandDefinition rect_set_params("RECT_SET_PARAMETERS",
                                       "Update rectangle pattern parameters by ID",
                                       "st_rect_set_parameters", "graphics");
    rect_set_params.addParameter("id", ParameterType::INT, "Rectangle ID")
                   .addParameter("param1", ParameterType::FLOAT, "First parameter")
                   .addParameter("param2", ParameterType::FLOAT, "Second parameter")
                   .addParameter("param3", ParameterType::FLOAT, "Third parameter");
    registry.registerCommand(std::move(rect_set_params));

    // RECT_SET_VISIBLE - Show or hide rectangle
    CommandDefinition rect_set_visible("RECT_SET_VISIBLE",
                                        "Show or hide a rectangle by ID",
                                        "st_rect_set_visible", "graphics");
    rect_set_visible.addParameter("id", ParameterType::INT, "Rectangle ID")
                    .addParameter("visible", ParameterType::INT, "1 to show, 0 to hide");
    registry.registerCommand(std::move(rect_set_visible));

    // RECT_DELETE - Delete a rectangle
    CommandDefinition rect_delete("RECT_DELETE",
                                   "Delete a rectangle by ID",
                                   "st_rect_delete", "graphics");
    rect_delete.addParameter("id", ParameterType::INT, "Rectangle ID");
    registry.registerCommand(std::move(rect_delete));

    // RECT_DELETE_ALL - Delete all rectangles
    CommandDefinition rect_delete_all("RECT_DELETE_ALL",
                                       "Delete all managed rectangles",
                                       "st_rect_delete_all", "graphics");
    registry.registerCommand(std::move(rect_delete_all));

    // RECT_SET_MAX - Set maximum number of rectangles
    CommandDefinition rect_set_max("RECT_SET_MAX",
                                    "Set the maximum number of rectangles that can be created",
                                    "st_rect_set_max", "graphics");
    rect_set_max.addParameter("max", ParameterType::INT, "Maximum number of rectangles");
    registry.registerCommand(std::move(rect_set_max));
}

// =============================================================================
// RECTANGLE FUNCTIONS
// =============================================================================

void SuperTerminalCommandRegistry::registerRectangleFunctions(CommandRegistry& registry) {
    // RECT_COUNT() - Get the total number of rectangles
    CommandDefinition rect_count("RECT_COUNT",
                                  "Get the total number of rectangles (managed + queued)",
                                  "st_rect_count", "graphics", false, ReturnType::INT);
    registry.registerFunction(std::move(rect_count));

    // RECT_IS_EMPTY() - Check if there are no rectangles
    CommandDefinition rect_is_empty("RECT_IS_EMPTY",
                                     "Check if there are no rectangles",
                                     "st_rect_is_empty", "graphics", false, ReturnType::INT);
    registry.registerFunction(std::move(rect_is_empty));

    // RECT_GET_MAX() - Get the maximum number of rectangles
    CommandDefinition rect_get_max("RECT_GET_MAX",
                                    "Get the maximum number of rectangles",
                                    "st_rect_get_max", "graphics", false, ReturnType::INT);
    registry.registerFunction(std::move(rect_get_max));

    // RECT_EXISTS() - Check if rectangle exists
    CommandDefinition rect_exists("RECT_EXISTS",
                                   "Check if a rectangle exists by ID",
                                   "st_rect_exists", "graphics", false, ReturnType::INT);
    rect_exists.addParameter("id", ParameterType::INT, "Rectangle ID");
    registry.registerFunction(std::move(rect_exists));

    // RECT_IS_VISIBLE() - Check if rectangle is visible
    CommandDefinition rect_is_visible("RECT_IS_VISIBLE",
                                       "Check if a rectangle is visible by ID",
                                       "st_rect_is_visible", "graphics", false, ReturnType::INT);
    rect_is_visible.addParameter("id", ParameterType::INT, "Rectangle ID");
    registry.registerFunction(std::move(rect_is_visible));
}

// =============================================================================
// CIRCLE COMMANDS
// =============================================================================

void SuperTerminalCommandRegistry::registerCircleCommands(CommandRegistry& registry) {
    // CIRCLE_CREATE - Create a solid-color circle (returns ID)
    CommandDefinition circle_create("CIRCLE_CREATE",
                                     "Create a solid-color circle and return its ID",
                                     "st_circle_create", "graphics", false, ReturnType::INT);
    circle_create.addParameter("x", ParameterType::FLOAT, "X coordinate in pixels (center)")
                 .addParameter("y", ParameterType::FLOAT, "Y coordinate in pixels (center)")
                 .addParameter("radius", ParameterType::FLOAT, "Radius in pixels")
                 .addParameter("color", ParameterType::COLOR, "RGBA color", true, "0xFFFFFFFF");
    registry.registerFunction(std::move(circle_create));

    // CIRCLE_CREATE_RADIAL - Create a radial gradient circle (returns ID)
    CommandDefinition circle_create_radial("CIRCLE_CREATE_RADIAL",
                                            "Create a radial gradient circle and return its ID",
                                            "st_circle_create_radial", "graphics", false, ReturnType::INT);
    circle_create_radial.addParameter("x", ParameterType::FLOAT, "X coordinate in pixels (center)")
                        .addParameter("y", ParameterType::FLOAT, "Y coordinate in pixels (center)")
                        .addParameter("radius", ParameterType::FLOAT, "Radius in pixels")
                        .addParameter("centerColor", ParameterType::COLOR, "Center color")
                        .addParameter("edgeColor", ParameterType::COLOR, "Edge color");
    registry.registerFunction(std::move(circle_create_radial));

    // CIRCLE_CREATE_RADIAL_3 - Create a three-color radial gradient circle (returns ID)
    CommandDefinition circle_create_radial3("CIRCLE_CREATE_RADIAL_3",
                                             "Create a three-color radial gradient circle and return its ID",
                                             "st_circle_create_radial_3", "graphics", false, ReturnType::INT);
    circle_create_radial3.addParameter("x", ParameterType::FLOAT, "X coordinate in pixels (center)")
                         .addParameter("y", ParameterType::FLOAT, "Y coordinate in pixels (center)")
                         .addParameter("radius", ParameterType::FLOAT, "Radius in pixels")
                         .addParameter("color1", ParameterType::COLOR, "Center color")
                         .addParameter("color2", ParameterType::COLOR, "Middle color")
                         .addParameter("color3", ParameterType::COLOR, "Edge color");
    registry.registerFunction(std::move(circle_create_radial3));

    // CIRCLE_CREATE_RADIAL_4 - Create a four-color radial gradient circle (returns ID)
    CommandDefinition circle_create_radial4("CIRCLE_CREATE_RADIAL_4",
                                             "Create a four-color radial gradient circle and return its ID",
                                             "st_circle_create_radial_4", "graphics", false, ReturnType::INT);
    circle_create_radial4.addParameter("x", ParameterType::FLOAT, "X coordinate in pixels (center)")
                         .addParameter("y", ParameterType::FLOAT, "Y coordinate in pixels (center)")
                         .addParameter("radius", ParameterType::FLOAT, "Radius in pixels")
                         .addParameter("color1", ParameterType::COLOR, "Center color")
                         .addParameter("color2", ParameterType::COLOR, "First ring color")
                         .addParameter("color3", ParameterType::COLOR, "Second ring color")
                         .addParameter("color4", ParameterType::COLOR, "Edge color");
    registry.registerFunction(std::move(circle_create_radial4));

    // CIRCLE_CREATE_OUTLINE - Create an outlined circle (returns ID)
    CommandDefinition circle_create_outline("CIRCLE_CREATE_OUTLINE",
                                             "Create an outlined circle and return its ID",
                                             "st_circle_create_outline", "graphics", false, ReturnType::INT);
    circle_create_outline.addParameter("x", ParameterType::FLOAT, "X coordinate in pixels (center)")
                         .addParameter("y", ParameterType::FLOAT, "Y coordinate in pixels (center)")
                         .addParameter("radius", ParameterType::FLOAT, "Radius in pixels")
                         .addParameter("fillColor", ParameterType::COLOR, "Fill color")
                         .addParameter("outlineColor", ParameterType::COLOR, "Outline color")
                         .addParameter("lineWidth", ParameterType::FLOAT, "Outline width in pixels", true, "2.0");
    registry.registerFunction(std::move(circle_create_outline));

    // CIRCLE_CREATE_DASHED_OUTLINE - Create a dashed outline circle (returns ID)
    CommandDefinition circle_create_dashed("CIRCLE_CREATE_DASHED_OUTLINE",
                                            "Create a dashed outline circle and return its ID",
                                            "st_circle_create_dashed_outline", "graphics", false, ReturnType::INT);
    circle_create_dashed.addParameter("x", ParameterType::FLOAT, "X coordinate in pixels (center)")
                        .addParameter("y", ParameterType::FLOAT, "Y coordinate in pixels (center)")
                        .addParameter("radius", ParameterType::FLOAT, "Radius in pixels")
                        .addParameter("fillColor", ParameterType::COLOR, "Fill color")
                        .addParameter("outlineColor", ParameterType::COLOR, "Outline color")
                        .addParameter("lineWidth", ParameterType::FLOAT, "Outline width in pixels", true, "2.0")
                        .addParameter("dashLength", ParameterType::FLOAT, "Dash length in pixels", true, "10.0");
    registry.registerFunction(std::move(circle_create_dashed));

    // CIRCLE_CREATE_RING - Create a ring (hollow circle) (returns ID)
    CommandDefinition circle_create_ring("CIRCLE_CREATE_RING",
                                          "Create a ring (hollow circle) and return its ID",
                                          "st_circle_create_ring", "graphics", false, ReturnType::INT);
    circle_create_ring.addParameter("x", ParameterType::FLOAT, "X coordinate in pixels (center)")
                      .addParameter("y", ParameterType::FLOAT, "Y coordinate in pixels (center)")
                      .addParameter("outerRadius", ParameterType::FLOAT, "Outer radius in pixels")
                      .addParameter("innerRadius", ParameterType::FLOAT, "Inner radius in pixels")
                      .addParameter("color", ParameterType::COLOR, "Ring color");
    registry.registerFunction(std::move(circle_create_ring));

    // CIRCLE_CREATE_PIE_SLICE - Create a pie slice (returns ID)
    CommandDefinition circle_create_pie("CIRCLE_CREATE_PIE_SLICE",
                                         "Create a pie slice and return its ID",
                                         "st_circle_create_pie_slice", "graphics", false, ReturnType::INT);
    circle_create_pie.addParameter("x", ParameterType::FLOAT, "X coordinate in pixels (center)")
                     .addParameter("y", ParameterType::FLOAT, "Y coordinate in pixels (center)")
                     .addParameter("radius", ParameterType::FLOAT, "Radius in pixels")
                     .addParameter("startAngle", ParameterType::FLOAT, "Start angle in radians")
                     .addParameter("endAngle", ParameterType::FLOAT, "End angle in radians")
                     .addParameter("color", ParameterType::COLOR, "Slice color");
    registry.registerFunction(std::move(circle_create_pie));

    // CIRCLE_CREATE_ARC - Create an arc segment (returns ID)
    CommandDefinition circle_create_arc("CIRCLE_CREATE_ARC",
                                         "Create an arc segment and return its ID",
                                         "st_circle_create_arc", "graphics", false, ReturnType::INT);
    circle_create_arc.addParameter("x", ParameterType::FLOAT, "X coordinate in pixels (center)")
                     .addParameter("y", ParameterType::FLOAT, "Y coordinate in pixels (center)")
                     .addParameter("radius", ParameterType::FLOAT, "Radius in pixels")
                     .addParameter("startAngle", ParameterType::FLOAT, "Start angle in radians")
                     .addParameter("endAngle", ParameterType::FLOAT, "End angle in radians")
                     .addParameter("color", ParameterType::COLOR, "Arc color")
                     .addParameter("lineWidth", ParameterType::FLOAT, "Arc thickness in pixels", true, "2.0");
    registry.registerFunction(std::move(circle_create_arc));

    // CIRCLE_CREATE_DOTS_RING - Create a ring of dots (returns ID)
    CommandDefinition circle_create_dots("CIRCLE_CREATE_DOTS_RING",
                                          "Create a ring of dots and return its ID",
                                          "st_circle_create_dots_ring", "graphics", false, ReturnType::INT);
    circle_create_dots.addParameter("x", ParameterType::FLOAT, "X coordinate in pixels (center)")
                      .addParameter("y", ParameterType::FLOAT, "Y coordinate in pixels (center)")
                      .addParameter("radius", ParameterType::FLOAT, "Ring radius in pixels")
                      .addParameter("dotColor", ParameterType::COLOR, "Dot color")
                      .addParameter("backgroundColor", ParameterType::COLOR, "Background color")
                      .addParameter("dotRadius", ParameterType::FLOAT, "Radius of each dot in pixels", true, "3.0")
                      .addParameter("numDots", ParameterType::INT, "Number of dots", true, "12");
    registry.registerFunction(std::move(circle_create_dots));

    // CIRCLE_CREATE_STAR_BURST - Create a star burst pattern (returns ID)
    CommandDefinition circle_create_star("CIRCLE_CREATE_STAR_BURST",
                                          "Create a star burst pattern and return its ID",
                                          "st_circle_create_star_burst", "graphics", false, ReturnType::INT);
    circle_create_star.addParameter("x", ParameterType::FLOAT, "X coordinate in pixels (center)")
                      .addParameter("y", ParameterType::FLOAT, "Y coordinate in pixels (center)")
                      .addParameter("radius", ParameterType::FLOAT, "Radius in pixels")
                      .addParameter("color1", ParameterType::COLOR, "First ray color")
                      .addParameter("color2", ParameterType::COLOR, "Second ray color")
                      .addParameter("numRays", ParameterType::INT, "Number of rays", true, "8");
    registry.registerFunction(std::move(circle_create_star));

    // CIRCLE_SET_POSITION - Update circle position
    CommandDefinition circle_set_pos("CIRCLE_SET_POSITION",
                                      "Update circle position by ID",
                                      "st_circle_set_position", "graphics");
    circle_set_pos.addParameter("id", ParameterType::INT, "Circle ID")
                  .addParameter("x", ParameterType::FLOAT, "New X coordinate (center)")
                  .addParameter("y", ParameterType::FLOAT, "New Y coordinate (center)");
    registry.registerCommand(std::move(circle_set_pos));

    // CIRCLE_SET_RADIUS - Update circle radius
    CommandDefinition circle_set_radius("CIRCLE_SET_RADIUS",
                                         "Update circle radius by ID",
                                         "st_circle_set_radius", "graphics");
    circle_set_radius.addParameter("id", ParameterType::INT, "Circle ID")
                     .addParameter("radius", ParameterType::FLOAT, "New radius in pixels");
    registry.registerCommand(std::move(circle_set_radius));

    // CIRCLE_SET_COLOR - Update circle color
    CommandDefinition circle_set_color("CIRCLE_SET_COLOR",
                                        "Update circle color by ID",
                                        "st_circle_set_color", "graphics");
    circle_set_color.addParameter("id", ParameterType::INT, "Circle ID")
                    .addParameter("color", ParameterType::COLOR, "New color");
    registry.registerCommand(std::move(circle_set_color));

    // CIRCLE_SET_COLORS - Update circle gradient colors
    CommandDefinition circle_set_colors("CIRCLE_SET_COLORS",
                                         "Update circle gradient colors by ID",
                                         "st_circle_set_colors", "graphics");
    circle_set_colors.addParameter("id", ParameterType::INT, "Circle ID")
                     .addParameter("color1", ParameterType::COLOR, "First color")
                     .addParameter("color2", ParameterType::COLOR, "Second color")
                     .addParameter("color3", ParameterType::COLOR, "Third color")
                     .addParameter("color4", ParameterType::COLOR, "Fourth color");
    registry.registerCommand(std::move(circle_set_colors));

    // CIRCLE_SET_PARAMETERS - Update circle pattern parameters
    CommandDefinition circle_set_params("CIRCLE_SET_PARAMETERS",
                                         "Update circle pattern parameters by ID",
                                         "st_circle_set_parameters", "graphics");
    circle_set_params.addParameter("id", ParameterType::INT, "Circle ID")
                     .addParameter("param1", ParameterType::FLOAT, "First parameter")
                     .addParameter("param2", ParameterType::FLOAT, "Second parameter")
                     .addParameter("param3", ParameterType::FLOAT, "Third parameter");
    registry.registerCommand(std::move(circle_set_params));

    // CIRCLE_SET_VISIBLE - Show or hide circle
    CommandDefinition circle_set_visible("CIRCLE_SET_VISIBLE",
                                          "Show or hide a circle by ID",
                                          "st_circle_set_visible", "graphics");
    circle_set_visible.addParameter("id", ParameterType::INT, "Circle ID")
                      .addParameter("visible", ParameterType::INT, "1 to show, 0 to hide");
    registry.registerCommand(std::move(circle_set_visible));

    // CIRCLE_DELETE - Delete a circle
    CommandDefinition circle_delete("CIRCLE_DELETE",
                                     "Delete a circle by ID",
                                     "st_circle_delete", "graphics");
    circle_delete.addParameter("id", ParameterType::INT, "Circle ID");
    registry.registerCommand(std::move(circle_delete));

    // CIRCLE_DELETE_ALL - Delete all circles
    CommandDefinition circle_delete_all("CIRCLE_DELETE_ALL",
                                         "Delete all managed circles",
                                         "st_circle_delete_all", "graphics");
    registry.registerCommand(std::move(circle_delete_all));

    // CIRCLE_SET_MAX - Set maximum number of circles
    CommandDefinition circle_set_max("CIRCLE_SET_MAX",
                                      "Set the maximum number of circles that can be created",
                                      "st_circle_set_max", "graphics");
    circle_set_max.addParameter("max", ParameterType::INT, "Maximum number of circles");
    registry.registerCommand(std::move(circle_set_max));
}

// =============================================================================
// CIRCLE FUNCTIONS
// =============================================================================

void SuperTerminalCommandRegistry::registerCircleFunctions(CommandRegistry& registry) {
    // CIRCLE_COUNT() - Get the total number of circles
    CommandDefinition circle_count("CIRCLE_COUNT",
                                    "Get the total number of circles",
                                    "st_circle_count", "graphics", false, ReturnType::INT);
    registry.registerFunction(std::move(circle_count));

    // CIRCLE_IS_EMPTY() - Check if there are no circles
    CommandDefinition circle_is_empty("CIRCLE_IS_EMPTY",
                                       "Check if there are no circles",
                                       "st_circle_is_empty", "graphics", false, ReturnType::INT);
    registry.registerFunction(std::move(circle_is_empty));

    // CIRCLE_GET_MAX() - Get the maximum number of circles
    CommandDefinition circle_get_max("CIRCLE_GET_MAX",
                                      "Get the maximum number of circles",
                                      "st_circle_get_max", "graphics", false, ReturnType::INT);
    registry.registerFunction(std::move(circle_get_max));

    // CIRCLE_EXISTS() - Check if a circle exists
    CommandDefinition circle_exists("CIRCLE_EXISTS",
                                     "Check if a circle exists by ID",
                                     "st_circle_exists", "graphics", false, ReturnType::INT);
    circle_exists.addParameter("id", ParameterType::INT, "Circle ID");
    registry.registerFunction(std::move(circle_exists));

    // CIRCLE_IS_VISIBLE() - Check if a circle is visible
    CommandDefinition circle_is_visible("CIRCLE_IS_VISIBLE",
                                         "Check if a circle is visible by ID",
                                         "st_circle_is_visible", "graphics", false, ReturnType::INT);
    circle_is_visible.addParameter("id", ParameterType::INT, "Circle ID");
    registry.registerFunction(std::move(circle_is_visible));
}

// =============================================================================
// LINE COMMANDS
// =============================================================================

void SuperTerminalCommandRegistry::registerLineCommands(CommandRegistry& registry) {
    // LINE_CREATE - Create a solid-color line (returns ID)
    CommandDefinition line_create("LINE_CREATE",
                                   "Create a solid-color line and return its ID",
                                   "st_line_create", "graphics", false, ReturnType::INT);
    line_create.addParameter("x1", ParameterType::FLOAT, "Start X coordinate in pixels")
               .addParameter("y1", ParameterType::FLOAT, "Start Y coordinate in pixels")
               .addParameter("x2", ParameterType::FLOAT, "End X coordinate in pixels")
               .addParameter("y2", ParameterType::FLOAT, "End Y coordinate in pixels")
               .addParameter("color", ParameterType::COLOR, "RGBA color", true, "0xFFFFFFFF")
               .addParameter("thickness", ParameterType::FLOAT, "Line thickness in pixels", true, "2.0");
    registry.registerFunction(std::move(line_create));

    // LINE_CREATE_GRADIENT - Create a gradient line (returns ID)
    CommandDefinition line_create_gradient("LINE_CREATE_GRADIENT",
                                            "Create a gradient line and return its ID",
                                            "st_line_create_gradient", "graphics", false, ReturnType::INT);
    line_create_gradient.addParameter("x1", ParameterType::FLOAT, "Start X coordinate in pixels")
                        .addParameter("y1", ParameterType::FLOAT, "Start Y coordinate in pixels")
                        .addParameter("x2", ParameterType::FLOAT, "End X coordinate in pixels")
                        .addParameter("y2", ParameterType::FLOAT, "End Y coordinate in pixels")
                        .addParameter("color1", ParameterType::COLOR, "Start color")
                        .addParameter("color2", ParameterType::COLOR, "End color")
                        .addParameter("thickness", ParameterType::FLOAT, "Line thickness in pixels", true, "2.0");
    registry.registerFunction(std::move(line_create_gradient));

    // LINE_CREATE_DASHED - Create a dashed line (returns ID)
    CommandDefinition line_create_dashed("LINE_CREATE_DASHED",
                                          "Create a dashed line and return its ID",
                                          "st_line_create_dashed", "graphics", false, ReturnType::INT);
    line_create_dashed.addParameter("x1", ParameterType::FLOAT, "Start X coordinate in pixels")
                      .addParameter("y1", ParameterType::FLOAT, "Start Y coordinate in pixels")
                      .addParameter("x2", ParameterType::FLOAT, "End X coordinate in pixels")
                      .addParameter("y2", ParameterType::FLOAT, "End Y coordinate in pixels")
                      .addParameter("color", ParameterType::COLOR, "RGBA color")
                      .addParameter("thickness", ParameterType::FLOAT, "Line thickness in pixels", true, "2.0")
                      .addParameter("dashLength", ParameterType::FLOAT, "Dash length in pixels", true, "10.0")
                      .addParameter("gapLength", ParameterType::FLOAT, "Gap length in pixels", true, "5.0");
    registry.registerFunction(std::move(line_create_dashed));

    // LINE_CREATE_DOTTED - Create a dotted line (returns ID)
    CommandDefinition line_create_dotted("LINE_CREATE_DOTTED",
                                          "Create a dotted line and return its ID",
                                          "st_line_create_dotted", "graphics", false, ReturnType::INT);
    line_create_dotted.addParameter("x1", ParameterType::FLOAT, "Start X coordinate in pixels")
                      .addParameter("y1", ParameterType::FLOAT, "Start Y coordinate in pixels")
                      .addParameter("x2", ParameterType::FLOAT, "End X coordinate in pixels")
                      .addParameter("y2", ParameterType::FLOAT, "End Y coordinate in pixels")
                      .addParameter("color", ParameterType::COLOR, "RGBA color")
                      .addParameter("thickness", ParameterType::FLOAT, "Line thickness in pixels", true, "2.0")
                      .addParameter("dotSpacing", ParameterType::FLOAT, "Distance between dot centers in pixels", true, "10.0");
    registry.registerFunction(std::move(line_create_dotted));

    // LINE_SET_ENDPOINTS - Update line endpoints
    CommandDefinition line_set_endpoints("LINE_SET_ENDPOINTS",
                                          "Update line endpoints by ID",
                                          "st_line_set_endpoints", "graphics");
    line_set_endpoints.addParameter("id", ParameterType::INT, "Line ID")
                      .addParameter("x1", ParameterType::FLOAT, "New start X coordinate")
                      .addParameter("y1", ParameterType::FLOAT, "New start Y coordinate")
                      .addParameter("x2", ParameterType::FLOAT, "New end X coordinate")
                      .addParameter("y2", ParameterType::FLOAT, "New end Y coordinate");
    registry.registerCommand(std::move(line_set_endpoints));

    // LINE_SET_THICKNESS - Update line thickness
    CommandDefinition line_set_thickness("LINE_SET_THICKNESS",
                                          "Update line thickness by ID",
                                          "st_line_set_thickness", "graphics");
    line_set_thickness.addParameter("id", ParameterType::INT, "Line ID")
                      .addParameter("thickness", ParameterType::FLOAT, "New thickness in pixels");
    registry.registerCommand(std::move(line_set_thickness));

    // LINE_SET_COLOR - Update line color
    CommandDefinition line_set_color("LINE_SET_COLOR",
                                      "Update line color by ID",
                                      "st_line_set_color", "graphics");
    line_set_color.addParameter("id", ParameterType::INT, "Line ID")
                  .addParameter("color", ParameterType::COLOR, "New color");
    registry.registerCommand(std::move(line_set_color));

    // LINE_SET_COLORS - Update line gradient colors
    CommandDefinition line_set_colors("LINE_SET_COLORS",
                                       "Update line gradient colors by ID",
                                       "st_line_set_colors", "graphics");
    line_set_colors.addParameter("id", ParameterType::INT, "Line ID")
                   .addParameter("color1", ParameterType::COLOR, "Start color")
                   .addParameter("color2", ParameterType::COLOR, "End color");
    registry.registerCommand(std::move(line_set_colors));

    // LINE_SET_DASH_PATTERN - Update dash pattern
    CommandDefinition line_set_dash("LINE_SET_DASH_PATTERN",
                                     "Update dash pattern by ID",
                                     "st_line_set_dash_pattern", "graphics");
    line_set_dash.addParameter("id", ParameterType::INT, "Line ID")
                 .addParameter("dashLength", ParameterType::FLOAT, "Dash length in pixels")
                 .addParameter("gapLength", ParameterType::FLOAT, "Gap length in pixels");
    registry.registerCommand(std::move(line_set_dash));

    // LINE_SET_VISIBLE - Show or hide line
    CommandDefinition line_set_visible("LINE_SET_VISIBLE",
                                        "Show or hide a line by ID",
                                        "st_line_set_visible", "graphics");
    line_set_visible.addParameter("id", ParameterType::INT, "Line ID")
                    .addParameter("visible", ParameterType::INT, "1 to show, 0 to hide");
    registry.registerCommand(std::move(line_set_visible));

    // LINE_DELETE - Delete a line
    CommandDefinition line_delete("LINE_DELETE",
                                   "Delete a line by ID",
                                   "st_line_delete", "graphics");
    line_delete.addParameter("id", ParameterType::INT, "Line ID");
    registry.registerCommand(std::move(line_delete));

    // LINE_DELETE_ALL - Delete all lines
    CommandDefinition line_delete_all("LINE_DELETE_ALL",
                                       "Delete all lines",
                                       "st_line_delete_all", "graphics");
    registry.registerCommand(std::move(line_delete_all));

    // LINE_SET_MAX - Set maximum number of lines
    CommandDefinition line_set_max("LINE_SET_MAX",
                                    "Set the maximum number of lines that can be created",
                                    "st_line_set_max", "graphics");
    line_set_max.addParameter("max", ParameterType::INT, "Maximum number of lines");
    registry.registerCommand(std::move(line_set_max));
}

// =============================================================================
// LINE FUNCTIONS
// =============================================================================

void SuperTerminalCommandRegistry::registerLineFunctions(CommandRegistry& registry) {
    // LINE_COUNT() - Get the total number of lines
    CommandDefinition line_count("LINE_COUNT",
                                  "Get the total number of lines",
                                  "st_line_count", "graphics", false, ReturnType::INT);
    registry.registerFunction(std::move(line_count));

    // LINE_IS_EMPTY() - Check if there are no lines
    CommandDefinition line_is_empty("LINE_IS_EMPTY",
                                     "Check if there are no lines",
                                     "st_line_is_empty", "graphics", false, ReturnType::INT);
    registry.registerFunction(std::move(line_is_empty));

    // LINE_GET_MAX() - Get the maximum number of lines
    CommandDefinition line_get_max("LINE_GET_MAX",
                                    "Get the maximum number of lines",
                                    "st_line_get_max", "graphics", false, ReturnType::INT);
    registry.registerFunction(std::move(line_get_max));

    // LINE_EXISTS() - Check if a line exists
    CommandDefinition line_exists("LINE_EXISTS",
                                   "Check if a line exists by ID",
                                   "st_line_exists", "graphics", false, ReturnType::INT);
    line_exists.addParameter("id", ParameterType::INT, "Line ID");
    registry.registerFunction(std::move(line_exists));

    // LINE_IS_VISIBLE() - Check if a line is visible
    CommandDefinition line_is_visible("LINE_IS_VISIBLE",
                                       "Check if a line is visible by ID",
                                       "st_line_is_visible", "graphics", false, ReturnType::INT);
    line_is_visible.addParameter("id", ParameterType::INT, "Line ID");
    registry.registerFunction(std::move(line_is_visible));
}

// =============================================================================
// Unified Video Mode Commands (V-prefix)
// =============================================================================

void SuperTerminalCommandRegistry::registerVideoModeCommands(CommandRegistry& registry) {
    // VPSET - Set pixel (unified, works in any video mode)
    CommandDefinition vpset("VPSET",
                           "Set a pixel in current video mode",
                           "video_pset", "video");
    vpset.addParameter("x", ParameterType::INT, "X coordinate")
        .addParameter("y", ParameterType::INT, "Y coordinate")
        .addParameter("color", ParameterType::INT, "Color value");
    registry.registerCommand(std::move(vpset));

    // VPGET - Get pixel color (unified)
    CommandDefinition vpget("VPGET",
                           "Get pixel color in current video mode",
                           "video_pget", "video");
    vpget.addParameter("x", ParameterType::INT, "X coordinate")
        .addParameter("y", ParameterType::INT, "Y coordinate");
    registry.registerCommand(std::move(vpget));

    // VCLEAR - Clear screen (unified)
    CommandDefinition vclear("VCLEAR",
                            "Clear screen in current video mode",
                            "video_clear", "video");
    vclear.addParameter("color", ParameterType::INT, "Color value");
    registry.registerCommand(std::move(vclear));

    // VCLS - Clear screen (alias for VCLEAR)
    CommandDefinition vcls("VCLS",
                          "Clear screen in current video mode (alias for VCLEAR)",
                          "video_clear", "video");
    vcls.addParameter("color", ParameterType::INT, "Color value");
    registry.registerCommand(std::move(vcls));

    // VLINE - Draw line (unified)
    CommandDefinition vline("VLINE",
                           "Draw a line in current video mode",
                           "video_line", "video");
    vline.addParameter("x1", ParameterType::INT, "Start X coordinate")
        .addParameter("y1", ParameterType::INT, "Start Y coordinate")
        .addParameter("x2", ParameterType::INT, "End X coordinate")
        .addParameter("y2", ParameterType::INT, "End Y coordinate")
        .addParameter("color", ParameterType::INT, "Color value");
    registry.registerCommand(std::move(vline));

    // VRECT - Draw filled rectangle (unified)
    CommandDefinition vrect("VRECT",
                           "Draw a filled rectangle in current video mode",
                           "video_rect", "video");
    vrect.addParameter("x", ParameterType::INT, "X coordinate")
        .addParameter("y", ParameterType::INT, "Y coordinate")
        .addParameter("width", ParameterType::INT, "Width")
        .addParameter("height", ParameterType::INT, "Height")
        .addParameter("color", ParameterType::INT, "Color value");
    registry.registerCommand(std::move(vrect));

    // VRECTF - Draw filled rectangle (alias for VRECT)
    CommandDefinition vrectf("VRECTF",
                            "Draw a filled rectangle in current video mode",
                            "video_rect", "video");
    vrectf.addParameter("x", ParameterType::INT, "X coordinate")
         .addParameter("y", ParameterType::INT, "Y coordinate")
         .addParameter("width", ParameterType::INT, "Width")
         .addParameter("height", ParameterType::INT, "Height")
         .addParameter("color", ParameterType::INT, "Color value");
    registry.registerCommand(std::move(vrectf));

    // VCIRCLE - Draw filled circle (unified)
    CommandDefinition vcircle("VCIRCLE",
                             "Draw a filled circle in current video mode",
                             "video_circle", "video");
    vcircle.addParameter("cx", ParameterType::INT, "Center X coordinate")
          .addParameter("cy", ParameterType::INT, "Center Y coordinate")
          .addParameter("radius", ParameterType::INT, "Radius")
          .addParameter("color", ParameterType::INT, "Color value");
    registry.registerCommand(std::move(vcircle));

    // VCIRCLEF - Draw filled circle (alias for VCIRCLE)
    CommandDefinition vcirclef("VCIRCLEF",
                              "Draw a filled circle in current video mode",
                              "video_circle", "video");
    vcirclef.addParameter("cx", ParameterType::INT, "Center X coordinate")
           .addParameter("cy", ParameterType::INT, "Center Y coordinate")
           .addParameter("radius", ParameterType::INT, "Radius")
           .addParameter("color", ParameterType::INT, "Color value");
    registry.registerCommand(std::move(vcirclef));

    // GPU-accelerated commands

    // VCLEAR_GPU - Clear GPU buffer
    CommandDefinition vclear_gpu("VCLEAR_GPU",
                                "Clear GPU buffer in current video mode",
                                "video_clear_gpu", "video");
    vclear_gpu.addParameter("buffer_id", ParameterType::INT, "Buffer ID")
             .addParameter("color", ParameterType::INT, "Color value");
    registry.registerCommand(std::move(vclear_gpu));

    // VLINE_GPU - Draw line (GPU-accelerated)
    CommandDefinition vline_gpu("VLINE_GPU",
                               "Draw a line (GPU-accelerated)",
                               "video_line_gpu", "video");
    vline_gpu.addParameter("buffer_id", ParameterType::INT, "Buffer ID")
            .addParameter("x1", ParameterType::INT, "Start X coordinate")
            .addParameter("y1", ParameterType::INT, "Start Y coordinate")
            .addParameter("x2", ParameterType::INT, "End X coordinate")
            .addParameter("y2", ParameterType::INT, "End Y coordinate")
            .addParameter("color", ParameterType::INT, "Color value");
    registry.registerCommand(std::move(vline_gpu));

    // VRECT_GPU - Draw rectangle (GPU-accelerated)
    CommandDefinition vrect_gpu("VRECT_GPU",
                               "Draw a filled rectangle (GPU-accelerated)",
                               "video_rect_gpu", "video");
    vrect_gpu.addParameter("buffer_id", ParameterType::INT, "Buffer ID")
            .addParameter("x", ParameterType::INT, "X coordinate")
            .addParameter("y", ParameterType::INT, "Y coordinate")
            .addParameter("width", ParameterType::INT, "Width")
            .addParameter("height", ParameterType::INT, "Height")
            .addParameter("color", ParameterType::INT, "Color value");
    registry.registerCommand(std::move(vrect_gpu));

    // VCIRCLE_GPU - Draw circle (GPU-accelerated)
    CommandDefinition vcircle_gpu("VCIRCLE_GPU",
                                 "Draw a filled circle (GPU-accelerated)",
                                 "video_circle_gpu", "video");
    vcircle_gpu.addParameter("buffer_id", ParameterType::INT, "Buffer ID")
              .addParameter("cx", ParameterType::INT, "Center X coordinate")
              .addParameter("cy", ParameterType::INT, "Center Y coordinate")
              .addParameter("radius", ParameterType::INT, "Radius")
              .addParameter("color", ParameterType::INT, "Color value");
    registry.registerCommand(std::move(vcircle_gpu));

    // Anti-aliased commands

    // VLINE_AA - Draw anti-aliased line
    CommandDefinition vline_aa("VLINE_AA",
                              "Draw an anti-aliased line",
                              "video_line_aa", "video");
    vline_aa.addParameter("buffer_id", ParameterType::INT, "Buffer ID")
           .addParameter("x1", ParameterType::INT, "Start X coordinate")
           .addParameter("y1", ParameterType::INT, "Start Y coordinate")
           .addParameter("x2", ParameterType::INT, "End X coordinate")
           .addParameter("y2", ParameterType::INT, "End Y coordinate")
           .addParameter("color", ParameterType::INT, "Color value");
    registry.registerCommand(std::move(vline_aa));

    // VCIRCLE_AA - Draw anti-aliased circle
    CommandDefinition vcircle_aa("VCIRCLE_AA",
                                "Draw an anti-aliased filled circle",
                                "video_circle_aa", "video");
    vcircle_aa.addParameter("buffer_id", ParameterType::INT, "Buffer ID")
             .addParameter("cx", ParameterType::INT, "Center X coordinate")
             .addParameter("cy", ParameterType::INT, "Center Y coordinate")
             .addParameter("radius", ParameterType::INT, "Radius")
             .addParameter("color", ParameterType::INT, "Color value");
    registry.registerCommand(std::move(vcircle_aa));

    // Gradient commands (URES only)

    // VRECT_GRADIENT - Rectangle with gradient
    CommandDefinition vrect_gradient("VRECT_GRADIENT",
                                    "Draw a rectangle with gradient (URES only)",
                                    "video_rect_gradient_gpu", "video");
    vrect_gradient.addParameter("buffer_id", ParameterType::INT, "Buffer ID")
                 .addParameter("x", ParameterType::INT, "X coordinate")
                 .addParameter("y", ParameterType::INT, "Y coordinate")
                 .addParameter("width", ParameterType::INT, "Width")
                 .addParameter("height", ParameterType::INT, "Height")
                 .addParameter("color_tl", ParameterType::INT, "Top-left color")
                 .addParameter("color_tr", ParameterType::INT, "Top-right color")
                 .addParameter("color_bl", ParameterType::INT, "Bottom-left color")
                 .addParameter("color_br", ParameterType::INT, "Bottom-right color");
    registry.registerCommand(std::move(vrect_gradient));

    // VCIRCLE_GRADIENT - Circle with radial gradient
    CommandDefinition vcircle_gradient("VCIRCLE_GRADIENT",
                                      "Draw a circle with radial gradient (URES only)",
                                      "video_circle_gradient_gpu", "video");
    vcircle_gradient.addParameter("buffer_id", ParameterType::INT, "Buffer ID")
                   .addParameter("cx", ParameterType::INT, "Center X coordinate")
                   .addParameter("cy", ParameterType::INT, "Center Y coordinate")
                   .addParameter("radius", ParameterType::INT, "Radius")
                   .addParameter("center_color", ParameterType::INT, "Center color")
                   .addParameter("edge_color", ParameterType::INT, "Edge color");
    registry.registerCommand(std::move(vcircle_gradient));

    // VRECT_GRADIENT_H - Horizontal gradient rectangle
    CommandDefinition vrect_gradient_h("VRECT_GRADIENT_H",
                                      "Draw a rectangle with horizontal gradient",
                                      "video_rect_gradient_h", "video");
    vrect_gradient_h.addParameter("x", ParameterType::INT, "X coordinate")
                   .addParameter("y", ParameterType::INT, "Y coordinate")
                   .addParameter("width", ParameterType::INT, "Width")
                   .addParameter("height", ParameterType::INT, "Height")
                   .addParameter("color_left", ParameterType::INT, "Left color")
                   .addParameter("color_right", ParameterType::INT, "Right color");
    registry.registerCommand(std::move(vrect_gradient_h));

    // VRECT_GRADIENT_V - Vertical gradient rectangle
    CommandDefinition vrect_gradient_v("VRECT_GRADIENT_V",
                                      "Draw a rectangle with vertical gradient",
                                      "video_rect_gradient_v", "video");
    vrect_gradient_v.addParameter("x", ParameterType::INT, "X coordinate")
                   .addParameter("y", ParameterType::INT, "Y coordinate")
                   .addParameter("width", ParameterType::INT, "Width")
                   .addParameter("height", ParameterType::INT, "Height")
                   .addParameter("color_top", ParameterType::INT, "Top color")
                   .addParameter("color_bottom", ParameterType::INT, "Bottom color");
    registry.registerCommand(std::move(vrect_gradient_v));

    // VCIRCLE_GRADIENT_AA - Anti-aliased circle with radial gradient (URES only)
    CommandDefinition vcircle_gradient_aa("VCIRCLE_GRADIENT_AA",
                                         "Draw an anti-aliased circle with radial gradient (URES only)",
                                         "video_circle_gradient_aa", "video");
    vcircle_gradient_aa.addParameter("buffer_id", ParameterType::INT, "Buffer ID")
                      .addParameter("cx", ParameterType::INT, "Center X coordinate")
                      .addParameter("cy", ParameterType::INT, "Center Y coordinate")
                      .addParameter("radius", ParameterType::INT, "Radius")
                      .addParameter("center_color", ParameterType::INT, "Center color")
                      .addParameter("edge_color", ParameterType::INT, "Edge color");
    registry.registerCommand(std::move(vcircle_gradient_aa));

    // Blit operations

    // VBLIT - Copy region
    CommandDefinition vblit("VBLIT",
                           "Copy rectangular region in current video mode",
                           "video_blit", "video");
    vblit.addParameter("src_x", ParameterType::INT, "Source X coordinate")
        .addParameter("src_y", ParameterType::INT, "Source Y coordinate")
        .addParameter("width", ParameterType::INT, "Width")
        .addParameter("height", ParameterType::INT, "Height")
        .addParameter("dst_x", ParameterType::INT, "Destination X coordinate")
        .addParameter("dst_y", ParameterType::INT, "Destination Y coordinate");
    registry.registerCommand(std::move(vblit));

    // VBLITT - Copy with transparency
    CommandDefinition vblitt("VBLITT",
                            "Copy rectangular region with transparency",
                            "video_blit_trans", "video");
    vblitt.addParameter("src_x", ParameterType::INT, "Source X coordinate")
         .addParameter("src_y", ParameterType::INT, "Source Y coordinate")
         .addParameter("width", ParameterType::INT, "Width")
         .addParameter("height", ParameterType::INT, "Height")
         .addParameter("dst_x", ParameterType::INT, "Destination X coordinate")
         .addParameter("dst_y", ParameterType::INT, "Destination Y coordinate");
    registry.registerCommand(std::move(vblitt));

    // VBLIT_GPU - Copy region (GPU)
    CommandDefinition vblit_gpu("VBLIT_GPU",
                               "Copy rectangular region (GPU-accelerated)",
                               "video_blit_gpu", "video");
    vblit_gpu.addParameter("src_buffer", ParameterType::INT, "Source buffer ID")
            .addParameter("dst_buffer", ParameterType::INT, "Destination buffer ID")
            .addParameter("src_x", ParameterType::INT, "Source X coordinate")
            .addParameter("src_y", ParameterType::INT, "Source Y coordinate")
            .addParameter("width", ParameterType::INT, "Width")
            .addParameter("height", ParameterType::INT, "Height")
            .addParameter("dst_x", ParameterType::INT, "Destination X coordinate")
            .addParameter("dst_y", ParameterType::INT, "Destination Y coordinate");
    registry.registerCommand(std::move(vblit_gpu));

    // VBLITT_GPU - Copy with transparency (GPU)
    CommandDefinition vblitt_gpu("VBLITT_GPU",
                                "Copy with transparency (GPU-accelerated)",
                                "video_blit_trans_gpu", "video");
    vblitt_gpu.addParameter("src_buffer", ParameterType::INT, "Source buffer ID")
             .addParameter("dst_buffer", ParameterType::INT, "Destination buffer ID")
             .addParameter("src_x", ParameterType::INT, "Source X coordinate")
             .addParameter("src_y", ParameterType::INT, "Source Y coordinate")
             .addParameter("width", ParameterType::INT, "Width")
             .addParameter("height", ParameterType::INT, "Height")
             .addParameter("dst_x", ParameterType::INT, "Destination X coordinate")
             .addParameter("dst_y", ParameterType::INT, "Destination Y coordinate");
    registry.registerCommand(std::move(vblitt_gpu));

    // Buffer management

    // VBUFFER - Set active buffer
    CommandDefinition vbuffer("VBUFFER",
                             "Set active buffer for drawing",
                             "video_buffer", "video");
    vbuffer.addParameter("buffer_id", ParameterType::INT, "Buffer ID");
    registry.registerCommand(std::move(vbuffer));

    // VFLIP - Flip buffers
    CommandDefinition vflip("VFLIP",
                           "Flip front and back buffers",
                           "video_flip", "video");
    registry.registerCommand(std::move(vflip));

    // VGPU_FLIP - GPU flip
    CommandDefinition vgpu_flip("VGPU_FLIP",
                               "GPU-accelerated buffer flip",
                               "video_gpu_flip", "video");
    registry.registerCommand(std::move(vgpu_flip));

    // VSWAP - Swap buffers
    CommandDefinition vswap("VSWAP",
                           "Swap front and back buffers",
                           "video_swap", "video");
    registry.registerCommand(std::move(vswap));

    // VSYNC - Sync GPU
    CommandDefinition vsync("VSYNC",
                           "Synchronize GPU operations",
                           "video_sync", "video");
    registry.registerCommand(std::move(vsync));

    // Palette management

    // VPALETTE_SET - Set palette color
    CommandDefinition vpalette_set("VPALETTE_SET",
                                  "Set global palette color",
                                  "video_set_palette", "video");
    vpalette_set.addParameter("index", ParameterType::INT, "Palette index (16-255)")
               .addParameter("r", ParameterType::INT, "Red (0-255)")
               .addParameter("g", ParameterType::INT, "Green (0-255)")
               .addParameter("b", ParameterType::INT, "Blue (0-255)");
    registry.registerCommand(std::move(vpalette_set));

    // VPALETTE_SET_ROW - Set row palette color
    CommandDefinition vpalette_set_row("VPALETTE_SET_ROW",
                                      "Set per-row palette color",
                                      "video_set_palette_row", "video");
    vpalette_set_row.addParameter("row", ParameterType::INT, "Row index")
                   .addParameter("index", ParameterType::INT, "Color index (0-15)")
                   .addParameter("r", ParameterType::INT, "Red (0-255)")
                   .addParameter("g", ParameterType::INT, "Green (0-255)")
                   .addParameter("b", ParameterType::INT, "Blue (0-255)");
    registry.registerCommand(std::move(vpalette_set_row));

    // VPALETTE_RESET - Reset palette to defaults
    CommandDefinition vpalette_reset("VPALETTE_RESET",
                                    "Reset palette to default colors",
                                    "video_reset_palette_to_default", "video");
    registry.registerCommand(std::move(vpalette_reset));

    // VPALETTE_ROW - Set row palette color (unified)
    CommandDefinition vpalette_row("VPALETTE_ROW",
                                   "Set per-row palette color in current video mode",
                                   "vpalette_row", "video");
    vpalette_row.addParameter("row", ParameterType::INT, "Row index")
                .addParameter("index", ParameterType::INT, "Color index (0-15)")
                .addParameter("r", ParameterType::INT, "Red (0-255)")
                .addParameter("g", ParameterType::INT, "Green (0-255)")
                .addParameter("b", ParameterType::INT, "Blue (0-255)");
    registry.registerCommand(std::move(vpalette_row));

    // XRES Palette commands

    // XRES_PALETTE_ROW - Set XRES per-row palette color
    CommandDefinition xres_palette_row("XRES_PALETTE_ROW",
                                       "Set XRES per-row palette color (0-15)",
                                       "xres_palette_row", "video");
    xres_palette_row.addParameter("row", ParameterType::INT, "Row index (0-239)")
                    .addParameter("index", ParameterType::INT, "Color index (0-15)")
                    .addParameter("r", ParameterType::INT, "Red (0-255)")
                    .addParameter("g", ParameterType::INT, "Green (0-255)")
                    .addParameter("b", ParameterType::INT, "Blue (0-255)");
    registry.registerCommand(std::move(xres_palette_row));

    // XRES_PALETTE_GLOBAL - Set XRES global palette color
    CommandDefinition xres_palette_global("XRES_PALETTE_GLOBAL",
                                          "Set XRES global palette color (16-255)",
                                          "xres_palette_global", "video");
    xres_palette_global.addParameter("index", ParameterType::INT, "Palette index (16-255)")
                       .addParameter("r", ParameterType::INT, "Red (0-255)")
                       .addParameter("g", ParameterType::INT, "Green (0-255)")
                       .addParameter("b", ParameterType::INT, "Blue (0-255)");
    registry.registerCommand(std::move(xres_palette_global));

    // XRES_PALETTE_RESET - Reset XRES palette to defaults
    CommandDefinition xres_palette_reset("XRES_PALETTE_RESET",
                                         "Reset XRES palette to default colors",
                                         "xres_palette_reset", "video");
    registry.registerCommand(std::move(xres_palette_reset));

    // WRES Palette commands

    // WRES_PALETTE_ROW - Set WRES per-row palette color
    CommandDefinition wres_palette_row("WRES_PALETTE_ROW",
                                       "Set WRES per-row palette color (0-15)",
                                       "wres_palette_row", "video");
    wres_palette_row.addParameter("row", ParameterType::INT, "Row index (0-239)")
                    .addParameter("index", ParameterType::INT, "Color index (0-15)")
                    .addParameter("r", ParameterType::INT, "Red (0-255)")
                    .addParameter("g", ParameterType::INT, "Green (0-255)")
                    .addParameter("b", ParameterType::INT, "Blue (0-255)");
    registry.registerCommand(std::move(wres_palette_row));

    // WRES_PALETTE_GLOBAL - Set WRES global palette color
    CommandDefinition wres_palette_global("WRES_PALETTE_GLOBAL",
                                          "Set WRES global palette color (16-255)",
                                          "wres_palette_global", "video");
    wres_palette_global.addParameter("index", ParameterType::INT, "Palette index (16-255)")
                       .addParameter("r", ParameterType::INT, "Red (0-255)")
                       .addParameter("g", ParameterType::INT, "Green (0-255)")
                       .addParameter("b", ParameterType::INT, "Blue (0-255)");
    registry.registerCommand(std::move(wres_palette_global));

    // WRES_PALETTE_RESET - Reset WRES palette to defaults
    CommandDefinition wres_palette_reset("WRES_PALETTE_RESET",
                                         "Reset WRES palette to default colors",
                                         "wres_palette_reset", "video");
    registry.registerCommand(std::move(wres_palette_reset));

    // PRES Palette commands

    // PRES_PALETTE_ROW - Set PRES per-row palette color
    CommandDefinition pres_palette_row("PRES_PALETTE_ROW",
                                       "Set PRES per-row palette color (0-15)",
                                       "pres_palette_row", "video");
    pres_palette_row.addParameter("row", ParameterType::INT, "Row index (0-719)")
                    .addParameter("index", ParameterType::INT, "Color index (0-15)")
                    .addParameter("r", ParameterType::INT, "Red (0-255)")
                    .addParameter("g", ParameterType::INT, "Green (0-255)")
                    .addParameter("b", ParameterType::INT, "Blue (0-255)");
    registry.registerCommand(std::move(pres_palette_row));

    // PRES_PALETTE_GLOBAL - Set PRES global palette color
    CommandDefinition pres_palette_global("PRES_PALETTE_GLOBAL",
                                          "Set PRES global palette color (16-255)",
                                          "pres_palette_global", "video");
    pres_palette_global.addParameter("index", ParameterType::INT, "Palette index (16-255)")
                       .addParameter("r", ParameterType::INT, "Red (0-255)")
                       .addParameter("g", ParameterType::INT, "Green (0-255)")
                       .addParameter("b", ParameterType::INT, "Blue (0-255)");
    registry.registerCommand(std::move(pres_palette_global));

    // PRES_PALETTE_RESET - Reset PRES palette to defaults
    CommandDefinition pres_palette_reset("PRES_PALETTE_RESET",
                                         "Reset PRES palette to default colors",
                                         "pres_palette_reset", "video");
    registry.registerCommand(std::move(pres_palette_reset));

    // =========================================================================
    // Palette Automation Commands (Copper-style effects)
    // =========================================================================

    // XRES_PALETTE_AUTO_GRADIENT - Enable gradient automation on XRES
    CommandDefinition xres_auto_gradient("XRES_PALETTE_AUTO_GRADIENT",
        "Enable automatic gradient effect on XRES palette index",
        "st_xres_palette_auto_gradient", "video");
    xres_auto_gradient.addParameter("paletteIndex", ParameterType::INT, "Palette index (0-15)")
        .addParameter("startRow", ParameterType::INT, "Start row (0-239)")
        .addParameter("endRow", ParameterType::INT, "End row (0-239)")
        .addParameter("startR", ParameterType::INT, "Start red (0-255)")
        .addParameter("startG", ParameterType::INT, "Start green (0-255)")
        .addParameter("startB", ParameterType::INT, "Start blue (0-255)")
        .addParameter("endR", ParameterType::INT, "End red (0-255)")
        .addParameter("endG", ParameterType::INT, "End green (0-255)")
        .addParameter("endB", ParameterType::INT, "End blue (0-255)")
        .addParameter("speed", ParameterType::FLOAT, "Animation speed (0.0 = static)");
    registry.registerCommand(std::move(xres_auto_gradient));

    // XRES_PALETTE_AUTO_BARS - Enable bars automation on XRES
    CommandDefinition xres_auto_bars("XRES_PALETTE_AUTO_BARS",
        "Enable automatic color bars effect on XRES palette index",
        "st_xres_palette_auto_bars", "video");
    xres_auto_bars.addParameter("paletteIndex", ParameterType::INT, "Palette index (0-15)")
        .addParameter("startRow", ParameterType::INT, "Start row (0-239)")
        .addParameter("endRow", ParameterType::INT, "End row (0-239)")
        .addParameter("barHeight", ParameterType::INT, "Bar height in scanlines")
        .addParameter("numColors", ParameterType::INT, "Number of colors (1-4)")
        .addParameter("r1", ParameterType::INT, "Color 1 red (0-255)")
        .addParameter("g1", ParameterType::INT, "Color 1 green (0-255)")
        .addParameter("b1", ParameterType::INT, "Color 1 blue (0-255)")
        .addParameter("r2", ParameterType::INT, "Color 2 red (0-255)")
        .addParameter("g2", ParameterType::INT, "Color 2 green (0-255)")
        .addParameter("b2", ParameterType::INT, "Color 2 blue (0-255)")
        .addParameter("r3", ParameterType::INT, "Color 3 red (0-255)")
        .addParameter("g3", ParameterType::INT, "Color 3 green (0-255)")
        .addParameter("b3", ParameterType::INT, "Color 3 blue (0-255)")
        .addParameter("r4", ParameterType::INT, "Color 4 red (0-255)")
        .addParameter("g4", ParameterType::INT, "Color 4 green (0-255)")
        .addParameter("b4", ParameterType::INT, "Color 4 blue (0-255)")
        .addParameter("speed", ParameterType::FLOAT, "Scroll speed (0.0 = static)");
    registry.registerCommand(std::move(xres_auto_bars));

    // XRES_PALETTE_AUTO_STOP - Disable XRES palette automation
    CommandDefinition xres_auto_stop("XRES_PALETTE_AUTO_STOP",
        "Disable all XRES palette automation",
        "st_xres_palette_auto_stop", "video");
    registry.registerCommand(std::move(xres_auto_stop));

    // XRES_PALETTE_AUTO_UPDATE - Update XRES palette automation
    CommandDefinition xres_auto_update("XRES_PALETTE_AUTO_UPDATE",
        "Update XRES palette automation (call once per frame)",
        "st_xres_palette_auto_update", "video");
    xres_auto_update.addParameter("deltaTime", ParameterType::FLOAT, "Time since last frame (seconds)");
    registry.registerCommand(std::move(xres_auto_update));

    // WRES_PALETTE_AUTO_GRADIENT - Enable gradient automation on WRES
    CommandDefinition wres_auto_gradient("WRES_PALETTE_AUTO_GRADIENT",
        "Enable automatic gradient effect on WRES palette index",
        "st_wres_palette_auto_gradient", "video");
    wres_auto_gradient.addParameter("paletteIndex", ParameterType::INT, "Palette index (0-15)")
        .addParameter("startRow", ParameterType::INT, "Start row (0-239)")
        .addParameter("endRow", ParameterType::INT, "End row (0-239)")
        .addParameter("startR", ParameterType::INT, "Start red (0-255)")
        .addParameter("startG", ParameterType::INT, "Start green (0-255)")
        .addParameter("startB", ParameterType::INT, "Start blue (0-255)")
        .addParameter("endR", ParameterType::INT, "End red (0-255)")
        .addParameter("endG", ParameterType::INT, "End green (0-255)")
        .addParameter("endB", ParameterType::INT, "End blue (0-255)")
        .addParameter("speed", ParameterType::FLOAT, "Animation speed (0.0 = static)");
    registry.registerCommand(std::move(wres_auto_gradient));

    // WRES_PALETTE_AUTO_BARS - Enable bars automation on WRES
    CommandDefinition wres_auto_bars("WRES_PALETTE_AUTO_BARS",
        "Enable automatic color bars effect on WRES palette index",
        "st_wres_palette_auto_bars", "video");
    wres_auto_bars.addParameter("paletteIndex", ParameterType::INT, "Palette index (0-15)")
        .addParameter("startRow", ParameterType::INT, "Start row (0-239)")
        .addParameter("endRow", ParameterType::INT, "End row (0-239)")
        .addParameter("barHeight", ParameterType::INT, "Bar height in scanlines")
        .addParameter("numColors", ParameterType::INT, "Number of colors (1-4)")
        .addParameter("r1", ParameterType::INT, "Color 1 red (0-255)")
        .addParameter("g1", ParameterType::INT, "Color 1 green (0-255)")
        .addParameter("b1", ParameterType::INT, "Color 1 blue (0-255)")
        .addParameter("r2", ParameterType::INT, "Color 2 red (0-255)")
        .addParameter("g2", ParameterType::INT, "Color 2 green (0-255)")
        .addParameter("b2", ParameterType::INT, "Color 2 blue (0-255)")
        .addParameter("r3", ParameterType::INT, "Color 3 red (0-255)")
        .addParameter("g3", ParameterType::INT, "Color 3 green (0-255)")
        .addParameter("b3", ParameterType::INT, "Color 3 blue (0-255)")
        .addParameter("r4", ParameterType::INT, "Color 4 red (0-255)")
        .addParameter("g4", ParameterType::INT, "Color 4 green (0-255)")
        .addParameter("b4", ParameterType::INT, "Color 4 blue (0-255)")
        .addParameter("speed", ParameterType::FLOAT, "Scroll speed (0.0 = static)");
    registry.registerCommand(std::move(wres_auto_bars));

    // WRES_PALETTE_AUTO_STOP - Disable WRES palette automation
    CommandDefinition wres_auto_stop("WRES_PALETTE_AUTO_STOP",
        "Disable all WRES palette automation",
        "st_wres_palette_auto_stop", "video");
    registry.registerCommand(std::move(wres_auto_stop));

    // WRES_PALETTE_AUTO_UPDATE - Update WRES palette automation
    CommandDefinition wres_auto_update("WRES_PALETTE_AUTO_UPDATE",
        "Update WRES palette automation (call once per frame)",
        "st_wres_palette_auto_update", "video");
    wres_auto_update.addParameter("deltaTime", ParameterType::FLOAT, "Time since last frame (seconds)");
    registry.registerCommand(std::move(wres_auto_update));

    // PRES_PALETTE_AUTO_GRADIENT - Enable gradient automation on PRES
    CommandDefinition pres_auto_gradient("PRES_PALETTE_AUTO_GRADIENT",
        "Enable automatic gradient effect on PRES palette index",
        "st_pres_palette_auto_gradient", "video");
    pres_auto_gradient.addParameter("paletteIndex", ParameterType::INT, "Palette index (0-15)")
        .addParameter("startRow", ParameterType::INT, "Start row (0-719)")
        .addParameter("endRow", ParameterType::INT, "End row (0-719)")
        .addParameter("startR", ParameterType::INT, "Start red (0-255)")
        .addParameter("startG", ParameterType::INT, "Start green (0-255)")
        .addParameter("startB", ParameterType::INT, "Start blue (0-255)")
        .addParameter("endR", ParameterType::INT, "End red (0-255)")
        .addParameter("endG", ParameterType::INT, "End green (0-255)")
        .addParameter("endB", ParameterType::INT, "End blue (0-255)")
        .addParameter("speed", ParameterType::FLOAT, "Animation speed (0.0 = static)");
    registry.registerCommand(std::move(pres_auto_gradient));

    // PRES_PALETTE_AUTO_BARS - Enable bars automation on PRES
    CommandDefinition pres_auto_bars("PRES_PALETTE_AUTO_BARS",
        "Enable automatic color bars effect on PRES palette index",
        "st_pres_palette_auto_bars", "video");
    pres_auto_bars.addParameter("paletteIndex", ParameterType::INT, "Palette index (0-15)")
        .addParameter("startRow", ParameterType::INT, "Start row (0-719)")
        .addParameter("endRow", ParameterType::INT, "End row (0-719)")
        .addParameter("barHeight", ParameterType::INT, "Bar height in scanlines")
        .addParameter("numColors", ParameterType::INT, "Number of colors (1-4)")
        .addParameter("r1", ParameterType::INT, "Color 1 red (0-255)")
        .addParameter("g1", ParameterType::INT, "Color 1 green (0-255)")
        .addParameter("b1", ParameterType::INT, "Color 1 blue (0-255)")
        .addParameter("r2", ParameterType::INT, "Color 2 red (0-255)")
        .addParameter("g2", ParameterType::INT, "Color 2 green (0-255)")
        .addParameter("b2", ParameterType::INT, "Color 2 blue (0-255)")
        .addParameter("r3", ParameterType::INT, "Color 3 red (0-255)")
        .addParameter("g3", ParameterType::INT, "Color 3 green (0-255)")
        .addParameter("b3", ParameterType::INT, "Color 3 blue (0-255)")
        .addParameter("r4", ParameterType::INT, "Color 4 red (0-255)")
        .addParameter("g4", ParameterType::INT, "Color 4 green (0-255)")
        .addParameter("b4", ParameterType::INT, "Color 4 blue (0-255)")
        .addParameter("speed", ParameterType::FLOAT, "Scroll speed (0.0 = static)");
    registry.registerCommand(std::move(pres_auto_bars));

    // PRES_PALETTE_AUTO_STOP - Disable PRES palette automation
    CommandDefinition pres_auto_stop("PRES_PALETTE_AUTO_STOP",
        "Disable all PRES palette automation",
        "st_pres_palette_auto_stop", "video");
    registry.registerCommand(std::move(pres_auto_stop));

    // PRES_PALETTE_AUTO_UPDATE - Update PRES palette automation
    CommandDefinition pres_auto_update("PRES_PALETTE_AUTO_UPDATE",
        "Update PRES palette automation (call once per frame)",
        "st_pres_palette_auto_update", "video");
    pres_auto_update.addParameter("deltaTime", ParameterType::FLOAT, "Time since last frame (seconds)");
    registry.registerCommand(std::move(pres_auto_update));

    // =========================================================================
    // Unified Palette Automation Commands (V commands - mode-aware)
    // =========================================================================

    // VPALETTE_AUTO_GRADIENT - Unified gradient automation (works in any mode)
    CommandDefinition vpalette_auto_gradient("VPALETTE_AUTO_GRADIENT",
        "Enable automatic gradient effect on current video mode palette",
        "vpalette_auto_gradient", "video");
    vpalette_auto_gradient.addParameter("paletteIndex", ParameterType::INT, "Palette index (0-15)")
        .addParameter("startRow", ParameterType::INT, "Start row")
        .addParameter("endRow", ParameterType::INT, "End row")
        .addParameter("startR", ParameterType::INT, "Start red (0-255)")
        .addParameter("startG", ParameterType::INT, "Start green (0-255)")
        .addParameter("startB", ParameterType::INT, "Start blue (0-255)")
        .addParameter("endR", ParameterType::INT, "End red (0-255)")
        .addParameter("endG", ParameterType::INT, "End green (0-255)")
        .addParameter("endB", ParameterType::INT, "End blue (0-255)")
        .addParameter("speed", ParameterType::FLOAT, "Animation speed (0.0 = static)");
    registry.registerCommand(std::move(vpalette_auto_gradient));

    // VPALETTE_AUTO_BARS - Unified bars automation (works in any mode)
    CommandDefinition vpalette_auto_bars("VPALETTE_AUTO_BARS",
        "Enable automatic color bars effect on current video mode palette",
        "vpalette_auto_bars", "video");
    vpalette_auto_bars.addParameter("paletteIndex", ParameterType::INT, "Palette index (0-15)")
        .addParameter("startRow", ParameterType::INT, "Start row")
        .addParameter("endRow", ParameterType::INT, "End row")
        .addParameter("barHeight", ParameterType::INT, "Bar height in scanlines")
        .addParameter("numColors", ParameterType::INT, "Number of colors (1-4)")
        .addParameter("r1", ParameterType::INT, "Color 1 red (0-255)")
        .addParameter("g1", ParameterType::INT, "Color 1 green (0-255)")
        .addParameter("b1", ParameterType::INT, "Color 1 blue (0-255)")
        .addParameter("r2", ParameterType::INT, "Color 2 red (0-255)")
        .addParameter("g2", ParameterType::INT, "Color 2 green (0-255)")
        .addParameter("b2", ParameterType::INT, "Color 2 blue (0-255)")
        .addParameter("r3", ParameterType::INT, "Color 3 red (0-255)")
        .addParameter("g3", ParameterType::INT, "Color 3 green (0-255)")
        .addParameter("b3", ParameterType::INT, "Color 3 blue (0-255)")
        .addParameter("r4", ParameterType::INT, "Color 4 red (0-255)")
        .addParameter("g4", ParameterType::INT, "Color 4 green (0-255)")
        .addParameter("b4", ParameterType::INT, "Color 4 blue (0-255)")
        .addParameter("speed", ParameterType::FLOAT, "Scroll speed (0.0 = static)");
    registry.registerCommand(std::move(vpalette_auto_bars));

    // VPALETTE_AUTO_STOP - Unified stop automation (works in any mode)
    CommandDefinition vpalette_auto_stop("VPALETTE_AUTO_STOP",
        "Disable all palette automation in current video mode",
        "vpalette_auto_stop", "video");
    registry.registerCommand(std::move(vpalette_auto_stop));

    // VPALETTE_AUTO_UPDATE - Unified update automation (works in any mode)
    CommandDefinition vpalette_auto_update("VPALETTE_AUTO_UPDATE",
        "Update palette automation in current video mode (call once per frame)",
        "vpalette_auto_update", "video");
    vpalette_auto_update.addParameter("deltaTime", ParameterType::FLOAT, "Time since last frame (seconds)");
    registry.registerCommand(std::move(vpalette_auto_update));

    // Batch operations

    // VBEGIN_BATCH - Begin GPU batch
    CommandDefinition vbegin_batch("VBEGIN_BATCH",
                                  "Begin batching GPU drawing commands",
                                  "video_begin_batch", "video");
    registry.registerCommand(std::move(vbegin_batch));

    // VEND_BATCH - End GPU batch
    CommandDefinition vend_batch("VEND_BATCH",
                                "End batching and submit GPU commands",
                                "video_end_batch", "video");
    registry.registerCommand(std::move(vend_batch));

    // VGPUBEGIN - Begin GPU batch with auto-promotion
    CommandDefinition vgpubegin("VGPUBEGIN",
                               "Begin GPU batch - auto-promotes V commands to GPU variants",
                               "video_gpu_begin", "video");
    vgpubegin.addParameter("buffer", ParameterType::INT, "Buffer ID (0-7, default 0)", true);
    registry.registerCommand(std::move(vgpubegin));

    // VGPUEND - End GPU batch with auto-promotion
    CommandDefinition vgpuend("VGPUEND",
                             "End GPU batch and submit commands",
                             "video_gpu_end", "video");
    registry.registerCommand(std::move(vgpuend));

    // Antialiasing control

    // VENABLE_AA - Enable antialiasing
    CommandDefinition venable_aa("VENABLE_AA",
                                "Enable or disable antialiasing",
                                "video_enable_antialias", "video");
    venable_aa.addParameter("enable", ParameterType::INT, "Enable (1) or disable (0)");
    registry.registerCommand(std::move(venable_aa));

    // VSET_LINE_WIDTH - Set line width
    CommandDefinition vset_line_width("VSET_LINE_WIDTH",
                                     "Set line width for antialiased rendering",
                                     "video_set_line_width", "video");
    vset_line_width.addParameter("width", ParameterType::FLOAT, "Line width");
    registry.registerCommand(std::move(vset_line_width));
}

void SuperTerminalCommandRegistry::registerVideoModeFunctions(CommandRegistry& registry) {
    // VIDEO_MODE() - Get current video mode
    CommandDefinition video_mode("VIDEO_MODE",
                                "Get current video mode ID",
                                "video_mode_get", "video", false, ReturnType::INT);
    registry.registerFunction(std::move(video_mode));

    // VIDEO_MODE_NAME$() - Get video mode name
    CommandDefinition video_mode_name("VIDEO_MODE_NAME",
                                     "Get current video mode name",
                                     "video_mode_name", "video", false, ReturnType::STRING);
    registry.registerFunction(std::move(video_mode_name));

    // VBUFFER_GET() - Get active buffer
    CommandDefinition vbuffer_get("VBUFFER_GET",
                                 "Get current active buffer ID",
                                 "video_buffer_get", "video", false, ReturnType::INT);
    registry.registerFunction(std::move(vbuffer_get));

    // VACTIVE_BUFFER() - Get active drawing buffer
    CommandDefinition vactive_buffer("VACTIVE_BUFFER",
                                     "Get current active drawing buffer ID",
                                     "video_get_active_buffer", "video", false, ReturnType::INT);
    registry.registerFunction(std::move(vactive_buffer));

    // VDISPLAY_BUFFER() - Get display buffer
    CommandDefinition vdisplay_buffer("VDISPLAY_BUFFER",
                                      "Get current display/front buffer ID",
                                      "video_get_display_buffer", "video", false, ReturnType::INT);
    registry.registerFunction(std::move(vdisplay_buffer));

    // VIDEO_COLOR_DEPTH() - Get color depth
    CommandDefinition video_color_depth("VIDEO_COLOR_DEPTH",
                                       "Get color depth of current mode",
                                       "video_get_color_depth", "video", false, ReturnType::INT);
    registry.registerFunction(std::move(video_color_depth));

    // VIDEO_HAS_PALETTE() - Check palette support
    CommandDefinition video_has_palette("VIDEO_HAS_PALETTE",
                                       "Check if current mode uses palette",
                                       "video_has_palette", "video", false, ReturnType::INT);
    registry.registerFunction(std::move(video_has_palette));

    // VIDEO_HAS_GPU() - Check GPU support
    CommandDefinition video_has_gpu("VIDEO_HAS_GPU",
                                   "Check if current mode has GPU acceleration",
                                   "video_has_gpu", "video", false, ReturnType::INT);
    registry.registerFunction(std::move(video_has_gpu));

    // VIDEO_MAX_BUFFERS() - Get max buffers
    CommandDefinition video_max_buffers("VIDEO_MAX_BUFFERS",
                                       "Get maximum number of buffers",
                                       "video_get_max_buffers", "video", false, ReturnType::INT);
    registry.registerFunction(std::move(video_max_buffers));

    // VIDEO_MEMORY() - Get memory usage
    CommandDefinition video_memory("VIDEO_MEMORY",
                                  "Get total memory used by video buffers",
                                  "video_get_memory_usage", "video", false, ReturnType::INT);
    registry.registerFunction(std::move(video_memory));

    // VGET_LINE_WIDTH() - Get line width
    CommandDefinition vget_line_width("VGET_LINE_WIDTH",
                                     "Get current line width",
                                     "video_get_line_width", "video", false, ReturnType::FLOAT);
    registry.registerFunction(std::move(vget_line_width));

    // VIDEO_SUPPORTS_GRADIENTS() - Check gradient support
    CommandDefinition video_supports_gradients("VIDEO_SUPPORTS_GRADIENTS",
                                              "Check if current mode supports gradients",
                                              "video_supports_gradients", "video", false, ReturnType::INT);
    registry.registerFunction(std::move(video_supports_gradients));

    // VIDEO_SUPPORTS_ANTIALIAS() - Check AA support
    CommandDefinition video_supports_antialias("VIDEO_SUPPORTS_ANTIALIAS",
                                              "Check if current mode supports antialiasing",
                                              "video_supports_antialias", "video", false, ReturnType::INT);
    registry.registerFunction(std::move(video_supports_antialias));
}

} // namespace SuperTerminalCommands
} // namespace FBRunner3
