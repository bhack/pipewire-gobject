import gi

gi.require_version("GLib", "2.0")
gi.require_version("Pwg", "0.1")
from gi.repository import GLib, Pwg


Pwg.init()
assert Pwg.get_library_version() == "0.1.0"
assert isinstance(Pwg.get_pipewire_library_version(), str)

core = Pwg.Core.new()
assert core.get_connected() is False
assert isinstance(core.get_library_version(), str)

registry = Pwg.Registry.new(core)
assert registry.get_core() == core
assert registry.get_running() is False
assert registry.get_globals().get_n_items() == 0
assert registry.lookup_global(0) is None

metadata = Pwg.Metadata.new(core, "default")
assert metadata.get_core() == core
assert metadata.get_name() == "default"
assert metadata.get_running() is False
assert metadata.get_bound() is False
assert metadata.dup_value(0, "default.audio.sink") is None
assert metadata.dup_value_type(0, "default.audio.sink") is None

audio_format = Pwg.AudioFormat.new("F32", 48000, 2, 4)
assert audio_format.get_sample_format() == "F32"
assert audio_format.get_rate() == 48000
assert audio_format.get_channels() == 2
assert audio_format.get_bytes_per_sample() == 4
assert audio_format.get_bytes_per_frame() == 8

audio_data = GLib.Bytes.new(b"\x00" * 16)
audio_block = Pwg.AudioBlock.new(audio_format, audio_data, 2, 7, 0.0)
assert audio_block.get_format() == audio_format
assert audio_block.get_data() is not None
assert audio_block.get_n_frames() == 2
assert audio_block.get_sequence() == 7
assert audio_block.get_peak() == 0.0

stream = Pwg.Stream.new_audio_capture(None, True)
assert stream.get_running() is False
assert stream.get_monitor() is True
assert stream.get_rate() == 0
assert stream.get_channels() == 0
assert stream.get_peak() == 0.0
assert stream.get_audio_format() is None
assert stream.get_deliver_audio_blocks() is False
stream.set_deliver_audio_blocks(True)
assert stream.get_deliver_audio_blocks() is True

capture = Pwg.AudioCapture.new(None, True)
assert capture.get_running() is False
assert capture.get_monitor() is True
assert capture.get_rate() == 0
assert capture.get_channels() == 0
assert capture.get_peak() == 0.0
print("Pwg import ok")
