import 'package:flutter/material.dart';
import 'package:dynamic_color/dynamic_color.dart';

import '../pages/home_page.dart';
import '../pages/device_page.dart';
import '../services/auth_service.dart';
import '../services/api_service.dart';
import '../models/sdrumo.dart';
import '../dialogs/login_dialog.dart';
import '../utils/error_helpers.dart';

/// Shared visual language for the whole app: one corner radius, one card
/// style, one input style, themed buttons. Keeps every page cohesive instead
/// of each widget styling itself ad hoc.
ThemeData _buildTheme(ColorScheme scheme) {
  const radius = 12.0;
  final shape = RoundedRectangleBorder(
    borderRadius: BorderRadius.circular(radius),
  );

  return ThemeData(
    useMaterial3: true,
    colorScheme: scheme,
    appBarTheme: AppBarTheme(
      backgroundColor: scheme.surface,
      foregroundColor: scheme.onSurface,
      centerTitle: false,
      scrolledUnderElevation: 2,
    ),
    cardTheme: CardThemeData(
      elevation: 0,
      color: scheme.surfaceContainerHighest,
      margin: EdgeInsets.zero,
      shape: shape,
      clipBehavior: Clip.antiAlias,
    ),
    listTileTheme: const ListTileThemeData(
      shape: RoundedRectangleBorder(
        borderRadius: BorderRadius.all(Radius.circular(radius)),
      ),
    ),
    inputDecorationTheme: InputDecorationTheme(
      filled: true,
      fillColor: scheme.surfaceContainerHighest,
      border: OutlineInputBorder(
        borderRadius: BorderRadius.circular(radius),
        borderSide: BorderSide.none,
      ),
      enabledBorder: OutlineInputBorder(
        borderRadius: BorderRadius.circular(radius),
        borderSide: BorderSide.none,
      ),
    ),
    filledButtonTheme: FilledButtonThemeData(
      style: FilledButton.styleFrom(
        shape: shape,
        padding: const EdgeInsets.symmetric(horizontal: 20, vertical: 14),
      ),
    ),
    elevatedButtonTheme: ElevatedButtonThemeData(
      style: ElevatedButton.styleFrom(shape: shape),
    ),
    dialogTheme: DialogThemeData(shape: shape),
    snackBarTheme: const SnackBarThemeData(
      behavior: SnackBarBehavior.floating,
    ),
    chipTheme: ChipThemeData(
      shape: StadiumBorder(side: BorderSide(color: scheme.outlineVariant)),
    ),
  );
}

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
          theme: _buildTheme(lightScheme),
          darkTheme: _buildTheme(darkScheme),
          home: const AuthGate(),
          onGenerateRoute: (settings) {
            if (settings.name == '/device') {
              final device = settings.arguments as Sdrump;
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
