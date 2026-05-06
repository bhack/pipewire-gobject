Title: Main Loop And Registry Discovery
SPDX-License-Identifier: MIT
SPDX-FileCopyrightText: 2026 pipewire-gobject contributors

# Main Loop And Registry Discovery

PipeWire registry updates are delivered through a GLib main context. A program
that starts [class@Pwg.Registry] should run a [struct@GLib.MainLoop] or otherwise
iterate the thread-default main context.

```python
import gi

gi.require_version("GLib", "2.0")
gi.require_version("Pwg", "0.1")
from gi.repository import GLib, Pwg

Pwg.init()

core = Pwg.Core.new()
registry = Pwg.Registry.new(core)
registry.start()

loop = GLib.MainLoop()


def on_global_added(_registry, global_):
    print(global_.get_id(), global_.get_interface_type(), global_.dup_name() or "")


registry.connect("global-added", on_global_added)
GLib.timeout_add_seconds(2, loop.quit)
loop.run()
```

The registry keeps immutable [class@Pwg.Global] descriptors in a
[iface@Gio.ListModel]. Snapshot helper methods return new list models so callers
can inspect a stable set of objects without holding the registry's internal
model.

For client-specific discovery, wrap a client global in [class@Pwg.ClientInfo]:

```python
clients = registry.dup_globals_by_interface("PipeWire:Interface:Client")
if clients.get_n_items() > 0:
    client = Pwg.ClientInfo.new_from_global(clients.get_item(0))
    if client is not None:
        print(
            client.dup_app_name() or client.dup_name() or "",
            client.dup_app_id() or "",
            client.dup_process_binary() or "",
        )
```

For device-specific discovery, wrap a device global in [class@Pwg.DeviceInfo]:

```python
devices = registry.dup_globals_by_interface("PipeWire:Interface:Device")
if devices.get_n_items() > 0:
    device = Pwg.DeviceInfo.new_from_global(devices.get_item(0))
    if device is not None:
        print(
            device.dup_description() or device.dup_nick() or device.dup_name() or "",
            device.dup_api() or "",
            device.dup_form_factor() or "",
        )
```

For link-specific discovery, wrap a link global in [class@Pwg.LinkInfo]:

```python
links = registry.dup_globals_by_interface("PipeWire:Interface:Link")
if links.get_n_items() > 0:
    link = Pwg.LinkInfo.new_from_global(links.get_item(0))
    if link is not None:
        print(
            link.dup_output_node_id() or "",
            link.dup_output_port_id() or "",
            "->",
            link.dup_input_node_id() or "",
            link.dup_input_port_id() or "",
        )
```

For node-specific discovery, wrap a node global in [class@Pwg.NodeInfo]:

```python
nodes = registry.dup_globals_by_interface("PipeWire:Interface:Node")
if nodes.get_n_items() > 0:
    node_info = Pwg.NodeInfo.new_from_global(nodes.get_item(0))
    if node_info is not None:
        print(node_info.dup_name() or "", node_info.dup_media_class() or "")
```

For node parameter inspection, bind the same global with
[class@Pwg.Node]. The live node proxy exposes copied [class@Pwg.ParamInfo]
descriptors and emits copied [class@Pwg.Param] values when enumeration results
arrive:

```python
node = Pwg.Node.new(core, nodes.get_item(0))
if node is not None:
    node.start()

    for index in range(node.get_param_infos().get_n_items()):
        param_info = node.get_param_infos().get_item(index)
        print(param_info.get_id(), param_info.dup_name() or "")

    def on_param(_node, param):
        print(
            param.get_id(),
            param.dup_name() or "",
            param.dup_format_media_type_name() or "",
            param.dup_format_media_subtype_name() or "",
            param.dup_summary(),
        )
        audio_format = param.dup_audio_format()
        if audio_format is not None:
            print(
                audio_format.get_sample_format(),
                audio_format.get_rate(),
                audio_format.get_channels(),
            )

    node.connect("param", on_param)
    node.enum_all_params()
```

Nodes can also queue copied parameter updates that were built by this library.
The return value only means PipeWire accepted the request for dispatch; it is
not an applied-state acknowledgment:

```python
param = Pwg.Param.new_props_volume(0.75)
if param is not None:
    node.set_param(param)
```

For port-specific discovery, wrap a port global in [class@Pwg.PortInfo]:

```python
ports = registry.dup_globals_by_interface("PipeWire:Interface:Port")
if ports.get_n_items() > 0:
    port = Pwg.PortInfo.new_from_global(ports.get_item(0))
    if port is not None:
        print(
            port.dup_direction() or "",
            port.dup_name() or "",
            port.dup_audio_channel() or "",
        )
```

The registry is a discovery API, not a session manager. It does not make routing
or default-device policy decisions.
