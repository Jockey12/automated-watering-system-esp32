#!/bin/bash
# ============================================================
#  Automated Irrigation System - Pi Setup Script
#  Run this on the Raspberry Pi to install all dependencies
#  Usage: chmod +x setup_pi.sh && sudo ./setup_pi.sh
#
#  Optionally set PI_USER and PI_HOSTNAME env vars before running:
#    PI_USER=pi PI_HOSTNAME=garden-pi sudo -E ./setup_pi.sh
# ============================================================

set -e

PI_USER="${PI_USER:-pi}"
PI_HOSTNAME="${PI_HOSTNAME:-YOUR_PI_HOSTNAME}"

echo "========================================="
echo "  Irrigation System - Pi Setup"
echo "  User: $PI_USER | Hostname: $PI_HOSTNAME"
echo "========================================="

# 1. System update
# echo ""
# echo "📦 Updating system..."
# apt-get update -qq

# # 2. Install Mosquitto MQTT broker
# echo ""
# echo "📦 Installing Mosquitto MQTT Broker..."
# apt-get install -y mosquitto mosquitto-clients
#
# # Enable and start mosquitto
# systemctl enable mosquitto
# systemctl start mosquitto
# echo "✅ Mosquitto active (port 1883)"

# 3. Install avahi-daemon for mDNS
echo ""
echo "📦 Installing Avahi mDNS service..."
apt-get install -y avahi-daemon
hostnamectl set-hostname "$PI_HOSTNAME"
systemctl enable avahi-daemon
systemctl start avahi-daemon
echo "✅ Pi is now accessible as '$PI_HOSTNAME.local'"

# 3. Install Python dependencies
echo ""
echo "📦 Installing Python dependencies..."
apt-get install -y python3-pip python3-venv

# Create venv if it doesn't exist
INSTALL_DIR="/home/$PI_USER/irrigation"
mkdir -p "$INSTALL_DIR"

if [ ! -d "$INSTALL_DIR/venv" ]; then
    python3 -m venv "$INSTALL_DIR/venv"
    echo "✅ Python venv created"
fi

"$INSTALL_DIR/venv/bin/pip" install --quiet flask paho-mqtt
echo "✅ Flask and paho-mqtt installed"

# 4. Copy server.py if running from repo
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
if [ -f "$SCRIPT_DIR/pi_server/server.py" ]; then
    cp "$SCRIPT_DIR/pi_server/server.py" "$INSTALL_DIR/server.py"
    echo "✅ server.py copied → $INSTALL_DIR/"
fi

# 5. Create systemd service
echo ""
echo "📦 Creating systemd service..."
cat >/etc/systemd/system/irrigation.service <<EOF
[Unit]
Description=Automated Irrigation Control Server
After=network.target mosquitto.service
Wants=mosquitto.service

[Service]
Type=simple
User=$PI_USER
WorkingDirectory=$INSTALL_DIR
ExecStart=$INSTALL_DIR/venv/bin/python $INSTALL_DIR/main.py
Restart=always
RestartSec=5
StandardOutput=journal
StandardError=journal

[Install]
WantedBy=multi-user.target
EOF

systemctl daemon-reload
systemctl enable irrigation
echo "✅ irrigation.service created and enabled"

# # 6. Configure Mosquitto for local network
# if [ ! -f /etc/mosquitto/conf.d/irrigation.conf ]; then
#     cat > /etc/mosquitto/conf.d/irrigation.conf << 'EOF'
# listener 1883
# allow_anonymous true
# EOF
#     systemctl restart mosquitto
#     echo "✅ Mosquitto configured for local network"
# fi

# 7. Summary
echo ""
echo "========================================="
echo "  ✅ SETUP COMPLETE"
echo "========================================="
echo ""
echo "  To start the server:"
echo "    sudo systemctl start irrigation"
echo ""
echo "  To check status:"
echo "    sudo systemctl status irrigation"
echo ""
echo "  To view logs:"
echo "    journalctl -u irrigation -f"
echo ""
echo "  Web interface:"
echo "    http://$PI_HOSTNAME.local:5000"
echo ""
echo "  MQTT test:"
echo "    mosquitto_sub -t 'sulama/#' -v"
echo ""

