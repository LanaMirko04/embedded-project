import sqlite3
from flask import Blueprint, request
from flask_jwt_extended import jwt_required, get_jwt_identity
from flaskr.db import get_db

import geopy

bp = Blueprint('sdrumoConfig', __name__)

def updateTimestamp(token):
    db = get_db()
    db.execute(
        'UPDATE sdrumos SET updated_at = CURRENT_TIMESTAMP WHERE token = ?',
        (token,)
    )
    db.commit()

@bp.route('/get/<token>', methods=['GET'])
@jwt_required()
def get_sdrumo_config(token):
    try:
        user_id = get_jwt_identity()
        db = get_db()

        sdrumo = db.execute(
            'SELECT id, name, token, location, location_latitude, location_longitude, stop_id '
            'FROM sdrumos WHERE token = ? AND user_id = ?',
            (token, user_id)
        ).fetchone()

        if not sdrumo:
            return {'error': 'Sdrumo token not found or not paired with this user'}, 404

        busses = db.execute(
            'SELECT b.id, b.bus_number, b.bus_name, b.color '
            'FROM sdrumo_busses sb JOIN busses b ON b.id = sb.bus_id '
            'WHERE sb.sdrumo_id = ?',
            (sdrumo['id'],)
        ).fetchall()

        return {
            'id': sdrumo['id'],
            'name': sdrumo['name'],
            'token': sdrumo['token'],
            'location': sdrumo['location'],
            'location_latitude': sdrumo['location_latitude'],
            'location_longitude': sdrumo['location_longitude'],
            'stop_id': sdrumo['stop_id'],
            'busses': [
                {
                    'id': bus['id'],
                    'bus_number': bus['bus_number'],
                    'bus_name': bus['bus_name'],
                    'color': bus['color'],
                }
                for bus in busses
            ],
        }, 200

    except sqlite3.Error as e:
        return {'error': f'Database error: {str(e)}'}, 500
    except Exception as e:
        return {'error': f'Fetching config failed: {str(e)}'}, 500

@bp.route('/pair', methods=['POST'])
@jwt_required()
def pair_sdrumo():
    try:
        data = request.get_json(silent=True) or {}
        token = data.get('token')
        
        if not token:
            return {'error': 'Token is required'}, 400
        
        user_id = get_jwt_identity()
        db = get_db()
        
        # Check if sdrumo exists
        sdrumo = db.execute(
            'SELECT id FROM sdrumos WHERE token = ? AND user_id IS NULL',
            (token,)
        ).fetchone()
        
        if not sdrumo:
            return {'error': 'Sdrumo token not found or already paired'}, 404
        
        db.execute(
            'UPDATE sdrumos SET user_id = ? WHERE token = ?',
            (user_id, token)
        )
        db.commit()

        updateTimestamp(token)
        return {'message': 'Sdrumo paired successfully'}, 200
        
    except sqlite3.Error as e:
        return {'error': f'Database error: {str(e)}'}, 500
    except Exception as e:
        return {'error': f'Pairing failed: {str(e)}'}, 500

@bp.route('/unpair', methods=['POST'])
@jwt_required()
def unpair_sdrumo():
    try:
        data = request.get_json(silent=True) or {}
        token = data.get('token')
        
        if not token:
            return {'error': 'Token is required'}, 400
        
        user_id = get_jwt_identity()
        db = get_db()
        
        # Check if sdrumo is paired with the user
        sdrumo = db.execute(
            'SELECT id FROM sdrumos WHERE token = ? AND user_id = ?',
            (token, user_id)
        ).fetchone()
        
        if not sdrumo:
            return {'error': 'Sdrumo token not found or not paired with this user'}, 404
        
        db.execute(
            'UPDATE sdrumos SET user_id = NULL WHERE token = ?',
            (token,)
        )
        db.commit()
        updateTimestamp(token)
        return {'message': 'Sdrumo unpaired successfully'}, 200
        
    except sqlite3.Error as e:
        return {'error': f'Database error: {str(e)}'}, 500
    except Exception as e:
        return {'error': f'Unpairing failed: {str(e)}'}, 500

