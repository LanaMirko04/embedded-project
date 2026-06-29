import 'package:flutter/material.dart';
import '../models/sdrumo.dart';
import '../models/bus_stop.dart';
import '../models/bus_route.dart';
import '../models/device_config.dart';
import '../services/api_service.dart';
import '../services/auth_service.dart';
import 'stop_picker_page.dart';
import 'route_picker_page.dart';

class DevicePage extends StatefulWidget {
  final Sdrumo device;

  const DevicePage({super.key, required this.device});

  @override
  State<DevicePage> createState() => _DevicePageState();
}

class _DevicePageState extends State<DevicePage> {
  late String _deviceName;
  late TextEditingController _nameController;
  bool _isEditing = false;
  bool _isSaving = false;

  DeviceConfig? _config;
  bool _loadingConfig = true;
  bool _busy = false;

  String? get _token => widget.device.token;

  @override
  void initState() {
    super.initState();
    _deviceName = widget.device.name;
    _nameController = TextEditingController(text: _deviceName);
    _loadConfig();
  }

  @override
  void dispose() {
    _nameController.dispose();
    super.dispose();
  }

  Future<void> _loadConfig() async {
    final token = _token;
    if (token == null) {
      setState(() => _loadingConfig = false);
      return;
    }
    final config = await ApiService().fetchDeviceConfig(token);
    if (!mounted) return;
    setState(() {
      _config = config;
      _loadingConfig = false;
    });
  }

