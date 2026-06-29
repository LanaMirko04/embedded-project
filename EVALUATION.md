# Sdrumo: Project Evaluation Guide

[Sdrumo](.) is an ESP32-based touchscreen system that displays real-time public-transport arrivals (Trentino Trasporti), local weather, and current time, paired to a user account via a companion mobile app.

The main components of the Sdrumo system are the [*firmware*](./firmware) running on the ESP32 CYD board, the [*webserver*](./webserver) Flask backend, and the [*flutter-client*](./flutter-client) companion application.

```
┌──────────────┐   HTTP JSON   ┌───────────────────┐   HTTPS   ┌──────────────────────┐
│  ESP32 CYD   │ ◄───────────► │  Flask backend    │ ────────► │ Trentino Trasporti   │
│  (firmware/) │               │  (webserver/)     │           │ + open-meteo APIs    │
└──────────────┘               └───────────────────┘           └──────────────────────┘
        ▲                               ▲
        │  WiFi pairing                 │ REST API (JWT)
        │                               │
┌──────────────────────────┐            │
│  Flutter companion app   │ ───────────┘
│  (flutter-client/)       │
└──────────────────────────┘
```

## Roadmap for Evaluation

1. You will **set up and run the Flask backend**, which is the hub that proxies Trentino Trasporti and open-meteo APIs and manages device and user accounts.
2. You will **build and flash the firmware** onto an ESP32 CYD board. The device will register itself with the backend automatically on first boot.
3. You will **use the Flutter companion app** to provision WiFi credentials via EspTouch v2 and pair the device to a user account.
4. You will see the device display **live bus arrivals, weather, and clock** and verify that state is preserved across resets.

## Requirements

### Hardware Requirements

Sdrumo targets the **ESP32 CYD** (Cheap Yellow Display) development board, which integrates:

- ESP32 microcontroller with WiFi
- 2.8" ILI9341 SPI TFT display, $320 \times 240$ px
- XPT2046 SPI resistive touch controller

To flash the firmware you need a USB cable and the ESP-IDF toolchain (see below).

### Software Requirements

| Component     | Requirement                  |
|---------------|------------------------------|
| Firmware      | ESP-IDF ≥ 5.2                |
| Backend       | Python 3.11+                 |
| Companion app | Flutter SDK (stable channel) |

