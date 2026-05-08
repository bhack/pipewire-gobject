# Release Checklist

This checklist is for experimental public `0.x` releases.

1. Verify the version in `meson.build` and `CHANGELOG.md`.
   Curate `CHANGELOG.md` when preparing a release. It should read like release
   notes, not a commit log: summarize user-visible changes, public API/ABI
   changes, compatibility notes, and release-significant build or documentation
   changes. Ordinary cleanup PRs do not need changelog entries.
2. Run the local test suite:

   ```bash
   rm -rf build
   meson setup build
   meson compile -C build
   meson test -C build --print-errorlogs
   G_DEBUG=fatal-warnings,fatal-criticals \
   G_SLICE=debug-blocks \
     meson test -C build --print-errorlogs

   rm -rf build-sanitize
   meson setup build-sanitize -Db_sanitize=address,undefined -Db_lundef=false
   meson compile -C build-sanitize
   asan="$(gcc -print-file-name=libasan.so)"
   ubsan="$(gcc -print-file-name=libubsan.so)"
   preload="$asan:$ubsan${LD_PRELOAD:+:$LD_PRELOAD}"
   sanitizer_wrapper="env LD_PRELOAD=$preload ASAN_OPTIONS=detect_leaks=0:halt_on_error=1:abort_on_error=1:print_summary=1 UBSAN_OPTIONS=halt_on_error=1:abort_on_error=1:print_summary=1:print_stacktrace=1"
   meson test -C build-sanitize --print-errorlogs --wrapper "$sanitizer_wrapper"
   leak_wrapper="env LD_PRELOAD=$preload ASAN_OPTIONS=detect_leaks=1:halt_on_error=1:abort_on_error=1:print_summary=1 UBSAN_OPTIONS=halt_on_error=1:abort_on_error=1:print_summary=1:print_stacktrace=1"
   meson test -C build-sanitize --print-errorlogs stream-dispatch --wrapper "$leak_wrapper"
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

   Build and inspect the Python source package and local smoke-test wheel:

   ```bash
   python3 -m venv --system-site-packages .venv
   .venv/bin/python -m pip install --upgrade pip
   .venv/bin/python -m pip install build meson-python twine
   rm -rf dist
   .venv/bin/python -m build
   .venv/bin/python -m twine check dist/*

   python3 -m venv --system-site-packages /tmp/pwg-wheel-test
   /tmp/pwg-wheel-test/bin/python -m pip install dist/*.whl
   /tmp/pwg-wheel-test/bin/python - <<'PY'
   import pipewire_gobject
   import gi

   gi.require_version("Pwg", "0.1")
   from gi.repository import Pwg

   assert Pwg.get_library_version()
   print("Pwg wheel import ok", Pwg.get_library_version())
   PY
   ```

   The wheel built by this command is a local validation artifact, not a
   portable manylinux artifact. Do not upload it to PyPI.

   For published releases, prefer the GitHub release workflow so the archive is
   built from the pushed tag in CI rather than uploaded from a local checkout.
   The workflow also builds the PyPI source distribution and runs
   `twine check`, but package-index publishing is opt-in and sdist-only.
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

   To publish the Python source distribution to TestPyPI or PyPI, first
   configure the corresponding GitHub environment and trusted publisher, then
   dispatch the release workflow with `publish_testpypi=true` or
   `publish_pypi=true`. The publish jobs upload only
   `dist/pipewire_gobject-X.Y.Z.tar.gz`.

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
