# Contributing

This project is experimental. Before proposing broad API additions, keep the
surface small and prove the behavior from at least Python via PyGObject.

Useful local checks:

```bash
meson setup build
meson test -C build --print-errorlogs
ruff check .
```

For changes touching PipeWire stream behavior, run a live smoke test with a
temporary PipeWire daemon as documented in `AGENTS.md`.

The repository root `Dockerfile` is the canonical development image when the
host does not have the C/GI/PipeWire dependencies installed:

```bash
docker build -t pipewire-gobject-dev:trixie .
```

## API Guidelines

- Prefer safe GObject wrappers over raw PipeWire C type exposure.
- Add GIR annotations for nullable values, ownership transfer, and errors.
- Do not emit public signals directly from PipeWire realtime callbacks.
- Keep WirePlumber/session-manager policy out of this project.
