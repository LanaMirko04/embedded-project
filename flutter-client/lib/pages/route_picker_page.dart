import 'package:flutter/material.dart';
import '../models/bus_route.dart';
import '../services/api_service.dart';

/// Searchable list of bus routes. Pops the selected [BusRoute].
/// [excludeIds] hides routes already subscribed.
class RoutePickerPage extends StatefulWidget {
  final Set<int> excludeIds;

  const RoutePickerPage({super.key, this.excludeIds = const {}});

  @override
  State<RoutePickerPage> createState() => _RoutePickerPageState();
}

class _RoutePickerPageState extends State<RoutePickerPage> {
  final _searchCtrl = TextEditingController();
  List<BusRoute> _all = [];
  List<BusRoute> _filtered = [];
  bool _loading = true;
  String? _error;

  @override
  void initState() {
    super.initState();
    _load();
    _searchCtrl.addListener(_applyFilter);
  }

  @override
  void dispose() {
    _searchCtrl.removeListener(_applyFilter);
    _searchCtrl.dispose();
    super.dispose();
  }

  Future<void> _load() async {
    final routes = await ApiService().fetchRoutes();
    if (!mounted) return;
    final available =
        routes.where((r) => !widget.excludeIds.contains(r.id)).toList();
    setState(() {
      _all = available;
      _filtered = available;
      _loading = false;
      _error = available.isEmpty ? 'No routes available' : null;
    });
  }

  void _applyFilter() {
    final q = _searchCtrl.text.trim().toLowerCase();
    setState(() {
      if (q.isEmpty) {
        _filtered = _all;
      } else {
        _filtered = _all.where((r) {
          return r.busNumber.toLowerCase().contains(q) ||
              r.busName.toLowerCase().contains(q);
        }).toList();
      }
    });
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: const Text('Add Bus Line')),
      body: Column(
        children: [
          Padding(
            padding: const EdgeInsets.all(12),
            child: TextField(
              controller: _searchCtrl,
              decoration: const InputDecoration(
                prefixIcon: Icon(Icons.search),
                hintText: 'Search by line number or name',
              ),
            ),
          ),
          Expanded(
            child: _loading
                ? const Center(child: CircularProgressIndicator())
                : _error != null
                    ? Center(child: Text(_error!))
                    : ListView.builder(
                        itemCount: _filtered.length,
                        itemBuilder: (context, index) {
                          final route = _filtered[index];
                          final color = route.displayColor;
                          return ListTile(
                            leading: CircleAvatar(
                              backgroundColor: color,
                              child: Text(
                                route.busNumber,
                                style: TextStyle(
                                  fontSize: 12,
                                  color: color == null ? null : Colors.white,
                                ),
                              ),
                            ),
                            title: Text(route.busName),
                            onTap: () => Navigator.pop(context, route),
                          );
                        },
                      ),
          ),
        ],
      ),
    );
  }
}
