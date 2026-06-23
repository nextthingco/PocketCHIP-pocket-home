#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

#include "LauncherComponent.h"
#include "PageStackComponent.h"

class MainContentComponent;

// Full-screen "launching..." spinner overlay (wait0-7.png), shown when an app or
// the settings-page wifi button launches something. Owned by MainContentComponent
// so it renders over any page (apps menu, settings, etc).
class LaunchSpinnerTimer : public Timer {
public:
    LaunchSpinnerTimer() {};
    void timerCallback();

    MainContentComponent* mainComponent = nullptr;

    int i = 0;
    int t = 0;
    int timeout = 30 * 1000;
};

class MainContentComponent : public Component {
public:
  ScopedPointer<LauncherComponent> launcher;
  ScopedPointer<LookAndFeel> lookAndFeel;
  ScopedPointer<ImageComponent> buttonPopup;

  ScopedPointer<PageStackComponent> pageStack;

  ScopedPointer<ImageComponent> launchSpinner;
  Array<Image> launchSpinnerImages;
  LaunchSpinnerTimer launchSpinnerTimer;

  MainContentComponent(const var &configJson);
  ~MainContentComponent();

  void paint(Graphics &) override;
  void resized() override;

  void showLaunchSpinner();
  void hideLaunchSpinner();

  void handleMainWindowInactive();

private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainContentComponent)
};
