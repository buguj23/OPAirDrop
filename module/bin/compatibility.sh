#!/system/bin/sh
# Shared install/start device gates: Oplus/Realme brand, sun/kalama platform.
compatibility_error() { echo "Compatibility: $*" >&2; return 2; }

compatibility_device() (
    brands=$(getprop ro.product.manufacturer; getprop ro.product.brand; getprop ro.product.vendor.manufacturer; getprop ro.product.vendor.brand)
    printf '%s\n' "$brands" | tr '[:upper:]' '[:lower:]' | grep -Eq '^(oplus|oppo|oneplus|realme)$' || {
        compatibility_error 'Oplus device required (OPPO / OnePlus / realme)'; exit 2;
    }
    board=$(getprop ro.board.platform)
    case "$board" in
        sun|kalama) ;;
        *) compatibility_error "sun platform or kalama platform required; found $board"; exit 2;;
    esac
)
