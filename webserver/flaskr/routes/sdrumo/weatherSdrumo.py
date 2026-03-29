from flask import (
    Blueprint, flash, g, redirect, render_template, request, session, url_for
)

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

def classify_hour_weather(cloud_cover, rain, showers, snowfall):
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
                    'days': dates,
                    'temperature_avg': [],
                    'apparent_temperature_avg': [],
                    'cloud_cover_avg': [],
                    'weather': []
                }

                for date_key in dates:
                    day_rain_avg = None
                    day_showers_avg = None
                    day_snowfall_avg = None
                    day_cloud_cover_avg = None

                    for metric, out_key in [
                        ('temperature_2m', 'temperature_avg'),
                        ('apparent_temperature', 'apparent_temperature_avg'),
                        ('cloud_cover', 'cloud_cover_avg')
                    ]:
                        count = counts[date_key][metric]
                        avg = (sums[date_key][metric] / count) if count else None
                        daily_averages[out_key].append(avg)

                    for metric in ['rain', 'showers', 'snowfall', 'cloud_cover']:
                        count = counts[date_key][metric]
                        avg = (sums[date_key][metric] / count) if count else None
                        if metric == 'rain':
                            day_rain_avg = avg
                        elif metric == 'showers':
                            day_showers_avg = avg
                        elif metric == 'snowfall':
                            day_snowfall_avg = avg
                        elif metric == 'cloud_cover':
                            day_cloud_cover_avg = avg

                    daily_averages['weather'].append(
                        classify_hour_weather(
                            day_cloud_cover_avg,
                            day_rain_avg,
                            day_showers_avg,
                            day_snowfall_avg,
                        )
                    )
            all_hourly_times = data['hourly']['time']
            all_hourly_temperature = data['hourly']['temperature_2m']
            all_hourly_apparent_temperature = data['hourly']['apparent_temperature']
            all_hourly_rain = data['hourly']['rain']
            all_hourly_showers = data['hourly']['showers']
            all_hourly_snowfall = data['hourly']['snowfall']
            all_hourly_cloud_cover = data['hourly']['cloud_cover']

            current_time = data['current_weather']['time']
            current_idx = get_current_hour_index(current_time, all_hourly_times)
            current_weather_label = classify_hour_weather(
                all_hourly_cloud_cover[current_idx] if current_idx < len(all_hourly_cloud_cover) else 0,
                all_hourly_rain[current_idx] if current_idx < len(all_hourly_rain) else 0,
                all_hourly_showers[current_idx] if current_idx < len(all_hourly_showers) else 0,
                all_hourly_snowfall[current_idx] if current_idx < len(all_hourly_snowfall) else 0,
            )

            # current weather without interval, weathercode, winddirection
            current_weather = {
                'is_day': data['current_weather']['is_day'],
                'temperature': data['current_weather']['temperature'],
                'time': current_time,
                'weather': current_weather_label,
            }
            slice_start = min(current_idx + 1, len(all_hourly_times))
            slice_end = slice_start + 3
            hourly_times = all_hourly_times[slice_start:slice_end]
            hourly_temperature = all_hourly_temperature[slice_start:slice_end]
            hourly_apparent_temperature = all_hourly_apparent_temperature[slice_start:slice_end]
            hourly_rain = all_hourly_rain[slice_start:slice_end]
            hourly_showers = all_hourly_showers[slice_start:slice_end]
            hourly_snowfall = all_hourly_snowfall[slice_start:slice_end]
            hourly_cloud_cover = all_hourly_cloud_cover[slice_start:slice_end]

            hourly_weather = []
            for idx in range(len(hourly_times)):
                hourly_weather.append(
                    classify_hour_weather(
                        hourly_cloud_cover[idx] if idx < len(hourly_cloud_cover) else 0,
                        hourly_rain[idx] if idx < len(hourly_rain) else 0,
                        hourly_showers[idx] if idx < len(hourly_showers) else 0,
                        hourly_snowfall[idx] if idx < len(hourly_snowfall) else 0,
                    )
                )

            response = {
                'current_weather': current_weather,
                # hourly for the next 3 hours
                'hourly_today': {
                    'time': hourly_times,
                    'temperature': hourly_temperature,
                    'apparent_temperature': hourly_apparent_temperature,
                    'weather': hourly_weather
                },
                # daily averages for the next days (excluding today)
                'next_days': daily_averages
            }
            return response, 200
        else:
            return {'error': 'Failed to fetch weather data'}, response.status_code
    except Exception as e:
        return {'error': f'Error fetching weather data: {str(e)}'}, 500
