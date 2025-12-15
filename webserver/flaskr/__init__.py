import os
from flask import Flask, render_template
from flask_jwt_extended import (
    JWTManager, jwt_required, create_access_token, get_jwt_identity
)

from datetime import timedelta

def create_app(test_config=None):
    # create and configure the app
    app = Flask(__name__, instance_relative_config=True)
    app.config.from_mapping(
        SECRET_KEY='dev',
        DATABASE=os.path.join(app.instance_path, 'flaskr.sqlite'),
        JWT_SECRET_KEY=os.getenv('JWT_SECRET_KEY'),
        ACCESS_TOKEN_EXPIRES=timedelta(minutes=int(os.getenv('ACCESS_TOKEN_EXPIRES_MINUTES', 10))),
        REFRESH_TOKEN_EXPIRES=timedelta(days=int(os.getenv('REFRESH_TOKEN_EXPIRES_DAYS', 30))),
    )

    jwt = JWTManager(app)

    if test_config is None:
        # load the instance config, if it exists, when not testing
        app.config.from_pyfile('config.py', silent=True)
    else:
        # load the test config if passed in
        app.config.from_mapping(test_config)

    # ensure the instance folder exists
    try:
        os.makedirs(app.instance_path)
    except OSError:
        pass

    # a simple page that says hello
    @app.route('/')
    @app.route('/home')
    def home():
        return render_template('home.html')

    from . import db
    db.init_app(app)

    from . import status
    from . import authUser
    from . import busUser
    from . import authSdrumo
    app.register_blueprint(authUser.bp, url_prefix='/userAuth')
    app.register_blueprint(status.bp, url_prefix='/api/status')
    app.register_blueprint(busUser.bp, url_prefix='/api/user/bus')
    app.register_blueprint(authSdrumo.bp, url_prefix='/sdrumoAuth')

    return app
