import 'package:flutter/material.dart';

class BusRoute {
  final int id;
  final String busNumber;
  final String busName;
  final String? color;

  BusRoute({
    required this.id,
    required this.busNumber,
    required this.busName,
    this.color,
  });

  factory BusRoute.fromJson(Map<String, dynamic> json) {
    return BusRoute(
      id: json['id'] as int,
      busNumber: (json['bus_number'] ?? '').toString(),
      busName: (json['bus_name'] ?? '').toString(),
      color: json['color']?.toString(),
    );
  }

  /// Parses the stored hex string (e.g. "C52720") into a Color.
  /// Returns null when the color is unset or the sentinel "None".
  Color? get displayColor {
    final c = color;
    if (c == null || c.isEmpty || c.toLowerCase() == 'none') return null;
    final hex = c.replaceFirst('#', '');
    if (hex.length != 6) return null;
    final value = int.tryParse(hex, radix: 16);
    if (value == null) return null;
    return Color(0xFF000000 | value);
  }
}
