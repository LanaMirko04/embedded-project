# Sdrumo

ESP32-based touchscreen that shows the real-time public-transport arrivals
(Trentino Trasporti), local weather and current time, paired to a user account via a
companion app.

## Repository layout

| Path              | Description                                                       |
| ------------------| ----------------------------------------------------------------- |
| `firmware/`       | ESP-IDF firmware for the ESP32 / CYD board (LVGL UI, Wi-Fi).      |
| `webserver/`      | Flask backend (REST API, SQLite, JWT, Trentino Trasporti proxy).  |
| `flutter-client/` | Mobile app for pairing devices and configuring them.              |

## Quick start

### Firmware (this branch)

```bash
cd firmware
# ESP-IDF must be installed and exported (see firmware/README.md)
idf.py build flash monitor
```

See [`firmware/README.md`](firmware/README.md) for full firmware docs.

### Server

Server code lives under `webserver/`. Setup and run instructions are in
[`webserver/README.md`](webserver/README.md).

```bash
cd webserver
# follow webserver/README.md
```

### Companion app

Mobile app code lives under `flutter-client/`. Setup and run instructions
are in [`flutter-client/README.md`](flutter-client/README.md).

```bash
cd flutter-client
# follow flutter-client/README.md
```

## Architecture overview

```
┌──────────────┐   HTTP(S) JSON    ┌───────────────────┐   HTTPS    ┌──────────────────────┐
│  ESP32 board │ ◄───────────────► │  Flask backend    │ ─────────► │ Trentino Trasporti   │
│  (firmware/) │                   │  (webserver/)     │            │ + open-meteo APIs    │
└──────────────┘                   └───────────────────┘            └──────────────────────┘
        ▲                                    ▲
        │ pairing via JWT                    │ REST API
        │                                    │
┌──────────────────────────┐                 │
│  Flutter companion app   │ ────────────────┘
│  (flutter-client branch) │
└──────────────────────────┘
```

1. The board registers itself once (`POST /api/sdrumo/auth/register`),
   receives an anonymous token;
2. The user pairs the token with their account through the app;
3. The board stores the token inside the NVS;
4. The board polls `/api/sdrumo/bus/getTrips/<token>` and
   `/api/sdrumo/weather/getWeather/<token>` to refresh its screens.

## Exam evaluation

See [`EVALUATION.md`](EVALUATION.md) for the exam/evaluation readme.

## License

See [`LICENSE`](LICENSE).
