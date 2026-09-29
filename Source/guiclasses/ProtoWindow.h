#pragma once

#include <JuceHeader.h>
#include "../LuaCodeTokeniser.h"
#include "HintedFeel.h"
#include "DarkSplitter.h"
#include "ParameterPanel.h"
#include "CustomGuiPanel.h"
#include "LuaEditor.h"
#include "Dockable.h"
#include "BottomPane.h"
#include <map>

#define MSG_POPOUT 1
#define MSG_ALWAYSONTOP 2

class LuaProtoplugJuceAudioProcessorEditor;
class ProtoWindow;

class ProtoCmd : public juce::ApplicationCommandManager
{
public:
    ProtoCmd (ProtoWindow* _pw)
    { pw = _pw; }
    juce::ApplicationCommandTarget* getFirstCommandTarget (juce::CommandID commandID);
    ProtoWindow* pw;
};

class ProtoWindow : public juce::Component,
                    public juce::MenuBarModel,
                    public juce::Timer,
                    public juce::ApplicationCommandTarget,
                    public ProtoTabButton::Listener,
                    public juce::CodeDocument::Listener
{
    friend class BottomPane;
public:
    ProtoWindow (juce::Component* parent, LuaProtoplugJuceAudioProcessor* ownerFilter);
    ~ProtoWindow() override;

    void paint (juce::Graphics& g) override;
    void resized() override;
    juce::PopupMenu getMenuForIndex (int menuIndex, const juce::String& /*menuName*/) override;
    void menuItemSelected (int menuItemID, int /*topLevelMenuIndex*/) override;
    void timerCallback() override;
    juce::StringArray getMenuBarNames() override;
    juce::ApplicationCommandTarget* getNextCommandTarget() override { return nullptr; }
    void getAllCommands (juce::Array<juce::CommandID>& commands) override;
    void getCommandInfo (juce::CommandID commandID, juce::ApplicationCommandInfo& result) override;
    bool perform (const juce::ApplicationCommandTarget::InvocationInfo& info) override;
    void tabButtonClicked (ProtoTabButton* b) override;
    void tabButtonDoubleClicked (ProtoTabButton* b) override;
    void codeDocumentTextInserted (const juce::String& newText, int insertIndex) override;
    void codeDocumentTextDeleted (int startIndex, int endIndex) override;
    void initProtoplugDir();

    void saveCode();
    void compile();
    void setActivePanel (int p);
    void setPoppedOut (bool popped);
    void takeFocus();
    void readPrefs();
    void readTheme (juce::File theme);
    bool dirty;
    bool msg_UpdateLog, msg_ParamsChanged, msg_TakeFocus; // notifications for the timer

    enum CommandIDs
    {
        cmdCompile      = 0x10,
        cmdStackDump    = 0x11,
        cmdLiveMode     = 0x12,
        cmdFindSelected = 0x25,
        cmdFindNext     = 0x26,
        cmdFindPrev     = 0x27,
        cmdShow0        = 0x30,
        cmdShow1        = 0x31,
        cmdShow2        = 0x32,
        cmdShowNext     = 0x40,
        cmdShowPrev     = 0x41,
        cmdOpen         = 0x50,
        cmdSaveAs       = 0x51,
        cmdOpenProto    = 0x52,
        cmdPopout       = 0x60,
        cmdAlwaysOnTop  = 0x61,
        cmdWebsite      = 0x70,
        cmdAPI          = 0x71,
        cmdAbout        = 0x72,
        cmdUndo         = juce::StandardApplicationCommandIDs::undo,
        cmdRedo         = juce::StandardApplicationCommandIDs::redo,
        cmdCut          = juce::StandardApplicationCommandIDs::cut,
        cmdCopy         = juce::StandardApplicationCommandIDs::copy,
        cmdPaste        = juce::StandardApplicationCommandIDs::paste,
    };

private:
    void findNext (bool direction, bool wrap = false);
    void addFolderToMenu (juce::File folder, juce::PopupMenu& menu, juce::String filter, int& mapCounter);

    HintedFeel newFeel;
    ProtoCmd commMgr;
    LuaProtoplugJuceAudioProcessor* processor;
    juce::ResizableCornerComponent resizer;
    juce::ComponentBoundsConstrainer resizeLimits;
    juce::StretchableLayoutManager horizontalLayout;
    std::unique_ptr<DarkSplitter> horizontalDividerBar;
    juce::MenuBarComponent menubar;
    std::map<int, juce::File> menuFiles;
    juce::CodeDocument doc;
    ProtoLuaTokeniser tok;
    juce::String themeFolder;
    LuaEditor editor;
    BottomPane bottomPane;
    ParameterPanel paramPanel;
    CustomGuiPanel guiPanel;
    int activePanel;
    juce::Component* panels[3];
    juce::Component* activePanelComponent;
    juce::Component* vstPanel;
    juce::Component* popoutWindow;
    Dockable paramDock, guiDock;
    juce::String searchTerm;
    ProtoTabButton tab1, tab2, tab3;
    int hackTimer;
};
