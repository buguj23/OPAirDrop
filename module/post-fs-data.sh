#!/system/bin/sh
MODDIR=${0%/*}
STATE=/data/adb/opairshare
mkdir -p "$STATE"
chmod 0700 "$STATE"

# Bind mount manifest_mosey.xml cleanly without overlaying partitions
if [ -d /system_ext/etc/vintf/manifest ] && [ -f "$MODDIR/manifest_mosey.xml" ]; then
    mkdir -p /dev/opairshare_vintf_manifest
    cp -af /system_ext/etc/vintf/manifest/* /dev/opairshare_vintf_manifest/
    cp -af "$MODDIR/manifest_mosey.xml" /dev/opairshare_vintf_manifest/
    chmod 644 /dev/opairshare_vintf_manifest/*
    chcon -R u:object_r:system_file:s0 /dev/opairshare_vintf_manifest
    mount --bind /dev/opairshare_vintf_manifest /system_ext/etc/vintf/manifest
fi

/system/bin/sh "$MODDIR/bin/gms-unlock.sh" "$MODDIR" > "$STATE/gms-unlock.log" 2>&1
