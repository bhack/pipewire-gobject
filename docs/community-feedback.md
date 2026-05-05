# Community Feedback

This project is intentionally small and experimental. The main public question
is whether this data-plane/control-plane boundary is useful, and where such an
API should live long term.

## Summary

`pipewire-gobject` is an experimental `0.x` GObject/GObject-Introspection
wrapper for a small PipeWire app-stream use case. It is intended to complement,
not replace, WirePlumber's GI control-plane API.

The proposed boundary is:

- WirePlumber GI: PipeWire graph, control plane, policy, routing, metadata,
  session-manager state, and management tools.
- `pipewire-gobject`: application-owned streams and safe copied/reduced stream
  data for Python, GJS, Vala, and other GI consumers.

## Problem

GI languages can use WirePlumber for graph/control-plane work, but application
code that wants to own a PipeWire stream still has to deal with native PipeWire
stream callbacks and realtime buffer lifetime rules. Exposing raw `pw_stream`
buffers or SPA buffer ownership directly to dynamic languages would be unsafe
and hard to make idiomatic.

This project explores a smaller API where realtime callbacks remain in C and GI
consumers receive only copied data or simple signal/property values.

## Initial API Shape

The current prototype exposes:

- `Pwg.Core` for minimal PipeWire core connection handling;
- `Pwg.Registry` and `Pwg.Global` for immutable, minimal global discovery;
- `Pwg.Stream` for high-level audio capture;
- `Pwg.AudioFormat` and `Pwg.AudioBlock` for immutable copied audio metadata and
  sample bytes;
- `Pwg.AudioCapture` as a simple compatibility wrapper around `Pwg.Stream`.

The generated namespace is `Pwg-0.1`. It does not track PipeWire release
numbers. The current dependency floor is `libpipewire-0.3 >= 1.0.2`.

## Explicit Non-Goals

This project does not aim to:

- replace WirePlumber;
- expose a complete PipeWire API;
- expose raw `pw_stream`, `pw_buffer`, `spa_buffer`, or `spa_pod` ownership to
  language bindings;
- provide node/port/link routing policy;
- become a session manager;
- duplicate WirePlumber's object-manager API.

`Pwg.Registry` is intentionally minimal and exists for app convenience. Richer
graph inspection and manipulation should remain WirePlumber's role.

## Questions For Maintainers And Users

1. Does this data-plane/control-plane boundary make sense?
2. Should a safe GI app-stream layer live as a standalone project, under the
   PipeWire organization, inside WirePlumber, or somewhere else?
3. Is `Pwg.Registry` acceptable as a minimal app convenience API, or should
   target discovery be delegated entirely to WirePlumber?
4. Are copied audio blocks and reduced level signals the right first data-plane
   surface for GI languages?
5. Are there PipeWire lifecycle or threading assumptions in this prototype that
   should be changed before public users try it?

