# Sdrumo backend

The backend is a **Flask** application that exposes a REST API consumed by
the ESP32 firmware and the Flutter companion app. It uses **SQLite** for
storage, **flask-jwt-extended** for user authentication, and proxies the
**Trentino Trasporti** + **open-meteo** APIs.

## 1. Install Python and create a virtualenv

```bash
cd webserver
python3 -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
```

Python 3.11+ recommended.

## 2. Environment variables

_TO-DO_

## 3. Initialise the database

```bash
flask --app flaskr init-db
```

This creates `instance/flaskr.sqlite` and seeds the `busses` table from
`flaskr/schema.sql`.

> [!WARNING]
> Re-running `init-db` **drops existing tables**. Back up the DB first
> if it contains real data.

## 4. Run the development server

```bash
flask --app flaskr run --host 0.0.0.0 --port 8000
```
## 5. Smoke test

```bash
curl http://localhost:8000/api/status/
# -> {"status":"ok"}
```

## 6. API surface (relevant to the firmware)

All routes are mounted under `/api/`.

| Method | Path                                      | Auth     | Used by      |
|   |  | 
| GET    | `/api/status/`                            | none     | both         |
| POST   | `/api/sdrumo/auth/register`               | none     | firmware     |
| GET    | `/api/sdrumo/bus/getTrips/<token>`        | none     | firmware     |
| GET    | `/api/sdrumo/weather/getWeather/<token>`  | none     | firmware     |
| POST   | `/api/user/auth/login` / `/register`      | none/JWT | mobile app   |
| GET    | `/api/config/get/<token>`                 | JWT      | mobile app   |
| POST   | `/api/config/pair` / `/unpair` / `/…`     | JWT      | mobile app   |

A [Bruno](https://www.usebruno.com/) collection is provided under
`webserver/bruno/` for manual testing.
