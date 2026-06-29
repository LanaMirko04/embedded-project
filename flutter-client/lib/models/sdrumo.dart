class Sdrumo {
  final int id;
  final String name;
  final String? token;

  Sdrumo({
    required this.id,
    required this.name,
    this.token,
  });

  factory Sdrumo.fromJson(Map<String, dynamic> json) {
    return Sdrumo(
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
