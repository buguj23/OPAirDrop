#!/usr/bin/env python3
"""Build the root-only LSPosed configuration helper (no device operations)."""
from pathlib import Path
import argparse
import os
import subprocess

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--sdk', type=Path, default=os.environ.get('ANDROID_HOME'))
    args = parser.parse_args()
    if not args.sdk:
        parser.error('Set --sdk or ANDROID_HOME')
    api = args.sdk / 'platforms/android-36/android.jar'
    if not api.is_file():
        available_apis = sorted((args.sdk / 'platforms').glob('android-*/android.jar'), reverse=True)
        if available_apis:
            api = available_apis[0]
    d8 = args.sdk / 'build-tools/36.0.0/d8'
    if not d8.is_file():
        available_d8 = sorted((args.sdk / 'build-tools').glob('*/d8'), reverse=True)
        if available_d8:
            d8 = available_d8[0]
    out = ROOT / 'build/lsposed'
    classes, dex = out / 'classes', out / 'dex'
    classes.mkdir(parents=True, exist_ok=True)
    dex.mkdir(parents=True, exist_ok=True)
    subprocess.run(['javac', '-source', '17', '-target', '17', '-cp', str(api), '-d', str(classes),
                    str(ROOT / 'native/OPAirShareLSPosed.java')], check=True)
    subprocess.run([str(d8), '--lib', str(api), '--min-api', '35', '--output', str(dex),
                    str(classes / 'OPAirShareLSPosed.class')], check=True)
    subprocess.run(['jar', 'cf', str(out / 'lsposed-setup.jar'), '-C', str(dex), 'classes.dex'], check=True)
    print(out / 'lsposed-setup.jar')


if __name__ == '__main__':
    main()
