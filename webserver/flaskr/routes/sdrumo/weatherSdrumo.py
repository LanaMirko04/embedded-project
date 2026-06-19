from flask import Blueprint, request
import requests
from datetime import datetime

from flaskr.db import get_db

bp = Blueprint('weatherSdrumo', __name__)

weather_api_url = 'https://api.open-meteo.com/v1/forecast'

def classify_cloud_cover(cloud_cover):
    cloud = float(cloud_cover or 0)
    if cloud <= 10:
        return 'sunny'
    if cloud <= 30:
        return 'mostly sunny'
    if cloud <= 60:
        return 'partly cloudy'
    if cloud <= 85:
        return 'mostly cloudy'
    return 'overcast'

def classify_weather_code(weather_code):
    # WMO weather codes: 45/48 = fog, 95/96/99 = thunderstorm
    try:
        code = int(weather_code)
    except (TypeError, ValueError):
        return None
    if code in (45, 48):
        return 'fog'
    if code in (95, 96, 99):
        return 'storm'
    return None

def classify_hour_weather(cloud_cover, rain, showers, snowfall, weather_code=None):
    coded = classify_weather_code(weather_code)
    if coded:
        return coded

    cloud = float(cloud_cover or 0)
    rain_val = float(rain or 0)
    showers_val = float(showers or 0)
    snowfall_val = float(snowfall or 0)

    precip_candidates = {
        'rainy': rain_val,
        'showers': showers_val,
        'snowfall': snowfall_val,
    }
    top_precip = max(precip_candidates, key=precip_candidates.get)
    if precip_candidates[top_precip] > 0:
        return top_precip

    return classify_cloud_cover(cloud)

def get_current_hour_index(current_time, hourly_times):
    if not hourly_times:
        return 0

    try:
        current_dt = datetime.strptime(current_time, '%Y-%m-%dT%H:%M')
        for idx, time_str in enumerate(hourly_times):
            hour_dt = datetime.strptime(time_str, '%Y-%m-%dT%H:%M')
            if hour_dt >= current_dt:
                return idx
        return len(hourly_times) - 1
    except Exception:
        return hourly_times.index(current_time) if current_time in hourly_times else 0

_ICON_MAP = {
    'sunny':        'sunny',
    'mostly sunny': 'sunny',
    'partly cloudy':'partly_cloudy',
    'mostly cloudy':'cloudy',
    'overcast':     'cloudy',
    'rainy':        'rainy',
    'showers':      'rainy',
    'snowfall':     'snow',
    'storm':        'storm',
    'fog':          'fog',
}

_ICON_NUM = {
    'sunny':         1,
    'partly_cloudy': 2,
    'cloudy':        3,
    'rainy':         4,
    'storm':         5,
    'snow':          6,
    'fog':           7,
}

def weather_icon(label):
    icon = _ICON_MAP.get(label, 'sunny')
    return _ICON_NUM.get(icon, 1)

_DISPLAY_MAP = {
    'mostly sunny':  'Sunny',
    'partly cloudy': 'Cloudy',
}

def weather_display(label):
    return _DISPLAY_MAP.get(label, label.title())

@bp.route('/getWeather/<token>', methods=['GET'])
def get_weather(token):
    db = get_db()

    row = db.execute(
        'SELECT location, location_latitude, location_longitude FROM sdrumos WHERE token = ?',
        (token,)
    ).fetchone()

    if not row or row['location_latitude'] is None or row['location_longitude'] is None:
        return {'error': 'Latitude and longitude are required'}, 400

    city = row['location'] if row['location'] else None

    params = {
        'latitude': row['location_latitude'],
        'longitude': row['location_longitude'],
        'hourly': 'temperature_2m,rain,showers,snowfall,cloud_cover,relative_humidity_2m,precipitation_probability,weather_code',
        'models': 'italia_meteo_arpae_icon_2i',
        'current_weather': 'true',
        'timezone': 'Europe/Berlin',
        'forecast_days': 4,
        'daily': 'temperature_2m_max,rain_sum,showers_sum,snowfall_sum,weather_code'
    }

    try:
        response = requests.get(weather_api_url, params=params)
        if response.status_code != 200:
            return {'error': 'Failed to fetch weather data'}, response.status_code

        data = response.json()
        hourly = data.get('hourly', {})
        hourly_times = hourly.get('time', [])

        current_time = data['current_weather']['time']
        current_idx = get_current_hour_index(current_time, hourly_times)

        def safe_hourly(key, idx):
            vals = hourly.get(key, [])
            return vals[idx] if idx < len(vals) and vals[idx] is not None else 0

        current_weather_label = classify_hour_weather(
            safe_hourly('cloud_cover', current_idx),
            safe_hourly('rain', current_idx),
            safe_hourly('showers', current_idx),
            safe_hourly('snowfall', current_idx),
            safe_hourly('weather_code', current_idx),
        )

        current_dt = datetime.strptime(current_time, '%Y-%m-%dT%H:%M')
        current_section = {
            'date': current_dt.strftime('%a %d %b'),
            'weather': weather_display(current_weather_label),
            'weather_icon': weather_icon(current_weather_label),
            'temperature': round(data['current_weather']['temperature']),
            'rain_probability': round(safe_hourly('precipitation_probability', current_idx)),
            'humidity': round(safe_hourly('relative_humidity_2m', current_idx)),
            'wind_speed': round(data['current_weather']['windspeed']),
        }

        # daily max temps by date (for forecast temperature)
        daily_data = data.get('daily', {})
        daily_max_by_date = {}
        for date, max_t in zip(daily_data.get('time', []), daily_data.get('temperature_2m_max', [])):
            daily_max_by_date[date] = max_t

        daily_code_by_date = {}
        for date, code in zip(daily_data.get('time', []), daily_data.get('weather_code', [])):
            daily_code_by_date[date] = code

        # compute per-day averages from hourly, excluding today
        today_date = hourly_times[0].split('T')[0] if hourly_times else None
        sums = {}
        counts = {}
        metrics = ['rain', 'showers', 'snowfall', 'cloud_cover', 'temperature_2m']

        for idx, time_str in enumerate(hourly_times):
            date_key = time_str.split('T')[0]
            if date_key == today_date:
                continue
            if date_key not in sums:
                sums[date_key] = {m: 0.0 for m in metrics}
                counts[date_key] = {m: 0 for m in metrics}
            for m in metrics:
                vals = hourly.get(m, [])
                if idx < len(vals) and vals[idx] is not None:
                    sums[date_key][m] += float(vals[idx])
                    counts[date_key][m] += 1

        forecast = []
        for date_key in sorted(sums.keys()):
            def day_avg(m):
                c = counts[date_key][m]
                return (sums[date_key][m] / c) if c else 0

            label = classify_hour_weather(
                day_avg('cloud_cover'),
                day_avg('rain'),
                day_avg('showers'),
                day_avg('snowfall'),
                daily_code_by_date.get(date_key),
            )
            max_t = daily_max_by_date.get(date_key)
            temp = round(max_t) if max_t is not None else round(day_avg('temperature_2m'))
            forecast.append({
                'day': datetime.strptime(date_key, '%Y-%m-%d').strftime('%a'),
                'weather': weather_display(label),
                'weather_icon': weather_icon(label),
                'temperature': temp,
            })

        return {
            'city': city,
            'current': current_section,
            'forecast': forecast,
        }, 200

    except Exception as e:
        return {'error': f'Error fetching weather data: {str(e)}'}, 500
