# Rationale

`pipewire-gobject` exists to explore a small GObject-Introspection friendly
data-plane API for applications that own PipeWire streams.

It is not intended to replace WirePlumber's GObject API. WirePlumber already
provides the richer control-plane API for graph inspection, policy, routing,
metadata, session management, and tools that manage PipeWire rather than stream
media themselves.

## Existing WirePlumber Boundary

Use WirePlumber GI for:

- discovering and managing devices, nodes, ports, links, clients, factories,
  metadata, settings, permissions, and session-manager state;
- writing PipeWire management or status tools;
- extending WirePlumber or building custom session-manager behavior;
- working with PipeWire object parameters through WirePlumber's object model.

Those APIs are intentionally richer than this project and should remain the
default recommendation for PipeWire graph/control-plane work from GI languages.

## Proposed Pwg Boundary

Use `pipewire-gobject` for:

- application-owned PipeWire streams;
- copied or reduced audio data delivered to Python, GJS, Vala, or other GI
  consumers;
- small immutable descriptors such as `Pwg.AudioFormat`, `Pwg.AudioBlock`, and
  `Pwg.Global`;
- minimal registry discovery needed for app target selection.

The central safety rule is that GI consumers must not receive ownership of raw
`pw_stream`, `pw_buffer`, `spa_buffer`, `spa_pod`, or realtime buffer memory.
PipeWire callbacks stay in C; public signals and properties expose copied data
or simple values from a normal GLib main context.

## Non-Goals

This project should not grow into:

- a general PipeWire binding;
- a WirePlumber replacement;
- a session manager or routing policy layer;
- a node/port/link object manager;
- a raw SPA POD editing API for dynamic languages.

`Pwg.Registry` and `Pwg.Global` are deliberately minimal. They are useful for
small app workflows, but richer discovery and graph manipulation should remain
WirePlumber territory.

## Why Standalone For Now

Keeping this as a standalone experimental `0.x` project makes the boundary easy
to review. It lets the community evaluate whether GI-language applications need
a safe app-stream layer at all, and whether that layer should remain separate,
move under a PipeWire/WirePlumber umbrella, or be reshaped before any stability
promise is made.

