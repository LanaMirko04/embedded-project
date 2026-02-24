import sqlite3
from flask import Blueprint, request
from flask_jwt_extended import jwt_required, get_jwt_identity
from flaskr.db import get_db

bp = Blueprint('sdrumoManagement', __name__)

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
        return {'message': 'Sdrumo name changed successfully'}, 200
        
    except sqlite3.Error as e:
        return {'error': f'Database error: {str(e)}'}, 500
    except Exception as e:
        return {'error': f'Changing name failed: {str(e)}'}, 500