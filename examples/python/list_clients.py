#!/usr/bin/env python3
from __future__ import annotations

import argparse

import gi

gi.require_version("GLib", "2.0")
gi.require_version("Pwg", "0.1")
from gi.repository import GLib, Pwg

CLIENT_INTERFACE = "PipeWire:Interface:Client"


def main() -> None:
    parser = argparse.ArgumentParser(description="List discovered PipeWire client globals.")
    parser.add_argument("--api", help="only print clients using this client.api value")
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

    clients = registry.dup_globals_by_interface(CLIENT_INTERFACE)
    for index in range(clients.get_n_items()):
        client = Pwg.ClientInfo.new_from_global(clients.get_item(index))
        if client is None:
            continue

        api = client.dup_api() or ""
        if args.api and api != args.api:
            continue

        print(
            client.get_id(),
            client.dup_object_serial() or "",
            api,
            client.dup_access() or "",
            client.dup_app_id() or "",
            client.dup_app_name() or client.dup_name() or "",
            client.dup_process_binary() or "",
            client.dup_process_id() or "",
        )

    registry.stop()
    core.disconnect()


if __name__ == "__main__":
    main()
