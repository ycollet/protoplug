/*============================================================================*/
/*
  VFLib: https://github.com/vinniefalco/VFLib

  Copyright (C) 2008 by Vinnie Falco <vinnie.falco@gmail.com>

  This library contains portions of other open source products covered by
  separate licenses. Please see the corresponding source files for specific
  terms.

  VFLib is provided under the terms of The MIT License (MIT):

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
  IN THE SOFTWARE.
*/
/*============================================================================*/

// NOTE: CustomTypeface was removed in JUCE 7. This file has been rewritten to
// use Typeface::createSystemTypefaceFor() for loading embedded fonts from
// memory. FreeType-based hinting is no longer used; the system/JUCE renderer
// handles all glyph rendering.

#include "vf_FreeTypeFaces.h"

namespace {

class FreeTypeFacesImplementation : public DeletedAtShutdown
{
private:
    struct MemoryFace
    {
        String   faceName;
        const void* faceFileData;
        int      faceFileBytes;
    };

    Array<MemoryFace> m_faces;

public:
    FreeTypeFacesImplementation() {}

    ~FreeTypeFacesImplementation()
    {
        clearSingletonInstance();
    }

    void addFaceFromMemory (float /*minHintedHeight*/,
                            float /*maxHintedHeight*/,
                            bool  /*useFreeTypeRendering*/,
                            const void* faceFileData,
                            int         faceFileBytes,
                            bool        /*appendStyleToFaceName*/)
    {
        // Load a temporary typeface just to extract the family name.
        Typeface::Ptr tf = Typeface::createSystemTypefaceFor (faceFileData, (size_t) faceFileBytes);
        MemoryFace mf;
        mf.faceName      = tf != nullptr ? tf->getName() : String{};
        mf.faceFileData  = faceFileData;
        mf.faceFileBytes = faceFileBytes;
        m_faces.add (mf);
    }

    Typeface::Ptr createTypefaceForFont (const Font& font)
    {
        for (int i = 0; i < m_faces.size(); ++i)
        {
            const MemoryFace& mf = m_faces[i];
            if (mf.faceName.isEmpty() || mf.faceName == font.getTypefaceName())
            {
                Typeface::Ptr tf = Typeface::createSystemTypefaceFor (mf.faceFileData,
                                                                       (size_t) mf.faceFileBytes);
                if (tf != nullptr)
                    return tf;
            }
        }
        return {};
    }

    juce_DeclareSingleton (FreeTypeFacesImplementation, false)
};

juce_ImplementSingleton (FreeTypeFacesImplementation)

} // anonymous namespace

//------------------------------------------------------------------------------

void FreeTypeFaces::addFaceFromMemory (float minHintedHeight,
                                       float maxHintedHeight,
                                       bool useFreeTypeRendering,
                                       const void* faceFileData,
                                       int faceFileBytes,
                                       bool appendStyleToFaceName)
{
    FreeTypeFacesImplementation::getInstance()->addFaceFromMemory (
        minHintedHeight, maxHintedHeight, useFreeTypeRendering,
        faceFileData, faceFileBytes, appendStyleToFaceName);
}

Typeface::Ptr FreeTypeFaces::createTypefaceForFont (const Font& font)
{
    return FreeTypeFacesImplementation::getInstance()->createTypefaceForFont (font);
}
