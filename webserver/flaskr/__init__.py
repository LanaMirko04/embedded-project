import os
from flask import Flask
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
        JWT_SECRET_KEY=os.getenv('JWT_SECRET_KEY') or app.config['SECRET_KEY'],
        JWT_ACCESS_TOKEN_EXPIRES=timedelta(minutes=int(os.getenv('ACCESS_TOKEN_EXPIRES_MINUTES', 1000))),
        JWT_REFRESH_TOKEN_EXPIRES=timedelta(days=int(os.getenv('REFRESH_TOKEN_EXPIRES_DAYS', 30))),
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

    from . import db
    db.init_app(app)

    from .routes import status

    from .routes.user import authUser
    from .routes.user import busUser
    from .routes.user import infoUser
    from .routes.user import weatherUser

    from .routes.sdrumo import authSdrumo
    from .routes import sdrumoConfig
    from .routes.sdrumo import busSdrumo
    from .routes.sdrumo import weatherSdrumo
    from .routes.sdrumo import configSdrumo

    app.register_blueprint(status.bp, url_prefix='/api/status')

    app.register_blueprint(authUser.bp, url_prefix='/api/user/auth')
    app.register_blueprint(busUser.bp, url_prefix='/api/user/bus')
    app.register_blueprint(infoUser.bp, url_prefix='/api/user/info')
    app.register_blueprint(weatherUser.bp, url_prefix='/api/user/weather')

    app.register_blueprint(authSdrumo.bp, url_prefix='/api/sdrumo/auth')
    app.register_blueprint(sdrumoConfig.bp, url_prefix='/api/config')
    app.register_blueprint(busSdrumo.bp, url_prefix='/api/sdrumo/bus')
    app.register_blueprint(weatherSdrumo.bp, url_prefix='/api/sdrumo/weather')
    app.register_blueprint(configSdrumo.bp, url_prefix='/api/sdrumo/config')

    return app

app = create_app()