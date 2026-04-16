# ESP32 e-Business Card

Firmware for an electronic business card (e-business card) built on the **Seeed Studio XIAO ESP32-C6** microcontroller with a **Waveshare 2.66" e-paper display**. The project is written in **C** using the **ESP-IDF** framework (native, not Arduino).

---

## Project Description

The device acts as a portable electronic business card — on startup it creates its own Wi-Fi access point and web server, allowing the display content to be managed from any device (phone, laptop) without installing any application.

The display is bistable (e-paper), meaning the shown content remains visible even after power is disconnected, with zero energy consumption.

---

## Features

- Wi-Fi access point and captive portal (automatic redirect to the web interface)
- Web interface for display management (no app required, works in any browser)
- Draw text, lines, rectangles, circles, and numbers on the display
- Upload a custom image (`.bmp`) via the web interface and show it on the display
- Rotate display content by 90°
- Undo last operation
- Clear display (white or black background)
- Live preview of the current display state in the browser

---

## Hardware

| Component | Model |
|---|---|
| Microcontroller | Seeed Studio XIAO ESP32-C6 |
| Display | Waveshare 2.66" e-Paper (296×152 px, black & white) |
| Battery | Rechargeable Li-Po |
| Charging circuit | TP4056 + FDN336P P-MOSFET + USB-C |
| Button | TL3305AF260QG (GPIO0, wake-up from deep sleep) |

---

## Software Stack

- **Framework:** ESP-IDF (native C, not Arduino)
- **Language:** C (C99/C11)
- **Build system:** CMake + `idf.py`

---

## Project Structure

```
├── CMakeLists.txt
├── main/
│   ├── CMakeLists.txt
│   ├── main.c                  # Application entry point
│   ├── flower_black.c          # Image (C array) — black flower
│   ├── flower_white.c          # Image (C array) — white flower
│   └── pusty_white.c           # Image (C array) — blank background
├── components/
│   ├── display_wraper/         # E-paper display abstraction (buffer, rotation, undo)
│   ├── server_setup/           # HTTP web server and request handlers (ESP-IDF httpd)
│   ├── web_site/               # HTML/CSS/JS web interface (embedded string)
│   ├── spiff_my/               # SPIFFS filesystem initialization
│   ├── wake_up/                # Deep sleep and wake-up logic (EXT1 button)
│   ├── captive_portal/         # DNS server for captive portal
│   ├── epd2in66/               # Waveshare 2.66" e-paper display driver
│   ├── GUI/                    # Drawing library (GUI_Paint)
│   ├── Fonts/                  # Bitmap fonts (8, 12, 16, 20, 24 px)
│   └── DEV_config/             # Hardware configuration SPI/GPIO
└── README.md
```

---

## Getting Started

### Prerequisites

- [ESP-IDF v5.x](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/get-started/index.html) installed and configured
- Seeed Studio XIAO ESP32-C6 connected via USB-C

### Build and Flash

```bash
idf.py set-target esp32c6
idf.py build
idf.py flash monitor
```

### Connecting to the Device

1. After startup, the device creates a Wi-Fi network — the SSID and password are shown directly on the e-paper display.
2. Connect to this network from your phone or laptop.
3. The browser will automatically open the web interface (captive portal), or navigate to `http://192.168.4.1`.

---

## Third-Party Components

The following components are external libraries and are not part of the original work of this project:

| Component | Description |
|---|---|
| `epd2in66` | Waveshare e-paper driver (EPD_2IN66) |
| `GUI` | GUI_Paint drawing library (Waveshare) |
| `Fonts` | Bitmap fonts (Waveshare) |
| `DEV_config` | Hardware abstraction layer SPI/GPIO (Waveshare) |
| `captive_portal` | DNS server for captive portal |

---

## Author

Tomáš Hronček — Bachelor's Thesis, 2026

