#pragma once

#include <JuceHeader.h>
#include "../LuaLink.h"

class CustomGuiPanel	:	public Component, public KeyListener
{
public:
	CustomGuiPanel (LuaLink *_luli)
	{
		luli = _luli;
		luli->customGui = this;
	}
	void paint (Graphics& g) override;
	void resized () override;

    void mouseMove (const MouseEvent& event) override;
    void mouseEnter (const MouseEvent& event) override;
    void mouseExit (const MouseEvent& event) override;
    void mouseDown (const MouseEvent& event) override;
    void mouseDrag (const MouseEvent& event) override;
    void mouseUp (const MouseEvent& event) override;
    void mouseDoubleClick (const MouseEvent& event) override;
    void mouseWheelMove (const MouseEvent& event, const MouseWheelDetails& wheel) override;

	// KeyListener's keyPressed/keyStateChanged (2-arg) intentionally have a
	// different signature than Component's own (1-arg) virtuals of the same
	// name; "using" keeps the Component overloads visible instead of merely
	// hiding them, which is what -Woverloaded-virtual is warning about.
	using Component::keyPressed;
	using Component::keyStateChanged;
	bool keyPressed (const KeyPress &key, Component *originatingComponent) override;
	bool keyStateChanged (bool isKeyDown, Component *originatingComponent) override;
	void modifierKeysChanged (const ModifierKeys &modifiers) override;
	void focusGained (FocusChangeType cause) override;
	void focusLost (FocusChangeType cause) override;

private:
	LuaLink *luli;
};
