import requests
from requests.auth import HTTPBasicAuth

# Configuration for Trentino Trasporti API
tt_base_url = 'https://app-tpl.tndigit.it/gtlservice/'
tt_basic_auth = HTTPBasicAuth('mittmobile','ecGsp.RHB3')

response = requests.get(tt_base_url + 'routes', auth=tt_basic_auth)

for route in response.json():
    with open("busses.txt", "a", encoding="utf-8") as f:
        f.write(f"({route['routeId']}, '{route['routeShortName']}', '{route['routeLongName']}', '{route['routeColor']}'),\n")