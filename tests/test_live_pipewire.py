import gi

gi.require_version("Pwg", "0.1")
from gi.repository import GLib, Pwg


Pwg.init()
print("pwg-version", Pwg.get_library_version())
print("pipewire-version", Pwg.get_pipewire_library_version())

core = Pwg.Core.new()
print("version", core.get_library_version())
print("core", core.connect())

registry = Pwg.Registry.new(core)
print("registry-start", registry.start())

loop = GLib.MainLoop()


def maybe_done(*_args):
    if registry.get_globals().get_n_items() > 0:
        loop.quit()
        return False
    return True


def timeout():
    loop.quit()
    return False


registry.connect("global-added", maybe_done)
GLib.timeout_add(50, maybe_done)
GLib.timeout_add(2000, timeout)
loop.run()

globals_model = registry.get_globals()
count = globals_model.get_n_items()
print("registry-count", count)
assert count > 0

first = globals_model.get_item(0)
assert first.get_id() >= 0
assert isinstance(first.get_interface_type(), str)
assert first.get_properties() is not None
assert registry.lookup_global(first.get_id()) is not None
print("registry-first", first.get_id(), first.get_interface_type())

registry.stop()
print("registry-running-after-stop", registry.get_running())

core.disconnect()

stream = Pwg.Stream.new_audio_capture(None, True)
stream.set_deliver_audio_blocks(True)
print("stream-deliver-audio-blocks", stream.get_deliver_audio_blocks())
print("stream-start", stream.start())
print("stream-running", stream.get_running())
stream.stop()
print("stream-running-after-stop", stream.get_running())

capture = Pwg.AudioCapture.new(None, True)
print("capture-start", capture.start())
print("capture-running", capture.get_running())
capture.stop()
print("capture-running-after-stop", capture.get_running())
