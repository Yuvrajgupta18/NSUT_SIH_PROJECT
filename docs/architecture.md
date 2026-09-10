# System Architecture

## High-level flow

```
Environment (bag / person / powder sample)
        |
        v
  +-----------------------------+       +-----------------------------+
  | MOX Gas Sensor Array        |       | AS7263 Spectral Sensor      |
  | (MQ135, MQ138, MQ137, MQ3)  |       | (6-channel NIR, I2C)        |
  +-----------------------------+       +-----------------------------+
        |                                          |
        +-------------------+---------------------+
                             v
                  ESP32 Microcontroller
              (reads both sensors, fuses data)
                             |
                             v
              On-device Reference Library
        (known-safe / flagged signatures, built via
         simulant testing: flour, baking soda, turmeric)
                             |
                             v
                Fusion / Alert Scoring Logic
                             |
        +--------------------+--------------------+
        v                                          v
  OLED Display                            Buzzer + LED
  (Red / Yellow / Green)                  (audible + visual alert)
        |
        v
   LoRa Transmitter Module
        |
        v
   LoRa Base Station Receiver
   (RPF control room / checkpoint)
        |
        v
  Officer Verification & Escalation
     (existing RPF workflow)
```

## Components

### MOX Gas Sensor Array
Four different metal-oxide semiconductor gas sensors : **MQ135, MQ138, MQ137, MQ3** — each sensitive to a different family of airborne vapors (general air quality, organic solvents, ammonia, and alcohol-type vapors respectively). Each sensor outputs one analog voltage read via the ESP32's ADC pins. Used together, their combined pattern acts as a "smell fingerprint" for whatever vapor is present near a bag or person.

### AS7263 Spectral Sensor
A 6-channel near-infrared spectral sensor (610–860 nm) connected over I2C. Shines light on a nearby powder or surface and measures how much light reflects back across each of its 6 channels, producing a "light fingerprint" that differs between substances even when they look visually identical.

### ESP32 Microcontroller
The central processing unit of the device. Continuously polls both sensor arrays (MOX via analog input, AS7263 via I2C), runs the on-device comparison/fusion logic against the stored reference library, and drives all output peripherals (display, buzzer, LED, LoRa).

### On-device Reference Library
A small dataset of known vapor and spectral signatures, captured during development using legally accessible simulant substances (flour, baking soda, turmeric, etc.) as stand-ins for real narcotics/explosives, since real samples aren't accessible for testing. New sensor readings are compared against this library for pattern matching.

### Fusion / Alert Scoring Logic
Combines the MOX and AS7263 readings into a single alert level:
- Deviation in **one** sensor → partial match → Yellow alert
- Deviation in **both** sensors simultaneously → high-confidence match → Red alert
- No deviation from baseline → Green (all clear)

### OLED Display
Local screen on the device showing the live alert status (Red / Yellow / Green) to the officer holding it.

### Buzzer + LED
Provides an immediate audible and visual alert the moment a high-confidence detection occurs, without requiring the officer to be looking at the screen.

### LoRa Module
A long-range, low-power wireless radio used to transmit alert data (device ID, alert level, timestamp) to a fixed base station — chosen specifically because it doesn't require WiFi or mobile network coverage, making it viable at remote/unmanned stations.

### LoRa Base Station Receiver
A fixed receiving unit (e.g. positioned at an RPF control room or checkpoint) that collects incoming alerts from one or more field-deployed handheld devices for logging and officer follow-up.

## Data Flow Summary
1. Device continuously samples ambient air (MOX array) and nearby surface reflectance (AS7263) in real time.
2. ESP32 compares both live readings against the on-device reference library.
3. Fusion logic converts the comparison into a single alert level.
4. Alert is shown locally (OLED + buzzer/LED) **and** transmitted via LoRa to the base station.
5. The receiving officer follows RPF's existing verification procedure (manual check, escalation); our device flags, it does not make the final call.
