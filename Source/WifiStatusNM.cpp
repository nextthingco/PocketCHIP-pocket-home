#ifdef LINUX

#include "WifiStatus.h"
#include "../JuceLibraryCode/JuceHeader.h"

namespace {

// Run a command, capture stdout. Blocks -- the nmcli queries here are quick.
// ChildProcess::start(String) tokenises on whitespace and execs directly (no
// shell); none of our args contain spaces.
String runCapture(const String& command) {
  ChildProcess proc;
  if (!proc.start(command, ChildProcess::wantStdOut))
    return String::empty;
  const String out = proc.readAllProcessOutput();
  proc.waitForProcessToFinish(5000);
  return out;
}

// Fire-and-forget launch (e.g. a terminal running nmtui). JUCE's ChildProcess
// does not kill the child on destruction, so the launched process keeps running.
void launchDetached(const String& command) {
  ChildProcess proc;
  proc.start(command, 0);
}

// `nmcli -t` terse output escapes ':' inside a field as '\:'. We always put the
// SSID (the only field that can contain ':') LAST and grab the remainder, so the
// leading fixed fields parse cleanly. Returns leadingFields + 1 entries.
StringArray splitTerse(const String& line, int leadingFields) {
  StringArray out;
  String rest = line;
  for (int i = 0; i < leadingFields; ++i) {
    int sep = -1;
    for (int c = 0; c < rest.length(); ++c) {
      if (rest[c] == '\\') { ++c; continue; }      // skip escaped char
      if (rest[c] == ':') { sep = c; break; }
    }
    if (sep < 0) { out.add(rest); rest = String::empty; }
    else { out.add(rest.substring(0, sep)); rest = rest.substring(sep + 1); }
  }
  out.add(rest.replace("\\:", ":"));               // remainder = SSID
  return out;
}

} // namespace

WifiStatusNM::WifiStatusNM() : listeners() {}

WifiStatusNM::~WifiStatusNM() { stopTimer(); }

OwnedArray<WifiAccessPoint> WifiStatusNM::nearbyAccessPoints() {
  OwnedArray<WifiAccessPoint> aps;
  // SIGNAL,SECURITY then SSID last. (Connecting still happens in nmtui; this
  // list is just for display.)
  const String out = runCapture("nmcli -t -f SIGNAL,SECURITY,SSID device wifi list");
  const StringArray lines = StringArray::fromLines(out);
  StringArray seen;
  for (int i = 0; i < lines.size(); ++i) {
    if (lines[i].isEmpty()) continue;
    const StringArray f = splitTerse(lines[i], 2);     // SIGNAL, SECURITY, SSID
    const String ssid = f[2].trim();
    if (ssid.isEmpty() || seen.contains(ssid)) continue;
    seen.add(ssid);
    WifiAccessPoint* ap = new WifiAccessPoint();
    ap->ssid = ssid;
    ap->signalStrength = f[0].getIntValue();
    const String sec = f[1].trim();
    ap->requiresAuth = sec.isNotEmpty() && sec != "--";
    ap->hash = ssid;
    aps.add(ap);
  }
  return aps;
}

ScopedPointer<WifiAccessPoint> WifiStatusNM::connectedAccessPoint() const {
  if (connectedAP == nullptr)
    return nullptr;
  return ScopedPointer<WifiAccessPoint>{ new WifiAccessPoint(*connectedAP) };
}

bool WifiStatusNM::isEnabled() const { return enabled; }
bool WifiStatusNM::isConnected() const { return connected; }

void WifiStatusNM::addListener(Listener* listener) { listeners.add(listener); }
void WifiStatusNM::clearListeners() { listeners.clear(); }

void WifiStatusNM::setEnabled()  { launchDetached("nmcli radio wifi on"); }
void WifiStatusNM::setDisabled() { launchDetached("nmcli radio wifi off"); }

void WifiStatusNM::setConnectedAccessPoint(WifiAccessPoint* /*ap*/, String /*psk*/) {
  // All connection management goes through nmtui (in a terminal), per design.
  launchDetached("x-terminal-emulator -e nmtui");
}

void WifiStatusNM::setDisconnected() {
  launchDetached("x-terminal-emulator -e nmtui");
}

void WifiStatusNM::poll() {
  const bool wasEnabled = enabled;
  const bool wasConnected = connected;

  enabled = runCapture("nmcli -t radio wifi").trim() == "enabled";

  // ACTIVE,SIGNAL then SSID last; the active row (ACTIVE == "yes") is the
  // network we're on -- that drives the launcher wifi icon.
  connected = false;
  connectedAP = nullptr;
  const String out = runCapture("nmcli -t -f ACTIVE,SIGNAL,SSID device wifi");
  const StringArray lines = StringArray::fromLines(out);
  for (int i = 0; i < lines.size(); ++i) {
    if (lines[i].isEmpty()) continue;
    const StringArray f = splitTerse(lines[i], 2);     // ACTIVE, SIGNAL, SSID
    if (f[0].trim() == "yes") {
      connected = true;
      WifiAccessPoint* ap = new WifiAccessPoint();
      ap->ssid = f[2].trim();
      ap->signalStrength = f[1].getIntValue();
      ap->requiresAuth = false;
      ap->hash = ap->ssid;
      connectedAP = ap;
      break;
    }
  }

  if (enabled != wasEnabled)
    for (int i = 0; i < listeners.size(); ++i)
      enabled ? listeners[i]->handleWifiEnabled() : listeners[i]->handleWifiDisabled();
  if (connected != wasConnected)
    for (int i = 0; i < listeners.size(); ++i)
      connected ? listeners[i]->handleWifiConnected() : listeners[i]->handleWifiDisconnected();
}

void WifiStatusNM::timerCallback() { poll(); }

void WifiStatusNM::initializeStatus() {
  poll();
  startTimer(3000);   // refresh the icon every few seconds
}

#endif // LINUX
