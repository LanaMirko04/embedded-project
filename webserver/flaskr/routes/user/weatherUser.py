from flask import (
    Blueprint, flash, g, redirect, render_template, request, session, url_for
)
from flask_jwt_extended import jwt_required, get_jwt_identity
import geopy
from flaskr.db import get_db

bp = Blueprint('weather', __name__)

@bp.route('/getLocation', methods=['GET'])
@jwt_required()
def get_location():
    db = get_db()
    user = db.execute(
        'SELECT location, location_latitude, location_longitude FROM users WHERE id = ?',
        (get_jwt_identity(),)
    ).fetchone()
    
    if user is None:
        return {'error': 'User not found'}, 404
    
    return {
        'location': user['location'],
        'latitude': user['location_latitude'],
        'longitude': user['location_longitude']
    }, 200

@bp.route('/clearLocation', methods=['POST'])
@jwt_required()
def clear_location():
    try:
        db = get_db()
        db.execute(
            'UPDATE users SET location = NULL, location_latitude = NULL, location_longitude = NULL WHERE id = ?',
            (get_jwt_identity(),)
        )
        db.commit()
        return {'message': 'Location cleared successfully'}, 200
    except Exception as e:
        return {'error': f'Database error: {str(e)}'}, 500