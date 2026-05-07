# Community Feedback

This page tracks the current community-review focus for `pipewire-gobject`.
It is intentionally shorter than the canonical project docs:

- [rationale.md](rationale.md): project boundary and non-goals.
- [roadmap.md](roadmap.md): released milestones and next checkpoints.
- [support-policy.md](support-policy.md): `0.x` stability, GIR, dependency, and
  CI policy.

## Current Review Focus

`pipewire-gobject` is an experimental `0.x` GObject/GObject-Introspection
wrapper for app-facing PipeWire APIs. The current question is whether its
boundary is useful before more public API is added.

The intended shape is a standalone, high-level-language PipeWire layer for
applications that need discovery, metadata, limited params/control helpers, and
app-owned streams from Python, GJS, Vala, or other GI consumers.

It should not become a WirePlumber session manager, routing policy layer,
default-device policy engine, smart-filter policy engine, Lua replacement, or
mechanical binding of raw PipeWire/SPA ownership.

## Background

The project started from an app-owned stream use case: replacing app-specific C
or JACK glue in a GTK/Python audio analyzer. Early discussion on the
PipeWire/WirePlumber side suggests that a standalone binding layer could be
useful in the PipeWire ecosystem. Discussion has also raised the possibility
that WirePlumber may eventually remove `libwireplumber` and its GIR/typelib
entirely, although no final decision has been made.

That broadens the long-term target from a stream helper toward a generic
app-facing PipeWire API for audio streaming, filters, analyzers, effects,
stream tools, mixer apps, panel applets, and similar tools.

## Questions For Maintainers And Users

1. Is the generic app-facing binding boundary correct?
2. Which APIs are essential before filter, analyzer, mixer, and panel
   applications can use it?
3. Should metadata and params be exposed as high-level typed helpers, generic
   POD-like builders, or both?
4. Which parts of discovery should be modeled as immutable globals versus live
   proxy objects?
5. Are copied audio blocks and reduced level signals the right first stream
   data surface for GI languages?
6. Are there PipeWire lifecycle or threading assumptions in this prototype that
   should be changed before public users try it?
