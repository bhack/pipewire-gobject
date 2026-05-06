#!/usr/bin/env python3
from __future__ import annotations

import gi

gi.require_version("GLib", "2.0")
gi.require_version("Pwg", "0.1")
from gi.repository import GLib, Pwg

INTERFACES = {
    "clients": "PipeWire:Interface:Client",
    "devices": "PipeWire:Interface:Device",
    "links": "PipeWire:Interface:Link",
    "nodes": "PipeWire:Interface:Node",
    "ports": "PipeWire:Interface:Port",
}


def wait_for_registry(registry: Pwg.Registry) -> None:
    loop = GLib.MainLoop()

    def maybe_done(*_args):
        if registry.get_globals().get_n_items() > 0:
            loop.quit()
            return False
        return True

    registry.connect("global-added", maybe_done)
    GLib.timeout_add(2000, loop.quit)
    loop.run()


def print_first_client(registry: Pwg.Registry) -> None:
    clients = registry.dup_globals_by_interface(INTERFACES["clients"])
    if clients.get_n_items() == 0:
        return

    client = Pwg.ClientInfo.new_from_global(clients.get_item(0))
    if client is not None:
        print("client", client.dup_app_name() or client.dup_name() or "")


def print_first_device(registry: Pwg.Registry) -> None:
    devices = registry.dup_globals_by_interface(INTERFACES["devices"])
    if devices.get_n_items() == 0:
        return

    device = Pwg.DeviceInfo.new_from_global(devices.get_item(0))
    if device is not None:
        print("device", device.dup_description() or device.dup_nick() or device.dup_name() or "")


def print_first_link(registry: Pwg.Registry) -> None:
    links = registry.dup_globals_by_interface(INTERFACES["links"])
    if links.get_n_items() == 0:
        return

    link = Pwg.LinkInfo.new_from_global(links.get_item(0))
    if link is not None:
        print(
            "link",
            link.dup_output_node_id() or "",
            link.dup_output_port_id() or "",
            "->",
            link.dup_input_node_id() or "",
            link.dup_input_port_id() or "",
        )


def main() -> None:
    Pwg.init()

    core = Pwg.Core.new()
    registry = Pwg.Registry.new(core)
    registry.start()
    wait_for_registry(registry)

    for label, interface in INTERFACES.items():
        print(label, registry.dup_globals_by_interface(interface).get_n_items())

    print_first_client(registry)
    print_first_device(registry)
    print_first_link(registry)

    registry.stop()
    core.disconnect()


if __name__ == "__main__":
    main()
