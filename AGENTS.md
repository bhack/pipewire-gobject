# Agent Guide

These notes apply to the whole repository.

## Project Context

This is an experimental GObject/GObject-Introspection binding layer for
PipeWire. The goal is to let Python, GJS, Vala, and other GI consumers use a
safe, high-level, app-facing subset of PipeWire without binding raw PipeWire or
SPA ownership directly.

The project is currently suitable for public `0.x` preview releases. Treat the
public API and ABI as unstable unless a future release policy says otherwise.

## Design Rules

- Keep the binding GLib/GObject-first. Public APIs should be introspectable and
  natural from PyGObject, GJS, and Vala.
- Do not expose raw PipeWire or SPA buffer ownership to language bindings.
- The intended scope is broader than app-owned streams: core lifecycle,
  registry/discovery, globals, metadata, properties, limited params, and streams
  are all valid app-facing API areas when they are modeled safely.
- Keep realtime PipeWire callbacks minimal. A production implementation must
  marshal signals and data delivery to a normal GLib main context instead of
  emitting directly from the PipeWire process callback.
- Keep stream callbacks in C and expose only copied/reduced values or GLib
  signals/properties to language bindings.
- Use `GError` for fallible public methods and add project-specific error
  domains before calling the API stable.
- Prefer small, safe wrapper objects over a mechanical one-to-one mapping of
  every PipeWire C type.
- Keep WirePlumber's role separate: this project may expose generic PipeWire
  core, registry, metadata, object/property, param, stream, and data-plane
  wrappers, but it should not become a session manager, routing policy layer,
  default-device policy engine, smart-filter implementation, or Lua script
  replacement.

## Code Map

- `.github/workflows/ci.yml`: Debian packaged PipeWire and source-built
  PipeWire compatibility CI.
- `.github/workflows/codeql.yml`: CodeQL C/C++ analysis.
- `.github/workflows/dependency-review.yml`: GitHub dependency review for PRs.
- `.github/workflows/release.yml`: tag/manual release archive builder and
  GitHub prerelease publisher.
- `.github/dependabot.yml`: weekly GitHub Actions dependency updates.
- `Dockerfile`: canonical Debian trixie development and validation image.
- `docs/pwg.toml` and `docs/api/`: gi-docgen configuration and additional
  generated API reference content.
- `ruff.toml`: Python lint configuration for tests and examples.
- `src/pwg-core.*`: minimal PipeWire core/context wrapper.
- `src/pwg-audio-format.*`: immutable audio format descriptor.
- `src/pwg-audio-block.*`: immutable copied audio sample block.
- `src/pwg-stream.*`: high-level PipeWire stream wrapper.
- `src/pwg-audio-capture.*`: compatibility audio capture wrapper.
- `src/pwg-client-info.*`: immutable client-focused view of a registry global.
- `src/pwg-defs.h`: public export macros for the C ABI.
- `src/pwg-device-info.*`: immutable device-focused view of a registry global.
- `src/pwg-global.*`: immutable descriptors for PipeWire registry globals.
- `src/pwg-impl-module.*`: app-owned PipeWire implementation module handles.
- `src/pwg-link-info.*`: immutable link-focused view of a registry global.
- `src/pwg-metadata.*`: named PipeWire metadata wrapper.
- `src/pwg-node.*`: live node proxy for parameter inspection and limited
  copied parameter updates.
- `src/pwg-node-info.*`: immutable node-focused view of a registry global.
- `src/pwg-param.*`: immutable copied node parameter enumeration result,
  typed parameter readers, and copied parameter builders.
- `src/pwg-param-info.*`: immutable node parameter capability descriptor.
- `src/pwg-port-info.*`: immutable port-focused view of a registry global.
- `src/pwg-registry.*`: PipeWire registry discovery wrapper.
- `src/pwg.h`: aggregate public include.
- `tests/test_import.py`: PyGObject import and basic API smoke test.
- `tests/test_stream_dispatch.c`: deterministic stream format, level, and
  copied audio-block dispatch test.
- `tests/test_symbols.py`: exported-symbol guard for the shared library.
- `meson.build`: library, GIR, typelib, and test build rules.
- `docs/roadmap.md`: experimental `0.x` scope and API growth plan.

## Development Commands

### Native Build

```bash
meson setup build
meson test -C build --print-errorlogs
meson compile -C build docs
```

Clean rebuild:

```bash
rm -rf build
meson setup build
meson test -C build --print-errorlogs
meson compile -C build docs
```

### Container Iteration

For faster container iteration, keep a local dev image and reuse the Meson
`build/` directory:

```bash
docker build -t pipewire-gobject-dev:trixie .

docker run --rm -v "$PWD:/work" -w /work pipewire-gobject-dev:trixie sh -lc '
set -e
test -d build || meson setup build >/dev/null
meson compile -C build >/dev/null
meson test -C build --print-errorlogs
meson compile -C build docs
ruff check .
'
```

With rootless Docker, use the default container user for bind-mounted repo
writes. In this setup container root maps back to the host user; passing
`--user "$(id -u):$(id -g)"` can map to an unmapped subordinate identity and
make `/work` unwritable. Keeping `--rm` is fine: it removes only the stopped
container, not the cached image layers or the bind-mounted `build/` directory.

Do not run source-built PipeWire compatibility jobs for ordinary local
iteration. They are intentionally covered by GitHub CI with cached prefixes;
reproduce them locally only when debugging a version-specific failure or when
changing the declared PipeWire dependency floor.

### Live Smoke Test

Clean container smoke test with a temporary PipeWire daemon:

