#!/usr/bin/env python3
from __future__ import annotations

import gi

gi.require_version("GLib", "2.0")
gi.require_version("Pwg", "0.1")
from gi.repository import GLib, Pwg


def wait_for_initial_globals(registry: Pwg.Registry, timeout_ms: int = 2000) -> None:
    loop = GLib.MainLoop()

    def maybe_done(*_args):
        if registry.get_globals().get_n_items() > 0:
            loop.quit()
            return False
        return True

    registry.connect("global-added", maybe_done)
    GLib.timeout_add(50, maybe_done)
    GLib.timeout_add(timeout_ms, loop.quit)
    loop.run()


def print_nodes(registry: Pwg.Registry) -> None:
    nodes = registry.dup_node_infos()
    print(f"nodes: {nodes.get_n_items()}")
    for index in range(nodes.get_n_items()):
        node = nodes.get_item(index)
        print(
            "  node",
            node.get_id(),
            node.dup_object_serial() or "",
            node.dup_media_class() or "",
            node.dup_name() or "",
            node.dup_description() or "",
        )


def print_ports(registry: Pwg.Registry) -> None:
    ports = registry.dup_port_infos()
    print(f"ports: {ports.get_n_items()}")
    for index in range(ports.get_n_items()):
        port = ports.get_item(index)
        print(
            "  port",
            port.get_id(),
            "node",
            port.dup_node_id() or "",
            "port",
            port.dup_port_id() or "",
            port.dup_direction() or "",
            port.dup_name() or "",
            port.dup_alias() or "",
        )


def print_links(registry: Pwg.Registry) -> None:
    links = registry.dup_link_infos()
    print(f"links: {links.get_n_items()}")
    for index in range(links.get_n_items()):
        link = links.get_item(index)
        print(
            "  link",
            link.get_id(),
            "out",
            link.dup_output_node_id() or "",
            link.dup_output_port_id() or "",
            "in",
            link.dup_input_node_id() or "",
            link.dup_input_port_id() or "",
        )


def print_clients(registry: Pwg.Registry) -> None:
    clients = registry.dup_client_infos()
    print(f"clients: {clients.get_n_items()}")
    for index in range(clients.get_n_items()):
        client = clients.get_item(index)
        print(
            "  client",
            client.get_id(),
            client.dup_client_id() or "",
            client.dup_name() or "",
            client.dup_api() or "",
            client.dup_application_name() or "",
        )


def print_devices(registry: Pwg.Registry) -> None:
    devices = registry.dup_device_infos()
    print(f"devices: {devices.get_n_items()}")
    for index in range(devices.get_n_items()):
        device = devices.get_item(index)
        print(
            "  device",
            device.get_id(),
            device.dup_device_id() or "",
            device.dup_api() or "",
            device.dup_device_class() or "",
            device.dup_name() or "",
            device.dup_description() or "",
        )


def print_metadata(registry: Pwg.Registry) -> None:
    metadata = registry.dup_metadata_infos()
    print(f"metadata: {metadata.get_n_items()}")
    for index in range(metadata.get_n_items()):
        item = metadata.get_item(index)
        print("  metadata", item.get_id(), item.dup_name() or "", item.dup_object_serial() or "")


def main() -> None:
    Pwg.init()

    core = Pwg.Core.new()
    registry = Pwg.Registry.new(core)
    registry.start()
    wait_for_initial_globals(registry)

    print_nodes(registry)
    print_ports(registry)
    print_links(registry)
    print_clients(registry)
    print_devices(registry)
    print_metadata(registry)

    registry.stop()
    core.disconnect()


if __name__ == "__main__":
    main()
