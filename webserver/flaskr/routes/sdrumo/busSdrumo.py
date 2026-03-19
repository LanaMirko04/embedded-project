from flaskr.db import get_db
import functools
import requests
from requests.auth import HTTPBasicAuth
from datetime import datetime, timezone
import json
from dotenv import load_dotenv
import os

load_dotenv()

from flask import (
    Blueprint, flash, g, redirect, render_template, request, session, url_for
)

bp = Blueprint('busSdrumo', __name__)

# Configuration for Trentino Trasporti API
tt_base_url = os.getenv('TT_BASE_URL', 'https://app-tpl.tndigit.it/gtlservice/')
tt_basic_auth = HTTPBasicAuth(os.getenv('TT_BASIC_AUTH_USER'), os.getenv('TT_BASIC_AUTH_PASS'))

def get_number_from_id(bus_id):
    number = get_db().execute(
        "SELECT bus_number FROM busses WHERE id = ?",
        (bus_id,)    ).fetchone()
    return number['bus_number'] if number else None

def get_name_from_id(bus_id):
    name = get_db().execute(
        "SELECT bus_name FROM busses WHERE id = ?",
        (bus_id,)    ).fetchone()
    return name['bus_name'] if name else None

def get_color_from_id(bus_id):
    color = get_db().execute(
        "SELECT color FROM busses WHERE id = ?",
        (bus_id,)    ).fetchone()
    if color and color['color']:
        try:
            return hex_to_rgb(color['color'])
        except (ValueError, TypeError):
            # Invalid hex color, return None
            return None
    return None

def hex_to_rgb(hex_color: str):
    if not hex_color:
        raise ValueError("Hex color cannot be empty")
    hex_color = hex_color.strip().lstrip("#")
    if len(hex_color) == 3:  # short form like "f0a"
        hex_color = "".join(c * 2 for c in hex_color)
    if len(hex_color) != 6:
        raise ValueError("Hex color must be 3 or 6 characters long")
    return tuple(int(hex_color[i:i+2], 16) for i in (0, 2, 4))

# Bus Trips Endpoint
@bp.route('/getTrips/<int:stopId>', methods=['GET'])
def get_trips(stopId):

    params = {
        'stopId': stopId,
        'type': 'U',
        'limit': 10,
        'refDateTime': datetime.now(timezone.utc).strftime('%Y-%m-%dT%H:%M:%SZ')
    }

    print(params)

    tt_response = requests.get(tt_base_url + 'trips_new', auth=tt_basic_auth, params=params)

    if tt_response.status_code == 200:
        response = []
        for trip in tt_response.json():
            response.append({
                'busId': trip['routeId'],
                'busNumber': get_number_from_id(trip['routeId']),
                'busName': get_name_from_id(trip['routeId']),
                'busColor': get_color_from_id(trip['routeId']),
                'delay': trip['delay'],
                'originalArrivalTime': trip['oraArrivoProgrammataAFermataSelezionata'],
                'updatedArrivalTime': trip['oraArrivoEffettivaAFermataSelezionata']
            })
        return response, 200
    else:
        return {'error': 'Failed to fetch trips'}, tt_response.status_code

@bp.route('/getTrips/<string:sdrumo_token>', methods=['GET'])
def get_sdrumo_trips(sdrumo_token):
    sdrumo_id = get_db().execute(
        'SELECT id FROM sdrumos WHERE token = ?',
        (sdrumo_token,)
    ).fetchone()
    if not sdrumo_id:
        return {'error': 'Sdrumo not found'}, 404

    db = get_db()
    bus_ids = db.execute(
        'SELECT bus_id FROM sdrumo_busses WHERE sdrumo_id = ?',
        (sdrumo_id['id'],)
    ).fetchall()

    if not bus_ids:
        return {'error': 'Sdrumo not found'}, 404

    stop_id = db.execute(
        'SELECT stop_id FROM sdrumos WHERE token = ?',
        (sdrumo_token,)
    ).fetchone()['stop_id']

    trips = get_trips(stop_id)
    filtered_trips = []
    for trip in trips[0]:  # trips is a tuple (response, status_code
        if any(trip['busId'] == bus_id['bus_id'] for bus_id in bus_ids):
            filtered_trips.append(trip)
    if filtered_trips:
        return filtered_trips, 200
        
    return {'error': 'No trips found for this Sdrumo'}, 404