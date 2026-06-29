# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

**Sdrumo** — an ESP32 CYD (Cheap Yellow Display) touchscreen that shows real-time
Trentino Trasporti bus arrivals, weather, and a clock. University embedded-systems
project. Three deployable components in one repo:

| Path              | Stack                          | Role                                              |
| ----------------- | ------------------------------ | ------------------------------------------------- |
| `firmware/`       | ESP-IDF (C + C++), LVGL        | Device firmware for the ESP32 CYD board.          |
| `webserver/`      | Flask + SQLite + JWT           | REST hub; proxies Trentino Trasporti + open-meteo.|
| `flutter-client/` | Flutter/Dart (Dio, provider)   | Companion app: WiFi provisioning + device pairing.|

Data flow: board ⇄ Flask backend ⇄ Trentino Trasporti / open-meteo. The Flutter
app talks only to the backend (JWT). The board talks only to the backend (token
in URL path, no JWT). See `EVALUATION.md` for the full setup/run walkthrough and
`README.md` for the diagram.

> **Branch note:** `master` historically split each component onto its own branch
> (`flutter-client`, `web-server`, `feature/ui`, …). The working branch `dev`
> carries all three directories together — work here unless told otherwise.

## Build / run

### Firmware (`firmware/`) — needs ESP-IDF ≥ 5.2 installed and exported

```bash
cd firmware
idf.py erase-flash      # before first flash, or to force re-register/re-pair
idf.py build flash      # build + upload
idf.py monitor          # serial log
idf.py fullclean        # build/ + managed_components/ (use after dependency changes)
```

The backend URL the device hits is `ApiClient::BASE_URL` in
`firmware/include/api_client.h` (currently `http://sdrumo.zagatti.me`) — change it
there to point at a local backend. (README/EVALUATION say `api_client.cpp`; the
constant actually lives in the header.)

### Backend (`webserver/`) — Python 3.11+

```bash
cd webserver
python3 -m venv .venv && source .venv/bin/activate
pip install -r requirements.txt
flask --app flaskr init-db          # creates instance/flaskr.sqlite, seeds busses
flask --app flaskr run --host 0.0.0.0
curl http://localhost:5000/api/status/   # -> {"status":"ok"}
```

`init-db` **drops all tables** — back up `instance/flaskr.sqlite` first if it has
real data. Requires a `.env` (see `webserver/README.md`): without
`TT_BASIC_AUTH_USER`/`TT_BASIC_AUTH_PASS` the bus endpoints return **503**. There
are no automated tests; manual testing uses the Bruno collection in
`webserver/bruno/`.

### Companion app (`flutter-client/`)

```bash
cd flutter-client
flutter pub get
flutter run                 # or: flutter run -d android
```

Backend URL is `baseUrl` in `flutter-client/lib/constants/api_config.dart`.

## Architecture

### Firmware — singletons + FSM (mixed C/C++)

The split is deliberate: **C++ = logic, C = LVGL UI**, bridged by `extern "C"`
guards in the `screen_*.h` headers.

- **Four singletons**, all via `get_instance()` with deleted copy/assign and
  FreeRTOS-mutex RAII (`MutexLock`, `include/mutex_lock.h`):
  `Fsm`, `Config` (NVS persistence), `ApiClient` (HTTP), `NetHandler` (WiFi+SNTP).
- **`Fsm` (`fsm.cpp`)** drives boot: `INIT → WIFI_CONNECTION → FETCH_CONFIG →
  UPDATE_VIEW`. Transitions are gated by a static `transition_matrix`. Each state
  has a registered `StateAction` returning a `Result`; any failure routes to
  `ERROR`, which logs, waits 5 s, and calls `esp_restart()`.
- **`Result` (`result.{h,cpp}`)** is the project-wide error type — functions
  return it instead of throwing.
- **`api_task.cpp`** is the FreeRTOS task that polls the backend to refresh
  screens; **`screen_*.c`** are LVGL screens (one `.h`/`.c` pair each), wired
  through `screen_manager` / `ui_navigation`.
- LVGL config: `include/lv_conf.h`, injected via `LV_CONF_PATH` in the root
  `CMakeLists.txt`. UI images live in `src/images/` as generated `.c` arrays.

### Backend — Flask app factory + blueprints grouped by consumer

`flaskr/__init__.py::create_app()` registers blueprints under three audiences —
the URL prefix tells you the auth model:

- `/api/sdrumo/*` → **device-facing**, token in URL path, **no JWT**
  (`routes/sdrumo/`).
- `/api/user/*` → **app-facing**, **JWT required** (`routes/user/`).
- `/api/config/*` → **mobile pairing** endpoints, JWT (`routes/sdrumoConfig.py`).

`db.py` holds the SQLite connection + `init-db` CLI command; schema in
`flaskr/schema.sql`. Core tables: `users`, `sdrumos` (devices, `token` + optional
`user_id` = pairing link), `busses`, `sdrumo_busses`, `sdrumo_stops`,
`refresh_tokens`. `busColor` in trip responses is a 24-bit `0xRRGGBB` int or
`null`. The `webserver/busses/` dir holds the bus-line seed data/script.

### Companion app — AuthGate over a single Dio client

- `main.dart` loads tokens from secure storage, then wraps `MyApp` in a
  `ChangeNotifierProvider<MyAppState>`.
- `services/auth_service.dart` is the auth source of truth; the root widget
  (`app/app.dart`) gates on it (logged-out → login, logged-in → home).
- `services/api_service.dart` wraps **one** Dio instance with a JWT-refresh
  interceptor that retries on **401/422**. Endpoint paths are centralized in
  `constants/api_config.dart` — add new routes there, not inline.
- `models/` mirror backend JSON; `pages/` are screens, `dialogs/` are modals
  (login, SmartConfig WiFi provisioning).

## Pairing / provisioning flow (cross-component)

1. Board first-boot → waits for WiFi creds over **EspTouch v2 (TCP)** sent by the
   app's SmartConfig dialog (`esp_smartconfig`); stores them in NVS.
2. Board `POST /api/sdrumo/auth/register` → gets an anonymous **token**, shows it
   on screen, persists it in NVS.
3. User enters that token in the app → `POST /api/config/pair` links token→account.
4. Board picks up the pairing on its next `getConfig` poll and switches to the
   clock screen. Config changes (e.g. stop) propagate on the next poll — no reboot.

A board RESET reuses NVS creds/token (no re-pair). `idf.py erase-flash` forces a
fresh register + pair.
