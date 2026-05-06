import gi

gi.require_version("Pwg", "0.1")
from gi.repository import GLib, Pwg


DEFAULT_AUDIO_SINK_KEY = "default.audio.sink"
DEFAULT_CONFIGURED_AUDIO_SINK_KEY = "default.configured.audio.sink"


Pwg.init()

core = Pwg.Core.new()
metadata = Pwg.Metadata.new(core, "default")
metadata.start()

loop = GLib.MainLoop()


def maybe_done(*_args):
    if metadata.get_bound():
        loop.quit()
        return False
    return True


metadata.connect("notify::bound", maybe_done)
GLib.timeout_add(2000, loop.quit)
loop.run()

if not metadata.get_bound():
    print("default metadata not found")
else:
    default_sink = metadata.dup_value(0, DEFAULT_AUDIO_SINK_KEY)
    configured_sink = metadata.dup_value(0, DEFAULT_CONFIGURED_AUDIO_SINK_KEY)
    print("default.audio.sink", default_sink or "")
    print("default.configured.audio.sink", configured_sink or "")

metadata.stop()
core.disconnect()
