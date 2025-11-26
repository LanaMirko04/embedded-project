from flaskr.db import get_db
import functools
import requests
from requests.auth import HTTPBasicAuth
from datetime import datetime, timezone
import json

from flask import (
    Blueprint, flash, g, redirect, render_template, request, session, url_for
)
from werkzeug.security import check_password_hash, generate_password_hash

bp = Blueprint('api', __name__, url_prefix='/api')

# Configuration for Trentino Trasporti API
tt_base_url = 'https://app-tpl.tndigit.it/gtlservice/'
tt_basic_auth = HTTPBasicAuth('mittmobile','ecGsp.RHB3')

# Status endpoint
@bp.route('/status', methods=['GET'])
def status():
    return {'status': 'ok'}, 200

# Trentino Trasporti

# Bus Stops endpoint
@bp.route('/bus/getStops', methods=['GET'])
def get_stops():
    params = {'type': 'U'}
    tt_response = requests.get(tt_base_url + 'stops', auth=tt_basic_auth, params=params)
    if tt_response.status_code == 200:
        response = []
        for stop in tt_response.json():
            busList = []
            for bus in stop['routes']:
                busList.append(bus['routeShortName'])
            response.append({'id': stop['stopId'], 'name': stop['stopName'], 'position': {'lat': stop['stopLat'], 'lon': stop['stopLon']}, 'buses': busList})
        return response, 200
    else:
        return {'error': 'Failed to fetch stops'}, tt_response.status_code
    
@bp.route('/bus/getStop/<int:stop_id>', methods=['GET'])
def get_stop(stop_id):
    params = {'type': 'U'}
    tt_response = requests.get(tt_base_url + 'stops', auth=tt_basic_auth, params=params)
    if tt_response.status_code == 200:
        for stop in tt_response.json():
            if stop['stopId'] == stop_id:
                busList = []
                for bus in stop['routes']:
                    busList.append(bus['routeShortName'])
                response = {'id': stop['stopId'], 'name': stop['stopName'], 'position': {'lat': stop['stopLat'], 'lon': stop['stopLon']}, 'buses': busList}
                return response, 200
        return {'error': 'Stop not found'}, 404
    else:
        return {'error': 'Failed to fetch stops'}, tt_response.status_code

# Bus Routes endpoint
@bp.route('/bus/getRoutes', methods=['GET'])
def get_routes():
    db = get_db()
    routes = db.execute('SELECT id, bus_number, bus_name, color FROM busses').fetchall()
    response = []
    for route in routes:
        response.append({
            'id': route['id'],
            'bus_number': route['bus_number'],
            'bus_name': route['bus_name'],
            'color': route['color']
        })
    return response, 200

@bp.route('/bus/getRoute/<int:route_id>', methods=['GET'])
def get_route(route_id):
    db = get_db()
    route = db.execute(
        'SELECT id, bus_number, bus_name, color FROM busses WHERE id = ?',
        (route_id,)
    ).fetchone()

    if route is None:
        return {'error': 'Route not found'}, 404

    response = {
        'id': route['id'],
        'bus_number': route['bus_number'],
        'bus_name': route['bus_name'],
        'color': route['color']
    }
    return response, 200
    
# Bus Trips Endpoint
@bp.route('/bus/getTrips/<int:stopId>', methods=['GET'])
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