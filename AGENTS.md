# Agent Guide

These notes apply to the whole repository.

## Project Context

This is an experimental GObject/GObject-Introspection wrapper around PipeWire.
The goal is to let Python, GJS, Vala, and other GI consumers use a safe subset
of PipeWire without binding raw `pw_stream` buffers directly.

The project is currently suitable for public `0.x` preview releases. Treat the
public API and ABI as unstable unless a future release policy says otherwise.

## Design Rules

- Keep the binding GLib/GObject-first. Public APIs should be introspectable and
  natural from PyGObject, GJS, and Vala.
- Do not expose raw PipeWire or SPA buffer ownership to language bindings.
- Treat WirePlumber GI as the richer graph/control-plane API. Keep this project
  focused on app-owned PipeWire streams and safe data-plane wrappers unless a
  future task explicitly changes that scope.
- Keep realtime PipeWire callbacks minimal. A production implementation must
  marshal signals and data delivery to a normal GLib main context instead of
  emitting directly from the PipeWire process callback.
- Keep stream callbacks in C and expose only copied/reduced values or GLib
  signals/properties to language bindings.
- Use `GError` for fallible public methods and add project-specific error
  domains before calling the API stable.
- Prefer small, safe wrapper objects over a mechanical one-to-one mapping of
  every PipeWire C type.
- Keep WirePlumber's role separate: this project may expose PipeWire core,
  registry, stream, and data-plane wrappers, but it should not become a session
  manager or routing policy layer.

## Code Map

- `.github/workflows/ci.yml`: Debian packaged PipeWire and source-built
  PipeWire compatibility CI.
- `.github/workflows/codeql.yml`: CodeQL C/C++ analysis.
- `.github/workflows/dependency-review.yml`: GitHub dependency review for PRs.
- `.github/dependabot.yml`: weekly GitHub Actions dependency updates.
- `src/pwg-core.*`: minimal PipeWire core/context wrapper.
- `src/pwg-audio-format.*`: immutable audio format descriptor.
- `src/pwg-audio-block.*`: immutable copied audio sample block.
- `src/pwg-stream.*`: high-level PipeWire stream wrapper.
- `src/pwg-audio-capture.*`: compatibility audio capture wrapper.
- `src/pwg-defs.h`: public export macros for the C ABI.
- `src/pwg-global.*`: immutable descriptors for PipeWire registry globals.
- `src/pwg-registry.*`: PipeWire registry discovery wrapper.
- `src/pwg.h`: aggregate public include.
- `tests/test_import.py`: PyGObject import and basic API smoke test.
- `tests/test_symbols.py`: exported-symbol guard for the shared library.
- `meson.build`: library, GIR, typelib, and test build rules.

## Development Commands

Native build:

```bash
meson setup build
meson test -C build --print-errorlogs
```

Clean rebuild:

```bash
rm -rf build
meson setup build
meson test -C build --print-errorlogs
```

Container smoke test with a temporary PipeWire daemon:

```bash
docker run --rm -v "$PWD:/work" -w /work debian:trixie-slim sh -lc '
set -e
export DEBIAN_FRONTEND=noninteractive
apt-get update >/dev/null
apt-get install -y --no-install-recommends \
  build-essential meson ninja-build pkg-config \
  libglib2.0-dev libpipewire-0.3-dev \
  gobject-introspection libgirepository1.0-dev python3 python3-gi \
  pipewire libspa-0.2-modules >/dev/null
rm -rf build /tmp/pwg-runtime
meson setup build >/dev/null
meson compile -C build >/dev/null
mkdir -p /tmp/pwg-runtime
chmod 700 /tmp/pwg-runtime
export XDG_RUNTIME_DIR=/tmp/pwg-runtime
pipewire >/tmp/pwg-pipewire.log 2>&1 &
pw_pid=$!
trap "kill $pw_pid 2>/dev/null || true" EXIT
sleep 1
meson test -C build --print-errorlogs
GI_TYPELIB_PATH=/work/build LD_LIBRARY_PATH=/work/build python3 tests/test_live_pipewire.py
'
```

## Public API Expectations

- Annotate nullable arguments, transfer ownership, and thrown errors explicitly
  for GIR. If Python behavior is surprising, inspect the generated `.gir`.
- Follow the GObject naming conventions and GI bindable-API guidance:
  <https://docs.gtk.org/gobject/concepts.html#conventions> and
  <https://gi.readthedocs.io/en/latest/writingbindableapis.html>.
- Check the full GI annotations reference when adding a new public method,
  signal, property, callback, array, or non-scalar type:
  <https://gi.readthedocs.io/en/latest/annotations/index.html>.
- Keep `gnome.generate_gir()` aligned with the Meson integration guidance,
  including an `export_packages` entry for the installed pkg-config package:
  <https://gi.readthedocs.io/en/latest/buildsystems/meson.html>.
- For language-binding design, assume consumers use typelibs through
  `libgirepository`: <https://gi.readthedocs.io/en/latest/writingbindings/libgirepository.html>.
- Keep constructor and method names stable only after they have been exercised
  from Python.
- Prefer high-level objects such as `PwgStream`, `PwgAudioBlock`, and
  `PwgAudioCapture` over exposing raw `pw_stream`, `pw_buffer`, `spa_buffer`,
  or `spa_pod` directly.
- Stream data delivered to language bindings must be copied out of realtime
  PipeWire buffers and marshalled back to a normal GLib main context.
- Expose PipeWire discovery results as immutable GObject data or GLib container
  interfaces such as `GListModel`. Do not expose raw registry/proxy pointers to
  GIR consumers.
- Mark every exported C function and public GType declaration with `PWG_API`;
  the library is built with hidden symbol visibility by default.
- Update `tests/test_gir_metadata.py` for every deliberate public GIR contract
  addition.
- Update `tests/test_symbols.py` for every deliberate C ABI addition.

## Release Notes

Before publishing, run the release checklist in `docs/release.md`. Keep the
community boundary in `docs/rationale.md` and `docs/community-feedback.md`
aligned with any public API growth. Public release notes must say that `0.x`
releases are experimental and may change API or ABI.
