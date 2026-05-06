import gi

gi.require_version("Pwg", "0.1")
from gi.repository import GLib, Pwg

Pwg.init()

core = Pwg.Core.new()
registry = Pwg.Registry.new(core)
metadata = Pwg.Metadata.new(core, "default")
registry.start()
metadata.start()

loop = GLib.MainLoop()


def maybe_done(*_args):
    if metadata.get_bound() and registry.get_globals().get_n_items() > 0:
        loop.quit()
        return False
    return True


registry.connect("global-added", maybe_done)
metadata.connect("notify::bound", maybe_done)
GLib.timeout_add(2000, loop.quit)
loop.run()


def print_node(label, node_name):
    print(label, node_name or "")
    if not node_name:
        return

    node = registry.lookup_global_by_property("node.name", node_name)
    if node is None:
        return

    node_info = Pwg.NodeInfo.new_from_global(node)
    if node_info is None:
        return

    print(
        f"{label}.global",
        node_info.get_id(),
        node_info.dup_media_class() or "",
        node_info.dup_description() or "",
    )


if not metadata.get_bound():
    print("default metadata not found")
else:
    print_node("default.audio.sink", metadata.dup_default_audio_sink_name())
    print_node("default.audio.source", metadata.dup_default_audio_source_name())
    print_node("default.configured.audio.sink", metadata.dup_configured_audio_sink_name())
    print_node("default.configured.audio.source", metadata.dup_configured_audio_source_name())

metadata.stop()
registry.stop()
core.disconnect()
