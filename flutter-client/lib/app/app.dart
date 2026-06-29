import 'package:flutter/material.dart';
import 'package:dynamic_color/dynamic_color.dart';

import '../pages/home_page.dart';
import '../pages/device_page.dart';
import '../services/auth_service.dart';
import '../services/api_service.dart';
import '../models/sdrumo.dart';
import '../dialogs/login_dialog.dart';
import '../utils/error_helpers.dart';

class MyApp extends StatelessWidget {
  const MyApp({super.key});

  @override
  Widget build(BuildContext context) {
    return DynamicColorBuilder(
      builder: (lightDynamic, darkDynamic) {
        final seed = Colors.cyan;

        final lightScheme =
            lightDynamic?.harmonized() ?? ColorScheme.fromSeed(seedColor: seed);

        final darkScheme = darkDynamic?.harmonized() ??
            ColorScheme.fromSeed(
              seedColor: seed,
              brightness: Brightness.dark,
            );

        return MaterialApp(
          title: 'sdrumoapp',
          themeMode: ThemeMode.system,
          theme: ThemeData(
            useMaterial3: true,
            colorScheme: lightScheme,
          ),
          darkTheme: ThemeData(
            useMaterial3: true,
            colorScheme: darkScheme,
          ),
          home: const AuthGate(),
          onGenerateRoute: (settings) {
            if (settings.name == '/device') {
              final device = settings.arguments as Sdrumo;
              return MaterialPageRoute(
                builder: (_) => DevicePage(device: device),
              );
            }
            return null;
          },
        );
      },
    );
  }
}

class AuthGate extends StatefulWidget {
  const AuthGate({super.key});

  @override
  State<AuthGate> createState() => _AuthGateState();
}

class _AuthGateState extends State<AuthGate> {
  @override
  void initState() {
    super.initState();
    // Show login dialog if not authenticated
    WidgetsBinding.instance.addPostFrameCallback((_) {
      if (AuthService().isKeyringLocked && mounted) {
        _showKeyringLockedPrompt();
      } else if (!AuthService().isAuthenticated && mounted) {
        _showLoginPrompt();
      } else if (AuthService().isAuthenticated && mounted) {
        _fetchUserInfo();
      }
    });
    // Listen for auth changes to dismiss dialog if user logs in elsewhere
    AuthService().addListener(_onAuthChanged);
  }

  Future<void> _fetchUserInfo() async {
    try {
      await ApiService().fetchUsername();
      await ApiService().fetchSdrumos();
    } catch (e) {
      print('Error fetching user info: $e');
    }
  }

  @override
  void dispose() {
    AuthService().removeListener(_onAuthChanged);
    super.dispose();
  }

  void _onAuthChanged() {
    if (!mounted || _isDialogShowing) return;
    if (AuthService().isKeyringLocked) {
      _showKeyringLockedPrompt();
    } else if (!AuthService().isAuthenticated) {
      _showLoginPrompt();
    }
  }

  bool _isDialogShowing = false;

  void _showKeyringLockedPrompt() {
    _isDialogShowing = true;
    showKeyringLockedDialog(
      context,
      onRetry: () {
        _isDialogShowing = false;
        AuthService().loadFromStorage().then((_) {
          if (!mounted) return;
          if (AuthService().isKeyringLocked) {
            _showKeyringLockedPrompt();
          } else if (AuthService().isAuthenticated) {
            _fetchUserInfo();
          } else {
            _showLoginPrompt();
          }
        }).catchError((_) {
          if (mounted) _showKeyringLockedPrompt();
        });
      },
      onRelogin: () {
        _isDialogShowing = false;
        if (mounted) _showLoginPrompt();
      },
    );
  }

  void _showLoginPrompt() {
    _isDialogShowing = true;
    showDialog(
      context: context,
      barrierDismissible: false,
      builder: (_) => const LoginDialog(),
    ).then((_) {
      _isDialogShowing = false;
    });
  }

  @override
  Widget build(BuildContext context) {
    return ListenableBuilder(
      listenable: AuthService(),
      builder: (context, _) {
        // Only show home page if authenticated
        if (AuthService().isAuthenticated) {
          return const MyHomePage();
        }
        // Show a blank screen while waiting for login
        return const Scaffold(
          body: Center(
            child: CircularProgressIndicator(),
          ),
        );
      },
    );
  }
}
