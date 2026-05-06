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

For node-specific discovery, wrap a node global in [class@Pwg.NodeInfo]:

```python
nodes = registry.dup_globals_by_interface("PipeWire:Interface:Node")
if nodes.get_n_items() > 0:
    node = Pwg.NodeInfo.new_from_global(nodes.get_item(0))
    if node is not None:
        print(node.dup_name() or "", node.dup_media_class() or "")
```

The registry is a discovery API, not a session manager. It does not make routing
or default-device policy decisions.
