#!/usr/bin/env python3
from __future__ import annotations

import argparse

import gi

gi.require_version("GLib", "2.0")
gi.require_version("Pwg", "0.1")
from gi.repository import GLib, Pwg


def main() -> None:
    parser = argparse.ArgumentParser(description="Print PipeWire capture peak levels.")
    parser.add_argument("target", nargs="?", help="PipeWire node.name or object serial to capture from")
    parser.add_argument("--monitor", action="store_true", help="capture sink monitor audio")
    args = parser.parse_args()

    loop = GLib.MainLoop()
    stream = Pwg.Stream.new_audio_capture(args.target, args.monitor)

    def on_level(_stream: Pwg.Stream, peak: float) -> None:
        print(f"{peak:.6f}")

    stream.connect("level", on_level)
    stream.start()

    try:
        loop.run()
    except KeyboardInterrupt:
        pass
    finally:
        stream.stop()


if __name__ == "__main__":
    main()
