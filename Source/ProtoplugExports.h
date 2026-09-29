
#pragma once

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4190) // C function returns C-incompatible UDT.
#elif defined(__clang__)
// These headers define protoplug's extern "C" FFI export API: each function
// here *is* its own declaration (there is no separate prototype elsewhere,
// since they are never called from other C++ translation units — only from
// Lua via ffi.cdef), and several intentionally return small C++ structs by
// value across the "C" linkage boundary, which is safe for the POD types
// used here despite the ABI-portability warning.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-prototypes"
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-prototypes"
#endif

#include "exports/typedefs.h"

#include "exports/pAudioFormatReader.h"
#include "exports/pColourGradient.h"
#include "exports/pComponent.h"
#include "exports/pFillType.h"
#include "exports/pFont.h"
#include "exports/pGraphics.h"
#include "exports/pImage.h"
#include "exports/pImageFileFormat.h"
#include "exports/pLagrangeInterpolator.h"
#include "exports/pMidiBuffer.h"
#include "exports/pPath.h"


PROTO_API     bool AudioPlayHead_getCurrentPosition(pAudioPlayHead self, AudioPlayHead::CurrentPositionInfo& result)
{
    // AudioPlayHead::getCurrentPosition() is deprecated in favour of
    // getPosition(), but the Lua-facing FFI API (see plugin.lua's
    // CurrentPositionInfo cdef) is built around the legacy struct layout,
    // so translate getPosition()'s result into it here instead of relying
    // on JUCE's own (deprecated) conversion helper.
    if (self.a == nullptr)
        return false;

    const auto pos = self.a->getPosition();
    if (! pos.hasValue())
        return false;

    result.resetToDefault();

    if (const auto sig = pos->getTimeSignature())
    {
        result.timeSigNumerator   = sig->numerator;
        result.timeSigDenominator = sig->denominator;
    }

    if (const auto loop = pos->getLoopPoints())
    {
        result.ppqLoopStart = loop->ppqStart;
        result.ppqLoopEnd   = loop->ppqEnd;
    }

    if (const auto frame = pos->getFrameRate())
        result.frameRate = *frame;

    if (const auto timeInSeconds = pos->getTimeInSeconds())
        result.timeInSeconds = *timeInSeconds;

    if (const auto lastBarStartPpq = pos->getPpqPositionOfLastBarStart())
        result.ppqPositionOfLastBarStart = *lastBarStartPpq;

    if (const auto ppqPosition = pos->getPpqPosition())
        result.ppqPosition = *ppqPosition;

    if (const auto originTime = pos->getEditOriginTime())
        result.editOriginTime = *originTime;

    if (const auto bpm = pos->getBpm())
        result.bpm = *bpm;

    if (const auto timeInSamples = pos->getTimeInSamples())
        result.timeInSamples = *timeInSamples;

    result.isPlaying   = pos->getIsPlaying();
    result.isRecording = pos->getIsRecording();
    result.isLooping   = pos->getIsLooping();

    return true;
}

#ifdef _MSC_VER
#pragma warning(pop)
#elif defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

/*
the protojuce api looks somewhat consistent on the outside, but on the inside 
it's not so pretty. I didn't use any of the c++ to Lua autowrapping libraries, 
instead i rolled my own wrappers and used LuaJIT's FFI. This may or may not 
have been the easier route, either way the end product is a little faster.

In a nutshell :
1. The required C++ juce functionality is wrapped into a C API and exported by 
   protoplug. This is done by the headers in the "Exports" folder.

2. This API is then imported into Lua using LuaJIT's FFI feature.

3. The "protojuce" lua module then takes this (somewhat messy) C API and 
   re-wraps it into a nicer lua API.

JUCE's c++ classes are handled in different ways :

classes that need to be managed by JUCE :
	export constuctor/destructor (always ffi.gc)
		ColourGradient
		FillType
		Path
		RectangleList
		Image
		Font
	do not export constructor/destructor
		Component
		Graphics
classes that convert to structs and need not export anything:
	Rectangle<>
	MouseEvent
	Time
	Point<>
	MouseWheelDetails
	Colour
	AffineTransform
	PathStrokeType
	Line<>
classes that convert to something else :
	String (to const char*)
	ModifierKeys (to int)
	Justification (to int)
	RectanglePlacement (to int)
classes that are opaque pointers and only referred to :
	MouseInputSource
*/


