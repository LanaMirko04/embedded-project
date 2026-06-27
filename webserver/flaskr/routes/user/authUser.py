import functools
import os

from flask import (
    Blueprint, flash, g, redirect, render_template, request, session, url_for
)
from werkzeug.security import check_password_hash, generate_password_hash

from flaskr.db import get_db

import bcrypt

from datetime import datetime, timedelta
import secrets
import hashlib

bp = Blueprint('auth', __name__)

# User Registration
@bp.route('/register', methods=['GET', 'POST'])
def register():
    if request.method == 'POST':
        payload = request.get_json(silent=True) or {}
        username = payload.get('username') or request.form.get('username')
        email = payload.get('email') or request.form.get('email')
        password = payload.get('password') or request.form.get('password')
        db = get_db()

        if not username:
            return {'error': 'Username is required.'}, 400
        elif not email:
            return {'error': 'Email is required.'}, 400
        elif not password:
            return {'error': 'Password is required.'}, 400
        elif db.execute(
            'SELECT id FROM users WHERE username = ?', (username,)
        ).fetchone() is not None:
            return {'error': f'User {username} is already registered.'}, 400
        elif db.execute(
            'SELECT id FROM users WHERE email = ?', (email,)
        ).fetchone() is not None:
            return {'error': f'Email {email} is already registered.'}, 400

        # Force a portable hash method to avoid relying on hashlib.scrypt availability
        hashed_password = generate_password_hash(password, method='pbkdf2:sha256')
        db.execute(
            'INSERT INTO users (username, email, password) VALUES (?, ?, ?)',
            (username, email, hashed_password)
        )
        db.commit()
        return {'message': 'User registered'}, 201

    return {'message': 'Register endpoint'}, 200

# User Login
@bp.route('/login', methods=['GET', 'POST'])
def login():
    if request.method == 'POST':
        payload = request.get_json(silent=True) or {}
        username = payload.get('username') or request.form.get('username')
        email = payload.get('email') or request.form.get('email')
        password = payload.get('password') or request.form.get('password')
        db = get_db()

        if email:
            user = db.execute(
                'SELECT * FROM users WHERE email = ?', (email,)
            ).fetchone()
        else:
            user = db.execute(
                'SELECT * FROM users WHERE username = ?', (username,)
            ).fetchone()

        if user is None:
            return {'error': 'Incorrect username or email.'}, 400
        elif not check_password_hash(user['password'], password):
            return {'error': 'Incorrect password.'}, 400

        from flask_jwt_extended import create_access_token, create_refresh_token, decode_token 
        user_id = str(user['id'])
        access_token = create_access_token(identity=user_id)
        refresh_token = create_refresh_token(identity=user_id)

        decoded_refresh = decode_token(refresh_token)
        jti = decoded_refresh['jti']

        db.execute(
            'UPDATE refresh_tokens SET revoked = 1 WHERE user_id = ?', (user['id'],)
        )
        db.commit()
        db.execute(
            'INSERT INTO refresh_tokens (user_id, jti, expires_at) VALUES (?, ?, ?)',
            (user['id'], jti, datetime.utcnow() + timedelta(days=int(os.getenv('REFRESH_TOKEN_EXPIRES_DAYS', 30))))
        )  
        db.commit()

        return { 'access_token': access_token, 'refresh_token': refresh_token }, 200

# Token Refresh
@bp.route('/refresh', methods=['POST'])
def refresh():
    from flask_jwt_extended import create_access_token, decode_token
    from jwt import InvalidTokenError

    payload = request.get_json(silent=True) or {}
    refresh_token = payload.get('refresh_token')
    if not refresh_token:
        return {'error': 'Refresh token is required.'}, 400

    try:
        decoded = decode_token(refresh_token)
    except (InvalidTokenError, Exception):
        return {'error': 'Invalid refresh token.'}, 401

    if decoded.get('type') != 'refresh':
        return {'error': 'Invalid refresh token.'}, 401

    jti = decoded.get('jti')
    user_id = decoded.get('sub')

    db = get_db()
    row = db.execute(
        'SELECT revoked, expires_at FROM refresh_tokens WHERE jti = ?', (jti,)
    ).fetchone()

    if row is None or row['revoked']:
        return {'error': 'Refresh token revoked or unknown.'}, 401

    try:
        expires_at = datetime.fromisoformat(row['expires_at'])
    except (TypeError, ValueError):
        expires_at = None
    if expires_at is not None and expires_at < datetime.utcnow():
        return {'error': 'Refresh token expired.'}, 401

    access_token = create_access_token(identity=user_id)
    return {'access_token': access_token, 'refresh_token': refresh_token}, 200

# User Logout
@bp.route('/logout', methods=['POST'])
def logout():
    from flask_jwt_extended import verify_jwt_in_request, get_jwt_identity
    try:
        verify_jwt_in_request(optional=True)
        user_id = get_jwt_identity()
        if user_id:
            db = get_db()
            db.execute('UPDATE refresh_tokens SET revoked = 1 WHERE user_id = ?', (user_id,))
            db.commit()
    except Exception:
        pass
    return {'message': 'Logged out'}, 200