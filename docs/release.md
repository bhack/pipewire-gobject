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
4. Build the generated API reference:

   ```bash
   meson compile -C build docs
   ```

5. Verify the installed layout:

   ```bash
   rm -rf install-root
   DESTDIR="$PWD/install-root" meson install -C build --no-rebuild
   ```

6. Build a release archive:

   ```bash
   meson dist -C build
   ```

   For published releases, prefer the GitHub release workflow so the archive is
   built from the pushed tag in CI rather than uploaded from a local checkout.
   Pushing a bare version tag triggers the workflow and creates a draft
   prerelease by default:

   ```bash
   version=X.Y.Z
   git tag -a "$version" -m "pipewire-gobject $version"
   git push origin "$version"
   ```

   To rerun the release build for an existing tag, dispatch the workflow
   manually:

   ```bash
   gh workflow run release.yml --ref main \
     -f tag_name="$version" \
     -f create_github_release=true \
     -f draft=true \
     -f prerelease=true
   ```

7. Verify CI has passed against:

   - the declared minimum PipeWire dependency,
   - the current Debian packaged PipeWire dependency,
   - the latest pinned stable PipeWire dependency.

8. Review the public scope docs before announcement:

   - `docs/rationale.md`
   - `docs/support-policy.md`
   - `docs/community-feedback.md`

9. Inspect the generated GIR for namespace, ownership, nullable, `throws`,
   signal parameter names, property getter links, and the `Pwg.Error` domain.
   The `gir-metadata` test should cover the expected public contract.
10. Inspect exported symbols when adding public C API. The `public-symbols` test
   should cover the expected `PWG_API` surface.
11. Check new public API against the GObject and GI bindable API references:

   - <https://docs.gtk.org/gobject/concepts.html#conventions>
   - <https://gi.readthedocs.io/en/latest/writingbindableapis.html>
   - <https://gi.readthedocs.io/en/latest/annotations/index.html>
   - <https://gi.readthedocs.io/en/latest/writingbindings/libgirepository.html>
   - <https://gi.readthedocs.io/en/latest/buildsystems/meson.html>

12. Do not announce API stability for `0.x` releases. Mention that the ABI/API is
   experimental and may change.
13. Keep stream test limitations visible in release notes: the lightweight
    temporary-daemon smoke verifies stream start/stop, the heavier
    WirePlumber-backed smoke verifies copied monitor blocks through a temporary
    graph, and a future deterministic graph test should still replace the
    heavier fixture before the stream data API is considered mature.
