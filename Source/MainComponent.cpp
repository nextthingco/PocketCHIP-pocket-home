#include "MainComponent.h"
#include "PokeLookAndFeel.h"
#include "LauncherComponent.h"
#include "Utils.h"

MainContentComponent::MainContentComponent(const var &configJson) {
  lookAndFeel = new PokeLookAndFeel();
  setLookAndFeel(lookAndFeel);

  LookAndFeel::setDefaultLookAndFeel(lookAndFeel);

  pageStack = new PageStackComponent();
  addAndMakeVisible(pageStack);

  launcher = new LauncherComponent(configJson);
  pageStack->pushPage(launcher, PageStackComponent::kTransitionNone);
  
  buttonPopup = new ImageComponent("Focus Button Popup");
  buttonPopup->setInterceptsMouseClicks(false, false);
  addChildComponent(buttonPopup);

  // Full-screen launch spinner, added last so it sits on top of every page
  // (apps menu, settings, ...). Shown by showLaunchSpinner() from the apps page
  // and the settings-page wifi button; hidden when the launched window takes
  // focus (handleMainWindowInactive) or after the timer's timeout.
  launchSpinnerTimer.mainComponent = this;
  Array<String> spinnerImgPaths{"wait0.png","wait1.png","wait2.png","wait3.png","wait4.png","wait5.png","wait6.png","wait7.png"};
  for (auto& path : spinnerImgPaths) {
    launchSpinnerImages.add(createImageFromFile(assetFile(path)));
  }
  launchSpinner = new ImageComponent();
  launchSpinner->setImage(launchSpinnerImages[0]);
  launchSpinner->setInterceptsMouseClicks(false, false);
  addChildComponent(launchSpinner);

  setSize(480, 272);
}

MainContentComponent::~MainContentComponent() {}

void MainContentComponent::paint(Graphics &g) {
  g.fillAll(Colours::white);
}

void MainContentComponent::resized() {
  auto bounds = getLocalBounds();
  pageStack->setBounds(bounds);
  launchSpinner->setBounds(bounds);
}

void MainContentComponent::showLaunchSpinner() {
  DBG("Show launch spinner");
  launchSpinner->setVisible(true);
  launchSpinnerTimer.startTimer(500);
}

void MainContentComponent::hideLaunchSpinner() {
  DBG("Hide launch spinner");
  launchSpinnerTimer.stopTimer();
  launchSpinner->setVisible(false);
}

void MainContentComponent::handleMainWindowInactive() {
  hideLaunchSpinner();
}

void LaunchSpinnerTimer::timerCallback() {
  if (mainComponent) {
    auto lsp = mainComponent->launchSpinner.get();
    const auto& lspImg = mainComponent->launchSpinnerImages;

    // change image
    i++;
    if (i == lspImg.size()) { i = 0; }
    lsp->setImage(lspImg[i]);

    // check timeout
    t += getTimerInterval();
    if (t > timeout) {
      t = 0;
      lsp->setVisible(false);
      stopTimer();
    }
  }
}
