from flaskr.db import get_db
import functools
import requests
from requests.auth import HTTPBasicAuth
from datetime import datetime, timezone
import json
from dotenv import load_dotenv
import os

from flask import (
    Blueprint, flash, g, redirect, render_template, request, session, url_for
)

bp = Blueprint('busSdrumo', __name__)

# Configuration for Trentino Trasporti API
tt_base_url = os.getenv('TT_BASE_URL', 'https://app-tpl.tndigit.it/gtlservice/')
tt_basic_auth = HTTPBasicAuth(os.getenv('TT_BASIC_AUTH_USER'), os.getenv('TT_BASIC_AUTH_PASS'))

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
                'delay': trip['delay'],
                'arrivalTime': trip['oraArrivoEffettivaAFermataSelezionata']
            })
        return response, 200
    else:
        return {'error': 'Failed to fetch trips'}, tt_response.status_code

@bp.route('/getTrips/<int:stopId>/<int:routeId>', methods=['GET'])
def get_trips_by_route(stopId, routeId):
    response = get_trips(stopId)
    if response[1] == 200:
        trips = response[0]
        filtered_trips = [trip for trip in trips if trip['busId'] == routeId]
        return filtered_trips, 200
    else:
        return {'error': 'Failed to fetch trips'}, response[1]

@bp.route('/getSdrumoTrips/<string:sdrumo_token>', methods=['GET'])
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

    for bus in bus_ids:
        response = get_trips_by_route(stop_id, bus['bus_id'])
        if response[1] == 200:
            return response
    return {'error': 'No trips found for this Sdrumo'}, 404