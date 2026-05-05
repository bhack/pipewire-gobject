# pipewire-gobject

Experimental GObject/GObject-Introspection wrapper around PipeWire.

This project is a prototype for exposing a safe PipeWire API to Python, GJS,
Vala, and other GI consumers. It is not a complete PipeWire binding yet. It is
intended to complement WirePlumber's GI control-plane API with application-owned
PipeWire data streams.

Use WirePlumber GI for graph, policy, routing, metadata, and session-manager
work. Use this project only for the narrower app-owned stream/data-plane
experiment. See [docs/rationale.md](docs/rationale.md) and
[docs/community-feedback.md](docs/community-feedback.md) for the proposed
community boundary.

## Current Scope

- `Pwg.Core`: create a PipeWire context/core connection.
- `Pwg.Registry`: discover PipeWire globals through a `Gio.ListModel`.
- `Pwg.Global`: immutable descriptors for discovered PipeWire globals.
- `Pwg.Stream`: high-level audio capture stream with optional copied sample
  block delivery.
- `Pwg.AudioFormat`: immutable negotiated audio format descriptor.
- `Pwg.AudioBlock`: immutable copied interleaved audio sample block.
- `Pwg.AudioCapture`: compatibility wrapper around `Pwg.Stream`.
- GIR and typelib generation through Meson.
- Python import smoke test through PyGObject.

## Non-Goals For The Prototype

- Replacing WirePlumber.
- Reimplementing WirePlumber's graph, policy, metadata, node, port, link, or
  session-manager APIs.
- Exposing raw `pw_stream`, `pw_buffer`, `spa_buffer`, or SPA POD ownership to
  dynamic languages.
- Providing stable ABI/API guarantees.

## Status

This repository is suitable for an experimental public `0.x` preview and is
seeking feedback on scope. The most important open question is whether a safe
GI app-stream layer should live standalone, under a PipeWire/WirePlumber
umbrella, or be reshaped before any stability promise.

## Build

```bash
meson setup build
meson test -C build --print-errorlogs
```

Required development packages include GLib/GObject, GObject Introspection, and
PipeWire development headers. The minimum supported PipeWire build dependency is
`libpipewire-0.3 >= 1.0.2`.

## Support Policy

The full support policy is in [docs/support-policy.md](docs/support-policy.md).

- `Pwg-0.1` is the wrapper API/GIR namespace version. It does not track
  PipeWire release numbers.
- `0.x` releases are experimental and may change API or ABI.
- PipeWire is an external pkg-config dependency, not a vendored submodule.
- Builds require `libpipewire-0.3 >= 1.0.2`.
- CI tests the declared minimum PipeWire release, the current Debian packaged
  release, and the latest pinned stable PipeWire release when practical.
- If new wrapper code requires newer PipeWire API, update the Meson dependency
  floor and CI source-build matrix in the same change.

## Python Smoke Test

After building:

```bash
GI_TYPELIB_PATH="$PWD/build" \
LD_LIBRARY_PATH="$PWD/build" \
python3 tests/test_import.py
```

Expected output:

```text
Pwg import ok
```

The `examples/python/` directory includes small PyGObject examples for registry
listing, peak-meter level signals, and copied audio block delivery.

Minimal peak-meter usage:

```python
import gi

gi.require_version("GLib", "2.0")
gi.require_version("Pwg", "0.1")
from gi.repository import GLib, Pwg

Pwg.init()

loop = GLib.MainLoop()
stream = Pwg.Stream.new_audio_capture(None, True)


def on_level(_stream, peak):
    print(f"{peak:.6f}")


stream.connect("level", on_level)
stream.start()

try:
    loop.run()
finally:
    stream.stop()
```

## Live PipeWire Smoke Test

With a PipeWire daemon available:

```bash
GI_TYPELIB_PATH="$PWD/build" \
LD_LIBRARY_PATH="$PWD/build" \
python3 tests/test_live_pipewire.py
```

Expected shape:

```text
pwg-version 0.1.0
pipewire-version 1.x.y
version 1.x.y
core True
registry-start True
registry-count N
registry-first ID TYPE
registry-running-after-stop False
stream-deliver-audio-blocks True
stream-start True
stream-running True
stream-running-after-stop False
capture-start True
capture-running True
capture-running-after-stop False
```

## API And Introspection Policy

The public API should be GI-native and safe for high-level languages. Realtime
PipeWire callbacks should stay inside C, with copied or reduced data delivered
to consumers from a normal GLib main context.

The binding follows the GObject and GObject-Introspection conventions for
public names, constructors, nullable values, ownership transfer, thrown errors,
signals, and properties:

- Public C symbols use the single `pwg_` prefix and map into the `Pwg`
  namespace.
- Public types are GObject wrappers, not raw PipeWire or SPA structs.
- PipeWire registry data is exposed as immutable `Pwg.Global` objects and
  `Gio.ListModel`, not as raw registry/proxy pointers.
- PipeWire stream callbacks stay in C; stream audio data is exposed as safe
  signal/property values instead of raw realtime buffers.
- Audio sample access uses copied `Pwg.AudioBlock` objects with `GLib.Bytes`,
  not borrowed PipeWire buffers.
- Fallible public methods use `GError` and the project `Pwg.Error` domain.
- The generated GIR is tested as part of the Meson test suite.
- The C shared library uses hidden symbol visibility by default; public ABI
  symbols are marked with `PWG_API` and checked in tests.
- Language bindings should consume the installed typelib through
  `libgirepository`.
- `0.x` releases are public but API/ABI unstable.

Reference docs:

- <https://docs.gtk.org/gobject/concepts.html#conventions>
- <https://gi.readthedocs.io/en/latest/writingbindableapis.html>
- <https://gi.readthedocs.io/en/latest/annotations/index.html>
- <https://gi.readthedocs.io/en/latest/writingbindings/libgirepository.html>
- <https://gi.readthedocs.io/en/latest/buildsystems/meson.html>

## License

MIT.
