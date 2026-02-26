from flask import (
    Blueprint, flash, g, redirect, render_template, request, session, url_for
)

import requests

from flaskr.db import get_db

bp = Blueprint('weatherSdrumo', __name__)

weather_api_url = 'https://api.open-meteo.com/v1/forecast'

@bp.route('/getWeather/<token>', methods=['GET'])
def get_weather(token):

    db = get_db()

    lat = db.execute(
        'SELECT location_latitude FROM sdrumos WHERE token = ?',
        (token,)
    ).fetchone()
    lon = db.execute(
        'SELECT location_longitude FROM sdrumos WHERE token = ?',
        (token,)
    ).fetchone()

    if not lat or not lon:
        return {'error': 'Latitude and longitude are required'}, 400

    params = {
        'latitude': lat,
        'longitude': lon,
        'hourly': 'temperature_2m,apparent_temperature,rain,showers,snowfall,cloud_cover',
        'models': 'italia_meteo_arpae_icon_2i',
        'current_weather': 'true',
        'precipitation': 'true',
        'rain': 'true',
        'showers': 'true',
        'snowfall': 'true',
        'wind_speed_10m': 'true',
        'timezone': 'Europe/Berlin',
        'forecast_days': 3,
        'daily': 'temperature_2m_max,temperature_2m_min,rain_sum,showers_sum,snowfall_sum,precipitation_hours'
    }

    try:
        response = requests.get(weather_api_url, params=params)
        if response.status_code == 200:
            data = response.json()
            hourly = data.get('hourly', {})
            hourly_times = hourly.get('time', [])

            daily_averages = {}
            if hourly_times:
                today_date = hourly_times[0].split('T')[0]
                sums = {}
                counts = {}

                metrics = [
                    'temperature_2m',
                    'apparent_temperature',
                    'rain',
                    'showers',
                    'snowfall',
                    'cloud_cover'
                ]

                for idx, time_str in enumerate(hourly_times):
                    date_key = time_str.split('T')[0]
                    if date_key == today_date:
                        continue

                    if date_key not in sums:
                        sums[date_key] = {metric: 0.0 for metric in metrics}
                        counts[date_key] = {metric: 0 for metric in metrics}

                    for metric in metrics:
                        values = hourly.get(metric, [])
                        if idx < len(values) and values[idx] is not None:
                            sums[date_key][metric] += float(values[idx])
                            counts[date_key][metric] += 1

                dates = sorted(sums.keys())
                daily_averages = {
                    'time': dates,
                    'temperature_2m_avg': [],
                    'apparent_temperature_avg': [],
                    'rain_avg': [],
                    'showers_avg': [],
                    'snowfall_avg': [],
                    'cloud_cover_avg': []
                }

                for date_key in dates:
                    for metric, out_key in [
                        ('temperature_2m', 'temperature_2m_avg'),
                        ('apparent_temperature', 'apparent_temperature_avg'),
                        ('rain', 'rain_avg'),
                        ('showers', 'showers_avg'),
                        ('snowfall', 'snowfall_avg'),
                        ('cloud_cover', 'cloud_cover_avg')
                    ]:
                        count = counts[date_key][metric]
                        avg = (sums[date_key][metric] / count) if count else None
                        daily_averages[out_key].append(avg)
            # current weather without interval, weathercode, winddirection
            current_weather = {
                'is_day': data['current_weather']['is_day'],
                'temperature': data['current_weather']['temperature'],
                'time': data['current_weather']['time'],
            }
            response = {
                'current_weather': current_weather,
                # hourly for the next 3 hours
                'hourly_today': {
                    'time': data['hourly']['time'][:3],
                    'temperature_2m': data['hourly']['temperature_2m'][:3],
                    'apparent_temperature': data['hourly']['apparent_temperature'][:3],
                    'rain': data['hourly']['rain'][:3],
                    'showers': data['hourly']['showers'][:3],
                    'snowfall': data['hourly']['snowfall'][:3],
                    'cloud_cover': data['hourly']['cloud_cover'][:3]
                },
                # daily averages for the next days (excluding today)
                'next_days': daily_averages
            }
            return response, 200
        else:
            return {'error': 'Failed to fetch weather data'}, response.status_code
    except Exception as e:
        return {'error': f'Error fetching weather data: {str(e)}'}, 500
