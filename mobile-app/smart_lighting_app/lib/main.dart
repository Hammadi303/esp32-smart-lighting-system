
import 'dart:async';
import 'dart:convert';

import 'package:flutter/material.dart';
import 'package:http/http.dart' as http;
import 'package:shared_preferences/shared_preferences.dart';

void main() {
  runApp(const SmartLightingApp());
}

class SmartLightingApp extends StatelessWidget {
  const SmartLightingApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      debugShowCheckedModeBanner: false,
      title: 'Smart Lighting',
      theme: ThemeData(
        useMaterial3: true,
        colorScheme: ColorScheme.fromSeed(
          seedColor: const Color(0xFF5478F5),
        ),
        scaffoldBackgroundColor: const Color(0xFFF4F6FA),
      ),
      home: const HomePage(),
    );
  }
}

class HomePage extends StatefulWidget {
  const HomePage({super.key});

  @override
  State<HomePage> createState() => _HomePageState();
}

class _HomePageState extends State<HomePage> {
  final ipController = TextEditingController();

  String ipAddress = '192.168.4.1';
  bool connected = false;
  bool loading = false;

  Map<String, dynamic> status = {};
  Map<String, dynamic> config = {};

  Timer? refreshTimer;

  final Color blue = const Color(0xFF5478F5);

  @override
  void initState() {
    super.initState();
    _initialize();

    refreshTimer = Timer.periodic(
      const Duration(seconds: 3),
      (_) {
        if (connected && !loading) {
          _refreshStatus();
        }
      },
    );
  }

  Future<void> _initialize() async {
    final prefs = await SharedPreferences.getInstance();
    ipAddress = prefs.getString('esp32_ip') ?? '192.168.4.1';
    ipController.text = ipAddress;

    if (mounted) {
      setState(() {});
    }

    // Automatically reconnect using the saved IP.
    await _connect();
  }

  String get baseUrl => 'http://$ipAddress';

  Future<Map<String, dynamic>> _get(String endpoint) async {
    final response = await http
        .get(Uri.parse('$baseUrl$endpoint'))
        .timeout(const Duration(seconds: 4));

    if (response.statusCode != 200) {
      throw Exception('HTTP ${response.statusCode}: ${response.body}');
    }

    return jsonDecode(response.body) as Map<String, dynamic>;
  }

  Future<Map<String, dynamic>> _post(
    String endpoint,
    Map<String, dynamic> data,
  ) async {
    final response = await http
        .post(
          Uri.parse('$baseUrl$endpoint'),
          headers: {'Content-Type': 'application/json'},
          body: jsonEncode(data),
        )
        .timeout(const Duration(seconds: 4));

    if (response.statusCode != 200) {
      throw Exception('HTTP ${response.statusCode}: ${response.body}');
    }

    return jsonDecode(response.body) as Map<String, dynamic>;
  }

  Future<void> _connect() async {
    final enteredIP = ipController.text.trim();

    if (enteredIP.isEmpty ||
        enteredIP.contains(' ') ||
        enteredIP.contains('/') ||
        enteredIP.contains('://')) {
      _showMessage('Enter a valid ESP32 IP address.');
      return;
    }

    setState(() {
      loading = true;
      connected = false;
      ipAddress = enteredIP;
    });

    try {
      final prefs = await SharedPreferences.getInstance();
      await prefs.setString('esp32_ip', ipAddress);

      await _refreshStatus();

      if (mounted) {
        _showMessage('Connected to ESP32 successfully!');
      }
    } catch (e) {
      if (mounted) {
        setState(() {
          connected = false;
        });
        _showMessage('Connection failed: $e');
      }
    } finally {
      if (mounted) {
        setState(() {
          loading = false;
        });
      }
    }
  }

  Future<void> _refreshStatus() async {
    try {
      final newStatus = await _get('/api/status');

      Map<String, dynamic> newConfig = config;

      try {
        newConfig = await _get('/api/config');
      } catch (_) {
        // Status is still useful if config retrieval fails.
      }

      if (!mounted) return;

      setState(() {
        status = newStatus;
        config = newConfig;
        connected = true;
      });
    } catch (e) {
      if (!mounted) return;

      setState(() {
        connected = false;
      });
    }
  }

