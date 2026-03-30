# Anemometr Project

Solar tracker protection system for Arduino. Monitors wind speed and grid outage, automatically moving solar trackers to a safe position when dangerous conditions are detected. Sleeps during night hours using solar elevation calculation and wakes at sunrise.

---

## Overview

The system runs on an Arduino (AVR) board and continuously:

1. **Checks grid state** — if the grid goes down, trackers are moved to the protection position immediately.
2. **Reads wind speed** — if wind exceeds the configured threshold, trackers go to the protection position and stay there until wind has been calm for a configurable duration.
3. **Sleeps at night** — solar elevation is calculated from GPS coordinates; when the sun is below the horizon the board enters low-power sleep and wakes every 3 minutes to re-check.
4. **Shows live status** on a 20×4 I2C LCD — current wind speed, solar azimuth/elevation, time, or active protection mode.
5. **Accepts configuration** via an IR remote at boot time — wind thresholds and calm-wait duration are editable and persisted to EEPROM.

---

## Hardware

| Component | Details |
|---|---|
| MCU | Arduino (AVR, e.g. Nano / Uno) |
| RTC | DS3231 (I2C) |
| Display | 20×4 LCD with I2C backpack (addr `0x3F`) |
| Anemometer | Analog voltage output (0–5 V = 0–30 m/s) |
| Grid sensor | Digital signal (HIGH = grid absent) |
| Tracker relay 1 | Digital output — power control pin 1 |
| Tracker relay 2 | Digital output — power control pin 2 |
| IR receiver | 38 kHz receiver module |

---

## Pin Assignment

| Signal | Pin | Direction | Notes |
|---|---|---|---|
| Anemometer signal | A7 | INPUT | Analog 0–5 V |
| Grid state signal | A3 | INPUT | HIGH = grid outage |
| Tracker power 1 | A1 | OUTPUT | Relay control |
| Tracker power 2 | A2 | OUTPUT | Relay control |
| IR receiver | D2 | INPUT | 38 kHz NEC remote |
| RTC SDA | A4 | I2C | Shared I2C bus |
| RTC SCL | A5 | I2C | Shared I2C bus |
| LCD SDA | A4 | I2C | Shared I2C bus, addr 0x3F |
| LCD SCL | A5 | I2C | Shared I2C bus |

---

## Software Architecture

```
Anemometr_Project.ino       — Main sketch: setup(), loop(), free functions
ApplicationProperties.h     — All constants, pin defines, EEPROM addresses/macros
LcdService.h / .cpp         — LCD display abstraction (20×4, I2C)
TrackerSystemService.h/.cpp — Wind & grid protection logic, tracker relay control
IrRemoteConfig.h / .cpp     — Boot-time IR remote configuration menu
SolarPosition.h / .cpp      — Solar azimuth/elevation calculation (third-party)
```

### Class responsibilities

| Class | Responsibility |
|---|---|
| `LcdService` | Wraps `LiquidCrystal_I2C`; provides named display methods for each system state |
| `TrackerSystemService` | Reads anemometer & grid pins; manages protect/normal mode transitions; controls relay pins |
| `IrRemoteConfig` | Presents boot menu via IR remote; reads/writes EEPROM config values |

---

## Required Libraries

Install all via **Arduino IDE → Sketch → Include Library → Manage Libraries**:

| Library | Author | Purpose |
|---|---|---|
| RTClib | Adafruit | DS3231 RTC driver |
| LiquidCrystal I2C | Frank de Brabander | I2C LCD driver |
| IRremote | shirriff / z3t0 | IR receiver decoding |
| Low-Power | rocketscream | AVR low-power sleep modes |
| SolarPosition | *(bundled in project)* | Solar azimuth/elevation |

---

## Configuration

Three parameters are stored in EEPROM and survive power cycles. Defaults apply on first boot:

| Parameter | Default | Description |
|---|---|---|
| Wind speed high (m/s) | 4.5 | Speed above which last-high-speed time is recorded |
| Wind speed max (m/s) | 5.0 | Speed that triggers protection mode |
| Calm wait (seconds) | 300 | How long wind must stay below high threshold before returning to normal |

### Changing values via IR remote

At boot, the system waits **10 seconds** on the LCD prompt `"Start config?"`. Press any button on the IR remote to enter the menu.

**Menu page 0:**

| Button | Action |
|---|---|
| `1` | Set date & time |
| `2` | Configure anemometer thresholds |
| `#` | Exit menu and continue boot |

**Anemometer config (option 2)** steps through three sub-prompts in sequence:
1. Wind high speed (m/s)
2. Wind max speed (m/s)
3. Calm wait duration (seconds)

Each shows the current EEPROM value, waits for numeric IR input, and saves on confirmation.

---

## Boot Sequence

```
power on
  │
  ├─ initEEPROM()         load thresholds from EEPROM (write defaults on first run)
  ├─ pinMode setup
  ├─ new LcdService       init LCD, show "Starting wind monitoring..."
  ├─ new IrRemoteConfig
  ├─ new TrackerSystemService
  ├─ initRtc()            verify DS3231; on failure → protect mode → abort / prompt time set
  ├─ initSolarPosition()  register getUnixtimeFromRtc() as time provider
  ├─ startIrRemoteConfigurationMenu()   10-sec prompt; enter if button pressed
  ├─ initEEPROM()         reload (picks up any changes made in menu)
  ├─ trackerSystemService->reload()
  └─ goToNormalMode()     power on tracker relays, show "Go to normal mode"
```

---

## Normal Operation (loop)

Every 250 ms:

1. `sleepIfNight()` — if solar elevation ≤ −1°, enter low-power sleep loop (8 s idle cycles); wake at sunrise and reset protection state.
2. `checkGridOutageAndGoToProtectModeIfNeeded()` — read grid pin; transition in/out of grid protect mode.
3. `checkWindSpeedAndGoToProtectModeIfNeeded(nowUnix)` — read anemometer; transition in/out of wind protect mode. Skipped while grid protect is active.
4. `printSystemState()` — update LCD with current mode.

---

## Protection Modes

### Grid protection
Triggered when `GRID_STATE_SIGNAL_PIN` reads HIGH (grid absent).

```
turn trackers OFF → wait 10 s → turn trackers ON (seek protect position)
→ wait 5 min → turn trackers OFF → display "Waiting for the better times"
```

Returns to normal automatically when grid is restored (pin goes LOW).

### Wind protection
Triggered when wind speed ≥ **wind speed max** threshold.

Same physical sequence as grid protection. Returns to normal when wind speed has been below the **wind speed high** threshold for at least **calm wait** seconds.

### Night sleep
Triggered when solar elevation ≤ −1.0°. The board idles in low-power mode, waking every 3 minutes to re-check elevation. On sunrise, `goToNormalMode()` is called and both protection flags are reset for re-evaluation.

---

## Location

Coordinates are hardcoded in `ApplicationProperties.h` for solar position calculation:

```
Latitude:  48.454642  (Petropavlivka, Ukraine)
Longitude: 36.419804
```

Update `LATITUDE` and `LONGITUDE` defines for a different site.
