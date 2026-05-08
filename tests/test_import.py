import gi

gi.require_version("GLib", "2.0")
gi.require_version("Pwg", "0.1")
from gi.repository import GLib, Pwg

Pwg.init()
assert Pwg.get_library_version() == "0.3.2"
assert isinstance(Pwg.get_pipewire_library_version(), str)

core = Pwg.Core.new()
assert core.get_connected() is False
assert isinstance(core.get_library_version(), str)
assert hasattr(Pwg, "ClientInfo")
assert hasattr(Pwg, "DeviceInfo")
assert hasattr(Pwg, "ImplModule")
assert hasattr(Pwg, "LinkInfo")
assert hasattr(Pwg, "Node")
assert hasattr(Pwg, "NodeInfo")
assert hasattr(Pwg, "Param")
assert hasattr(Pwg, "ParamInfo")
assert hasattr(Pwg, "PortInfo")

registry = Pwg.Registry.new(core)
assert registry.get_core() == core
assert registry.get_running() is False
assert registry.get_globals().get_n_items() == 0
assert registry.lookup_global(0) is None
assert registry.lookup_global_by_property("node.name", "missing") is None
assert registry.lookup_global_by_object_serial("1") is None
assert registry.dup_globals_by_property("node.name", "missing").get_n_items() == 0
assert registry.dup_globals_by_interface("PipeWire:Interface:Node").get_n_items() == 0
assert registry.dup_globals_by_media_class("Audio/Sink").get_n_items() == 0

metadata = Pwg.Metadata.new(core, "default")
assert metadata.get_core() == core
assert metadata.get_name() == "default"
assert metadata.get_running() is False
assert metadata.get_bound() is False
assert metadata.dup_value(0, "default.audio.sink") is None
assert metadata.dup_value_type(0, "default.audio.sink") is None
assert metadata.dup_default_audio_sink_name() is None
assert metadata.dup_default_audio_source_name() is None
assert metadata.dup_configured_audio_sink_name() is None
assert metadata.dup_configured_audio_source_name() is None

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

volume_param = Pwg.Param.new_props_volume(0.5)
assert volume_param is not None
assert volume_param.dup_name() == "Props"
assert volume_param.dup_object_type_name() == "Props"
assert volume_param.dup_summary().startswith("Object Props")
mute_param = Pwg.Param.new_props_mute(True)
assert mute_param is not None
assert mute_param.dup_name() == "Props"
assert mute_param.dup_object_type_name() == "Props"
controls = GLib.Variant(
    "a{sd}",
    {
        "band_l_0:b0": 1.0,
        "band_l_0:b1": -0.25,
    },
)
control_param = Pwg.Param.new_props_controls(controls)
assert control_param is not None
assert control_param.dup_name() == "Props"
assert control_param.dup_object_type_name() == "Props"
assert control_param.dup_summary().endswith("1 properties")

stream = Pwg.Stream.new_audio_capture(None, True)
assert stream.get_running() is False
assert stream.get_monitor() is True
assert stream.get_requested_sample_format() == "F32"
assert stream.get_requested_rate() == 48000
assert stream.get_requested_channels() == 2
assert stream.set_requested_format("F32", 44100, 1) is True
assert stream.get_requested_sample_format() == "F32"
assert stream.get_requested_rate() == 44100
assert stream.get_requested_channels() == 1
try:
    stream.set_requested_format("S16", 44100, 1)
except GLib.Error:
    pass
else:
    raise AssertionError("unsupported requested sample format was accepted")
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
