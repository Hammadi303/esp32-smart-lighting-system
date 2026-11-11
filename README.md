# ESP32 Smart Lighting System

An IoT smart lighting system using an ESP32-S3 and a Flutter mobile application.

## Features

- Light-level measurement using an LDR sensor.
- Manual light ON/OFF control through a mobile application.
- Wi-Fi communication.
- REST API hosted on the ESP32.
- Flutter dashboard for remote control and status monitoring.

## Architecture

- ESP32-S3: Main controller.
- LDR: Ambient light measurement.
- Relay: Light switching.
- Wi-Fi: Communication.
- Flutter: Android user interface.

## API Endpoints

| Method | Endpoint |
|---|---|
| GET | /api/status |
| POST | /api/light |
| GET | /api/config |
| POST | /api/config |

## Technologies

C++, Arduino, ESP32-S3, Flutter, Dart, REST API, Wi-Fi.

## Project Structure

- firmware/: ESP32 firmware.
- mobile-app/: Flutter application.
- docs/: Technical documentation.
- assets/: Screenshots and project visuals.
- hardware/: Hardware information.
- tests/: Testing material.