  /// Runs a config mutation, then re-fetches config. Shows a snackbar on failure.
  Future<void> _runMutation(
    Future<bool> Function() action,
    String failureMessage,
  ) async {
    setState(() => _busy = true);
    final ok = await action();
    if (!mounted) return;
    if (ok) {
      await _loadConfig();
    }
    if (!mounted) return;
    setState(() => _busy = false);
    if (!ok) {
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(content: Text(failureMessage)),
      );
    }
  }

  Future<void> _saveName() async {
    final newName = _nameController.text.trim();
    if (newName.isEmpty || newName == _deviceName) {
      setState(() => _isEditing = false);
      return;
    }

    final token = _token;
    if (token == null) {
      ScaffoldMessenger.of(context).showSnackBar(
        const SnackBar(content: Text('Device token not available')),
      );
      return;
    }

    setState(() => _isSaving = true);
    final success = await ApiService().changeDeviceName(token, newName);
    if (!mounted) return;
    setState(() => _isSaving = false);

    if (success) {
      setState(() {
        _deviceName = newName;
        _isEditing = false;
      });
      ScaffoldMessenger.of(context).showSnackBar(
        const SnackBar(content: Text('Device name updated')),
      );
    } else {
      ScaffoldMessenger.of(context).showSnackBar(
        const SnackBar(content: Text('Failed to update device name')),
      );
    }
  }

  Future<void> _deleteDevice() async {
    final token = _token;
    if (token == null) {
      ScaffoldMessenger.of(context).showSnackBar(
        const SnackBar(content: Text('Device token not available')),
      );
      return;
    }

    final confirmed = await showDialog<bool>(
      context: context,
      builder: (context) => AlertDialog(
        title: const Text('Remove Device'),
        content: Text('Are you sure you want to remove "${widget.device.name}"?'),
        actions: [
          TextButton(
            onPressed: () => Navigator.pop(context, false),
            child: const Text('Cancel'),
          ),
          TextButton(
            onPressed: () => Navigator.pop(context, true),
            child: const Text('Remove', style: TextStyle(color: Colors.red)),
          ),
        ],
      ),
    );

    if (confirmed != true) return;

    final success = await ApiService().unpairDevice(token);
    if (!mounted) return;

    if (success) {
      // Refresh the device list and return to homepage
      final freshDevices = await ApiService().fetchSdrumos() ?? [];
      AuthService().setSdrumos(freshDevices);
      if (!mounted) return;
      Navigator.popUntil(context, (route) => route.isFirst);
      ScaffoldMessenger.of(context).showSnackBar(
        const SnackBar(content: Text('Device removed')),
      );
    } else {
      ScaffoldMessenger.of(context).showSnackBar(
        const SnackBar(content: Text('Failed to remove device')),
      );
    }
  }

  // ---- Location ----

  Future<void> _editLocation() async {
    final token = _token;
    if (token == null) return;
    final controller =
        TextEditingController(text: _config?.location ?? '');
    final location = await showDialog<String>(
      context: context,
      builder: (context) => AlertDialog(
        title: const Text('Set Location'),
        content: TextField(
          controller: controller,
          autofocus: true,
          decoration: const InputDecoration(
            labelText: 'City or address',
            hintText: 'e.g. Trento',
          ),
        ),
        actions: [
          TextButton(
            onPressed: () => Navigator.pop(context),
            child: const Text('Cancel'),
          ),
          TextButton(
            onPressed: () => Navigator.pop(context, controller.text.trim()),
            child: const Text('Save'),
          ),
        ],
      ),
    );
    if (location == null || location.isEmpty) return;
    await _runMutation(
      () => ApiService().setLocation(token, location),
      'Failed to set location (not found?)',
    );
  }

  Future<void> _clearLocation() async {
    final token = _token;
    if (token == null) return;
    await _runMutation(
      () => ApiService().clearLocation(token),
      'Failed to clear location',
    );
  }

  // ---- Bus stops ----

  Future<void> _addStop() async {
    final token = _token;
    if (token == null) return;
    final existing = (_config?.stops ?? []).map((s) => s.id).toSet();
    final stop = await Navigator.push<BusStop>(
      context,
      MaterialPageRoute(builder: (_) => const StopPickerPage()),
    );
    if (stop == null) return;
    if (existing.contains(stop.id)) return;
    await _runMutation(
      () => ApiService().addStop(token, stop.id, stop.name),
      'Failed to add stop',
    );
  }

  Future<void> _removeStop(BusStop stop) async {
    final token = _token;
    if (token == null) return;
    await _runMutation(
      () => ApiService().removeStop(token, stop.id),
      'Failed to remove stop',
    );
  }

  // ---- Bus lines ----

  Future<void> _addBus() async {
    final token = _token;
    if (token == null) return;
    final exclude = (_config?.busses ?? []).map((b) => b.id).toSet();
    final route = await Navigator.push<BusRoute>(
      context,
      MaterialPageRoute(
        builder: (_) => RoutePickerPage(excludeIds: exclude),
      ),
    );
    if (route == null) return;
    await _runMutation(
      () => ApiService().addBus(token, route.id),
      'Failed to add bus line',
    );
  }

  Future<void> _removeBus(BusRoute route) async {
    final token = _token;
    if (token == null) return;
    await _runMutation(
      () => ApiService().removeBus(token, route.id),
      'Failed to remove bus line',
    );
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Row(
          mainAxisSize: MainAxisSize.min,
          children: [
            _isEditing
                ? SizedBox(
                    width: 200,
                    child: TextField(
                      controller: _nameController,
                      autofocus: true,
                      decoration: const InputDecoration(
                        border: InputBorder.none,
                        hintText: 'Device name',
                      ),
                      onSubmitted: (_) => _saveName(),
                    ),
                  )
                : Text(_deviceName),
            const SizedBox(width: 8),
            if (_isEditing)
              _isSaving
                  ? const SizedBox(
                      width: 24,
                      height: 24,
                      child: CircularProgressIndicator(
                        strokeWidth: 2,
                        valueColor: AlwaysStoppedAnimation<Color>(Colors.white),
                      ),
                    )
                  : IconButton(
                      icon: const Icon(Icons.check),
                      onPressed: _saveName,
                      tooltip: 'Save',
                    )
            else
              IconButton(
                icon: const Icon(Icons.edit),
                onPressed: () {
                  setState(() => _isEditing = true);
                  _nameController.text = _deviceName;
                },
                tooltip: 'Edit name',
              ),
          ],
        ),
        actions: [
          IconButton(
            icon: const Icon(Icons.delete),
            onPressed: _deleteDevice,
            tooltip: 'Remove device',
          ),
        ],
      ),
      body: _loadingConfig
          ? const Center(child: CircularProgressIndicator())
          : RefreshIndicator(
              onRefresh: _loadConfig,
              child: ListView(
                padding: const EdgeInsets.all(16),
                children: [
                  if (_busy)
                    const Padding(
                      padding: EdgeInsets.only(bottom: 12),
                      child: LinearProgressIndicator(),
                    ),
                  _buildLocationCard(),
                  const SizedBox(height: 12),
                  _buildStopsCard(),
                  const SizedBox(height: 12),
                  _buildBusLinesCard(),
                ],
              ),
            ),
    );
  }

  Widget _buildLocationCard() {
    final location = _config?.location;
    return Card(
      child: Padding(
        padding: const EdgeInsets.all(16),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Row(
              children: [
                const Icon(Icons.location_on_outlined),
                const SizedBox(width: 8),
                Text('Location', style: Theme.of(context).textTheme.titleMedium),
              ],
            ),
            const SizedBox(height: 8),
            Text(location == null || location.isEmpty ? 'Not set' : location),
            const SizedBox(height: 8),
            Row(
              children: [
                TextButton.icon(
                  onPressed: _busy ? null : _editLocation,
                  icon: const Icon(Icons.edit),
                  label: Text(location == null ? 'Set' : 'Change'),
                ),
                if (location != null && location.isNotEmpty)
                  TextButton.icon(
                    onPressed: _busy ? null : _clearLocation,
                    icon: const Icon(Icons.clear),
                    label: const Text('Clear'),
                  ),
              ],
            ),
          ],
        ),
      ),
    );
  }

  Widget _buildStopsCard() {
    final stops = _config?.stops ?? [];
    return Card(
      child: Padding(
        padding: const EdgeInsets.all(16),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Row(
              children: [
                const Icon(Icons.directions_bus_outlined),
                const SizedBox(width: 8),
                Text('Bus Stops', style: Theme.of(context).textTheme.titleMedium),
                const Spacer(),
                IconButton(
                  onPressed: _busy ? null : _addStop,
                  icon: const Icon(Icons.add),
                  tooltip: 'Add stop',
                ),
              ],
            ),
            const SizedBox(height: 8),
            if (stops.isEmpty)
              const Text('No stops added')
            else
              Wrap(
                spacing: 8,
                runSpacing: 8,
                children: stops.map((stop) {
                  return Chip(
                    label: Text(stop.name),
                    onDeleted: _busy ? null : () => _removeStop(stop),
                  );
                }).toList(),
              ),
          ],
        ),
      ),
    );
  }

  Widget _buildBusLinesCard() {
    final busses = _config?.busses ?? [];
    return Card(
      child: Padding(
        padding: const EdgeInsets.all(16),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Row(
              children: [
                const Icon(Icons.route_outlined),
                const SizedBox(width: 8),
                Text('Bus Lines', style: Theme.of(context).textTheme.titleMedium),
                const Spacer(),
                IconButton(
                  onPressed: _busy ? null : _addBus,
                  icon: const Icon(Icons.add),
                  tooltip: 'Add line',
                ),
              ],
            ),
            const SizedBox(height: 8),
            if (busses.isEmpty)
              const Text('No lines added')
            else
              Wrap(
                spacing: 8,
                runSpacing: 8,
                children: busses.map((route) {
                  final color = route.displayColor;
                  return Chip(
                    avatar: CircleAvatar(
                      backgroundColor: color,
                      child: Text(
                        route.busNumber,
                        style: TextStyle(
                          fontSize: 10,
                          color: color == null ? null : Colors.white,
                        ),
                      ),
                    ),
                    label: Text(route.busName),
                    onDeleted: _busy ? null : () => _removeBus(route),
                  );
                }).toList(),
              ),
          ],
        ),
      ),
    );
  }
}
