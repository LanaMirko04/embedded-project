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
  late List<Sdrump> _sdrumos;
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

    final theme = Theme.of(context);

    if (_sdrumos.isEmpty) {
      return Center(
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: [
            Icon(
              Icons.devices_other_outlined,
              size: 48,
              color: theme.colorScheme.onSurfaceVariant,
            ),
            const SizedBox(height: 16),
            Text('No devices yet', style: theme.textTheme.titleMedium),
            const SizedBox(height: 24),
            FilledButton.icon(
              onPressed: _showAddDeviceDialog,
              icon: const Icon(Icons.add),
              label: const Text('Add New Device'),
            ),
          ],
        ),
      );
    }

    return ListView.separated(
      itemCount: _sdrumos.length + 1,
      padding: const EdgeInsets.all(16),
      separatorBuilder: (_, _) => const SizedBox(height: 12),
      itemBuilder: (context, index) {
        // Last item is the add button
        if (index == _sdrumos.length) {
          return Card(
            child: ListTile(
              onTap: _showAddDeviceDialog,
              leading: CircleAvatar(
                backgroundColor: theme.colorScheme.primaryContainer,
                foregroundColor: theme.colorScheme.onPrimaryContainer,
                child: const Icon(Icons.add),
              ),
              title: const Text('Add New Device'),
            ),
          );
        }

        final device = _sdrumos[index];
        return Card(
          child: ListTile(
            contentPadding:
                const EdgeInsets.symmetric(horizontal: 16, vertical: 8),
            leading: CircleAvatar(
              backgroundColor: theme.colorScheme.secondaryContainer,
              foregroundColor: theme.colorScheme.onSecondaryContainer,
              child: const Icon(Icons.alarm),
            ),
            title: Text(device.name, style: theme.textTheme.titleMedium),
            subtitle: Text(
              device.token ?? 'N/A',
              maxLines: 1,
              overflow: TextOverflow.ellipsis,
              style: theme.textTheme.bodySmall?.copyWith(
                color: theme.colorScheme.onSurfaceVariant,
              ),
            ),
            trailing: const Icon(Icons.chevron_right),
            onTap: () {
              Navigator.pushNamed(context, '/device', arguments: device);
            },
          ),
        );
      },
    );
  }
}
