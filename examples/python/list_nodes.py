#!/usr/bin/env python3
from __future__ import annotations

import argparse

import gi

gi.require_version("GLib", "2.0")
gi.require_version("Pwg", "0.1")
from gi.repository import GLib, Pwg

NODE_INTERFACE = "PipeWire:Interface:Node"


def main() -> None:
    parser = argparse.ArgumentParser(description="List discovered PipeWire node globals.")
    parser.add_argument("--media-class", help="only print nodes with this media.class")
    args = parser.parse_args()

    Pwg.init()

    core = Pwg.Core.new()
    registry = Pwg.Registry.new(core)
    registry.start()

    loop = GLib.MainLoop()

    def maybe_done(*_args):
        if registry.get_globals().get_n_items() > 0:
            loop.quit()
            return False
        return True

    registry.connect("global-added", maybe_done)
    GLib.timeout_add(2000, loop.quit)
    loop.run()

    if args.media_class:
        nodes = registry.dup_globals_by_media_class(args.media_class)
    else:
        nodes = registry.dup_globals_by_interface(NODE_INTERFACE)

    for index in range(nodes.get_n_items()):
        node = nodes.get_item(index)
        print(
            node.get_id(),
            node.dup_object_serial() or "",
            node.dup_media_class() or "",
            node.dup_name() or "",
            node.dup_description() or "",
        )

    registry.stop()
    core.disconnect()


if __name__ == "__main__":
    main()
