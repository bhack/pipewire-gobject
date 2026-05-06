#!/usr/bin/env python3
from __future__ import annotations

import argparse

import gi

gi.require_version("GLib", "2.0")
gi.require_version("Pwg", "0.1")
from gi.repository import GLib, Pwg

DEVICE_INTERFACE = "PipeWire:Interface:Device"


def main() -> None:
    parser = argparse.ArgumentParser(description="List discovered PipeWire device globals.")
    parser.add_argument("--api", help="only print devices using this device.api value")
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

    devices = registry.dup_globals_by_interface(DEVICE_INTERFACE)
    for index in range(devices.get_n_items()):
        device = Pwg.DeviceInfo.new_from_global(devices.get_item(index))
        if device is None:
            continue

        api = device.dup_api() or ""
        if args.api and api != args.api:
            continue

        print(
            device.get_id(),
            device.dup_object_serial() or "",
            api,
            device.dup_class() or "",
            device.dup_form_factor() or "",
            device.dup_vendor_name() or "",
            device.dup_product_name() or "",
            device.dup_description() or device.dup_nick() or device.dup_name() or "",
        )

    registry.stop()
    core.disconnect()


if __name__ == "__main__":
    main()
