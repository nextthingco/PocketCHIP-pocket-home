#pragma once

#ifdef LINUX

#include "Utils.h"
#include "WifiStatus.h"
#include "../JuceLibraryCode/JuceHeader.h"

/*
 * nmcli/nmtui-backed wifi status.
 *
 * The original implementation linked libnm-glib, which was removed from Debian
 * (trixie). Rather than port to the modern libnm C API, this keeps Network
 * Manager integration to the minimum pocket-home actually needs: read-only
 * status via `nmcli` (drives the launcher wifi icon, polled on a timer), while
 * all configuration -- enabling the radio, scanning, entering PSKs, connecting
 * -- is delegated to `nmtui` launched in a terminal. The class name is kept so
 * Main.h's LINUX toggle and the Projucer Makefile object list are unchanged.
 */
class WifiStatusNM : public WifiStatus, private Timer {
public:
  WifiStatusNM();
  ~WifiStatusNM() override;

  OwnedArray<WifiAccessPoint> nearbyAccessPoints() override;
  ScopedPointer<WifiAccessPoint> connectedAccessPoint() const override;
  bool isEnabled() const override;
  bool isConnected() const override;

  void addListener(Listener* listener) override;
  void clearListeners() override;

  void setEnabled() override;
  void setDisabled() override;
  void setConnectedAccessPoint(WifiAccessPoint *ap, String psk = String::empty) override;
  void setDisconnected() override;

  void initializeStatus() override;

private:
  void timerCallback() override;   // poll nmcli, notify listeners on change
  void poll();                     // refresh enabled/connected/connectedAP

  Array<Listener*> listeners;
  ScopedPointer<WifiAccessPoint> connectedAP = nullptr;
  bool enabled = false;
  bool connected = false;
};

#endif // LINUX
