#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

#include "LauncherBarComponent.h"
#include "PageStackComponent.h"
#include "BatteryMonitor.h"

class LauncherComponent;
class AppsPageComponent;

class BatteryIconTimer : public Timer {
public:
    BatteryIconTimer() {};
    void timerCallback();
    LauncherComponent* launcherComponent;
};

class WifiIconTimer : public Timer {
public:
  WifiIconTimer() {};
  void timerCallback();
  LauncherComponent* launcherComponent;
};

class LauncherComponent : public Component, private Button::Listener {
public:
    BatteryMonitor batteryMonitor;
    ScopedPointer<LauncherBarComponent> botButtons;
    ScopedPointer<LauncherBarComponent> topButtons;

    Array<Image> batteryIconImages;
    Array<Image> batteryIconChargingImages;
    Array<Image> wifiIconImages;

    BatteryIconTimer batteryIconTimer;
    WifiIconTimer wifiIconTimer;
    Component* defaultPage;
  
    // FIXME: we have no need for the pages/pagesByName if we're using scoped pointers for each page.
    // All these variables do is add an extra string key the compiler can't see through.
    OwnedArray<Component> pages;
    ScopedPointer<PageStackComponent> pageStack;
    HashMap<String, Component *> pagesByName;
    
    bool resize = false;
    
    StretchableLayoutManager categoryButtonLayout;
    
    LauncherComponent(const var &configJson);
    ~LauncherComponent();
    
    void paint(Graphics &) override;
    void resized() override;

private:
    Colour bgColor;
    Image bgImage;
  
    void buttonClicked(Button *) override;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LauncherComponent)
};