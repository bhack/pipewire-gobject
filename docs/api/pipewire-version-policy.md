Title: PipeWire Version Policy
SPDX-License-Identifier: MIT
SPDX-FileCopyrightText: 2026 pipewire-gobject contributors

# PipeWire Version Policy

`Pwg-0.1` is the wrapper namespace version. It is intentionally independent from
PipeWire release numbers.

The project builds against PipeWire as an external system dependency. It does
not vendor PipeWire source code or expose raw PipeWire ownership to language
bindings.

The current build dependency floor is:

```text
libpipewire-0.3 >= 1.0.2
```

CI should cover:

- the declared minimum PipeWire release
- the current Debian packaged PipeWire release
- the latest pinned stable PipeWire release

When a new PipeWire release is published, the first maintenance step is to
update the latest pinned CI version and verify that the wrapper still builds and
tests cleanly. The dependency floor should only be raised when a new public
wrapper API actually needs newer PipeWire headers or behavior.

New PipeWire APIs should not be exposed mechanically. Add a `Pwg` wrapper only
when the API fits the GI-safe object model, has explicit ownership semantics,
and can be tested from at least one dynamic binding.
