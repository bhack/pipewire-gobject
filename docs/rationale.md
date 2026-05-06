# Rationale

`pipewire-gobject` exists to explore a GObject-Introspection friendly PipeWire
API for applications written in Python, GJS, Vala, and other GI languages.

The goal is a standalone, app-facing binding layer that is safer and more
idiomatic than exposing raw PipeWire C objects directly. It should cover common
application needs such as core lifecycle, registry/discovery, metadata, object
properties, limited params, and app-owned streams.

It is not intended to replace the WirePlumber daemon or its session-management
logic. WirePlumber remains responsible for policy, routing decisions,
default-device behavior, smart filters, and Lua scripts loaded inside the
daemon.

## PipeWire Binding Boundary

Use `pipewire-gobject` for app-side access to generic PipeWire concepts:

- core connection, synchronization, and lifecycle;
- registry and global discovery;
- immutable descriptors for discovered objects;
- node/global properties and app-oriented object metadata;
- PipeWire metadata find, set, and change notifications;
- limited params and SPA POD helpers where they can be exposed safely;
- application-owned PipeWire streams;
- copied or reduced stream data delivered to GI consumers.

The API should be high-level and GObject-native. It should use objects,
properties, signals, `Gio.ListModel`, `GError`, and explicit ownership transfer
instead of raw PipeWire pointers and borrowed realtime buffers.

## WirePlumber Boundary

Do not use `pipewire-gobject` to implement WirePlumber-specific session
management:

- policy engines;
- routing decisions;
- default-device selection policy;
- smart-filter policy;
- Lua script replacement;
- daemon component loading or behavior.

The project may expose generic PipeWire metadata, nodes, globals, params, and
streams because those are useful to applications. It should not expose
WirePlumber's policy/session-manager APIs or become a second session manager.

The central safety rule is that GI consumers must not receive ownership of raw
`pw_stream`, `pw_buffer`, `spa_buffer`, `spa_pod`, or realtime buffer memory.
PipeWire callbacks stay in C; public signals and properties expose copied data
or simple values from a normal GLib main context.

## Non-Goals

This project should not grow into:

- a WirePlumber replacement;
- a session manager or routing policy layer;
- a low-level one-to-one binding of every PipeWire C API;
- a raw node/port/link routing policy API;
- a raw SPA POD editing API for dynamic languages;
- a compatibility promise for unstable `0.x` APIs.

APIs for registry, discovery, metadata, params, and streams are in scope when
they are generic PipeWire application APIs. Policy and session-management
behavior remains out of scope.

## Why Standalone For Now

Keeping this as a standalone experimental `0.x` project makes the boundary easy
to review. It lets the community evaluate whether GI-language applications need
a generic app-facing PipeWire binding layer, whether the API should live under
a PipeWire/freedesktop umbrella, and which parts should be stabilized before
applications and distributions depend on it.
