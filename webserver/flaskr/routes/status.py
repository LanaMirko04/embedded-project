from flask import Blueprint, request

bp = Blueprint('status', __name__)

# Status endpoint
@bp.route('/', methods=['GET'])
def status():
    return {'status': 'ok'}, 200