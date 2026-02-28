import 'package:flutter/material.dart';
import '../models/sdrumo.dart';
import '../services/api_service.dart';
import '../services/auth_service.dart';

class DevicePage extends StatefulWidget {
  final Sdrump device;

  const DevicePage({super.key, required this.device});

  @override
  State<DevicePage> createState() => _DevicePageState();
}

class _DevicePageState extends State<DevicePage> {
  late String _deviceName;
  late TextEditingController _nameController;
  bool _isEditing = false;
  bool _isSaving = false;

  @override
  void initState() {
    super.initState();
    _deviceName = widget.device.name;
    _nameController = TextEditingController(text: _deviceName);
  }

  @override
  void dispose() {
    _nameController.dispose();
    super.dispose();
  }

  Future<void> _saveName() async {
    final newName = _nameController.text.trim();
    if (newName.isEmpty || newName == _deviceName) {
      setState(() => _isEditing = false);
      return;
    }

    if (widget.device.token == null) {
      ScaffoldMessenger.of(context).showSnackBar(
        const SnackBar(content: Text('Device token not available')),
      );
      return;
    }

    setState(() => _isSaving = true);
    final success = await ApiService().changeDeviceName(
      widget.device.token!,
      newName,
    );
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
    if (widget.device.token == null) {
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

    if (!mounted) return;
    ScaffoldMessenger.of(context).showSnackBar(
      const SnackBar(content: Text('Removing device...')),
    );

    final success = await ApiService().unpairDevice(widget.device.token!);

    if (!mounted) return;

    if (success) {
      // Refresh the device list and return to homepage
      final freshDevices = await ApiService().fetchSdrumos() ?? [];
      AuthService().setSdrumos(freshDevices);
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
      body: Center(
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: [
            Text(
              'Device ID: ${widget.device.id}',
              style: Theme.of(context).textTheme.bodyMedium,
            ),
            const SizedBox(height: 32),
            const Text('Device controls and details will appear here'),
          ],
        ),
      ),
    );
  }
}
