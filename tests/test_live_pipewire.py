import gi

gi.require_version("Pwg", "0.1")
from gi.repository import GLib, Pwg

Pwg.init()
print("pwg-version", Pwg.get_library_version())
print("pipewire-version", Pwg.get_pipewire_library_version())

core = Pwg.Core.new()
print("version", core.get_library_version())
print("core", core.connect())
assert core.sync(2000) is True
print("core-sync", True)

module = core.load_module("libpipewire-module-profiler", None)
assert module.get_core() == core
assert module.get_name() == "libpipewire-module-profiler"
assert module.get_arguments() is None
assert module.get_loaded() is True
print("module-loaded", module.get_name(), module.get_loaded())
module.unload()
assert module.get_loaded() is False
print("module-loaded-after-unload", module.get_loaded())

registry = Pwg.Registry.new(core)
print("registry-start", registry.start())
print("registry-sync", registry.sync(2000))

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
print("registry-sync-after-stream", registry.sync(2000))
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

node = Pwg.Node.new(core, node_info.get_global())
assert node is not None
assert node.get_core() == core
assert node.get_global().get_id() == node_info.get_id()
assert node.get_running() is False
assert node.get_bound() is False
assert node.get_param_infos().get_n_items() == 0
assert node.get_params().get_n_items() == 0
print("node-start", node.start())
assert node.get_running() is True
assert node.get_bound() is True
print("node-sync", node.sync(2000))
node_param_info_count = node.get_param_infos().get_n_items()
print("node-param-info-count", node_param_info_count)
for index in range(min(node_param_info_count, 3)):
    param_info = node.get_param_infos().get_item(index)
    print(
        "node-param-info",
        param_info.get_id(),
        param_info.dup_name() or "",
        param_info.get_readable(),
        param_info.get_writable(),
    )


def on_node_param(_node, param):
    audio_format = param.dup_audio_format()
    print(
        "node-param",
        param.get_seq(),
        param.get_id(),
        param.dup_name() or "",
        param.get_pod_type(),
        param.dup_pod_type_name() or "",
        param.dup_format_media_type_name() or "",
        param.dup_format_media_subtype_name() or "",
        param.dup_summary(),
    )
    if audio_format is not None:
        print(
            "node-param-audio-format",
            audio_format.get_sample_format(),
            audio_format.get_rate(),
            audio_format.get_channels(),
        )


node.connect("param", on_node_param)
for index in range(node_param_info_count):
    param_info = node.get_param_infos().get_item(index)
    if not param_info.get_readable():
        continue
    try:
        node_params = node.enum_params_sync(param_info.get_id(), 0, 0, 2000)
    except GLib.GError as exc:
        print("node-enum-params-sync-error", exc.message)
    else:
        print("node-enum-params-sync", param_info.get_id(), node_params.get_n_items())
    break
node.stop()
print("node-running-after-stop", node.get_running())

client_globals = registry.dup_globals_by_interface("PipeWire:Interface:Client")
client_count = client_globals.get_n_items()
print("registry-client-count", client_count)
for index in range(client_count):
    client_info = Pwg.ClientInfo.new_from_global(client_globals.get_item(index))
    assert client_info is not None
    assert client_info.get_global().is_client() is True
    assert client_info.get_id() == client_info.get_global().get_id()
    print(
        "registry-client",
        client_info.dup_name() or "",
        client_info.dup_app_name() or "",
        client_info.dup_api() or "",
    )

device_globals = registry.dup_globals_by_interface("PipeWire:Interface:Device")
device_count = device_globals.get_n_items()
print("registry-device-count", device_count)
for index in range(device_count):
    device_info = Pwg.DeviceInfo.new_from_global(device_globals.get_item(index))
    assert device_info is not None
    assert device_info.get_global().is_device() is True
    assert device_info.get_id() == device_info.get_global().get_id()
    print(
        "registry-device",
        device_info.dup_name() or "",
        device_info.dup_description() or "",
        device_info.dup_api() or "",
    )

if device_count > 0:
    device = Pwg.Device.new(core, device_globals.get_item(0))
    assert device is not None
    print("device-start", device.start())
    print("device-sync", device.sync(2000))
    device_param_info_count = device.get_param_infos().get_n_items()
    print("device-param-info-count", device_param_info_count)
    for index in range(device_param_info_count):
        param_info = device.get_param_infos().get_item(index)
        if not param_info.get_readable():
            continue
        try:
            device_params = device.enum_params_sync(param_info.get_id(), 0, 0, 2000)
        except GLib.GError as exc:
            print("device-enum-params-sync-error", exc.message)
        else:
            print("device-enum-params-sync", param_info.get_id(), device_params.get_n_items())
        break
    device.stop()
    print("device-running-after-stop", device.get_running())

port_globals = registry.dup_globals_by_interface("PipeWire:Interface:Port")
port_count = port_globals.get_n_items()
print("registry-port-count", port_count)
for index in range(port_count):
    port_info = Pwg.PortInfo.new_from_global(port_globals.get_item(index))
    assert port_info is not None
    assert port_info.get_global().is_port() is True
    assert port_info.get_id() == port_info.get_global().get_id()
    print(
        "registry-port",
        port_info.dup_direction() or "",
        port_info.dup_name() or "",
        port_info.dup_audio_channel() or "",
    )

link_globals = registry.dup_globals_by_interface("PipeWire:Interface:Link")
link_count = link_globals.get_n_items()
print("registry-link-count", link_count)
for index in range(link_count):
    link_info = Pwg.LinkInfo.new_from_global(link_globals.get_item(index))
    assert link_info is not None
    assert link_info.get_global().is_link() is True
    assert link_info.get_id() == link_info.get_global().get_id()
    print(
        "registry-link",
        link_info.dup_output_node_id() or "",
        link_info.dup_output_port_id() or "",
        "->",
        link_info.dup_input_node_id() or "",
        link_info.dup_input_port_id() or "",
    )

node_probe_stream.stop()

registry.stop()
print("registry-running-after-stop", registry.get_running())

metadata = Pwg.Metadata.new(core, "settings")
assert metadata.get_name() == "settings"
print("metadata-start", metadata.start())
print("metadata-running", metadata.get_running())
print("metadata-sync", metadata.sync(2000))
print("metadata-bound", metadata.get_bound())
assert metadata.get_bound()


def on_metadata_changed(_metadata, subject, key, value_type, value):
    print("metadata-changed", subject, key, value_type, value)


metadata.connect("changed", on_metadata_changed)
print("metadata-set", metadata.set(0, "pwg.test", "Spa:String", "test-value"))
print("metadata-sync-after-set", metadata.sync(2000))
assert metadata.dup_value(0, "pwg.test") == "test-value"
assert metadata.dup_value_type(0, "pwg.test") == "Spa:String"

print(
    "metadata-set-default-audio-sink",
    metadata.set(0, "default.audio.sink", "Spa:String:JSON", '{"name":"pwg-test-sink"}'),
)
print("metadata-sync-after-default-set", metadata.sync(2000))
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
