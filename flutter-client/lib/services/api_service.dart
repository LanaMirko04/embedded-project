import 'package:dio/dio.dart';
import 'dart:async';

import '../models/sdrumo.dart';
import '../models/bus_stop.dart';
import '../models/bus_route.dart';
import '../models/device_config.dart';
import '../constants/api_config.dart';
import 'auth_service.dart';

class ApiService {
  final Dio _dio = Dio(BaseOptions(baseUrl: ApiConfig.apiBaseUrl));

  // Single shared future while a refresh is in-flight
  Future<bool>? _refreshInProgress;

  ApiService() {
    // Add interceptor to handle 401 and refresh token
    _dio.interceptors.add(
      InterceptorsWrapper(
        onRequest: (options, handler) async {
          final token = AuthService().jwt;
          if (token != null) {
            options.headers['Authorization'] = 'Bearer $token';
          }
          return handler.next(options);
        },
        onError: (error, handler) async {
          final status = error.response?.statusCode;
          final alreadyRetried = error.requestOptions.extra['retried'] == true;

          // Only attempt refresh on 401 and if we haven't retried this request yet
          if (status == 401 && !alreadyRetried) {
            print('Got 401 error, attempting token refresh...');
            try {
              // If another refresh is in progress, wait for it instead of starting a new one
              _refreshInProgress ??= AuthService().refresh();
              final refreshed = await _refreshInProgress!;
              // clear the shared future after it completes (success or failure)
              _refreshInProgress = null;

              if (refreshed) {
                print('Token refresh succeeded, retrying original request');
                // mark request as retried so we don't loop
                error.requestOptions.extra['retried'] = true;
                // update auth header with new token
                final token = AuthService().jwt;
                if (token != null) {
                  error.requestOptions.headers['Authorization'] =
                      'Bearer $token';
                }
                // Retry the original request and resolve with its response
                final opts = Options(
                  method: error.requestOptions.method,
                  headers: error.requestOptions.headers,
                );
                final response = await _dio.request(
                  error.requestOptions.path,
                  data: error.requestOptions.data,
                  queryParameters: error.requestOptions.queryParameters,
                  options: opts,
                );
                return handler.resolve(response);
              } else {
                // refresh failed -> logout and forward original error
                print('Token refresh failed, logging out');
                await AuthService().logout();
                return handler.next(error);
              }
            } catch (e) {
              print('Error during token refresh: $e');
              _refreshInProgress = null;
              // something went wrong while refreshing -> forward original error
              return handler.next(error);
            }
          }

          return handler.next(error);
        },
      ),
    );
  }

  Future<Map<String, dynamic>> checkServerHealth() async {
    try {
      final response = await _dio
          .get(ApiConfig.statusPath)
          .timeout(
            const Duration(seconds: 5),
            onTimeout: () {
              throw TimeoutException('Server request timed out');
            },
          );
      return {
        'status': 'online',
        'statusCode': response.statusCode,
        'message': 'Server is responding',
      };
    } on DioException catch (e) {
      if (e.type == DioExceptionType.connectionTimeout ||
          e.type == DioExceptionType.receiveTimeout ||
          e.type == DioExceptionType.sendTimeout) {
        return {
          'status': 'timeout',
          'statusCode': null,
          'message': 'Connection timeout',
        };
      } else if (e.type == DioExceptionType.connectionError) {
        return {
          'status': 'offline',
          'statusCode': null,
          'message': 'Cannot connect to server',
        };
      } else if (e.response != null) {
        return {
          'status': 'error',
          'statusCode': e.response!.statusCode,
          'message': 'Server error: ${e.response!.statusCode}',
        };
      }
      return {
        'status': 'offline',
        'statusCode': null,
        'message': 'Server unreachable',
      };
    } on TimeoutException {
      return {
        'status': 'timeout',
        'statusCode': null,
        'message': 'Connection timeout',
      };
    } catch (e) {
      return {
        'status': 'error',
        'statusCode': null,
        'message': 'Unknown error: ${e.toString()}',
      };
    }
  }

  Future<String?> fetchUsername() async {
    try {
      final response = await _dio.get(ApiConfig.getUsernamePath);
      if (response.statusCode == 200 && response.data is Map) {
        final username = response.data['username']?.toString();
        if (username != null && username.isNotEmpty) {
          AuthService().setCurrentUserProfile(username: username);
          return username;
        }
      }
      return null;
    } catch (e) {
      print('fetchUsername error: $e');
      return null;
    }
  }

