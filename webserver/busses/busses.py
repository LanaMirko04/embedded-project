import requests
from requests.auth import HTTPBasicAuth
import os
from dotenv import load_dotenv

load_dotenv()

# Configuration for Trentino Trasporti API
tt_base_url = os.getenv('TT_BASE_URL', 'https://app-tpl.tndigit.it/gtlservice/')
tt_basic_auth = HTTPBasicAuth(os.getenv('TT_BASIC_AUTH_USER'), os.getenv('TT_BASIC_AUTH_PASS'))

response = requests.get(tt_base_url + 'routes', auth=tt_basic_auth)

for route in response.json():
    with open("busses.txt", "a", encoding="utf-8") as f:
        f.write(f"({route['routeId']}, '{route['routeShortName']}', '{route['routeLongName']}', '{route['routeColor']}'),\n")