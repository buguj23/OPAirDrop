#!/system/bin/sh
set -eu
MODDIR=$(CDPATH= cd -- "${0%/*}/.." && pwd)
. "$MODDIR/bin/common.sh"
init_state
for permission in android.permission.NEARBY_WIFI_DEVICES android.permission.ACCESS_LOCAL_NETWORK android.permission.BLUETOOTH_SCAN android.permission.BLUETOOTH_ADVERTISE android.permission.ACCESS_FINE_LOCATION android.permission.ACCESS_COARSE_LOCATION android.permission.ACCESS_BACKGROUND_LOCATION android.permission.ACCESS_MEDIA_LOCATION; do
    result=$(framework pm grant --user 0 "$PKG" "$permission" 2>&1) || {
        case "$result" in
            *"Unknown permission"*) continue;;
            *) echo "$result" >> "$STATE/permission-grants.log"; echo "Permission grant failed: $permission"; exit 3;;
        esac
    }
    printf '%s\n' "$result" >> "$STATE/permission-grants.log"
done
