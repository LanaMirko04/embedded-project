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

## 2. Configure environment variables

Create a `.env` file in the `webserver/` directory:

```dotenv
# Trentino Trasporti API credentials (required)
TT_BASIC_AUTH_USER=your_tt_username
TT_BASIC_AUTH_PASS=your_tt_password

# Optional — defaults shown
TT_BASE_URL=https://app-tpl.tndigit.it/gtlservice/

# JWT signing key (required in production; falls back to Flask SECRET_KEY in dev)
JWT_SECRET_KEY=change-me-in-production

# Token lifetimes (optional)
ACCESS_TOKEN_EXPIRES_MINUTES=1000
REFRESH_TOKEN_EXPIRES_DAYS=30
```

> [!NOTE]
> Without `TT_BASIC_AUTH_USER` and `TT_BASIC_AUTH_PASS` the bus trip
> endpoints return **503** — the server starts but live departures won't work.

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
flask --app flaskr run --host 0.0.0.0
```

## 5. Smoke test

```bash
curl http://localhost:5000/api/status/
# Response = {"status":"ok"}
```

## 6. API surface (relevant to the firmware)

All routes are mounted under `/api/`.

| Method | Path | Auth | Used by |
|--------|------|------|---------|
| GET | `/api/status/` | none | both |
| POST | `/api/sdrumo/auth/register` | none | firmware |
| GET | `/api/sdrumo/bus/getTrips/<token>` | none | firmware |
| GET | `/api/sdrumo/bus/getTrips/<token>?stop_id=<int>` | none | firmware |
| GET | `/api/sdrumo/weather/getWeather/<token>` | none | firmware |
| GET | `/api/sdrumo/config/getConfig/<token>` | none | firmware |
| POST | `/api/user/auth/login` / `/register` | none/JWT | mobile app |
| GET | `/api/config/get/<token>` | JWT | mobile app |
| POST | `/api/config/pair` / `/unpair` / `/…` | JWT | mobile app |

`busColor` in trip responses is a 24-bit integer (`0xRRGGBB`), or `null` if no color is configured for that line.

A [Bruno](https://www.usebruno.com/) collection is provided under
`webserver/bruno/` for manual testing.
