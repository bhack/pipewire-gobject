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
assert first.is_interface(first.get_interface_type()) is True
print("registry-first-name", first.dup_name() or "")
print("registry-first-description", first.dup_description() or "")
print("registry-first-media-class", first.dup_media_class() or "")
print("registry-first-object-serial", first.dup_object_serial() or "")
matching_interface = registry.dup_globals_by_interface(first.get_interface_type())
assert matching_interface.get_n_items() > 0
if first.dup_object_serial() is not None:
    assert registry.lookup_global_by_object_serial(first.dup_object_serial()) is not None
print("registry-first", first.get_id(), first.get_interface_type())

node_probe_stream = Pwg.Stream.new_audio_capture(None, True)
assert node_probe_stream.start()
node_loop = GLib.MainLoop()


def node_maybe_done(*_args):
    if registry.dup_globals_by_interface("PipeWire:Interface:Node").get_n_items() > 0:
        node_loop.quit()
        return False
    return True


registry.connect("global-added", node_maybe_done)
GLib.timeout_add(50, node_maybe_done)
GLib.timeout_add(2000, node_loop.quit)
node_loop.run()
node_globals = registry.dup_globals_by_interface("PipeWire:Interface:Node")
node_count = node_globals.get_n_items()
print("registry-node-count", node_count)
assert node_count > 0
node_info = Pwg.NodeInfo.new_from_global(node_globals.get_item(0))
assert node_info is not None
assert node_info.get_global().is_node() is True
assert node_info.get_id() == node_info.get_global().get_id()
print("registry-node-name", node_info.dup_name() or "")
print("registry-node-description", node_info.dup_description() or "")
print("registry-node-media-class", node_info.dup_media_class() or "")
print("registry-node-object-serial", node_info.dup_object_serial() or "")

node_infos = registry.dup_node_infos()
port_infos = registry.dup_port_infos()
link_infos = registry.dup_link_infos()
client_infos = registry.dup_client_infos()
device_infos = registry.dup_device_infos()
metadata_infos = registry.dup_metadata_infos()
print("registry-node-info-count", node_infos.get_n_items())
print("registry-port-info-count", port_infos.get_n_items())
print("registry-link-info-count", link_infos.get_n_items())
print("registry-client-info-count", client_infos.get_n_items())
print("registry-device-info-count", device_infos.get_n_items())
print("registry-metadata-info-count", metadata_infos.get_n_items())
assert node_infos.get_n_items() > 0

if port_infos.get_n_items() > 0:
    port_info = port_infos.get_item(0)
    assert port_info.get_global().is_port() is True
    print(
        "registry-port-first",
        port_info.get_id(),
        port_info.dup_node_id() or "",
        port_info.dup_port_id() or "",
        port_info.dup_direction() or "",
        port_info.dup_name() or "",
        port_info.dup_alias() or "",
    )

if link_infos.get_n_items() > 0:
    link_info = link_infos.get_item(0)
    assert link_info.get_global().is_link() is True
    print(
        "registry-link-first",
        link_info.get_id(),
        link_info.dup_output_node_id() or "",
        link_info.dup_output_port_id() or "",
        link_info.dup_input_node_id() or "",
        link_info.dup_input_port_id() or "",
    )

if client_infos.get_n_items() > 0:
    client_info = client_infos.get_item(0)
    assert client_info.get_global().is_client() is True
    print(
        "registry-client-first",
        client_info.get_id(),
        client_info.dup_client_id() or "",
        client_info.dup_name() or "",
        client_info.dup_api() or "",
        client_info.dup_application_name() or "",
    )

if device_infos.get_n_items() > 0:
    device_info = device_infos.get_item(0)
    assert device_info.get_global().is_device() is True
    print(
        "registry-device-first",
        device_info.get_id(),
        device_info.dup_device_id() or "",
        device_info.dup_name() or "",
        device_info.dup_api() or "",
        device_info.dup_device_class() or "",
    )

if metadata_infos.get_n_items() > 0:
    metadata_info = metadata_infos.get_item(0)
    assert metadata_info.get_global().is_metadata() is True
    print(
        "registry-metadata-first",
        metadata_info.get_id(),
        metadata_info.dup_name() or "",
        metadata_info.dup_object_serial() or "",
    )

node_probe_stream.stop()

registry.stop()
print("registry-running-after-stop", registry.get_running())

metadata = Pwg.Metadata.new(core, "settings")
assert metadata.get_name() == "settings"
print("metadata-start", metadata.start())
print("metadata-running", metadata.get_running())

metadata_loop = GLib.MainLoop()


def metadata_maybe_done(*_args):
    if metadata.get_bound():
        metadata_loop.quit()
        return False
    return True


metadata.connect("notify::bound", metadata_maybe_done)
GLib.timeout_add(50, metadata_maybe_done)
GLib.timeout_add(2000, metadata_loop.quit)
metadata_loop.run()

print("metadata-bound", metadata.get_bound())
assert metadata.get_bound()

changed_loop = GLib.MainLoop()


def on_metadata_changed(_metadata, subject, key, value_type, value):
    print("metadata-changed", subject, key, value_type, value)
    if subject == 0 and key == "pwg.test":
        changed_loop.quit()


metadata.connect("changed", on_metadata_changed)
print("metadata-set", metadata.set(0, "pwg.test", "Spa:String", "test-value"))
GLib.timeout_add(2000, changed_loop.quit)
changed_loop.run()
assert metadata.dup_value(0, "pwg.test") == "test-value"
assert metadata.dup_value_type(0, "pwg.test") == "Spa:String"

default_node_loop = GLib.MainLoop()


def on_default_node_changed(_metadata, subject, key, _value_type, _value):
    if subject == 0 and key == "default.audio.sink":
        default_node_loop.quit()


metadata.connect("changed", on_default_node_changed)
print(
    "metadata-set-default-audio-sink",
    metadata.set(0, "default.audio.sink", "Spa:String:JSON", '{"name":"pwg-test-sink"}'),
)
GLib.timeout_add(2000, default_node_loop.quit)
default_node_loop.run()
assert metadata.dup_default_audio_sink_name() == "pwg-test-sink"
assert metadata.dup_default_audio_source_name() is None
assert metadata.dup_configured_audio_sink_name() is None
assert metadata.dup_configured_audio_source_name() is None

print("metadata-clear-key", metadata.set(0, "pwg.test", "Spa:String", None))
print(
    "metadata-clear-default-audio-sink",
    metadata.set(0, "default.audio.sink", "Spa:String:JSON", None),
)
metadata.stop()
print("metadata-running-after-stop", metadata.get_running())

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
