from flask import (
    Blueprint, flash, g, redirect, render_template, request, session, url_for
)
from flask_jwt_extended import jwt_required, get_jwt_identity

from faker import Faker
import random
import sqlite3

from flaskr.db import get_db

def anon_name():
    fake = Faker()

    color = fake.color_name().lower().replace(" ", "")
    word = fake.word()
    num = random.randint(1000, 9999)
    return f"{color}-{word}-{num}"

bp = Blueprint('sdrumoAuth', __name__)

@bp.route('/register' , methods=['POST'])
def register_sdrumo():
    try:
        token = anon_name()
        db = get_db()
        db.execute(
            'INSERT INTO sdrumos (token, name) VALUES (?, ?)',
            (token, token)
        )
        db.commit()
        return {'token': token}, 201
    except sqlite3.Error as e:
        return {'error': f'Database error: {str(e)}'}, 500
    except Exception as e:
        return {'error': f'Registration failed: {str(e)}'}, 500

