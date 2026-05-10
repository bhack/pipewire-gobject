#!/usr/bin/env python3
"""Check release-version metadata that must stay in sync."""

from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

VERSION_PATTERN = r"[0-9]+\.[0-9]+\.[0-9]+(?:[.-][A-Za-z0-9][A-Za-z0-9.-]*)?"


def _read(path: Path) -> str:
    return path.read_text(encoding="utf-8")


def _match(pattern: str, text: str, label: str) -> str:
    match = re.search(pattern, text, re.MULTILINE)
    if match is None:
        raise ValueError(f"Could not find {label}")
    return match.group(1)


def _collect_versions(root: Path) -> dict[str, str]:
    meson = _read(root / "meson.build")
    changelog = _read(root / "CHANGELOG.md")
    docs_config_template = _read(root / "docs" / "pwg.toml.in")
    readme = _read(root / "README.md")

    if 'version = "@PROJECT_VERSION@"' not in docs_config_template:
        raise ValueError("docs/pwg.toml.in must derive its version from Meson")

    readme_match = re.search(
        rf"\| Latest preview \| \[`({VERSION_PATTERN})`\]"
        rf"\(https://github\.com/bhack/pipewire-gobject/releases/tag/({VERSION_PATTERN})\)",
        readme,
    )
    if readme_match is None:
        raise ValueError("Could not find README latest-preview version/link")

    readme_label, readme_link = readme_match.groups()
    if readme_label != readme_link:
        raise ValueError(
            "README latest-preview label and release link disagree: "
            f"{readme_label} != {readme_link}"
        )

    return {
        "meson.build": _match(
            rf"^\s*version:\s*'({VERSION_PATTERN})'",
            meson,
            "Meson project version",
        ),
        "CHANGELOG.md": _match(
            rf"^## ({VERSION_PATTERN})\s+-\s+\d{{4}}-\d{{2}}-\d{{2}}$",
            changelog,
            "top changelog version",
        ),
        "README.md": readme_label,
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--expected-version",
        help="Version that all metadata must match, usually the release tag",
    )
    args = parser.parse_args()

    root = Path(__file__).resolve().parents[1]
    try:
        versions = _collect_versions(root)
    except ValueError as error:
        print(f"release metadata error: {error}", file=sys.stderr)
        return 1

    expected = args.expected_version or next(iter(versions.values()))
    mismatches = {
        label: version for label, version in versions.items() if version != expected
    }

    if mismatches:
        print(f"release metadata error: expected {expected}", file=sys.stderr)
        for label, version in versions.items():
            marker = "!" if label in mismatches else " "
            print(f"{marker} {label}: {version}", file=sys.stderr)
        return 1

    print(f"release metadata ok: {expected}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
