#!/usr/bin/env bash
set -e

# Skrypt do lokalnej kompilacji firmware ZMK dla Sofle przy użyciu Dockera

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ZMK_DIR="${ZMK_DIR:-$HOME/gits/zmk}"
OUTPUT_DIR="$SCRIPT_DIR/build_output"

mkdir -p "$OUTPUT_DIR"

build_side() {
  local shield=$1
  local output_name=$2
  echo "========================================"
  echo "Kompilacja $shield ..."
  echo "========================================"
  sg docker -c "docker run --rm \
    -v \"$ZMK_DIR\":/workspace/zmk \
    -v \"$SCRIPT_DIR\":/config \
    -w /workspace/zmk/app \
    zmkfirmware/zmk-build-arm:stable \
    west build -p always -b nice_nano -- -DSHIELD=$shield -DZMK_CONFIG=/config" 2>&1 | tee "$OUTPUT_DIR/$output_name.log"
  
  cp "$ZMK_DIR/app/build/zephyr/zmk.uf2" "$OUTPUT_DIR/$output_name.uf2"
  [ -f "$ZMK_DIR/app/build/zephyr/zephyr.dts" ] && cp "$ZMK_DIR/app/build/zephyr/zephyr.dts" "$OUTPUT_DIR/$output_name.dts"
  [ -f "$ZMK_DIR/app/build/zephyr/.config" ] && cp "$ZMK_DIR/app/build/zephyr/.config" "$OUTPUT_DIR/$output_name.config"

  echo "--> Sukces! Pliki zapisano w $OUTPUT_DIR/:"
  echo "    - $output_name.uf2 (firmware)"
  echo "    - $output_name.dts (wynikowe Devicetree)"
  echo "    - $output_name.config (wynikowy Kconfig)"
  echo "    - $output_name.log (log kompilacji)"
}

case "${1:-all}" in
  left)
    build_side "my_sofle_left" "my_sofle_left"
    ;;
  right)
    build_side "my_sofle_right" "my_sofle_right"
    ;;
  reset)
    build_side "settings_reset" "settings_reset"
    ;;
  all)
    build_side "my_sofle_left" "my_sofle_left"
    build_side "my_sofle_right" "my_sofle_right"
    ;;
  *)
    echo "Użycie: $0 [left|right|reset|all]"
    exit 1
    ;;
esac
