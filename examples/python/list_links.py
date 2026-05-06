#!/usr/bin/env python3
from __future__ import annotations

import gi

gi.require_version("GLib", "2.0")
gi.require_version("Pwg", "0.1")
from gi.repository import GLib, Pwg

LINK_INTERFACE = "PipeWire:Interface:Link"


def main() -> None:
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

    links = registry.dup_globals_by_interface(LINK_INTERFACE)
    for index in range(links.get_n_items()):
        link = Pwg.LinkInfo.new_from_global(links.get_item(index))
        if link is None:
            continue

        print(
            link.get_id(),
            link.dup_object_serial() or "",
            link.dup_link_id() or "",
            link.dup_output_node_id() or "",
            link.dup_output_port_id() or "",
            "->",
            link.dup_input_node_id() or "",
            link.dup_input_port_id() or "",
            "passive" if link.get_passive() else "",
            "feedback" if link.get_feedback() else "",
        )

    registry.stop()
    core.disconnect()


if __name__ == "__main__":
    main()
