from flask import (
    Blueprint, flash, g, redirect, render_template, request, session, url_for
)

from faker import Faker
import random

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
    token = anon_name()
    db = get_db()
    db.execute(
        'INSERT INTO sdrumos (token, name) VALUES (?, ?)',
        (token, token)
    )
    db.commit()
    return {'token': token}, 201
