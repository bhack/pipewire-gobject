# Changelog

## 0.2.0 - Unreleased

- Add immutable typed graph info wrappers for port, link, client, device, and
  metadata globals.
- Add `Pwg.Registry` helpers that return `Gio.ListModel` snapshots of node,
  port, link, client, device, and metadata info wrappers.
- Add `Pwg.Global` helpers for port, link, client, and device interface checks.
- Add a Python graph-listing example for nodes, ports, links, clients,
  devices, and metadata objects.
- Bump the development version to `0.2.0`.

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
