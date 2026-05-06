# Support Policy

This policy applies to experimental `0.x` releases.

## API And ABI

- `0.x` releases are public previews, not stable API or ABI commitments.
- Public C symbols use the `pwg_` prefix and are guarded by
  `tests/test_symbols.py`.
- Public GIR metadata is guarded by `tests/test_gir_metadata.py`.
- Public API reference generation is guarded by the `docs` Meson target, which
  runs `gi-docgen check` and `gi-docgen generate`.
- Breaking API or ABI changes are allowed in `0.x`, but release notes must call
  them out clearly.

## GIR Versioning

- `Pwg-0.1` is the wrapper API/GIR namespace version.
- The GIR version does not track PipeWire release numbers.
- Do not create `Pwg-1.6`, `Pwg-1.6.4`, or similar namespaces for PipeWire
  releases.

## PipeWire Dependency

- PipeWire is an external pkg-config dependency, not a vendored submodule.
- The minimum build dependency is `libpipewire-0.3 >= 1.0.2`.
- If code starts using newer PipeWire API, update the Meson dependency floor,
  README, release notes, and CI source-build matrix in the same change.

## CI Coverage

Required CI coverage should stay small and meaningful:

- current Debian packaged PipeWire for a normal distro integration path;
- source-built PipeWire `1.0.2` to prove the declared dependency floor;
- latest pinned stable PipeWire, currently `1.6.4`, to catch forward
  compatibility issues.

The source-built PipeWire jobs may use a cached installed prefix. The source is
a CI test fixture only; it is not part of this repository's source boundary.

## Live Stream Coverage

The current live smoke test verifies that a temporary PipeWire daemon can be
used for core connection, registry discovery, metadata binding/change delivery,
default audio node-name helpers, registry lookup/filter helpers, typed
client/device/node/port/link views for discovered globals, stream start/stop,
and the compatibility capture wrapper.

It does not yet assert negotiated format delivery, `level` signals, or
`audio-block` signals. A future deterministic graph test should cover those
before the stream data API is considered mature.