- **ESP-IDF**: follow the [official installation guide](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/index.html#installation). On Arch Linux: `yay -S esp-idf`.
- **Python**: standard install; `python3 -m venv` is used to isolate dependencies.
- **Flutter**: follow the [Flutter install guide](https://docs.flutter.dev/get-started/install). Run `flutter doctor` to verify setup.

## Project Layout

```
.
├── README.md
├── EVALUATION.md               # this file
├── firmware/                   # ESP32 ESP-IDF project
│   ├── CMakeLists.txt
│   ├── src/                    # all C / C++ sources
│   │   ├── main.cpp            # FSM boot sequence
│   │   ├── fsm.cpp             # state machine
│   │   ├── config.cpp          # NVS persistence
│   │   ├── api_client.cpp      # HTTP client singleton
│   │   ├── net.cpp             # WiFi + SNTP
│   │   ├── api_task.cpp        # FreeRTOS polling task
│   │   ├── screen_clock.c      # LVGL screen — clock
│   │   ├── screen_bus.c        # LVGL screen — bus arrivals
│   │   ├── screen_weather.c    # LVGL screen — weather
│   │   ├── screen_alarm.c      # LVGL screen — alarm picker
│   │   ├── screen_timer.c      # LVGL screen — countdown timer
│   │   └── ...
│   └── include/                # headers (one per module)
├── webserver/                  # Flask backend
│   ├── flaskr/                 # app factory + blueprints
│   ├── bruno/                  # Bruno API collection (manual testing)
│   └── requirements.txt
└── flutter-client/             # Dart companion app
    └── lib/
        ├── main.dart
        ├── app/                # AuthGate, theme
        ├── services/           # AuthService, ApiService (Dio)
        ├── pages/              # Home, DeviceList, DevicePage, …
        └── dialogs/            # Login, SmartConfigDialog
```

### Source code organisation

**`firmware/src/`** uses a mixed C / C++ split:

- *C++* (`main.cpp`, `fsm.cpp`, `config.cpp`, `api_client.cpp`, `net.cpp`, `api_task.cpp`): core logic layer. Four singletons (`Fsm`, `Config`, `ApiClient`, `NetHandler`) share a uniform `get_instance()` pattern with deleted copy/assign and FreeRTOS mutex RAII via `MutexLock`.
- *C* (`screen_*.c`, `lcd.c`, `touch.c`): LVGL UI screens. Each screen is a `screen_*.{h,c}` pair; headers use `extern "C"` guards for C++ compatibility.

The FSM in `fsm.cpp` drives the boot sequence: `INIT -> WIFI_CONNECTION -> FETCH_CONFIG -> UPDATE_VIEW`. Any step returning a failure transitions to `ERROR`, which logs the error, waits 5 seconds, and calls `esp_restart()`.

**`webserver/flaskr/`** is a Flask app factory with blueprints grouped by consumer:

- `routes/sdrumo/`: device-facing endpoints (token in URL, no JWT)
- `routes/user/`: user-facing endpoints (JWT required)
- `routes/sdrumoConfig.py`: mobile pairing endpoints (JWT required)

**`flutter-client/lib/`** follows an `AuthGate` pattern: `AuthService` drives the root widget; `ApiService` wraps a single Dio instance with a JWT refresh interceptor that retries on 401/422.

## Getting Started

### 1. Set up the backend

```bash
cd webserver
python3 -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
```

Create a `.env` file in `webserver/`:

```dotenv
# Required — Trentino Trasporti credentials
TT_BASIC_AUTH_USER=your_tt_username
TT_BASIC_AUTH_PASS=your_tt_password

# Required in production
JWT_SECRET_KEY=change-me-in-production

# Optional — defaults shown
TT_BASE_URL=https://app-tpl.tndigit.it/gtlservice/
ACCESS_TOKEN_EXPIRES_MINUTES=1000
REFRESH_TOKEN_EXPIRES_DAYS=30
```

> **Note:** without `TT_BASIC_AUTH_USER` / `TT_BASIC_AUTH_PASS` the bus trip endpoints return **503** — the server starts but live departures will not work.

Initialise the database (creates `instance/flaskr.sqlite`):

```bash
flask --app flaskr init-db
```

> **Warning:** re-running `init-db` **drops existing tables**. Back up `instance/flaskr.sqlite` first if it contains real data.

Run the development server:

```bash
flask --app flaskr run --host 0.0.0.0
```

Smoke test:

```bash
curl http://localhost:5000/api/status/
# Expected: {"status":"ok"}
```

### 2. Build and flash the firmware

Update `BASE_URL` in `firmware/src/api_client.cpp` to match the machine running the Flask backend, then:

```bash
cd firmware

# First flash: erase NVS so the device registers fresh
idf.py erase-flash

# Build and flash
idf.py build flash

# Monitor serial output
idf.py monitor
```

Other useful commands:

```bash
idf.py clean        # remove build artifacts
idf.py fullclean    # remove build/ and managed_components/
```

### 3. Run the companion app

```bash
cd flutter-client
flutter pub get
flutter run          # or: flutter run -d android
```

## User Guide

### First boot: WiFi provisioning

On first boot (or after `idf.py erase-flash`) the device shows the WiFi setup screen and waits for EspTouch v2 credentials.

1. Connect your phone to the **2.4 GHz** WiFi network you want the device to join.
2. Open the companion app and navigate to the **WiFi setup** screen.
3. Enter the WiFi password and start provisioning. On Linux the app auto-fills SSID via `iwgetid`.
4. The device receives the credentials over EspTouch v2 (TCP), connects, stores them in NVS, and proceeds.

### First boot: device pairing

After connecting to WiFi the device registers itself with the backend and displays an **anonymous token** on screen.

1. Open the companion app and log in (or register a new account).
2. Navigate to the **pairing screen** and enter the token shown on the device.
3. The app sends `POST /api/config/pair` to link the token to your account.
4. The device detects the pairing on the next config poll and transitions to the main clock screen.

### Navigating screens

Swipe or tap the on-screen arrows to move between screens:

| Screen      | Content                                                                                     |
|-------------|---------------------------------------------------------------------------------------------|
| **Clock**   | Current time (updates every second)                                                         |
| **Bus**     | Live arrivals for your configured stop; color-coded by line, with time-to-arrival and delay |
| **Weather** | Current conditions + 3-day forecast                                                         |

### Changing the bus stop

Open the companion app, select your paired device, and pick a different stop from the stop list. The device picks up the change on the next `/getConfig` poll (no restart needed).

### Resetting the device

Press the **RESET button** on the board. The device reconnects using the stored NVS credentials and token; no re-pairing or re-provisioning required.

To fully reset (force re-registration and re-pairing):

```bash
idf.py erase-flash
```

### API surface

All routes are mounted under `/api/`. Device routes use a **token in the URL path**; user routes use **JWT**.

| Method | Path                                     | Auth | Consumer |
|--------|------------------------------------------|------|----------|
| GET    | `/api/status/`                           | none | both     |
| POST   | `/api/sdrumo/auth/register`              | none | firmware |
| GET    | `/api/sdrumo/bus/getTrips/<token>`       | none | firmware |
| GET    | `/api/sdrumo/weather/getWeather/<token>` | none | firmware |
| GET    | `/api/sdrumo/config/getConfig/<token>`   | none | firmware |
| POST   | `/api/user/auth/login`                   | none | app      |
| POST   | `/api/user/auth/register`                | none | app      |
| GET    | `/api/config/get/<token>`                | JWT  | app      |
| POST   | `/api/config/pair`                       | JWT  | app      |

A [Bruno](https://www.usebruno.com/) collection covering every endpoint is provided under `webserver/bruno/`.

## Presentation and Demo

| Resource   | Link                                        |
|------------|---------------------------------------------|
| Slides     | [presentation.pdf](./docs/presentation.pdf) |
| Demo video | *[add link]*                                |

## Team Members

| Name                | GitHub                                         |
|---------------------|------------------------------------------------|
| Mirko Lana          | [@LanaMirko04](https://github.com/LanaMirko04) |
| Giulia Cristofolini | [@giulzzz04](https://github.com/giulzzz04)     |
| Mattia Zagatti      | [@MatPrayer](https://github.com/MatPrayer)     |
