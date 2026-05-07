## Summary

Describe the change and the application workflow or maintenance problem it
addresses.

## Scope

- [ ] This keeps the public API GI-friendly and GObject-native.
- [ ] This does not expose raw PipeWire or SPA ownership to language bindings.
- [ ] This does not add WirePlumber session-management or routing policy.
- [ ] Public API/ABI changes are intentional and documented.

## Validation

List the checks you ran, or explain why they were not run.

```text
meson test -C build --print-errorlogs
meson compile -C build docs
ruff check .
```
