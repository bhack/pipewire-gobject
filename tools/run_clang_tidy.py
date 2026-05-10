#!/usr/bin/env python3
"""Run clang-tidy once per C translation unit from a Meson build directory."""

from __future__ import annotations

import argparse
import json
import subprocess
import sys
from pathlib import Path
from typing import Any


def _resolve_entry_file(entry: dict[str, Any]) -> Path:
    path = Path(entry["file"])
    if path.is_absolute():
        return path.resolve()
    return (Path(entry["directory"]) / path).resolve()


def _matches_path(path: Path, requested: list[Path]) -> bool:
    for candidate in requested:
        if path == candidate:
            return True
        if candidate.is_dir():
            try:
                path.relative_to(candidate)
            except ValueError:
                continue
            return True
    return False


def _source_paths(root: Path, raw_paths: list[str]) -> list[Path]:
    paths = raw_paths or ["src", "tests"]
    return [
        (root / path).resolve() if not Path(path).is_absolute() else Path(path).resolve()
        for path in paths
    ]


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--build-dir",
        default="build",
        help="Meson build directory containing compile_commands.json",
    )
    parser.add_argument("--clang-tidy", default="clang-tidy", help="clang-tidy executable")
    parser.add_argument(
        "--prepare-only",
        action="store_true",
        help="Only write a deduplicated compile database for another clang-tidy frontend",
    )
    parser.add_argument(
        "--output-build-dir",
        help="Directory for the deduplicated compile database, default: BUILD_DIR/clang-tidy",
    )
    parser.add_argument("paths", nargs="*", help="Optional source files or directories to analyze")
    args = parser.parse_args()

    root = Path(__file__).resolve().parents[1]
    build_dir = (root / args.build_dir).resolve()
    compile_commands = build_dir / "compile_commands.json"
    if not compile_commands.exists():
        print(f"{compile_commands} does not exist; run meson setup first", file=sys.stderr)
        return 2

    requested = _source_paths(root, args.paths)
    entries = json.loads(compile_commands.read_text(encoding="utf-8"))
    seen: set[Path] = set()
    deduped_entries: list[dict[str, Any]] = []
    files: list[str] = []

    for entry in entries:
        source = _resolve_entry_file(entry)
        if source.suffix != ".c" or source in seen or not _matches_path(source, requested):
            continue

        seen.add(source)
        tidy_entry = dict(entry)
        tidy_entry["file"] = str(source)
        deduped_entries.append(tidy_entry)
        files.append(str(source))

    if not files:
        print("No C translation units matched the requested paths.", file=sys.stderr)
        return 2

    if args.output_build_dir is None:
        tidy_build_dir = build_dir / "clang-tidy"
    else:
        output_build_dir = Path(args.output_build_dir)
        tidy_build_dir = (
            output_build_dir.resolve()
            if output_build_dir.is_absolute()
            else (root / output_build_dir).resolve()
        )
    tidy_build_dir.mkdir(exist_ok=True)
    (tidy_build_dir / "compile_commands.json").write_text(
        json.dumps(deduped_entries, indent=2) + "\n",
        encoding="utf-8",
    )

    if args.prepare_only:
        print(f"Wrote {tidy_build_dir / 'compile_commands.json'} for {len(files)} C files.")
        return 0

    command = [args.clang_tidy, "--quiet", "-p", str(tidy_build_dir), *files]
    return subprocess.run(command, check=False).returncode


if __name__ == "__main__":
    raise SystemExit(main())
