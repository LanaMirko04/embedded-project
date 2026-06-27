import 'dart:async';
import 'dart:io';

import 'package:esp_smartconfig/esp_smartconfig.dart';
import 'package:flutter/material.dart';

class SmartConfigDialog extends StatefulWidget {
  const SmartConfigDialog({super.key});

  @override
  State<SmartConfigDialog> createState() => _SmartConfigDialogState();
}

class _SmartConfigDialogState extends State<SmartConfigDialog> {
  final _ssidCtrl = TextEditingController();
  final _bssidCtrl = TextEditingController();
  final _passwordCtrl = TextEditingController();

  bool _showPassword = false;
  bool _openNetwork = false;
  bool _running = false;

  Provisioner? _provisioner;
  Timer? _timeoutTimer;

  final List<ProvisioningResponse> _responses = [];
  String? _error;

  static const _timeoutSeconds = 60;

  @override
  void initState() {
    super.initState();
    _autoDetectWifi();
  }

  Future<void> _autoDetectWifi() async {
    try {
      final ssidResult = await Process.run('iwgetid', ['-r']);
      final ssid = (ssidResult.stdout as String).trim();
      if (ssid.isNotEmpty && mounted) _ssidCtrl.text = ssid;

      final bssidResult = await Process.run('iwgetid', ['-r', '-a']);
      final bssid = (bssidResult.stdout as String).trim();
      if (bssid.isNotEmpty && mounted) _bssidCtrl.text = bssid;
    } catch (_) {}
  }

  String? _validateBssid(String raw) {
    if (raw.isEmpty) return null;
    final parts = raw.split(':');
    if (parts.length != 6) return 'BSSID must be aa:bb:cc:dd:ee:ff';
    for (final p in parts) {
      if (p.length != 2 || int.tryParse(p, radix: 16) == null) {
        return 'BSSID must be aa:bb:cc:dd:ee:ff';
      }
    }
    return null;
  }

  Future<void> _start() async {
    final ssid = _ssidCtrl.text.trim();
    final bssidRaw = _bssidCtrl.text.trim();
    final password = _openNetwork ? null : _passwordCtrl.text;

    if (ssid.isEmpty) {
      setState(() => _error = 'SSID is required');
      return;
    }

    final bssidError = _validateBssid(bssidRaw);
    if (bssidError != null) {
      setState(() => _error = bssidError);
      return;
    }

    if (!_openNetwork && (password == null || password.length < 8)) {
      setState(() => _error = 'Password must be at least 8 characters');
      return;
    }

    setState(() {
      _running = true;
      _error = null;
      _responses.clear();
    });

    final provisioner = Provisioner.espTouchV2();
    _provisioner = provisioner;

    provisioner.listen(
      (response) {
        if (mounted) setState(() => _responses.add(response));
      },
      onError: (e) {
        if (mounted) setState(() => _error = e.toString());
        _stop();
      },
      onDone: () {
        if (mounted) setState(() => _running = false);
      },
    );

    try {
      await provisioner.start(ProvisioningRequest.fromStrings(
        ssid: ssid,
        bssid: bssidRaw.isEmpty ? '00:00:00:00:00:00' : bssidRaw,
        password: password,
      ));
    } catch (e) {
      if (mounted) setState(() => _error = e.toString());
      _stop();
      return;
    }

    _timeoutTimer = Timer(const Duration(seconds: _timeoutSeconds), () {
      if (_responses.isEmpty) {
        if (mounted) setState(() => _error = 'Timed out — no device responded');
      }
      _stop();
    });
  }

  void _stop() {
    _timeoutTimer?.cancel();
    _timeoutTimer = null;
    _provisioner?.stop();
    _provisioner = null;
    if (mounted) setState(() => _running = false);
  }

  @override
  void dispose() {
    _stop();
    _ssidCtrl.dispose();
    _bssidCtrl.dispose();
    _passwordCtrl.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    final theme = Theme.of(context);
    return AlertDialog(
      title: const Text('WiFi SmartConfig'),
      content: SizedBox(
        width: 420,
        child: SingleChildScrollView(
          child: Column(
            mainAxisSize: MainAxisSize.min,
            crossAxisAlignment: CrossAxisAlignment.stretch,
            children: [
              TextField(
                controller: _ssidCtrl,
                enabled: !_running,
                decoration: const InputDecoration(
                  labelText: 'WiFi SSID',
                  border: OutlineInputBorder(),
                ),
              ),
              const SizedBox(height: 12),
              TextField(
                controller: _bssidCtrl,
                enabled: !_running,
                decoration: const InputDecoration(
                  labelText: 'BSSID (optional)',
                  hintText: 'aa:bb:cc:dd:ee:ff',
                  border: OutlineInputBorder(),
                ),
              ),
              const SizedBox(height: 8),
              CheckboxListTile(
                value: _openNetwork,
                onChanged: _running
                    ? null
                    : (v) => setState(() => _openNetwork = v ?? false),
                title: const Text('Open network (no password)'),
                contentPadding: EdgeInsets.zero,
                dense: true,
              ),
              if (!_openNetwork) ...[
                const SizedBox(height: 4),
                TextField(
                  controller: _passwordCtrl,
                  enabled: !_running,
                  obscureText: !_showPassword,
                  decoration: InputDecoration(
                    labelText: 'WiFi Password',
                    border: const OutlineInputBorder(),
                    suffixIcon: IconButton(
                      icon: Icon(_showPassword
                          ? Icons.visibility_off
                          : Icons.visibility),
                      onPressed: () =>
                          setState(() => _showPassword = !_showPassword),
                    ),
                  ),
                ),
              ],
              if (_error != null) ...[
                const SizedBox(height: 12),
                Text(
                  _error!,
                  style: TextStyle(color: theme.colorScheme.error),
                ),
              ],
              if (_running) ...[
                const SizedBox(height: 16),
                const Row(
                  children: [
                    SizedBox(
                      width: 18,
                      height: 18,
                      child: CircularProgressIndicator(strokeWidth: 2),
                    ),
                    SizedBox(width: 12),
                    Text('Broadcasting credentials...'),
                  ],
                ),
              ],
              if (_responses.isNotEmpty) ...[
                const SizedBox(height: 16),
                Text(
                  'Configured devices',
                  style: theme.textTheme.labelLarge,
                ),
                const SizedBox(height: 6),
                ..._responses.map(
                  (r) => ListTile(
                    dense: true,
                    contentPadding: EdgeInsets.zero,
                    leading: const Icon(Icons.check_circle, color: Colors.green),
                    title: Text(r.bssidText),
                    subtitle: r.ipAddressText != null
                        ? Text(r.ipAddressText!)
                        : null,
                  ),
                ),
              ],
            ],
          ),
        ),
      ),
      actions: [
        TextButton(
          onPressed: () {
            _stop();
            Navigator.of(context).pop();
          },
          child: const Text('Close'),
        ),
        if (_running)
          ElevatedButton(
            onPressed: _stop,
            child: const Text('Stop'),
          )
        else
          ElevatedButton(
            onPressed: _start,
            child: const Text('Start'),
          ),
      ],
    );
  }
}
