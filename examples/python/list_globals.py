import gi

gi.require_version("Pwg", "0.1")
from gi.repository import GLib, Pwg

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

globals_model = registry.get_globals()
for index in range(globals_model.get_n_items()):
    global_ = globals_model.get_item(index)
    name = global_.dup_name() or ""
    media_class = global_.dup_media_class() or ""
    object_serial = global_.dup_object_serial() or ""
    print(global_.get_id(), global_.get_interface_type(), object_serial, media_class, name)

registry.stop()
core.disconnect()