@bp.route('/changeName', methods=['POST'])
@jwt_required()
def change_sdrumo_name():
    try:
        data = request.get_json(silent=True) or {}
        token = data.get('token')
        new_name = data.get('name')
        
        if not token or not new_name:
            return {'error': 'Token and name are required'}, 400
        
        user_id = get_jwt_identity()
        db = get_db()
        
        # Check if sdrumo is paired with the user
        sdrumo = db.execute(
            'SELECT id FROM sdrumos WHERE token = ? AND user_id = ?',
            (token, user_id)
        ).fetchone()
        
        if not sdrumo:
            return {'error': 'Sdrumo token not found or not paired with this user'}, 404
        
        db.execute(
            'UPDATE sdrumos SET name = ? WHERE token = ?',
            (new_name, token)
        )
        db.commit()
        updateTimestamp(token)
        return {'message': 'Sdrumo name changed successfully'}, 200
        
    except sqlite3.Error as e:
        return {'error': f'Database error: {str(e)}'}, 500
    except Exception as e:
        return {'error': f'Changing name failed: {str(e)}'}, 500

@bp.route('/setLocation', methods=['POST'])
@jwt_required()
def set_sdrumo_location():
    try:
        data = request.get_json(silent=True) or {}
        token = data.get('token')
        location = data.get('location')
        
        if not token or not location:
            return {'error': 'Token and location are required'}, 400
        
        user_id = get_jwt_identity()
        db = get_db()
        
        # Check if sdrumo is paired with the user
        sdrumo = db.execute(
            'SELECT id FROM sdrumos WHERE token = ? AND user_id = ?',
            (token, user_id)
        ).fetchone()
        
        if not sdrumo:
            return {'error': 'Sdrumo token not found or not paired with this user'}, 404
        
        geolocator = geopy.Nominatim(user_agent="sdrumo_app")
        loc = geolocator.geocode(location)
        
        if loc is None:
            return {'error': 'Location not found'}, 404
        
        db.execute(
            'UPDATE sdrumos SET location = ?, location_latitude = ?, location_longitude = ? WHERE token = ?',
            (location, loc.latitude, loc.longitude, token)
        )
        db.commit()
        updateTimestamp(token)
        return {'message': 'Sdrumo location updated successfully'}, 200
        
    except sqlite3.Error as e:
        return {'error': f'Database error: {str(e)}'}, 500
    except Exception as e:
        return {'error': f'Setting location failed: {str(e)}'}, 500

@bp.route('/clearLocation', methods=['POST'])
@jwt_required()
def clear_sdrumo_location():
    try:
        data = request.get_json(silent=True) or {}
        token = data.get('token')
        
        if not token:
            return {'error': 'Token is required'}, 400
        
        user_id = get_jwt_identity()
        db = get_db()
        
        # Check if sdrumo is paired with the user
        sdrumo = db.execute(
            'SELECT id FROM sdrumos WHERE token = ? AND user_id = ?',
            (token, user_id)
        ).fetchone()
        
        if not sdrumo:
            return {'error': 'Sdrumo token not found or not paired with this user'}, 404
        
        db.execute(
            'UPDATE sdrumos SET location = NULL, location_latitude = NULL, location_longitude = NULL WHERE token = ?',
            (token,)
        )
        db.commit()
        updateTimestamp(token)
        return {'message': 'Sdrumo location cleared successfully'}, 200
        
    except sqlite3.Error as e:
        return {'error': f'Database error: {str(e)}'}, 500
    except Exception as e:
        return {'error': f'Clearing location failed: {str(e)}'}, 500

@bp.route('/setStop', methods=['POST'])
@jwt_required()
def set_sdrumo_stop():
    try:
        data = request.get_json(silent=True) or {}
        token = data.get('token')
        stop_id = data.get('stop_id')
        
        if not token or not stop_id:
            return {'error': 'Token and stop_id are required'}, 400
        
        user_id = get_jwt_identity()
        db = get_db()
        
        # Check if sdrumo is paired with the user
        sdrumo = db.execute(
            'SELECT id FROM sdrumos WHERE token = ? AND user_id = ?',
            (token, user_id)
        ).fetchone()
        
        if not sdrumo:
            return {'error': 'Sdrumo token not found or not paired with this user'}, 404
        
        db.execute(
            'UPDATE sdrumos SET stop_id = ? WHERE token = ?',
            (stop_id, token)
        )
        db.commit()
        updateTimestamp(token)
        return {'message': 'Sdrumo stop updated successfully'}, 200
        
    except sqlite3.Error as e:
        return {'error': f'Database error: {str(e)}'}, 500
    except Exception as e:
        return {'error': f'Setting stop failed: {str(e)}'}, 500

