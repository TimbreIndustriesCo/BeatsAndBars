/*
  Eurorack Beat Counter
  ----------------------
  Tracks 4 beats and 8 bars for live performance reference.

  Inputs:
    CLOCK (In_Advance_Beat) - A0 - rising edge advances the beat/bar counter
    START (In_First_Beat)   - A1 - rising edge resets counter to Beat 1 / Bar 1

  Outputs:
    LED1-LED4  (A3, A4, D2, D3)  - Out_Beat_1 .. Out_Beat_4
    LED5-LED12 (D4-D10, A2) - Out_Bar_1  .. Out_Bar_8

  Behaviour:
    - Exactly one beat LED and one bar LED are lit at all times.
    - On power-up, on a START pulse, or after 3000ms with no CLOCK rising
      edge, the counter resets to Beat 1 / Bar 1. The idle timer is driven
      only by CLOCK activity, not START.
    - Reaching Beat 4 rolls the bar forward and resets the beat to 1.
      Reaching Beat 4 while on Bar 8 wraps both back to Beat 1 / Bar 1.
    - Any reset (power-on, START, or idle timeout) arms an "awaiting
      confirm" flag. The next CLOCK rising edge after a reset is absorbed
      to confirm Beat 1 rather than advancing to Beat 2 - this is what
      keeps a START pulse (or the very first clock after power-on) from
      pushing the count a beat ahead of the actual modular system.
    - Some clock sources output their reset trigger slightly AFTER the
      CLOCK pulse it belongs to, rather than simultaneously (this shows
      up with e.g. microcontroller-based sequencers whose main loop is
      also busy redrawing a display). If a CLOCK edge was just accepted
      immediately before a START reset (within RESET_SYNC_WINDOW_MS), that
      edge is treated as the Beat 1 confirmation, and the confirm flag is
      NOT re-armed - otherwise the next real beat would be wrongly
      absorbed too, and the counter would lag by a full extra beat.
    - CLOCK/START are conditioned in hardware (series resistor, zener
      clamp, Schottky diode, pulldown), so digitalRead() is used directly.
      A small software debounce is kept as a defensive measure.


*/

// ---- Pin definitions ----
const uint8_t PIN_CLOCK = A0;   // In_Advance_Beat
const uint8_t PIN_START = A1;   // In_First_Beat

const uint8_t BEAT_PINS[4] = {A3, A4, 2, 3};                  // Out_Beat_1..4
const uint8_t BAR_PINS[8]  = {4, 5, 6, 7, 8, 9, 10, A2};      // Out_Bar_1..8

// ---- Timing ----
const unsigned long IDLE_RESET_MS = 3000;  // reset if no CLOCK edge in this long
const unsigned long DEBOUNCE_MS   = 5;     // minimum time between accepted edges
const unsigned long RESET_SYNC_WINDOW_MS = 150; // how "late" a START can arrive after
                                                 // its CLOCK edge and still count as
                                                 // the same beat, not a new one. Sized
                                                 // for a manually-tapped reset trigger,
                                                 // not just processing jitter - keep this
                                                 // under ~half your fastest quarter-note
                                                 // period so it can't eat a real next beat.

// ---- State ----
uint8_t currentBeat = 1;   // 1..4
uint8_t currentBar  = 1;   // 1..8

bool awaitingConfirmBeat = true; // true right after any reset; absorbs next CLOCK edge

bool lastClockState = LOW;
bool lastStartState = LOW;

unsigned long lastClockEdgeTime = 0;      // last accepted CLOCK edge (debounce)
unsigned long lastStartEdgeTime = 0;      // last accepted START edge (debounce)
unsigned long lastClockActivityTime = 0;  // last accepted CLOCK edge, used for idle timeout

void setup() {
  pinMode(PIN_CLOCK, INPUT);
  pinMode(PIN_START, INPUT);

  for (uint8_t i = 0; i < 4; i++) pinMode(BEAT_PINS[i], OUTPUT);
  for (uint8_t i = 0; i < 8; i++) pinMode(BAR_PINS[i], OUTPUT);

  lastClockActivityTime = millis(); // don't idle-timeout immediately on boot

  resetCounter(true); // power-on reset -> Beat 1 / Bar 1, awaiting confirm
  updateLeds();
}

void loop() {
  unsigned long now = millis();

  bool clockState = digitalRead(PIN_CLOCK);
  bool startState = digitalRead(PIN_START);

  // --- START: rising edge triggers an immediate reset ---
  if (startState == HIGH && lastStartState == LOW && (now - lastStartEdgeTime) > DEBOUNCE_MS) {
    lastStartEdgeTime = now;

    // If a CLOCK edge was accepted just before this START, that edge
    // already stands in for Beat 1 - don't also absorb the next one.
    bool clockJustHappened = (now - lastClockEdgeTime) < RESET_SYNC_WINDOW_MS;
    resetCounter(!clockJustHappened);
    updateLeds();
  }
  lastStartState = startState;

  // --- CLOCK: rising edge advances the beat/bar counter ---
  if (clockState == HIGH && lastClockState == LOW && (now - lastClockEdgeTime) > DEBOUNCE_MS) {
    lastClockEdgeTime = now;
    lastClockActivityTime = now; // idle timer is driven only by CLOCK activity

    if (awaitingConfirmBeat) {
      // This edge confirms Beat 1 (from a reset) rather than advancing it.
      awaitingConfirmBeat = false;
    } else {
      advanceBeat();
      updateLeds();
    }
  }
  lastClockState = clockState;

  // --- Idle timeout: no CLOCK edge for IDLE_RESET_MS -> reset ---
  if ((now - lastClockActivityTime) > IDLE_RESET_MS) {
    resetCounter(true);
    updateLeds();
    lastClockActivityTime = now; // avoid re-firing every loop while still idle
  }
}

// Advance the beat counter by one CLOCK pulse, handling the beat/bar
// rollover rules. The Beat4+Bar8 wraparound is checked before the
// general "Beat 4 advances the bar" rule.
void advanceBeat() {
  if (currentBeat == 4 && currentBar == 8) {
    currentBeat = 1;
    currentBar = 1;
  } else if (currentBeat == 4) {
    currentBeat = 1;
    currentBar++;
  } else {
    currentBeat++;
  }
}

// Reset to Beat 1 / Bar 1. armConfirm decides whether the "absorb next
// CLOCK edge" flag gets (re-)armed - see the RESET_SYNC_WINDOW_MS note
// above for when it's skipped. Called on power-up, on a START pulse,
// and on the 3s CLOCK idle timeout.
void resetCounter(bool armConfirm) {
  currentBeat = 1;
  currentBar = 1;
  if (armConfirm) {
    awaitingConfirmBeat = true;
  }
}

// Light exactly the LED for the current beat and the current bar;
// every other beat/bar LED is off.
void updateLeds() {
  for (uint8_t i = 0; i < 4; i++) {
    digitalWrite(BEAT_PINS[i], (i + 1 == currentBeat) ? HIGH : LOW);
  }
  for (uint8_t i = 0; i < 8; i++) {
    digitalWrite(BAR_PINS[i], (i + 1 == currentBar) ? HIGH : LOW);
  }
}
