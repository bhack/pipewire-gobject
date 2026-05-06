#!/usr/bin/env python3
from __future__ import annotations

import argparse

import gi

gi.require_version("GLib", "2.0")
gi.require_version("Pwg", "0.1")
from gi.repository import GLib, Pwg

PORT_INTERFACE = "PipeWire:Interface:Port"


def main() -> None:
    parser = argparse.ArgumentParser(description="List discovered PipeWire port globals.")
    parser.add_argument("--direction", choices=("in", "out"), help="only print ports with this direction")
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

    ports = registry.dup_globals_by_interface(PORT_INTERFACE)
    for index in range(ports.get_n_items()):
        port = Pwg.PortInfo.new_from_global(ports.get_item(index))
        if port is None:
            continue

        direction = port.dup_direction() or ""
        if args.direction and direction != args.direction:
            continue

        print(
            port.get_id(),
            port.dup_object_serial() or "",
            direction,
            port.dup_audio_channel() or "",
            "physical" if port.get_physical() else "",
            port.dup_name() or "",
            port.dup_alias() or "",
        )

    registry.stop()
    core.disconnect()


if __name__ == "__main__":
    main()
