# Roadmap

This roadmap is for experimental `0.x` development. It is a direction of travel,
not a stability promise.

The target is a standalone, high-level, GI-friendly PipeWire binding layer for
applications. It should be useful for audio streaming and filter apps, mixer
apps, panel applets, and similar tools written in Python, GJS, Vala, or other
GI languages.

It must not become a WirePlumber session manager. WirePlumber-specific policy,
routing decisions, default-device policy, smart filters, Lua scripts, and daemon
behavior remain out of scope.

## 0.1: Foundation

The first preview establishes the library and GIR shape:

- `Pwg.init()` and version helpers.
- `Pwg.Core` for basic PipeWire connection lifecycle.
- `Pwg.Registry` and `Pwg.Global` for immutable global discovery through
  `Gio.ListModel`.
- `Pwg.Metadata` for named metadata discovery, cached reads, change signals,
  `set()`, and `clear()`.
- `Pwg.Stream` for app-owned audio capture.
- `Pwg.AudioFormat` and `Pwg.AudioBlock` for copied audio data.
- `Pwg.AudioCapture` as a compatibility wrapper around `Pwg.Stream`.
- GIR metadata and public-symbol tests.
- Python examples for registry listing, metadata reads, peak levels, and audio
  blocks.

## 0.2: Discovery And Metadata

The next useful layer is generic app-side graph visibility without importing
WirePlumber policy concepts:

- typed helpers for common global interfaces such as nodes, ports, clients, and
  metadata objects;
- property access helpers that are safe and idiomatic from GI languages;
- optional discovery filters for media class, node name, object serial, and
  application properties;
- clear object lifetime rules for globals that disappear;
- live smoke tests for metadata and discovery updates.

## 0.3: Params And Control Helpers

Applications also need limited control APIs, but not a raw low-level SPA API:

- safe builders for common params used by applications;
- enough SPA POD helpers for simple node params without exposing raw ownership;
- typed convenience helpers where common PipeWire params are stable enough;
- async or sync completion patterns that behave predictably from PyGObject and
  GJS;
- tests that inspect generated GIR for ownership, nullable values, and thrown
  errors.

## 0.4: Stream Maturity

The stream API should become useful beyond peak-meter demos:

- deterministic tests for negotiated audio format delivery;
- tests for `level` and `audio-block` signal delivery;
- bounded buffering and backpressure behavior for copied blocks;
- clearer capture/playback/monitor constructors;
- target selection by object serial or node name where PipeWire supports it;
- examples that feed real analyzer/filter use cases.

## Later: Mixer And Panel App Use Cases

Once the object model holds up, grow toward high-level app workflows:

- volume/mute helpers where they can be modeled cleanly;
- default-node metadata helpers;
- app stream lists suitable for panel applets;
- mixer-friendly node and route views;
- examples that cover pavucontrol-like inspection and simple control flows.
