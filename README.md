# SIH 2026 Project Repository — ARES

## 1. Project Information

- **Project Title:** Narcotics & Explosives detector
- **PS ID:** SIH26026
- **PS Title:** Development of Mobile (Quadruped)/Handheld Device/System for Real-Time Detection of Narcotics and Explosives across Indian Railways
- **Category:** Hardware
- **Theme:** Blockchain & Cybersecurity
- **Team ID:** 

## 2. Problem Statement

Indian Railways carries millions of passengers daily across thousands of stations, but outside a handful of major hubs, there is little to no systematic screening for narcotics or explosives. Fixed X-ray scanners and DFMDs exist only at select high-risk stations, and trained sniffer-dog units are limited in number and cannot scale to cover the network. This leaves thousands of smaller/unmanned stations, platforms, and parcel yards with effectively zero detection capability today.

## 3. Proposed Solution

A low-cost, handheld screening device that combines two independent sensing layers — a metal-oxide gas sensor array that detects unusual vapor patterns in the air, and a near-infrared spectral sensor that reads a powder or surface's light-reflectance signature. Both readings are fused on an onboard microcontroller and compared against a stored reference library in real time. When a reading deviates from baseline, the device raises a local alert (display + buzzer) and transmits the alert over a long-range, low-power radio link to a base station — without requiring WiFi or mobile network coverage. The device is designed as a **Stage-1 screening tool** that feeds into RPF/GRP's existing manual verification workflow, rather than acting as a final chemical identification system.

## 3a. How It Works

1. On power-up, the device samples clean air and a neutral surface to set a baseline reading.
2. The MOX gas sensor array continuously samples the surrounding air for vapor changes.
3. The AS7263 spectral sensor shines near-infrared light on a nearby surface/powder and reads its 6-channel reflectance.
4. Both readings are compared in real time against an on-device reference library of known signatures.
5. If either signal deviates from baseline, the device raises a partial (Yellow) alert. If both deviate together, it raises a high-confidence (Red) alert.
6. The alert is shown instantly on the OLED display with a buzzer/LED, and simultaneously transmitted over LoRa to a base station — working even without WiFi or mobile network coverage.
7. The receiving officer follows RPF's existing verification procedure — the device flags for review, it does not make a final determination on its own.

## 4. Key Features

- Dual-sensor detection: vapor signature (MOX array) + spectral reflectance signature (AS7263)
- Real-time on-device alert scoring (Red / Yellow / Green)
- Local OLED display + buzzer/LED alert
- Long-range wireless alert transmission via LoRa (no WiFi/mobile network required)
- Portable, battery-powered, handheld wand form factor
- Low unit cost (~₹4,000–5,000) enabling wide deployment across stations

## 5. Technology Stack

- **Sensors:** MQ135, MQ138, MQ137, MQ3 (MOX gas sensor array) + AS7263 (6-channel NIR spectral sensor)
- **Microcontroller:** ESP32 dev board
- **Communication Module:** LoRa (e.g. RFM95W / SX1278)
- **Output/Actuators:** SSD1306 OLED display (I2C), piezo buzzer, status LED
- **Power:** 3.7V Li-ion cell + TP4056 charging module
- **Firmware:** Arduino / C++ (ESP32)
- **On-device Logic:** Lightweight reference-pattern matching / fusion scoring (no cloud dependency required for core alert function)

## 6. Expected Output / Impact

**What the final output looks like:**
When swept near a bag, parcel, or person's hands, the device continuously displays a live status on its OLED screen — Green (no anomaly), Yellow (one sensor signal deviating from baseline), or Red (both sensors deviating, high-confidence alert with buzzer). Each Red alert is also transmitted over LoRa to a base station receiver, where it appears as a logged entry (device ID, alert level, timestamp) for an officer to follow up on. The physical device itself is a battery-powered handheld wand, matching the familiar shape of existing metal-detector tools used by RPF/GRP.

**Expected impact:**
- Extends screening capability to the thousands of smaller/unmanned stations that currently have none, at a fraction of the cost of imported trace-detection equipment (~₹4,000–5,000/unit vs. lakhs)
- Acts as a force-multiplier for RPF/GRP, supplementing limited sniffer-dog units and fixed scanners
- Improves passenger safety and supports disruption of narcotics/explosives trafficking through the rail network
- Designed to scale from a station-level pilot to zonal and national rollout without redesign

## 7. Architecture

See [docs/architecture.md](docs/architecture.md).

```
Environment (bag / person / powder sample)
        |
        v
  MOX Gas Sensor Array        AS7263 Spectral Sensor
  (MQ135, MQ138, MQ137, MQ3)  (6-channel NIR, I2C)
        |                            |
        +-------------+--------------+
                       v
              ESP32 Microcontroller
          (reads both sensors, fuses data)
                       |
                       v
          On-device Reference Library
                       |
                       v
            Fusion / Alert Scoring Logic
                       |
        +--------------+--------------+
        v                             v
   OLED Display                Buzzer + LED
   (Red/Yellow/Green)          (audible + visual alert)
        |
        v
   LoRa Transmitter → LoRa Base Station Receiver
                       |
                       v
        Officer Verification & Escalation
             (existing RPF workflow)
```

