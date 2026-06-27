class Sdrump {
  final int id;
  final String name;
  final String? token;

  Sdrump({
    required this.id,
    required this.name,
    this.token,
  });

  factory Sdrump.fromJson(Map<String, dynamic> json) {
    return Sdrump(
      id: json['id'] as int,
      name: json['name'] as String,
      token: json['token'] as String?,
    );
  }

  Map<String, dynamic> toJson() {
    return {
      'id': id,
      'name': name,
      if (token != null) 'token': token,
    };
  }
}
