//const baseUrl = 'https://api.example.com';
const baseUrl = 'http://127.0.0.1:5000';

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

  // Config endpoints
  static const String changeDeviceNamePath = '/config/changeName';
  static const String unpairDevicePath = '/config/unpair';
  static const String pairDevicePath = '/config/pair';

  // Health/Status endpoint
  static const String statusPath = '/status';
}