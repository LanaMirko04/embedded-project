import 'package:dio/dio.dart';
import 'dart:async';

import '../models/user.dart';
import '../models/sdrumo.dart';
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

  Future<User> fetchUserProfile(String userId, {String? bearerToken}) async {
    final options = Options(headers: {});
    if (bearerToken != null && bearerToken.isNotEmpty) {
      options.headers!['Authorization'] = 'Bearer $bearerToken';
    }
    final response = await _dio.get('/me', options: options);
    print(bearerToken);
    print(response.data);
    return User.fromJson(response.data);
  }

  // Accept a Map payload for data
  Future<Map<String, dynamic>> addPhrase(dynamic phraseOrData, {String? bearerToken}) async {
    final Map<String, dynamic> data = phraseOrData is Map<String, dynamic>
        ? Map<String, dynamic>.from(phraseOrData)
        : phraseOrData.toJson();

    final options = Options(headers: {});
    if (bearerToken != null && bearerToken.isNotEmpty) {
      options.headers!['Authorization'] = 'Bearer $bearerToken';
    }

    final response = await _dio.post(
      '/addPhrase', // adjust endpoint if different
      data: data,
      options: options,
    );

    // Normalize common response envelopes:
    // { data: {...} } | { phrase: {...} } | { item: {...} } | {...} | [ {...} ]
    dynamic payload = response.data;
    Map<String, dynamic> serverMap = {};

    if (payload is Map<String, dynamic>) {
      // unwrap common envelopes if present
      if (payload.containsKey('data') && payload['data'] is Map) {
        serverMap = Map<String, dynamic>.from(payload['data']);
      } else if (payload.containsKey('phrase') && payload['phrase'] is Map) {
        serverMap = Map<String, dynamic>.from(payload['phrase']);
      } else if (payload.containsKey('item') && payload['item'] is Map) {
        serverMap = Map<String, dynamic>.from(payload['item']);
      } else if (payload.containsKey('result') && payload['result'] is Map) {
        serverMap = Map<String, dynamic>.from(payload['result']);
      } else {
        serverMap = Map<String, dynamic>.from(payload);
      }
    } else if (payload is List && payload.isNotEmpty && payload.first is Map) {
      serverMap = Map<String, dynamic>.from(payload.first);
    } else {
      // payload is not a map/list (could be id or message). Try to salvage id.
      if (payload != null) {
        serverMap = {'id': payload.toString()};
      }
    }

    // Merge serverMap with the original request data:
    // keep original request values as fallback when server didn't return them,
    // but prefer server values when available.
    final merged = <String, dynamic>{};
    merged.addAll(data); // request values as fallback
    serverMap.forEach((k, v) {
      if (v != null && v.toString().isNotEmpty)
        merged[k] = v; // override with server values when present
    });

    // Ensure there's at least content in the merged map; if not, try to read common text fields
    if ((merged['content'] == null ||
            merged['content'].toString().trim().isEmpty) &&
        serverMap.isNotEmpty) {
      // try common alternatives
      merged['content'] =
          serverMap['text'] ??
          serverMap['body'] ??
          serverMap['phrase'] ??
          merged['content'];
    }

    return merged;
  }

  Future<Map<String, dynamic>> updatePhrase(
    String id,
    dynamic phraseOrData, {
    String? bearerToken,
  }) async {
    final Map<String, dynamic> data = phraseOrData is Map<String, dynamic>
        ? Map<String, dynamic>.from(phraseOrData)
        : phraseOrData.toJson();

    final options = Options(headers: {});
    if (bearerToken != null && bearerToken.isNotEmpty) {
      options.headers!['Authorization'] = 'Bearer $bearerToken';
    }

    final response = await _dio.put(
      '/updatePhrase/$id',
      data: data,
      options: options,
    );

    // Normalize common response envelopes
    dynamic payload = response.data;
    Map<String, dynamic> serverMap = {};

    if (payload is Map<String, dynamic>) {
      if (payload.containsKey('data') && payload['data'] is Map) {
        serverMap = Map<String, dynamic>.from(payload['data']);
      } else if (payload.containsKey('phrase') && payload['phrase'] is Map) {
        serverMap = Map<String, dynamic>.from(payload['phrase']);
      } else if (payload.containsKey('item') && payload['item'] is Map) {
        serverMap = Map<String, dynamic>.from(payload['item']);
      } else if (payload.containsKey('result') && payload['result'] is Map) {
        serverMap = Map<String, dynamic>.from(payload['result']);
      } else {
        serverMap = Map<String, dynamic>.from(payload);
      }
    } else if (payload is List && payload.isNotEmpty && payload.first is Map) {
      serverMap = Map<String, dynamic>.from(payload.first);
    }

    final merged = <String, dynamic>{};
    merged.addAll(data);
    serverMap.forEach((k, v) {
      if (v != null && v.toString().isNotEmpty) merged[k] = v;
    });

    if ((merged['content'] == null ||
            merged['content'].toString().trim().isEmpty) &&
        serverMap.isNotEmpty) {
      merged['content'] =
          serverMap['text'] ??
          serverMap['body'] ??
          serverMap['phrase'] ??
          merged['content'];
    }

    return merged;
  }

  Future<void> deletePhrase(String id, {String? bearerToken}) async {
    final options = Options(headers: {});
    if (bearerToken != null && bearerToken.isNotEmpty) {
      options.headers!['Authorization'] = 'Bearer $bearerToken';
    }

    await _dio.delete('/deletePhrase/$id', options: options);
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

}

