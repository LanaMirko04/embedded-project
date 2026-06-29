import 'package:flutter/material.dart';

import 'device_list_page.dart';
import '../dialogs/profile_dialog.dart';
import '../dialogs/smartconfig_dialog.dart';
import '../services/auth_service.dart';

class MyHomePage extends StatefulWidget {
  const MyHomePage({super.key});

  @override
  State<MyHomePage> createState() => _MyHomePageState();
}

class _MyHomePageState extends State<MyHomePage> {
  @override
  void initState() {
    super.initState();
    AuthService().addListener(_onAuthChanged);
  }

  void _onAuthChanged() {}

  @override
  void dispose() {
    AuthService().removeListener(_onAuthChanged);
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('Devices'),
        actions: [
          IconButton(
            tooltip: 'WiFi SmartConfig',
            icon: const Icon(Icons.wifi_tethering),
            onPressed: () => showDialog(
              context: context,
              builder: (_) => const SmartConfigDialog(),
            ),
          ),
          IconButton(
            tooltip: 'Account',
            icon: const Icon(Icons.person_outline),
            onPressed: () async {
              await showDialog(
                context: context,
                builder: (_) => const ProfileDialog(),
              );
            },
          ),
        ],
      ),
      body: const DeviceListPage(),
    );
  }
}
