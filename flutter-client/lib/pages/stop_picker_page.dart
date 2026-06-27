import 'package:flutter/material.dart';
import '../models/bus_stop.dart';
import '../services/api_service.dart';

/// Searchable list of Trentino Trasporti stops. Pops the selected [BusStop].
class StopPickerPage extends StatefulWidget {
  const StopPickerPage({super.key});

  @override
  State<StopPickerPage> createState() => _StopPickerPageState();
}

class _StopPickerPageState extends State<StopPickerPage> {
  final _searchCtrl = TextEditingController();
  List<BusStop> _all = [];
  List<BusStop> _filtered = [];
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
    final stops = await ApiService().fetchStops();
    if (!mounted) return;
    setState(() {
      _all = stops;
      _filtered = stops;
      _loading = false;
      _error = stops.isEmpty ? 'No stops available' : null;
    });
  }

  void _applyFilter() {
    final q = _searchCtrl.text.trim().toLowerCase();
    setState(() {
      if (q.isEmpty) {
        _filtered = _all;
      } else {
        _filtered = _all.where((s) {
          final inName = s.name.toLowerCase().contains(q);
          final inLines = s.busses.any((b) => b.toLowerCase().contains(q));
          return inName || inLines;
        }).toList();
      }
    });
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: const Text('Select Bus Stop')),
      body: Column(
        children: [
          Padding(
            padding: const EdgeInsets.all(12),
            child: TextField(
              controller: _searchCtrl,
              decoration: const InputDecoration(
                prefixIcon: Icon(Icons.search),
                hintText: 'Search by stop name or line',
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
                          final stop = _filtered[index];
                          return ListTile(
                            title: Text(stop.name),
                            subtitle: stop.busses.isEmpty
                                ? null
                                : Text('Lines: ${stop.busses.join(', ')}'),
                            onTap: () => Navigator.pop(context, stop),
                          );
                        },
                      ),
          ),
        ],
      ),
    );
  }
}