```bash
docker run --rm -v "$PWD:/work" -w /work pipewire-gobject-dev:trixie sh -lc '
set -e
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

Optional local reproduction of the CI stream data-plane smoke test with a
private D-Bus session, WirePlumber, a virtual sink, and generated playback
audio:

```bash
docker build -t pipewire-gobject-dev:trixie .
docker run --rm -v "$PWD:/work" -w /work pipewire-gobject-dev:trixie sh -lc '
set -e
test -d build || meson setup build >/dev/null
meson compile -C build >/dev/null
GI_TYPELIB_PATH=/work/build LD_LIBRARY_PATH=/work/build python3 tests/test_live_stream_audio.py
'
```

CI runs this against Debian's packaged PipeWire/WirePlumber stack. Keep it out
of ordinary local iteration unless you are touching stream data delivery,
monitor capture, or the container audio fixture.

Strict compiler and GIR scanner warning check:

```bash
docker run --rm -v "$PWD:/work" -w /work pipewire-gobject-dev:trixie sh -lc '
set -e
rm -rf build-strict
meson setup build-strict -Dwerror=true >/dev/null
meson compile -C build-strict >/dev/null
meson test -C build-strict --print-errorlogs
'
```

### GI Tool Diagnostics

- Meson invokes `g-ir-scanner` and `g-ir-compiler` through
  `gnome.generate_gir()`. Do not hand-maintain scanner command lines in normal
  development.
- To make scanner warnings fatal during a focused check, configure with
  `-Dwerror=true`; this also makes C compiler warnings fatal in this project.
- If the scanner behaves unexpectedly, run the compile with
  `GI_SCANNER_DEBUG=save-temps` so the generated introspection helper files are
  kept for inspection.
- Compile the generated GIR as a standalone artifact:

  ```bash
  g-ir-compiler --includedir=build --output=/tmp/Pwg-0.1.typelib build/Pwg-0.1.gir
  ```

- Inspect what bindings can reconstruct from the typelib:

  ```bash
  g-ir-generate build/Pwg-0.1.typelib >/tmp/Pwg-0.1.roundtrip.gir
  ```

- Build the generated API reference without adding generated docs to the
  repository:

  ```bash
  meson compile -C build docs
  ```

## Public API Expectations

Use the GObject Introspection docs as the operational baseline, especially the
GI bindable API guidance and annotations reference:

- <https://gi.readthedocs.io/en/latest/writingbindableapis.html>
- <https://gi.readthedocs.io/en/latest/annotations/index.html>

The following rules apply when adding public `Pwg` API.

### API Shape

- Follow GObject naming so GIR groups functions, methods, properties, and
  namespace members correctly.
- Prefer GObject classes, interfaces, or boxed types over raw structs with
  custom ownership.
- Do not expose raw PipeWire or SPA pointers, buffers, PODs, or ownership to GI
  consumers.
- Do not create macro-only or inline-only public API. Bindable API must have an
  exported function, method, property, or signal that exists in the shared
  library.
- Avoid varargs, in-out parameters, multiple out parameters, callback-heavy
  entry points, and public struct fields.
- Use accessors, properties, signals, and simple return values instead of
  direct C structure access.
- Use `GError` for fallible methods and keep error quark names aligned with the
  public error enum.
- Keep `*_new()` constructors equivalent to `g_object_new()` with construct
  properties. Put initialization logic in `*_init()` or `constructed`, not in
  the constructor helper only.
- Treat `const char *` public parameters as NUL-terminated UTF-8 strings. Use
  `GLib.Bytes` or an explicitly annotated array for byte data that may contain
  embedded NULs.
- Do not give a property and method the same binding-visible name.

### Ownership And Data

- Use `GLib.Bytes` for binary data, `Gio.ListModel` for object lists, and
  narrow wrapper objects or `GVariant` for structured values. Avoid exposing
  `GArray`, `GPtrArray`, `GByteArray`, `GList`, `GSList`, `GQueue`, or
  `GHashTable` in public GIR.
- Annotate nullable values, transfer ownership, arrays, callbacks, signals, and
  thrown errors explicitly.
- Use `(nullable)` only when `NULL` is a valid value passed in or returned to
  the caller. Use `(optional)` only for out/inout parameters that the caller may
  pass as `NULL` to ignore. Do not use deprecated forms such as `(allow-none)`,
  `(null-ok)`, or `(in-out)` in source comments.
- Add explicit `(transfer full)`, `(transfer none)`, or `(transfer container)`
  annotations for public object, boxed, string, variant, and container returns
  when the default would not make ownership obvious to bindings.
- Avoid public callback parameters. When a callback API is unavoidable, annotate
  callback/user-data/destroy ownership with `(scope)`, `(closure)`, and
  `(destroy)` and add a PyGObject smoke test for the binding shape.
- If future public interfaces or vfuncs can be implemented by bindings, avoid
  transfer-none object, boxed, and string return values from those callbacks.

### GIR Tooling And Tests

- Keep `gnome.generate_gir()` aligned with the installed pkg-config package
  through `export_packages`.
- Assume consumers use typelibs through existing bindings such as PyGObject,
  GJS, Vala, and libgirepository consumers; do not design a parallel custom
  binding layer.
- Keep constructor and method names stable only after they have been exercised
  from Python.
- Mark every exported C function and public GType declaration with `PWG_API`;
  the library is built with hidden symbol visibility by default.
- Update `tests/test_gir_metadata.py` for every deliberate public GIR contract
  addition.
- Update `tests/test_symbols.py` for every deliberate C ABI addition.
- Update `docs/api/` or public doc comments when new API changes developer
  workflows, then run `meson compile -C build docs`.

## Release Notes

Before publishing, run the release checklist in `docs/release.md`. Keep the
community boundary in `docs/rationale.md`, `docs/community-feedback.md`, and
`docs/roadmap.md` aligned with any public API growth. Public release notes must
say that `0.x` releases are experimental and may change API or ABI.
