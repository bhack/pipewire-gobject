# Changelog

## 0.3.3 - 2026-05-08

- Add bindable PipeWire property overrides for `Pwg.Core` and `Pwg.Stream` so
  applications can set client and stream identity or policy hints before
  connecting to PipeWire.
- Use more explicit default properties for monitor streams.
- Fix optimized FORTIFY/GNOME SDK builds by avoiding the PipeWire inline
  `spa_pod_builder_pop()` path that can trigger false-positive bounds
  warnings, and add a FORTIFY warning check to CI.

## 0.3.2 - 2026-05-08

- Add `setuptools` to the isolated PyPI source-package build requirements so
  GObject-Introspection's scanner can import `distutils` on Python 3.12+
  environments while generating `Pwg-0.1.gir`.

## 0.3.1 - 2026-05-08

- Add PyPI source-package metadata, a pip wheel GIRepository helper shim, and
  CI/release checks for local wheel smoke testing without publishing binary
  wheels.
- Fix GIRepository shim path registration for local wheel installs.
- Add deterministic GLib fatal-warning and sanitizer release checks.

## 0.3.0 - 2026-05-07

- Use standard C/C99 scalar types in public headers for a more idiomatic
  bindable C API.

## 0.2.0 - 2026-05-07

- Add generated API reference support with `gi-docgen` and a Meson `docs`
  target.
- Add `Pwg.ClientInfo` as an immutable client-focused view of `Pwg.Global`.
- Add `Pwg.Global.is_client()` and a Python client-listing example.
- Add `Pwg.DeviceInfo` as an immutable device-focused view of `Pwg.Global`.
- Add `Pwg.LinkInfo` as an immutable link-focused view of `Pwg.Global`.
- Add `Pwg.PortInfo` as an immutable port-focused view of `Pwg.Global`.
- Add `Pwg.Global.is_port()` and include port names/aliases in generic global
  name and description helpers.
- Add a Python port-listing example.
- Add a Python graph summary example and broaden live discovery smoke coverage.
- Add `Pwg.Node`, `Pwg.ParamInfo`, and `Pwg.Param` as the first read-only
  node parameter inspection API.
- Add typed raw audio format helpers for copied node parameters.
- Add copied `Props` builders for volume, mute, and named float controls, plus
  `Pwg.Node.set_param()` for limited writable node parameter updates.
- Add `Pwg.Core.load_module()` and `Pwg.ImplModule` for app-owned PipeWire
  implementation modules with explicit unload.
- Add deterministic stream dispatch coverage for negotiated format updates,
  `level` signals, and copied `audio-block` delivery.
- Add CI coverage for a WirePlumber-backed live stream audio smoke test with
  copied monitor blocks and non-silent samples.
- Add requested stream format configuration for F32 mono/stereo capture rates.

## 0.1.0 - 2026-05-06

- Reframe the project roadmap around a generic app-facing PipeWire GIR layer
  for discovery, metadata, control helpers, and streams.
- Document that WirePlumber session management, policy, routing decisions, Lua
  scripts, and daemon behavior remain out of scope.
- Add an experimental `0.x` API roadmap.
- Add `Pwg.Metadata` for named PipeWire metadata discovery, cached reads,
  change signals, `set()`, and `clear()`.
- Add `Pwg.Metadata` helpers for standard default and configured audio
  sink/source node names.
- Add `Pwg.Global` convenience accessors and `Pwg.Registry` lookup/filter
  helpers for common discovery properties.
- Add `Pwg.NodeInfo` as an immutable node-focused view of `Pwg.Global`.
- Add a canonical Debian trixie development `Dockerfile`, Ruff configuration,
  and strict warning CI checks.
- Add `Pwg.init()` and library/PipeWire version helpers.
- Add initial `Pwg.Core` GObject wrapper.
- Add `Pwg.Registry` and `Pwg.Global` for safe PipeWire global discovery.
- Add `Pwg.Stream` as a high-level audio capture stream abstraction.
- Add `Pwg.AudioFormat` and `Pwg.AudioBlock` for safe copied stream data.
- Add experimental `Pwg.AudioCapture` compatibility wrapper.
- Generate `Pwg-0.1.gir` and `Pwg-0.1.typelib`.
- Add PyGObject import and live PipeWire smoke tests.
- Add Python peak-meter, registry, and audio-block examples.
- Document experimental support for `libpipewire-0.3 >= 1.0.2`.
- Add rationale, support-policy, and community feedback docs.
