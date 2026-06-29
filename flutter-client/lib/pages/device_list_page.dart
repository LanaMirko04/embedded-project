import 'package:flutter/material.dart';
import '../models/sdrumo.dart';
import '../services/auth_service.dart';
import '../services/api_service.dart';

class DeviceListPage extends StatefulWidget {
  const DeviceListPage({super.key});

  @override
  State<DeviceListPage> createState() => _DeviceListPageState();
}

class _DeviceListPageState extends State<DeviceListPage> {
  late List<Sdrumo> _sdrumos;
  bool _isLoading = false;

  @override
  void initState() {
    super.initState();
    _sdrumos = AuthService().sdrumos;
    AuthService().addListener(_onSdrumosChanged);
    _loadDevices();
  }

  @override
  void dispose() {
    AuthService().removeListener(_onSdrumosChanged);
    super.dispose();
  }

  void _onSdrumosChanged() {
    if (mounted) {
      setState(() => _sdrumos = AuthService().sdrumos);
    }
  }

  Future<void> _loadDevices() async {
    if (_sdrumos.isNotEmpty) {
      return; // Already loaded
    }
    
    setState(() => _isLoading = true);
    try {
      final sdrumos = await ApiService().fetchSdrumos();
      if (mounted) {
        setState(() {
          _sdrumos = sdrumos ?? [];
          _isLoading = false;
        });
      }
    } catch (e) {
      if (mounted) {
        setState(() => _isLoading = false);
        ScaffoldMessenger.of(context).showSnackBar(
          SnackBar(content: Text('Error loading devices: $e')),
        );
      }
    }
  }

  Future<void> _showAddDeviceDialog() async {
    final tokenController = TextEditingController();

    final token = await showDialog<String>(
      context: context,
      builder: (context) => AlertDialog(
        title: const Text('Pair New Device'),
        content: TextField(
          controller: tokenController,
          decoration: const InputDecoration(
            labelText: 'Device Token',
            hintText: 'Enter the device token',
          ),
          autofocus: true,
        ),
        actions: [
          TextButton(
            onPressed: () => Navigator.pop(context),
            child: const Text('Cancel'),
          ),
          TextButton(
            onPressed: () => Navigator.pop(context, tokenController.text.trim()),
            child: const Text('Pair'),
          ),
        ],
      ),
    );

    if (token == null || token.isEmpty) return;

    final error = await ApiService().pairDevice(token);

    if (!mounted) return;

    if (error == null) {
      final freshDevices = await ApiService().fetchSdrumos() ?? [];
      if (!mounted) return;
      AuthService().setSdrumos(freshDevices);
      setState(() {
        _sdrumos = freshDevices;
      });
      ScaffoldMessenger.of(context).showSnackBar(
        const SnackBar(content: Text('Device paired successfully')),
      );
    } else {
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(content: Text(error)),
      );
    }
  }

  @override
  Widget build(BuildContext context) {
    if (_isLoading) {
      return const Center(child: CircularProgressIndicator());
    }

    if (_sdrumos.isEmpty) {
      return Center(
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: [
            const Text('No devices found'),
            const SizedBox(height: 24),
            ElevatedButton.icon(
              onPressed: _showAddDeviceDialog,
              icon: const Icon(Icons.add),
              label: const Text('Add New Device'),
            ),
          ],
        ),
      );
    }

    return ListView.builder(
      itemCount: _sdrumos.length + 1,
      padding: const EdgeInsets.all(16),
      itemBuilder: (context, index) {
        // Last item is the add button
        if (index == _sdrumos.length) {
          return Padding(
            padding: const EdgeInsets.symmetric(vertical: 8),
            child: ElevatedButton.icon(
              onPressed: _showAddDeviceDialog,
              style: ElevatedButton.styleFrom(
                shape: RoundedRectangleBorder(
                  borderRadius: BorderRadius.circular(8),
                ),
              ),
              icon: const Icon(Icons.add),
              label: const Text('Add New Device'),
            ),
          );
        }

        final device = _sdrumos[index];
        return Padding(
          padding: const EdgeInsets.symmetric(vertical: 8),
          child: ElevatedButton(
            onPressed: () {
              // Navigate to device page
              Navigator.pushNamed(
                context,
                '/device',
                arguments: device,
              );
            },
            style: ElevatedButton.styleFrom(
              shape: RoundedRectangleBorder(
                borderRadius: BorderRadius.circular(8),
              ),
            ),
            child: Padding(
              padding: const EdgeInsets.symmetric(vertical: 16),
              child: Row(
                children: [
                  Expanded(
                    child: Column(
                      crossAxisAlignment: CrossAxisAlignment.start,
                      children: [
                        Text(
                          device.name,
                          style: Theme.of(context).textTheme.titleMedium,
                        ),
                        const SizedBox(height: 4),
                        Text(
                          device.token ?? 'N/A',
                          style: Theme.of(context).textTheme.bodySmall?.copyWith(
                            color: Colors.grey,
                          ),
                        ),
                      ],
                    ),
                  ),
                  const Icon(Icons.arrow_forward),
                ],
              ),
            ),
          ),
        );
      },
    );
  }
}
