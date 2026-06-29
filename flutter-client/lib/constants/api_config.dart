const baseUrl = 'http://sdrumo.zagatti.me';

class ApiConfig {
  static const String apiBaseUrl = '$baseUrl/api';

  static const String registerPath = '/user/auth/register';
  static const String loginPath = '/user/auth/login';
  static const String refreshPath = '/user/auth/refresh';

  static const String getUsernamePath = '/user/info/getUsername';
  static const String getSdrumpsPath = '/user/info/getSdrumos';

  static const String getStopsPath = '/user/bus/getStops';
  static const String getRoutesPath = '/user/bus/getRoutes';

  static const String changeDeviceNamePath = '/config/changeName';
  static const String unpairDevicePath = '/config/unpair';
  static const String pairDevicePath = '/config/pair';
  static const String getDeviceConfigPath = '/config/get';
  static const String setLocationPath = '/config/setLocation';
  static const String clearLocationPath = '/config/clearLocation';
  static const String addStopPath = '/config/addStop';
  static const String removeStopPath = '/config/removeStop';
  static const String addBusPath = '/config/addBus';
  static const String removeBusPath = '/config/removeBus';

  static const String statusPath = '/status';
}
