class BusStop {
  final int id;
  final String name;
  final double? lat;
  final double? lon;
  final List<String> busses;

  BusStop({
    required this.id,
    required this.name,
    this.lat,
    this.lon,
    this.busses = const [],
  });

  factory BusStop.fromJson(Map<String, dynamic> json) {
    final position = json['position'];
    double? lat;
    double? lon;
    if (position is Map) {
      lat = (position['lat'] as num?)?.toDouble();
      lon = (position['lon'] as num?)?.toDouble();
    }
    final rawBusses = json['busses'];
    return BusStop(
      id: json['id'] as int,
      name: (json['name'] ?? '').toString(),
      lat: lat,
      lon: lon,
      busses: rawBusses is List
          ? rawBusses.map((e) => e.toString()).toList()
          : const [],
    );
  }
}
