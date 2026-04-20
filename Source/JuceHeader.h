/*
  Protoplug compatibility header.

  This file replaces the Projucer-generated JuceHeader.h.  It includes the
  JUCE modules that protoplug needs and pulls the juce:: namespace into the
  global scope so that the existing source code (written against the old
  Projucer-generated JuceHeader.h) compiles without requiring juce:: prefixes.

  NOTE: Do not use this pattern in new code; prefer explicit juce:: qualifiers.
*/

#pragma once

// juce_audio_utils is a high-level module that transitively depends on
// juce_audio_basics, juce_audio_devices, juce_audio_formats,
// juce_audio_processors, juce_data_structures, juce_events, juce_graphics,
// juce_gui_basics, juce_gui_extra, and juce_core.
#include <juce_audio_utils/juce_audio_utils.h>

// Pull the juce:: namespace into the global scope, matching the behaviour
// of the old Projucer-generated JuceHeader.h.
using namespace juce;
