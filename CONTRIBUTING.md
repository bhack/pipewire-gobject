# Contributing

This project is experimental. Before proposing broad API additions, keep the
surface small and prove the behavior from at least Python via PyGObject.

Useful local checks:

```bash
meson setup build
meson test -C build --print-errorlogs
python3 tools/run_clang_tidy.py --build-dir build
ruff check .
```

C code follows the existing GLib/GObject-style layout in this repository:
two-space indentation, no tabs, return types on their own line for non-inline
functions, opening braces on their own line for function bodies, and gtk-doc
comments for public API. Use `.editorconfig` for editor defaults.
`clang-tidy` is used for compiler/analyzer diagnostics, not as a formatter.
Avoid broad clang-format rewrites unless they are introduced as a separate,
formatting-only change with a project-specific style file.

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
