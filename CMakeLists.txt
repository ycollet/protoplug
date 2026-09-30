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
# Linux system-wide ProtoplugFiles install directory
# ------------------------------------------------------------
# On Linux, ProtoplugDir.cpp looks for the scripts/examples/docs directory
# at this fixed path (see the packaging recipe / install step that copies
# ProtoplugFiles/ there). Override at configure time if your distro/package
# uses a different location:
#   cmake -B build -DPROTOPLUG_LINUX_SHARE_DIR=/usr/local/share/protoplug
# ============================================================
set(PROTOPLUG_LINUX_SHARE_DIR "/usr/share/protoplug" CACHE STRING
    "Path to the system-wide ProtoplugFiles directory used on Linux")

# ============================================================
# Find LuaJIT
# ------------------------------------------------------------
# protoplug's scripting layer (ProtoplugFiles/include/core/*.lua) relies on
# LuaJIT's `ffi` library for fast buffer/struct access from Lua scripts, and
# on LuaJIT's JIT compiler for realtime DSP performance — plain PUC-Lua
# cannot run these scripts (no `ffi` library) and would in any case be much
# slower for numeric code, so LuaJIT is required, not just "a" Lua 5.1.
#
# On Fedora: sudo dnf install luajit-devel
# On Debian/Ubuntu: sudo apt install libluajit-5.1-dev
# On macOS (Homebrew): brew install luajit
#
# LuaJIT is located via its pkg-config file (module name "luajit"), which is
# installed by all of the packages above.
# ============================================================
find_package(PkgConfig REQUIRED)
pkg_check_modules(LuaJIT REQUIRED IMPORTED_TARGET luajit)

# ============================================================
# Find FreeType (used only for the "Freetype x.y.z" version string shown
# in the About box; JUCE's own font engine links the system FreeType on
# Linux independently of this). Optional: on macOS/Windows, JUCE's font
# handling doesn't rely on FreeType (CoreText/DirectWrite instead), so
# freetype2 may not be present there — the About box falls back gracefully
# (see Source/guiclasses/AboutBox.h).
# ============================================================
pkg_check_modules(Freetype2 QUIET IMPORTED_TARGET freetype2)

# ============================================================
# Fetch JUCE via FetchContent
# ============================================================
include(FetchContent)

FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        9.0.3          # update to a newer tag as needed
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
    Source/vflib/vf_FreeTypeFaces.cpp
    Source/vflib/vf_FreeTypeFaces.h
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
            # This build never produces a VST2 binary, so there is no VST2
            # automation compatibility to preserve; disable VST3's "replace
            # VST2" parameter-ID mode (JUCE 9 otherwise refuses to build).
            JUCE_VST3_CAN_REPLACE_VST2=0
        PRIVATE
            # _PROTOGEN=1 distinguishes the instrument plugin from the effect.
            $<$<BOOL:${is_synth}>:_PROTOGEN=1>
            # System-wide ProtoplugFiles location used by ProtoplugDir.cpp on Linux.
            PROTOPLUG_LINUX_SHARE_DIR="${PROTOPLUG_LINUX_SHARE_DIR}"
    )

    # LuaJIT — found by pkg_check_modules(LuaJIT ...) above.
    target_link_libraries(${target} PRIVATE PkgConfig::LuaJIT)

    # Real system FreeType — used only for the "Freetype x.y.z" string in
    # the About box (see AboutBox.h). Not present on all platforms (JUCE's
    # own font engine doesn't need it outside Linux), so link it only when
    # pkg-config found it above.
    if(TARGET PkgConfig::Freetype2)
        target_link_libraries(${target} PRIVATE PkgConfig::Freetype2)
        target_compile_definitions(${target} PRIVATE PROTOPLUG_HAS_FREETYPE=1)
    endif()

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
    LV2URI                      "https://osar.fr/protoplug/fx"
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
    LV2URI                      "https://osar.fr/protoplug/gen"
)

target_sources(protoplug_gen PRIVATE ${PROTOPLUG_SOURCES})
configure_protoplug_plugin(protoplug_gen TRUE)

if(PLUGIN_USE_CLAP)
    clap_juce_extensions_plugin(TARGET protoplug_gen
        CLAP_ID       "fr.osar.protopluggen"
        CLAP_FEATURES instrument synthesizer)
endif()