@bp.route('/clearStop', methods=['POST'])
@jwt_required()
def clear_sdrumo_stop():
    try:
        data = request.get_json(silent=True) or {}
        token = data.get('token')
        
        if not token:
            return {'error': 'Token is required'}, 400
        
        user_id = get_jwt_identity()
        db = get_db()
        
        # Check if sdrumo is paired with the user
        sdrumo = db.execute(
            'SELECT id FROM sdrumos WHERE token = ? AND user_id = ?',
            (token, user_id)
        ).fetchone()
        
        if not sdrumo:
            return {'error': 'Sdrumo token not found or not paired with this user'}, 404
        
        db.execute(
            'UPDATE sdrumos SET stop_id = NULL WHERE token = ?',
            (token,)
        )
        db.commit()
        updateTimestamp(token)
        return {'message': 'Sdrumo stop cleared successfully'}, 200
        
    except sqlite3.Error as e:
        return {'error': f'Database error: {str(e)}'}, 500
    except Exception as e:
        return {'error': f'Clearing stop failed: {str(e)}'}, 500

@bp.route('/addBus', methods=['POST'])
@jwt_required()
def add_sdrumo_bus():
    try:
        data = request.get_json(silent=True) or {}
        token = data.get('token')
        bus_id = data.get('bus_id')
        
        if not token or not bus_id:
            return {'error': 'Token and bus_id are required'}, 400
        
        user_id = get_jwt_identity()
        db = get_db()
        
        # Check if sdrumo is paired with the user
        sdrumo = db.execute(
            'SELECT id FROM sdrumos WHERE token = ? AND user_id = ?',
            (token, user_id)
        ).fetchone()
        
        if not sdrumo:
            return {'error': 'Sdrumo token not found or not paired with this user'}, 404
        
        existing = db.execute(
            'SELECT id FROM sdrumo_busses WHERE sdrumo_id = ? AND bus_id = ?',
            (sdrumo['id'], bus_id)
        ).fetchone()
        if existing:
            return {'message': 'Bus already added to Sdrumo'}, 200

        db.execute(
            'INSERT INTO sdrumo_busses (sdrumo_id, bus_id) VALUES (?, ?)',
            (sdrumo['id'], bus_id)
        )
        db.commit()
        updateTimestamp(token)
        return {'message': 'Bus added to Sdrumo successfully'}, 200
        
    except sqlite3.Error as e:
        return {'error': f'Database error: {str(e)}'}, 500
    except Exception as e:
        return {'error': f'Adding bus failed: {str(e)}'}, 500

@bp.route('/removeBus', methods=['POST'])
@jwt_required()
def remove_sdrumo_bus():
    try:
        data = request.get_json(silent=True) or {}
        token = data.get('token')
        bus_id = data.get('bus_id')
        
        if not token or not bus_id:
            return {'error': 'Token and bus_id are required'}, 400
        
        user_id = get_jwt_identity()
        db = get_db()
        
        # Check if sdrumo is paired with the user
        sdrumo = db.execute(
            'SELECT id FROM sdrumos WHERE token = ? AND user_id = ?',
            (token, user_id)
        ).fetchone()
        
        if not sdrumo:
            return {'error': 'Sdrumo token not found or not paired with this user'}, 404
        
        db.execute(
            'DELETE FROM sdrumo_busses WHERE sdrumo_id = ? AND bus_id = ?',
            (sdrumo['id'], bus_id)
        )
        db.commit()
        updateTimestamp(token)
        return {'message': 'Bus removed from Sdrumo successfully'}, 200
        
    except sqlite3.Error as e:
        return {'error': f'Database error: {str(e)}'}, 500
    except Exception as e:
        return {'error': f'Removing bus failed: {str(e)}'}, 500