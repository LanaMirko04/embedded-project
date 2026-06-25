from flask import Blueprint
from flask_jwt_extended import jwt_required, get_jwt_identity

from flaskr.db import get_db

bp = Blueprint('info', __name__)

# Get Username
@bp.route('/getUsername', methods=['GET'])
@jwt_required()
def getUsername():
    try:
        user_id = get_jwt_identity()
        db = get_db()
        user = db.execute(
            'SELECT id, username FROM users WHERE id = ?', (user_id,)
        ).fetchone()
        
        if user is None:
            return {'error': 'User not found.'}, 404
        
        return {'username': user['username']}, 200
    except Exception as e:
        return {'error': str(e)}, 500

# Get Sdrumos associated with the user
@bp.route('/getSdrumos', methods=['GET'])
@jwt_required()
def getSdrumos():
    try:
        user_id = get_jwt_identity()
        db = get_db()
        sdrumos = db.execute(
            'SELECT id, name, token FROM sdrumos WHERE user_id = ?', (user_id,)
        ).fetchall()
        
        sdrumo_list = [{'id': sdrumo['id'], 'name': sdrumo['name'], 'token': sdrumo['token']} for sdrumo in sdrumos]
        
        return {'sdrumos': sdrumo_list}, 200
    except Exception as e:
        return {'error': str(e)}, 500