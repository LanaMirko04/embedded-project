import 'bus_route.dart';

class DeviceConfig {
  final int id;
  final String name;
  final String? token;
  final String? location;
  final double? latitude;
  final double? longitude;
  final int? stopId;
  final List<BusRoute> busses;

  DeviceConfig({
    required this.id,
    required this.name,
    this.token,
    this.location,
    this.latitude,
    this.longitude,
    this.stopId,
    this.busses = const [],
  });

  factory DeviceConfig.fromJson(Map<String, dynamic> json) {
    final rawBusses = json['busses'];
    return DeviceConfig(
      id: json['id'] as int,
      name: (json['name'] ?? '').toString(),
      token: json['token']?.toString(),
      location: json['location']?.toString(),
      latitude: (json['location_latitude'] as num?)?.toDouble(),
      longitude: (json['location_longitude'] as num?)?.toDouble(),
      stopId: json['stop_id'] as int?,
      busses: rawBusses is List
          ? rawBusses
              .map((e) => BusRoute.fromJson(e as Map<String, dynamic>))
              .toList()
          : const [],
    );
  }
}
