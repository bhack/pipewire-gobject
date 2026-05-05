#!/usr/bin/env python3
from __future__ import annotations

import argparse

import gi

gi.require_version("GLib", "2.0")
gi.require_version("Pwg", "0.1")
from gi.repository import GLib, Pwg


def main() -> None:
    parser = argparse.ArgumentParser(description="Print copied PipeWire audio block metadata.")
    parser.add_argument("target", nargs="?", help="PipeWire node.name or object serial to capture from")
    parser.add_argument("--monitor", action="store_true", help="capture sink monitor audio")
    parser.add_argument("--count", type=int, default=8, help="number of blocks to print before exiting")
    args = parser.parse_args()

    loop = GLib.MainLoop()
    stream = Pwg.Stream.new_audio_capture(args.target, args.monitor)
    stream.set_deliver_audio_blocks(True)
    remaining = args.count

    def on_audio_block(_stream: Pwg.Stream, block: Pwg.AudioBlock) -> None:
        nonlocal remaining

        audio_format = block.get_format()
        data = block.get_data()
        print(
            block.get_sequence(),
            audio_format.get_sample_format(),
            audio_format.get_rate(),
            audio_format.get_channels(),
            block.get_n_frames(),
            data.get_size(),
            f"{block.get_peak():.6f}",
        )
        remaining -= 1
        if remaining <= 0:
            loop.quit()

    stream.connect("audio-block", on_audio_block)
    stream.start()

    try:
        loop.run()
    except KeyboardInterrupt:
        pass
    finally:
        stream.stop()


if __name__ == "__main__":
    main()
