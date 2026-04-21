cmake_minimum_required(VERSION 3.22)

project(protoplug VERSION 1.4.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ============================================================
# Build-format options
# ------------------------------------------------------------
# Enable additional plugin formats at configure time:
#   cmake -B build -DPLUGIN_USE_LV2=ON
#   cmake -B build -DPLUGIN_USE_CLAP=ON
# ============================================================
option(PLUGIN_USE_LV2  "Build LV2 plugin format"  OFF)
option(PLUGIN_USE_CLAP "Build CLAP plugin format" OFF)

# ============================================================
# Find system Lua 5.4
# ------------------------------------------------------------
# On Fedora: sudo dnf install lua5.4-devel   (or just "lua-devel"
#            if Lua 5.4 is the default version)
# On Debian/Ubuntu: sudo apt install liblua5.4-dev
# On macOS (Homebrew): brew install lua@5.4
#   NOTE: lua@5.4 is keg-only; pass explicit paths at configure time:
#     cmake -B build \
#       -DLUA_INCLUDE_DIR=$(brew --prefix lua@5.4)/include/lua5.4 \
#       -DLUA_LIBRARY=$(brew --prefix lua@5.4)/lib/liblua.a \
#       -DLUA_LIBRARIES=$(brew --prefix lua@5.4)/lib/liblua.a
#   If a newer Lua (e.g. 5.5) is also installed, cmake's find_package
#   may pick it up instead; the explicit -D flags above prevent that.
# ============================================================
find_package(Lua 5.4 REQUIRED)

# ============================================================
# Fetch JUCE via FetchContent
# ============================================================
include(FetchContent)

FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.4          # update to a newer tag as needed
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(JUCE)

# ============================================================
# Optionally fetch clap-juce-extensions (CLAP support)
# ============================================================
if(PLUGIN_USE_CLAP)
    FetchContent_Declare(
        clap-juce-extensions
        GIT_REPOSITORY https://github.com/free-audio/clap-juce-extensions.git
        GIT_TAG        main
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(clap-juce-extensions)
endif()

# ============================================================
# Plugin format list — always VST3 + AU; LV2/CLAP are opt-in
# Note: JUCE silently drops AU on non-Apple platforms.
# CLAP is handled separately via clap_juce_extensions_plugin().
# ============================================================
set(PROTOPLUG_FORMATS VST3 AU)
if(PLUGIN_USE_LV2)
    list(APPEND PROTOPLUG_FORMATS LV2)
endif()

# ============================================================
# Shared source list (used by both plugins)
# ============================================================
set(PROTOPLUG_SOURCES
    Source/LuaCodeTokeniser.cpp
    Source/LuaCodeTokeniser.h
    Source/LuaLink.cpp
    Source/LuaLink.h
    Source/LuaState.cpp
    Source/LuaState.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/ProtoplugDir.cpp
    Source/ProtoplugDir.h
    Source/ProtoplugExports.h
    Source/guiclasses/AboutBox.h
    Source/guiclasses/BottomPane.cpp
    Source/guiclasses/BottomPane.h
    Source/guiclasses/CustomGuiPanel.cpp
    Source/guiclasses/CustomGuiPanel.h
    Source/guiclasses/DarkSplitter.h
    Source/guiclasses/Dockable.cpp
    Source/guiclasses/Dockable.h
    Source/guiclasses/HintedFeel.cpp
    Source/guiclasses/HintedFeel.h
    Source/guiclasses/LuaEditor.h
    Source/guiclasses/ParameterPanel.h
    Source/guiclasses/ProtoPopout.h
    Source/guiclasses/ProtoTabButton.h
    Source/guiclasses/ProtoWindow.cpp
    Source/guiclasses/ProtoWindow.h
    Source/vflib/BinaryDejaVu.cpp
    Source/vflib/BinaryDejaVu.h
    Source/vflib/BinarySourceCodePro.cpp
    Source/vflib/BinarySourceCodePro.h
    Source/vflib/FreeTypeAmalgam.c
    Source/vflib/FreeTypeAmalgam.h
    Source/vflib/vf_FreeTypeFaces.cpp
    Source/vflib/vf_FreeTypeFaces.h
)

# FreeTypeAmalgam.c is plain C — compile it as C, not as C++.
set_source_files_properties(
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/vflib/FreeTypeAmalgam.c
    PROPERTIES LANGUAGE C
)

# ============================================================
# Helper function — applies common settings to both plugins
# ============================================================
function(configure_protoplug_plugin target is_synth)

    target_compile_features(${target} PRIVATE cxx_std_17)

    # Source/ is added so that:
    #   #include <JuceHeader.h>   finds Source/JuceHeader.h
    #   #include "JuceHeader.h"   (from Source/exports/) also finds it
    # Source/exports/ is added for the Lua export headers.
    target_include_directories(${target}
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/Source
            ${CMAKE_CURRENT_SOURCE_DIR}/Source/exports
    )

    target_compile_definitions(${target}
        PUBLIC
            # Allow the synchronous FileChooser / modal-loop API used in the editor.
            JUCE_MODAL_LOOPS_PERMITTED=1
            # Suppress the JUCE splash screen (requires a commercial licence or
            # an open-source project — adjust as appropriate).
            JUCE_DISPLAY_SPLASH_SCREEN=0
            # Disable unused JUCE features to speed up the build.
            JUCE_WEB_BROWSER=0
            JUCE_USE_CURL=0
        PRIVATE
            # _PROTOGEN=1 distinguishes the instrument plugin from the effect.
            $<$<BOOL:${is_synth}>:_PROTOGEN=1>
    )

    # Lua 5.4 — found by find_package(Lua 5.4 REQUIRED) above.
    target_include_directories(${target} PRIVATE ${LUA_INCLUDE_DIR})
    target_link_libraries(${target} PRIVATE ${LUA_LIBRARIES})

    # juce_audio_utils transitively pulls in juce_audio_basics,
    # juce_audio_devices, juce_audio_formats, juce_audio_processors,
    # juce_data_structures, juce_events, juce_graphics, juce_gui_basics,
    # juce_gui_extra, and juce_core.
    target_link_libraries(${target}
        PRIVATE
            juce::juce_audio_utils
        PUBLIC
            juce::juce_recommended_config_flags
            juce::juce_recommended_warning_flags
    )

endfunction()

# ============================================================
# Lua Protoplug Fx  (audio effect plugin)
# ============================================================
juce_add_plugin(protoplug_fx
    COMPANY_NAME                "osar.fr"
    PLUGIN_MANUFACTURER_CODE    "_PAC"
    PLUGIN_CODE                 "ppgf"
    FORMATS                     ${PROTOPLUG_FORMATS}
    PRODUCT_NAME                "Lua_Protoplug_Fx"
    DESCRIPTION                 "Lua Protoplug Effect"
    IS_SYNTH                    FALSE
    NEEDS_MIDI_INPUT            TRUE
    NEEDS_MIDI_OUTPUT           TRUE
    IS_MIDI_EFFECT              FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    BUNDLE_ID                   "com.pac.protoplugfx"
    VST3_CATEGORIES             "Fx"
    LV2_URI                     "https://osar.fr/protoplug/fx"
)

target_sources(protoplug_fx PRIVATE ${PROTOPLUG_SOURCES})
configure_protoplug_plugin(protoplug_fx FALSE)

if(PLUGIN_USE_CLAP)
    clap_juce_extensions_plugin(TARGET protoplug_fx
        CLAP_ID       "fr.osar.protoplugfx"
        CLAP_FEATURES audio-effect)
endif()

# ============================================================
# Lua Protoplug Gen  (instrument / synthesiser plugin)
# ============================================================
juce_add_plugin(protoplug_gen
    COMPANY_NAME                "osar.fr"
    PLUGIN_MANUFACTURER_CODE    "_PAC"
    PLUGIN_CODE                 "ppgg"
    FORMATS                     ${PROTOPLUG_FORMATS}
    PRODUCT_NAME                "Lua_Protoplug_Gen"
    DESCRIPTION                 "Lua Protoplug Instrument"
    IS_SYNTH                    TRUE
    NEEDS_MIDI_INPUT            TRUE
    NEEDS_MIDI_OUTPUT           TRUE
    IS_MIDI_EFFECT              FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    BUNDLE_ID                   "com.pac.protopluggen"
    VST3_CATEGORIES             "Instrument|Synth"
    LV2_URI                     "https://osar.fr/protoplug/gen"
)

target_sources(protoplug_gen PRIVATE ${PROTOPLUG_SOURCES})
configure_protoplug_plugin(protoplug_gen TRUE)

if(PLUGIN_USE_CLAP)
    clap_juce_extensions_plugin(TARGET protoplug_gen
        CLAP_ID       "fr.osar.protopluggen"
        CLAP_FEATURES instrument synthesizer)
endif()
