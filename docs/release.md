# Release Checklist

This checklist is for the first public `0.x` releases.

1. Verify the version in `meson.build` and `CHANGELOG.md`.
2. Run the local test suite:

   ```bash
   rm -rf build
   meson setup build
   meson compile -C build
   meson test -C build --print-errorlogs
   ```

3. Run the container smoke test from `AGENTS.md`.
4. Verify the installed layout:

   ```bash
   rm -rf install-root
   DESTDIR="$PWD/install-root" meson install -C build --no-rebuild
   ```

5. Build a release archive:

   ```bash
   meson dist -C build
   ```

6. Verify CI has passed against:

   - the declared minimum PipeWire dependency,
   - the current Debian packaged PipeWire dependency,
   - the latest pinned stable PipeWire dependency.

7. Review the public scope docs before announcement:

   - `docs/rationale.md`
   - `docs/support-policy.md`
   - `docs/community-feedback.md`

8. Inspect the generated GIR for namespace, ownership, nullable, `throws`,
   signal parameter names, property getter links, and the `Pwg.Error` domain.
   The `gir-metadata` test should cover the expected public contract.
9. Inspect exported symbols when adding public C API. The `public-symbols` test
   should cover the expected `PWG_API` surface.
10. Check new public API against the GObject and GI bindable API references:

   - <https://docs.gtk.org/gobject/concepts.html#conventions>
   - <https://gi.readthedocs.io/en/latest/writingbindableapis.html>
   - <https://gi.readthedocs.io/en/latest/annotations/index.html>
   - <https://gi.readthedocs.io/en/latest/writingbindings/libgirepository.html>
   - <https://gi.readthedocs.io/en/latest/buildsystems/meson.html>

11. Do not announce API stability for `0.x` releases. Mention that the ABI/API is
   experimental and may change.
12. Keep the live stream test limitation visible in release notes if it has not
    been replaced by a deterministic graph test: the current smoke test verifies
    stream start/stop against a temporary PipeWire daemon, but not negotiated
    audio block delivery.
