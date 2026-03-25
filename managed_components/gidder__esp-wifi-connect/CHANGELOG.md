
# Changelog

## [2.5.0] - 2025-06-22

### Added

- Save authmode to protect from Wi-Fi security downgrade attack

## [2.4.0] - 2025-06-08

### Added

- Detect browser language
- Option to define default language (if desired translation does not exist)
- Add languages:
  - Čeština
  - 日本語
  - 한국어
  - Slovenčina
  - 繁體中文

### Changed

- Language agnostic backend
- All translations in one place - wifi_languages.js

## [2.3.8] - 2025-05-18

### Added

- Filter APs with the same name
- Filter Enterprise authentication APs

### Changed

- Default name set to ESP32
- Rework APs list view

### Fixed

- Refreash APs after entering wrong password

## [2.3.7] - 2025-05-17

### Added

- Show AP signal as icon

### Changed

- Update README.md
- Split changelog to CHANGELOG.md

## [2.3.6] - 2025-04-26

### Changed

- Update AP list handling according base repo

### Security

- Sanitize web input

## [2.3.5]

### Fixed

- More stable list of APs on configuration web
- Dont stop refresh AP list in configuration web

## [2.3.4]

### Changed

- Set English as default language.
- More English translations

## [2.3.0]

- Add support for language request.

## [2.2.0]

- Add support for ESP32 SmartConfig(ESPTouch v2)

## [2.1.0]

- Improve WiFi connection logic.

## [2.0.0]

- Add support for multiple WiFi SSID management.
- Auto switch to the best WiFi network.
- Captive portal for WiFi configuration.
- Support for multiple languages (English, Chinese).