  Future<void> _setLight(bool turnOn) async {
    if (!connected || loading) return;

    setState(() {
      loading = true;
    });

    try {
      final result = await _post('/api/light', {
        'state': turnOn ? 'on' : 'off',
      });

      await _refreshStatus();

      if (mounted) {
        _showMessage('Lamp ${result['relay_state']?.toString().toUpperCase()}');
      }
    } catch (e) {
      if (mounted) {
        _showMessage('Could not control the lamp: $e');
      }
    } finally {
      if (mounted) {
        setState(() {
          loading = false;
        });
      }
    }
  }

  Future<void> _saveConfig(
    double threshold,
    double hysteresis,
    int confirmTime,
  ) async {
    try {
      await _post('/api/config', {
        'dark_threshold_pct': threshold,
        'hysteresis_pct': hysteresis,
        'confirm_time_s': confirmTime,
      });

      await _refreshStatus();

      if (mounted) {
        _showMessage('Detection settings saved successfully.');
      }
    } catch (e) {
      if (mounted) {
        _showMessage('Configuration error: $e');
      }
    }
  }

  void _showMessage(String message) {
    ScaffoldMessenger.of(context).showSnackBar(
      SnackBar(content: Text(message)),
    );
  }

  Future<void> _openSettings() async {
    if (!connected) {
      _showMessage('Connect to the ESP32 first.');
      return;
    }

    final thresholdController = TextEditingController(
      text: '${config['dark_threshold_pct'] ?? 30}',
    );

    final hysteresisController = TextEditingController(
      text: '${config['hysteresis_pct'] ?? 10}',
    );

    final confirmController = TextEditingController(
      text: '${config['confirm_time_s'] ?? 10}',
    );

    await showDialog(
      context: context,
      builder: (dialogContext) {
        return AlertDialog(
          title: const Text('Darkness detection settings'),
          content: SingleChildScrollView(
            child: Column(
              mainAxisSize: MainAxisSize.min,
              children: [
                TextField(
                  controller: thresholdController,
                  keyboardType: TextInputType.number,
                  decoration: const InputDecoration(
                    labelText: 'Darkness threshold (%)',
                  ),
                ),
                TextField(
                  controller: hysteresisController,
                  keyboardType: TextInputType.number,
                  decoration: const InputDecoration(
                    labelText: 'Hysteresis (%)',
                  ),
                ),
                TextField(
                  controller: confirmController,
                  keyboardType: TextInputType.number,
                  decoration: const InputDecoration(
                    labelText: 'Confirmation time (seconds)',
                  ),
                ),
              ],
            ),
          ),
          actions: [
            TextButton(
              onPressed: () => Navigator.pop(dialogContext),
              child: const Text('Cancel'),
            ),
            ElevatedButton(
              onPressed: () {
                final threshold =
                    double.tryParse(thresholdController.text);
                final hysteresis =
                    double.tryParse(hysteresisController.text);
                final confirmTime =
                    int.tryParse(confirmController.text);

                if (threshold == null ||
                    hysteresis == null ||
                    confirmTime == null) {
                  _showMessage('Enter valid numeric values.');
                  return;
                }

                if (threshold < 1 ||
                    threshold > 99 ||
                    hysteresis < 1 ||
                    threshold + hysteresis > 100 ||
                    confirmTime < 1 ||
                    confirmTime > 600) {
                  _showMessage('Values are outside the allowed limits.');
                  return;
                }

                Navigator.pop(dialogContext);

                _saveConfig(threshold, hysteresis, confirmTime);
              },
              child: const Text('Save'),
            ),
          ],
        );
      },
    );

    thresholdController.dispose();
    hysteresisController.dispose();
    confirmController.dispose();
  }

  @override
  void dispose() {
    refreshTimer?.cancel();
    ipController.dispose();
    super.dispose();
  }

  Widget _card({required Widget child}) {
    return Container(
      width: double.infinity,
      padding: const EdgeInsets.all(18),
      decoration: BoxDecoration(
        color: Colors.white,
        borderRadius: BorderRadius.circular(18),
      ),
      child: child,
    );
  }

  Widget _sectionTitle(String title) {
    return Padding(
      padding: const EdgeInsets.only(bottom: 10),
      child: Text(
        title,
        style: const TextStyle(
          color: Colors.grey,
          fontSize: 12,
          fontWeight: FontWeight.bold,
          letterSpacing: 1.2,
        ),
      ),
    );
  }

