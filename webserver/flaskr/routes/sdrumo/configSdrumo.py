from flask import Blueprint
from flaskr.db import get_db
import sqlite3

bp = Blueprint('configSdrumo', __name__)

@bp.route('/getConfig/<string:device_token>', methods=['GET'])
def get_device_config(device_token):
    try:
        db = get_db()
        row = db.execute(
            "SELECT stop_id, COALESCE(CAST(strftime('%s', updated_at) AS INTEGER), 0) AS cfg_rev "
            "FROM sdrumos WHERE token = ?",
            (device_token,)
        ).fetchone()
        if not row:
            return {'error': 'Device not found'}, 404
        return {
            'cfg_rev': row['cfg_rev'],
            'stop_id': row['stop_id'],
        }, 200
    except sqlite3.Error as e:
        return {'error': f'Database error: {str(e)}'}, 500

@bp.route('/getStops/<string:device_token>', methods=['GET'])
def get_device_stops(device_token):
    try:
        db = get_db()
        sdrumo = db.execute(
            'SELECT id FROM sdrumos WHERE token = ?', (device_token,)
        ).fetchone()
        if not sdrumo:
            return {'error': 'Device not found'}, 404
        rows = db.execute(
            'SELECT stop_id, stop_name FROM sdrumo_stops WHERE sdrumo_id = ? ORDER BY id',
            (sdrumo['id'],)
        ).fetchall()
        return [{'stop_id': r['stop_id'], 'stop_name': r['stop_name']} for r in rows], 200
    except sqlite3.Error as e:
        return {'error': f'Database error: {str(e)}'}, 500
