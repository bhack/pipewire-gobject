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

## 0.1: Foundation (Implemented)

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

## 0.2: Discovery And Metadata (Implemented)

This layer adds generic app-side graph visibility without importing
WirePlumber policy concepts:

- typed wrapper objects for clients, devices, links, ports, nodes, and
  metadata-facing discovery;
- richer property helpers that are safe and idiomatic from GI languages;
- additional discovery filters for application properties and common
  object-specific keys;
- generated API reference support through `gi-docgen`;
- broader live smoke tests for metadata and discovery updates.

## 0.3: Params And Control Helpers (Implemented)

Applications also need limited control APIs, but not a raw low-level SPA API:

- read-only live node parameter inspection through `Pwg.Node`,
  `Pwg.ParamInfo`, and copied `Pwg.Param` values;
- typed raw audio format helpers for copied Format parameters;
- copied `Props` builders for simple volume and mute updates;
- `Pwg.Node.set_param()` with explicit queued-request semantics rather than
  applied-state confirmation;
- tests that inspect generated GIR for ownership, nullable values, and thrown
  errors.

## 0.4: Stream Maturity (Next Checkpoint)

The stream API should become useful beyond peak-meter demos:

- deterministic tests for negotiated audio format delivery;
- tests for `level` and `audio-block` signal delivery;
- bounded buffering and backpressure behavior for copied blocks;
- clearer capture/playback/monitor constructors;
- target selection by object serial or node name where PipeWire supports it;
- examples that feed real analyzer/filter use cases.

Completing this milestone is the next useful project checkpoint. At that point
the library should have discovery, metadata, limited control helpers, and a
credible app-owned stream API. The project should then be validated against
real consumers before expanding wrapper coverage further.

The validation targets should be concrete:

- a filter/analyzer application experiment that removes app-specific JACK or
  custom C glue where the generic API can support it;
- a small mixer or panel-applet style example that lists useful nodes and
  exercises limited copied `Props` updates;
- a stream/analyzer example that proves copied audio blocks are enough for
  real-time UI analysis without exposing raw PipeWire buffers.

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