  @override
  Widget build(BuildContext context) {
    final wifi = status['wifi'] is Map
        ? Map<String, dynamic>.from(status['wifi'])
        : <String, dynamic>{};

    final sensor = status['sensor'] is Map
        ? Map<String, dynamic>.from(status['sensor'])
        : <String, dynamic>{};

    final relayOn = status['relay_state'] == 'on';

    final brightness =
        (sensor['brightness_pct'] as num?)?.toDouble() ?? 0.0;

    final environment = status['environment'] ?? '--';
    final fsmState = status['fsm_state'] ?? '--';

    return Scaffold(
      appBar: AppBar(
        backgroundColor: Colors.white,
        surfaceTintColor: Colors.white,
        title: const Row(
          children: [
            Icon(Icons.lightbulb_outline, color: Color(0xFF5478F5)),
            SizedBox(width: 10),
            Text(
              'Smart Lighting',
              style: TextStyle(fontWeight: FontWeight.bold),
            ),
          ],
        ),
        actions: [
          IconButton(
            onPressed: _openSettings,
            icon: const Icon(Icons.settings_outlined),
          ),
          IconButton(
            onPressed: connected ? _refreshStatus : _connect,
            icon: const Icon(Icons.refresh),
          ),
        ],
      ),
      body: SafeArea(
        child: SingleChildScrollView(
          padding: const EdgeInsets.all(20),
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.start,
            children: [
              const Text(
                'Welcome back!',
                style: TextStyle(
                  fontSize: 27,
                  fontWeight: FontWeight.bold,
                  color: Color(0xFF20283A),
                ),
              ),
              const SizedBox(height: 6),
              const Text(
                'Monitor and control your lighting system.',
                style: TextStyle(color: Colors.grey),
              ),
              const SizedBox(height: 24),

              // Connection
              _card(
                child: Row(
                  children: [
                    Icon(
                      connected ? Icons.wifi : Icons.wifi_off,
                      color: connected ? Colors.green : Colors.red,
                      size: 30,
                    ),
                    const SizedBox(width: 14),
                    Expanded(
                      child: Column(
                        crossAxisAlignment: CrossAxisAlignment.start,
                        children: [
                          const Text(
                            'ESP32 connection',
                            style: TextStyle(fontWeight: FontWeight.bold),
                          ),
                          Text(
                            connected ? 'Connected' : 'Not connected',
                            style: TextStyle(
                              color: connected ? Colors.green : Colors.red,
                            ),
                          ),
                          if (wifi['ip'] != null)
                            Text(
                              'Device IP: ${wifi['ip']}',
                              style: const TextStyle(
                                color: Colors.grey,
                                fontSize: 12,
                              ),
                            ),
                        ],
                      ),
                    ),
                    Icon(
                      Icons.circle,
                      color: connected ? Colors.green : Colors.red,
                      size: 11,
                    ),
                  ],
                ),
              ),

              const SizedBox(height: 24),
              _sectionTitle('DEVICE CONFIGURATION'),

              _card(
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    const Text(
                      'ESP32 IP address',
                      style: TextStyle(fontWeight: FontWeight.w600),
                    ),
                    const SizedBox(height: 10),
                    TextField(
                      controller: ipController,
                      keyboardType: TextInputType.url,
                      decoration: InputDecoration(
                        prefixIcon: const Icon(Icons.router_outlined),
                        hintText: '192.168.1.100',
                        border: OutlineInputBorder(
                          borderRadius: BorderRadius.circular(12),
                        ),
                      ),
                    ),
                    const SizedBox(height: 12),
                    SizedBox(
                      width: double.infinity,
                      child: ElevatedButton.icon(
                        onPressed: loading ? null : _connect,
                        icon: loading
                            ? const SizedBox(
                                height: 18,
                                width: 18,
                                child: CircularProgressIndicator(
                                  strokeWidth: 2,
                                ),
                              )
                            : const Icon(Icons.wifi),
                        label: const Text('Connect to ESP32'),
                        style: ElevatedButton.styleFrom(
                          backgroundColor: blue,
                          foregroundColor: Colors.white,
                          padding: const EdgeInsets.symmetric(vertical: 14),
                        ),
                      ),
                    ),
                  ],
                ),
              ),

              const SizedBox(height: 24),
              _sectionTitle('LIGHT CONTROL'),

              _card(
                child: Column(
                  children: [
                    Row(
                      children: [
                        Icon(
                          relayOn ? Icons.lightbulb : Icons.lightbulb_outline,
                          size: 38,
                          color: relayOn ? Colors.amber : Colors.grey,
                        ),
                        const SizedBox(width: 15),
                        Expanded(
                          child: Column(
                            crossAxisAlignment: CrossAxisAlignment.start,
                            children: [
                              const Text(
                                'Main light',
                                style: TextStyle(
                                  fontSize: 17,
                                  fontWeight: FontWeight.bold,
                                ),
                              ),
                              Text(
                                relayOn ? 'Turned ON' : 'Turned OFF',
                                style: TextStyle(
                                  color: relayOn ? Colors.green : Colors.grey,
                                ),
                              ),
                            ],
                          ),
                        ),
                        Switch(
                          value: relayOn,
                          activeThumbColor: blue,
                          onChanged: connected && !loading
                              ? _setLight
                              : null,
                        ),
                      ],
                    ),
                    const Divider(height: 30),

                    Row(
                      mainAxisAlignment: MainAxisAlignment.spaceBetween,
                      children: [
                        const Text(
                          'Ambient brightness',
                          style: TextStyle(fontWeight: FontWeight.w600),
                        ),
                        Text(
                          '${brightness.toStringAsFixed(1)}%',
                          style: TextStyle(
                            color: blue,
                            fontWeight: FontWeight.bold,
                          ),
                        ),
                      ],
                    ),
                    const SizedBox(height: 12),
                    LinearProgressIndicator(
                      value: (brightness / 100).clamp(0.0, 1.0),
                      minHeight: 9,
                      borderRadius: BorderRadius.circular(10),
                      color: blue,
                      backgroundColor: Colors.grey.shade200,
                    ),
                    const SizedBox(height: 8),
                    Text(
                      'ADC raw: ${sensor['adc_raw'] ?? '--'}'
                      '    |    Filtered: ${sensor['adc_filtered'] ?? '--'}',
                      style: const TextStyle(
                        color: Colors.grey,
                        fontSize: 12,
                      ),
                    ),
                  ],
                ),
              ),

              const SizedBox(height: 24),
              _sectionTitle('ENVIRONMENT MONITORING'),

              _card(
                child: Column(
                  children: [
                    Row(
                      children: [
                        Icon(
                          environment == 'dark'
                              ? Icons.nightlight_round
                              : Icons.wb_sunny_outlined,
                          color: Colors.orange,
                          size: 30,
                        ),
                        const SizedBox(width: 12),
                        Expanded(
                          child: Text(
                            'Environment: ${environment.toString().toUpperCase()}',
                            style: const TextStyle(
                              fontWeight: FontWeight.bold,
                            ),
                          ),
                        ),
                      ],
                    ),
                    const Divider(height: 25),
                    _infoRow('Detection state', fsmState.toString()),
                    _infoRow('Dark event ID', '${status['dark_event_id'] ?? 0}'),
                    _infoRow('Event age (s)', '${status['dark_event_age_s'] ?? '--'}'),
                    _infoRow('Wi-Fi RSSI', '${wifi['rssi'] ?? '--'} dBm'),
                    _infoRow('Uptime', '${status['uptime_s'] ?? '--'} s'),
                  ],
                ),
              ),

              const SizedBox(height: 24),
              _sectionTitle('DETECTION CONFIGURATION'),

              _card(
                child: Column(
                  children: [
                    _infoRow(
                      'Darkness threshold',
                      '${config['dark_threshold_pct'] ?? '--'}%',
                    ),
                    _infoRow(
                      'Hysteresis',
                      '${config['hysteresis_pct'] ?? '--'}%',
                    ),
                    _infoRow(
                      'Confirmation time',
                      '${config['confirm_time_s'] ?? '--'} s',
                    ),
                    const SizedBox(height: 12),
                    SizedBox(
                      width: double.infinity,
                      child: OutlinedButton.icon(
                        onPressed: _openSettings,
                        icon: const Icon(Icons.tune),
                        label: const Text('Configure detection'),
                      ),
                    ),
                  ],
                ),
              ),

              const SizedBox(height: 20),
              Center(
                child: Text(
                  connected
                      ? 'Data refreshes automatically every 3 seconds.'
                      : 'Connect your ESP32 to view live data.',
                  style: const TextStyle(
                    color: Colors.grey,
                    fontSize: 12,
                  ),
                  textAlign: TextAlign.center,
                ),
              ),
            ],
          ),
        ),
      ),
    );
  }

  Widget _infoRow(String title, String value) {
    return Padding(
      padding: const EdgeInsets.symmetric(vertical: 7),
      child: Row(
        mainAxisAlignment: MainAxisAlignment.spaceBetween,
        children: [
          Text(title, style: const TextStyle(color: Colors.grey)),
          Text(
            value,
            style: const TextStyle(fontWeight: FontWeight.w600),
          ),
        ],
      ),
    );
  }
}
