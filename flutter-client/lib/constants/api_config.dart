//const baseUrl = 'https://api.example.com';
//const baseUrl = 'http://127.0.0.1:5000'; // iOS simulator / web
const baseUrl = 'http://10.0.2.2:5000'; // Android emulator

class ApiConfig {
  // Main API base URL
  static const String apiBaseUrl = '$baseUrl/api';

  // Auth endpoints (using /api/user)
  static const String registerPath = '/user/auth/register';
  static const String loginPath = '/user/auth/login';
  static const String refreshPath = '/user/auth/refresh';

  // User info endpoints
  static const String getUsernamePath = '/user/info/getUsername';
  static const String getSdrumpsPath = '/user/info/getSdrumos';

  // Bus lookup endpoints (populate pickers)
  static const String getStopsPath = '/user/bus/getStops';
  static const String getRoutesPath = '/user/bus/getRoutes';

  // Config endpoints
  static const String changeDeviceNamePath = '/config/changeName';
  static const String unpairDevicePath = '/config/unpair';
  static const String pairDevicePath = '/config/pair';
  static const String getDeviceConfigPath = '/config/get'; // append /<token>
  static const String setLocationPath = '/config/setLocation';
  static const String clearLocationPath = '/config/clearLocation';
  static const String setStopPath = '/config/setStop';
  static const String clearStopPath = '/config/clearStop';
  static const String addBusPath = '/config/addBus';
  static const String removeBusPath = '/config/removeBus';

  // Health/Status endpoint
  static const String statusPath = '/status';
}