  Future<List<Sdrump>?> fetchSdrumos() async {
    try {
      final response = await _dio.get(ApiConfig.getSdrumpsPath);
      if (response.statusCode == 200 && response.data is Map) {
        // Handle wrapped response: { "sdrumos": [...] }
        List<dynamic> sdrumosList = response.data['sdrumos'] ?? [];
        final sdrumos = sdrumosList
            .map((json) => Sdrump.fromJson(json as Map<String, dynamic>))
            .toList();
        AuthService().setSdrumos(sdrumos);
        return sdrumos;
      }
      return null;
    } catch (e) {
      print('fetchSdrumos error: $e');
      return null;
    }
  }

  Future<bool> changeDeviceName(String deviceToken, String newName) async {
    try {
      final response = await _dio.post(
        ApiConfig.changeDeviceNamePath,
        data: {
          'token': deviceToken,
          'name': newName,
        },
      );
      return response.statusCode == 200;
    } catch (e) {
      print('changeDeviceName error: $e');
      return false;
    }
  }

  Future<bool> unpairDevice(String deviceToken) async {
    try {
      final response = await _dio.post(
        ApiConfig.unpairDevicePath,
        data: {
          'token': deviceToken,
        },
      );
      return response.statusCode == 200;
    } catch (e) {
      print('unpairDevice error: $e');
      return false;
    }
  }

  Future<bool> pairDevice(String deviceToken) async {
    try {
      final response = await _dio.post(
        ApiConfig.pairDevicePath,
        data: {
          'token': deviceToken,
        },
      );
      return response.statusCode == 200;
    } catch (e) {
      print('pairDevice error: $e');
      return false;
    }
  }

  // ---- Device configuration ----

  Future<DeviceConfig?> fetchDeviceConfig(String deviceToken) async {
    try {
      final response =
          await _dio.get('${ApiConfig.getDeviceConfigPath}/$deviceToken');
      if (response.statusCode == 200 && response.data is Map) {
        return DeviceConfig.fromJson(
          Map<String, dynamic>.from(response.data),
        );
      }
      return null;
    } catch (e) {
      print('fetchDeviceConfig error: $e');
      return null;
    }
  }

  Future<bool> setLocation(String deviceToken, String location) async {
    try {
      final response = await _dio.post(
        ApiConfig.setLocationPath,
        data: {'token': deviceToken, 'location': location},
      );
      return response.statusCode == 200;
    } catch (e) {
      print('setLocation error: $e');
      return false;
    }
  }

  Future<bool> clearLocation(String deviceToken) async {
    try {
      final response = await _dio.post(
        ApiConfig.clearLocationPath,
        data: {'token': deviceToken},
      );
      return response.statusCode == 200;
    } catch (e) {
      print('clearLocation error: $e');
      return false;
    }
  }

  Future<bool> setStop(String deviceToken, int stopId) async {
    try {
      final response = await _dio.post(
        ApiConfig.setStopPath,
        data: {'token': deviceToken, 'stop_id': stopId},
      );
      return response.statusCode == 200;
    } catch (e) {
      print('setStop error: $e');
      return false;
    }
  }

  Future<bool> clearStop(String deviceToken) async {
    try {
      final response = await _dio.post(
        ApiConfig.clearStopPath,
        data: {'token': deviceToken},
      );
      return response.statusCode == 200;
    } catch (e) {
      print('clearStop error: $e');
      return false;
    }
  }

  Future<bool> addBus(String deviceToken, int busId) async {
    try {
      final response = await _dio.post(
        ApiConfig.addBusPath,
        data: {'token': deviceToken, 'bus_id': busId},
      );
      return response.statusCode == 200;
    } catch (e) {
      print('addBus error: $e');
      return false;
    }
  }

  Future<bool> removeBus(String deviceToken, int busId) async {
    try {
      final response = await _dio.post(
        ApiConfig.removeBusPath,
        data: {'token': deviceToken, 'bus_id': busId},
      );
      return response.statusCode == 200;
    } catch (e) {
      print('removeBus error: $e');
      return false;
    }
  }

  // ---- Bus lookup (pickers) ----

  Future<List<BusStop>> fetchStops() async {
    try {
      final response = await _dio.get(ApiConfig.getStopsPath);
      if (response.statusCode == 200 && response.data is List) {
        return (response.data as List)
            .map((e) => BusStop.fromJson(e as Map<String, dynamic>))
            .toList();
      }
      return [];
    } catch (e) {
      print('fetchStops error: $e');
      return [];
    }
  }

  Future<List<BusRoute>> fetchRoutes() async {
    try {
      final response = await _dio.get(ApiConfig.getRoutesPath);
      if (response.statusCode == 200 && response.data is List) {
        return (response.data as List)
            .map((e) => BusRoute.fromJson(e as Map<String, dynamic>))
            .toList();
      }
      return [];
    } catch (e) {
      print('fetchRoutes error: $e');
      return [];
    }
  }
}

