Title: Getting Started
SPDX-License-Identifier: MIT
SPDX-FileCopyrightText: 2026 pipewire-gobject contributors

# Getting Started

`pipewire-gobject` exposes the `Pwg-0.1` introspection namespace. The namespace
version belongs to this wrapper API; it does not track PipeWire release
numbers.

Python applications load the namespace through PyGObject:

```python
import gi

gi.require_version("Pwg", "0.1")
from gi.repository import Pwg

Pwg.init()
```

Most applications start with a [class@Pwg.Core] and one or more wrapper objects
that use that core connection:

```python
core = Pwg.Core.new()
core.connect()

registry = Pwg.Registry.new(core)
registry.start()
registry.sync(2000)
```

The API is experimental while the project is in `0.x`. Prefer using small
feature probes and keep application code tolerant of API changes until a future
stable release policy exists.

## Capture Streams

Use [class@Pwg.Stream] for app-owned audio capture:

```python
stream = Pwg.Stream.new_audio_capture(None, True)
stream.start()
```

The negotiated audio rate (`stream.get_rate()`) describes delivered samples.
The graph clock rate (`stream.get_graph_rate()`, since 0.3.10) describes the
graph driving the stream. Resampling can make these different. Subscribe to
`notify::graph-rate` for changes on the stream's GLib main context; the value
is zero before processing and after stopping, and otherwise is the last
observed rate. It is not a hardware sample-rate setting. Use the audio block's
format rate for sample analysis, and graph timing only for graph-related
calculations.

## Installed Files

A normal installation provides:

- `libpwg-0.1.so`, the C shared library
- `Pwg-0.1.gir`, the XML introspection data
- `Pwg-0.1.typelib`, the binary runtime typelib used by bindings
- public headers under `pwg-0.1/pwg`

Bindings such as PyGObject and GJS load the installed typelib at runtime.
Build-time tools can inspect the GIR file.
