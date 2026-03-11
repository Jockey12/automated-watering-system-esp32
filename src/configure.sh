#!/bin/bash
# ============================================================
#  Automated Irrigation System — Configuration Script
#
#  Run this once after cloning to fill in your settings.
#  It creates config.json (read by Python server) and patches
#  the .ino firmware files with your WiFi/MQTT credentials.
#
#  Usage:
#    chmod +x configure.sh
#    ./configure.sh
# ============================================================

set -e
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR"

echo ""
echo "=================================================="
echo "  Irrigation System — Interactive Configuration"
echo "=================================================="
echo ""
echo "Press Enter to accept the [default] shown in brackets."
echo ""

# ── Prompts ─────────────────────────────────────────────────

read -rp "Garden/project name          [My Garden]:         " GARDEN_NAME
GARDEN_NAME="${GARDEN_NAME:-My Garden}"

read -rp "WiFi SSID                    [YOUR_WIFI_SSID]:    " WIFI_SSID
WIFI_SSID="${WIFI_SSID:-YOUR_WIFI_SSID}"

read -rsp "WiFi password               (hidden):             " WIFI_PASS
echo ""

read -rp "Pi hostname (no .local)      [irrigation-pi]:     " PI_HOSTNAME
PI_HOSTNAME="${PI_HOSTNAME:-irrigation-pi}"

read -rp "Pi Linux username            [pi]:                " PI_USER
PI_USER="${PI_USER:-pi}"

read -rp "Timezone (IANA format)       [UTC]:               " TIMEZONE
TIMEZONE="${TIMEZONE:-UTC}"
echo ""
echo "  Examples: America/New_York  Europe/London  Asia/Tokyo"
echo ""

# ── Write config.json ────────────────────────────────────────

CONFIG_FILE="$SCRIPT_DIR/config.json"
cat >"$CONFIG_FILE" <<EOF
{
  "garden_name":   "$GARDEN_NAME",
  "pi_hostname":   "$PI_HOSTNAME",
  "pi_username":   "$PI_USER",
  "timezone":      "$TIMEZONE"
}
EOF
echo "✅ config.json written (WiFi credentials are only in .ino files)"

# ── Patch .ino files ─────────────────────────────────────────

INO_FILES=(esp1_pump.ino esp2_valve8r.ino esp3_valve4r.ino esp4_valve4r.ino)

for f in "${INO_FILES[@]}"; do
    fpath="$SCRIPT_DIR/$f"
    if [ ! -f "$fpath" ]; then
        echo "⚠️  $f not found, skipping"
        continue
    fi
    # Use a temp file to avoid partial writes
    tmp="$(mktemp)"
    sed \
        -e "s|YOUR_WIFI_SSID|$WIFI_SSID|g" \
        -e "s|YOUR_WIFI_PASSWORD|$WIFI_PASS|g" \
        -e "s|YOUR_PI_HOSTNAME|$PI_HOSTNAME|g" \
        -e "s|YOUR_GARDEN_NAME|$GARDEN_NAME|g" \
        "$fpath" >"$tmp"
    mv "$tmp" "$fpath"
    echo "✅ $f patched"
done

# ── Patch setup_pi.sh ────────────────────────────────────────

SETUP_SH="$SCRIPT_DIR/setup_pi.sh"
if [ -f "$SETUP_SH" ]; then
    tmp="$(mktemp)"
    sed \
        -e "s|YOUR_PI_HOSTNAME|$PI_HOSTNAME|g" \
        "$SETUP_SH" >"$tmp"
    mv "$tmp" "$SETUP_SH"
    chmod +x "$SETUP_SH"
    echo "✅ setup_pi.sh patched"
fi

# ── Summary ──────────────────────────────────────────────────

echo ""
echo "=================================================="
echo "  ✅ Configuration complete!"
echo "=================================================="
echo ""
echo "  Garden:    $GARDEN_NAME"
echo "  Pi:        $PI_HOSTNAME.local  (user: $PI_USER)"
echo "  Timezone:  $TIMEZONE"
echo ""
echo "  Next steps:"
echo "  1. Flash each .ino to its ESP32 via Arduino IDE"
echo "  2. Copy this folder to the Pi:  scp -r . $PI_USER@$PI_HOSTNAME.local:~/irrigation/"
echo "  3. On the Pi:  sudo PI_USER=$PI_USER PI_HOSTNAME=$PI_HOSTNAME ./setup_pi.sh"
echo "  4. Start:      sudo systemctl start irrigation"
echo "  5. Open:       http://$PI_HOSTNAME.local:5000"
echo ""
echo "  ⚠️  Do NOT commit your .ino files or config.json — they contain your credentials."
echo ""