## 8. Repository Structure

```
SIH26026-PROJECT/
├── README.md
├── SUBMISSION_GUIDE.md
├── submission/
│   ├── PRESENTATION.md
│   └── DEMO.md
├── firmware/
│   └── main.ino
├── docs/
│   └── architecture.md
├── hardware/
│   ├── circuit-diagram/
│   └── cad-design/
├── assets/
│   └── screenshots/
│       └── README.md
└── LICENSE
```

### What goes where?

| Item                                   | Location                    |
| --------------------------------------- | ---------------------------- |
| Firmware / embedded source code         | `firmware/`                  |
| Circuit diagrams, wiring references     | `hardware/circuit-diagram/`  |
| CAD design files / renders              | `hardware/cad-design/`       |
| Architecture / technical documentation  | `docs/`                      |
| Project / hardware photos               | `assets/screenshots/`        |
| Final PPT / presentation                | `submission/`                |
| Demo video link                         | `submission/DEMO.md`         |
| Project overview                        | `README.md`                  |

## 9. Final Presentation

Final SIH presentation is kept in the repository under `submission/`. See [submission/PRESENTATION.md](submission/PRESENTATION.md) for the required format.

## 10. Demo Video

See [submission/DEMO.md](submission/DEMO.md) for the video link and script.

## 11. Screenshots / Prototype Photos

Circuit diagram and CAD renders are added to `assets/screenshots/` and `hardware/`. See [assets/screenshots/README.md](assets/screenshots/README.md) for naming conventions.

## 10a. Expected Output

When the device is powered on and swept near a bag, parcel, or person's hands:

- **Green (all clear):** OLED shows "Normal" — both sensors match baseline/known-safe signatures, no buzzer.
- **Yellow (partial match):** OLED shows "Check Advised" — one sensor (vapor or spectral) deviates from baseline; a short beep sounds.
- **Red (high-confidence alert):** OLED shows "Alert" in red text — both sensors deviate together; the buzzer sounds continuously and the alert (device ID, alert level, timestamp) is transmitted over LoRa to the base station dashboard/log for officer follow-up.

Circuit diagram and CAD renders showing the physical wand design are available in `hardware/` and `assets/screenshots/`. *(Add actual photos/screen-recordings of the OLED output once the physical prototype is assembled.)*

## 10b. Note for Reviewers (Hardware Project)

This is a hardware device, not a runnable software app — a reviewer cannot "run" it via `git clone` the way a web app works. To evaluate it, reviewers can:
1. Review the firmware source code in `firmware/` for the sensing and alert logic.
2. View the circuit diagram (`hardware/circuit-diagram/`) and CAD design (`hardware/cad-design/`) to see the full wiring and physical layout.
3. Watch the demo video ([submission/DEMO.md](submission/DEMO.md)) for a walkthrough of the design and (once available) live sensor readings.
4. If reproducing physically: flash `firmware/main.ino` onto an ESP32 wired per the circuit diagram — steps below.

## 12. How a Reviewer Can Run / Verify This Project

This is a hardware device, not a hosted app — there is no live link to click. A reviewer can verify the project in two ways:

**Without physical hardware:**
- Review the finalized circuit diagram in `hardware/circuit-diagram/` and CAD design in `hardware/cad-design/`
- Review `docs/architecture.md` for the full sensor-to-alert data flow
- Watch the demo video linked in `submission/DEMO.md`, which walks through the design and workflow

**With physical hardware (once assembled):**
```
git clone <YOUR_REPOSITORY_URL>
cd <YOUR_PROJECT_FOLDER>/firmware
# Open main.ino in the Arduino IDE (or PlatformIO)
# Install required libraries: AS726X, LoRa, Adafruit_SSD1306
# Select board: ESP32 Dev Module
# Connect ESP32 via USB, select the correct COM port
# Click Upload to flash firmware/main.ino to the device
```

Update this section with real, tested flash instructions once firmware is finalized and the physical build is complete.

## 14. Future Scope

- Replace the identification layer with a certified, lab-grade sensor (e.g. NQR-based module, following proven handheld precedents) once government/RDSO partnership enables access to certified reference data
- Add GPS geo-tagging for alert location once network/power budget allows
- Extend to a quadruped-mounted variant for autonomous patrol of parcel yards and platforms
- Expand reference library through partnership-based access to real (controlled) test data

## Important

Before submission, make sure the repository is accessible to reviewers. Do **not** upload passwords, API keys, access tokens, `.env` files containing secrets, or other confidential credentials.
