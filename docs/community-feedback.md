# Community Feedback

This project is intentionally experimental. The main public question is how a
standalone, GI-friendly PipeWire binding layer should be scoped, and where such
an API should live long term.

## Summary

`pipewire-gobject` is an experimental `0.x` GObject/GObject-Introspection
wrapper for app-facing PipeWire APIs. The intended direction is broader than a
stream helper, but still smaller and safer than a mechanical binding of the
entire PipeWire C API.

The proposed boundary is:

- `pipewire-gobject`: generic PipeWire application APIs such as core lifecycle,
  registry/discovery, globals, node/global properties, metadata, limited params,
  app-owned streams, and safe copied/reduced stream data.
- WirePlumber: session-management policy, routing decisions, default-device
  behavior, smart filters, Lua scripts, and daemon behavior.

## Upstream Direction Feedback

Early maintainer feedback on the PipeWire/WirePlumber side suggests that a
standalone binding layer is a welcome addition to the PipeWire ecosystem.
Maintainers have also noted that WirePlumber may eventually remove
`libwireplumber` and its GIR/typelib entirely, although no final decision has
been made.

That feedback changes the long-term target: this project should not be only an
app-stream wrapper. Ideally, it should grow toward a generic
high-level-language PipeWire API that can support audio streaming and filter
apps, mixer apps, panel applets, and similar tools.

The explicit boundary remains that it should not implement WirePlumber-specific
session-management APIs. Session management is expected to remain inside the
WirePlumber daemon, handled by Lua scripts rather than GIR consumers.

## Problem

GI languages need a safe way to use PipeWire without every application carrying
custom C glue. The useful API surface is broader than streams: panel applets,
mixer apps, filter apps, and audio tools need discovery, metadata, properties,
params, and app-owned streams.

At the same time, exposing raw PipeWire pointers, SPA POD ownership, realtime
buffers, and low-level callbacks directly to dynamic languages would be unsafe
and hard to make idiomatic.

This project explores a higher-level API where PipeWire callbacks remain in C
and GI consumers receive GObject properties, signals, immutable descriptors,
`Gio.ListModel` models, copied data, or simple values.

## Initial API Shape

The current prototype exposes:

- `Pwg.Core` for minimal PipeWire core connection handling;
- `Pwg.Registry` and `Pwg.Global` for immutable global discovery, common
  property accessors, and snapshot filter helpers;
- `Pwg.Metadata` for named metadata discovery, cached reads, change signals,
  `set()`, and `clear()`;
- `Pwg.Stream` for high-level audio capture;
- `Pwg.AudioFormat` and `Pwg.AudioBlock` for immutable copied audio metadata and
  sample bytes;
- `Pwg.AudioCapture` as a simple compatibility wrapper around `Pwg.Stream`.

The next API areas to design are typed node/global wrapper objects, richer
metadata conveniences, params, and a shape suitable for mixer/panel
applications. See [roadmap.md](roadmap.md).

The generated namespace is `Pwg-0.1`. It does not track PipeWire release
numbers. The current dependency floor is `libpipewire-0.3 >= 1.0.2`.

## Explicit Non-Goals

This project does not aim to:

- replace the WirePlumber daemon;
- expose a low-level one-to-one binding of the complete PipeWire C API;
- expose raw `pw_stream`, `pw_buffer`, `spa_buffer`, or `spa_pod` ownership to
  language bindings;
- provide node/port/link routing policy or default-device policy;
- become a session manager;
- duplicate WirePlumber-specific daemon, policy, or Lua script APIs.

## Questions For Maintainers And Users

1. Is the generic app-facing binding boundary correct?
2. Which APIs are essential before mixer/panel/filter applications can use it?
3. Should metadata and params be exposed as high-level typed helpers, generic
   POD-like builders, or both?
4. Which parts of discovery should be modeled as immutable globals versus live
   proxy objects?
5. Are copied audio blocks and reduced level signals the right first stream
   data surface for GI languages?
6. Are there PipeWire lifecycle or threading assumptions in this prototype that
   should be changed before public users try it?
