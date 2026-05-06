# Roadmap

This roadmap is for experimental `0.x` development. It is a direction of travel,
not a stability promise.

The target is a standalone, high-level, GI-friendly PipeWire binding layer for
applications. It should be useful for audio streaming, filters, analyzers,
effects, stream tools, mixer apps, panel applets, and similar tools written in
Python, GJS, Vala, or other GI languages.

It must not become a WirePlumber session manager. WirePlumber-specific policy,
routing decisions, default-device policy, smart filters, Lua scripts, and daemon
behavior remain out of scope.

## 0.1: Foundation

The first preview establishes the library and GIR shape:

- `Pwg.init()` and version helpers.
- `Pwg.Core` for basic PipeWire connection lifecycle.
- `Pwg.Registry` and `Pwg.Global` for immutable global discovery through
  `Gio.ListModel`.
- Common `Pwg.Global` property accessors and `Pwg.Registry` lookup/filter
  helpers for app-side discovery.
- `Pwg.NodeInfo` for node-focused views of immutable global descriptors.
- `Pwg.Metadata` for named metadata discovery, cached reads, change signals,
  `set()`, and `clear()`.
- `Pwg.Metadata` helpers for standard default and configured audio sink/source
  node names.
- `Pwg.Stream` for app-owned audio capture.
- `Pwg.AudioFormat` and `Pwg.AudioBlock` for copied audio data.
- `Pwg.AudioCapture` as a compatibility wrapper around `Pwg.Stream`.
- GIR metadata and public-symbol tests.
- Python examples for registry and node listing, default metadata resolution,
  peak levels, and audio blocks.

## 0.2: Discovery And Metadata

The next useful layer is generic app-side graph visibility without importing
WirePlumber policy concepts:

- typed wrapper objects for additional global interfaces such as ports,
  clients, devices, links, and metadata objects;
- richer property helpers that are safe and idiomatic from GI languages;
- additional discovery filters for application properties and common
  object-specific keys;
- clear object lifetime rules for globals that disappear;
- broader live smoke tests for metadata and discovery updates.

## 0.3: Params And Control Helpers

Applications also need limited control APIs, but not a raw low-level SPA API:

- safe builders for common params used by applications;
- enough SPA POD helpers for simple node params without exposing raw ownership;
- typed convenience helpers where common PipeWire params are stable enough;
- read-only live node parameter inspection before writable controls;
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

## Later: Higher-Level Application Workflows

Once the object model holds up, grow toward high-level app workflows:

- filter, analyzer, and effect-app helpers where generic PipeWire APIs can
  support them cleanly;
- volume/mute helpers where they can be modeled cleanly;
- richer default-node metadata and control helpers;
- app stream lists suitable for panel applets;
- mixer-friendly node and route views;
- examples that cover both audio-tool workflows and pavucontrol-like
  inspection/control flows.
