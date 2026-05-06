#!/usr/bin/env python3
from __future__ import annotations

import gi

gi.require_version("GLib", "2.0")
gi.require_version("Pwg", "0.1")
from gi.repository import GLib, Pwg

NODE_INTERFACE = "PipeWire:Interface:Node"


def main() -> None:
    Pwg.init()

    core = Pwg.Core.new()
    registry = Pwg.Registry.new(core)
    registry.start()

    loop = GLib.MainLoop()

    def maybe_done(*_args):
        if registry.dup_globals_by_interface(NODE_INTERFACE).get_n_items() > 0:
            loop.quit()
            return False
        return True

    registry.connect("global-added", maybe_done)
    GLib.timeout_add(2000, loop.quit)
    loop.run()

    nodes = registry.dup_globals_by_interface(NODE_INTERFACE)
    if nodes.get_n_items() == 0:
        registry.stop()
        core.disconnect()
        return

    node_global = nodes.get_item(0)
    node_info = Pwg.NodeInfo.new_from_global(node_global)
    node = Pwg.Node.new(core, node_global)
    if node_info is None or node is None:
        registry.stop()
        core.disconnect()
        return

    node.start()
    print(node_info.dup_name() or "", node_info.dup_media_class() or "")

    info_loop = GLib.MainLoop()

    def info_maybe_done(*_args):
        if node.get_param_infos().get_n_items() > 0:
            info_loop.quit()
            return False
        return True

    node.get_param_infos().connect("items-changed", info_maybe_done)
    GLib.timeout_add(500, info_loop.quit)
    info_loop.run()

    print("advertised params")
    param_infos = node.get_param_infos()
    for index in range(param_infos.get_n_items()):
        param_info = param_infos.get_item(index)
        access = []
        if param_info.get_readable():
            access.append("read")
        if param_info.get_writable():
            access.append("write")
        print(param_info.get_id(), param_info.dup_name() or "", ",".join(access))

    enum_loop = GLib.MainLoop()

    def on_param(_node, param):
        audio_format = param.dup_audio_format()
        media_type = param.dup_format_media_type_name() or ""
        media_subtype = param.dup_format_media_subtype_name() or ""
        print(
            "value",
            param.get_seq(),
            param.get_id(),
            param.dup_name() or "",
            media_type,
            media_subtype,
            param.dup_summary(),
        )
        if audio_format is not None:
            print(
                "audio-format",
                audio_format.get_sample_format(),
                audio_format.get_rate(),
                audio_format.get_channels(),
                audio_format.get_bytes_per_sample(),
            )

    node.connect("param", on_param)
    try:
        node.enum_all_params()
    except GLib.GError as exc:
        print("enum failed", exc.message)
    else:
        GLib.timeout_add(500, enum_loop.quit)
        enum_loop.run()

    node.stop()
    registry.stop()
    core.disconnect()


if __name__ == "__main__":
    main()
