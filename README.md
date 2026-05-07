# pipewire-gobject

[![CI](https://github.com/bhack/pipewire-gobject/actions/workflows/ci.yml/badge.svg)](https://github.com/bhack/pipewire-gobject/actions/workflows/ci.yml)
[![CodeQL](https://github.com/bhack/pipewire-gobject/actions/workflows/codeql.yml/badge.svg)](https://github.com/bhack/pipewire-gobject/actions/workflows/codeql.yml)
[![Release](https://img.shields.io/github/v/release/bhack/pipewire-gobject?include_prereleases&label=release)](https://github.com/bhack/pipewire-gobject/releases)
[![License: MIT](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)

Experimental GObject/GObject-Introspection binding layer for PipeWire.

This project is a prototype for exposing a safe, high-level, app-facing
PipeWire API to Python, GJS, Vala, and other GI consumers. It is not a complete
PipeWire binding yet, and it is not a mechanical one-to-one binding of the C
API.

The intended direction is a standalone GIR layer for application code that
needs PipeWire discovery, metadata, control helpers, and app-owned streams,
without implementing WirePlumber-specific session management. WirePlumber's
daemon and Lua scripts remain responsible for policy, routing decisions,
default-device behavior, and session-manager logic. See
[docs/rationale.md](docs/rationale.md),
[docs/community-feedback.md](docs/community-feedback.md), and
[docs/roadmap.md](docs/roadmap.md) for the proposed boundary and roadmap.

## Quick Status

| Area | Status |
| --- | --- |
| Latest preview | [`0.3.0`](https://github.com/bhack/pipewire-gobject/releases/tag/0.3.0), experimental and intended for API feedback. |
| Works today | GIR/typelib generation, PyGObject import, registry/global discovery, metadata helpers, typed global views, read-only node param inspection, limited copied `Props` updates, app-owned module handles, and F32 capture streams with copied audio blocks. |
| Still maturing | Stream ergonomics, live graph/data-plane tests, confirmed control workflows, real consumer validation, and higher-level app examples. |
| Not in scope | WirePlumber session-manager policy, routing/default-device policy, smart-filter policy, Lua replacement, or raw PipeWire/SPA ownership in bindings. |

## Feedback Wanted

The project is looking for early API-shape feedback before more public surface
is added:

- whether this boundary is useful for standalone PipeWire GIR bindings;
- whether any part overlaps poorly with WirePlumber's intended public API;
- whether ownership, threading, naming, or GIR annotation choices look
  problematic for long-term binding consumption;
- which real application workflows should validate the API before it grows.

## Documentation Map

- `README.md`: public overview, build instructions, and short examples.
- `docs/rationale.md`: canonical project boundary and non-goals.
- `docs/roadmap.md`: released milestones and next checkpoints.
- `docs/support-policy.md`: `0.x` API, ABI, GIR, dependency, and CI policy.
- `docs/community-feedback.md`: current community-review questions.
- `docs/api/`: extra pages included in the generated API reference.
- `docs/release.md`: maintainer release checklist.

## Build

```bash
meson setup build
meson test -C build --print-errorlogs
```

Canonical dev container:

```bash
docker build -t pipewire-gobject-dev:trixie .
docker run --rm -v "$PWD:/work" -w /work pipewire-gobject-dev:trixie sh -lc '
test -d build || meson setup build
meson compile -C build
meson test -C build --print-errorlogs
ruff check .
'
```

Required development packages include GLib/GObject, GObject Introspection, and
PipeWire development headers. The minimum supported PipeWire build dependency is
`libpipewire-0.3 >= 1.0.2`.

## Support Policy

`0.x` releases are public previews and may change API or ABI. `Pwg-0.1` is the
wrapper API/GIR namespace version; it does not track PipeWire release numbers.

The full policy is in [docs/support-policy.md](docs/support-policy.md).

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

The `examples/python/` directory includes small PyGObject examples for registry,
graph summary, client, device, node, port and link listing, node parameter
inspection, default metadata resolution, peak-meter level signals, and copied
audio block delivery.

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

## API Reference

The generated API reference is built from the installed GIR metadata and the
GI annotations in the public headers. It is not committed to the repository.
Hosted API reference is published at
<https://bhack.github.io/pipewire-gobject/>.

With `gi-docgen` installed:

```bash
meson compile -C build docs
```

The HTML output is written to `build/api-docs/`.

## Live PipeWire Smoke Test

With a PipeWire daemon available:

```bash
GI_TYPELIB_PATH="$PWD/build" \
LD_LIBRARY_PATH="$PWD/build" \
python3 tests/test_live_pipewire.py
```

The script prints the wrapper/PipeWire versions and checks core lifecycle,
registry discovery, metadata helpers, node inspection, stream start/stop, and
the compatibility capture wrapper against a running PipeWire daemon.

## License

MIT.
