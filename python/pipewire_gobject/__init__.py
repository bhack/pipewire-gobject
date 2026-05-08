from __future__ import annotations

import warnings
from pathlib import Path

_CONFIGURED = False
_PRELOADED_LIBRARIES: list[object] = []


def _package_dir() -> Path:
    return Path(__file__).resolve().parent


def _existing_directories(paths: list[Path]) -> tuple[Path, ...]:
    result: list[Path] = []
    seen: set[Path] = set()

    for path in paths:
        try:
            resolved = path.resolve()
        except OSError:
            resolved = path
        if resolved in seen or not path.is_dir():
            continue
        seen.add(resolved)
        result.append(path)

    return tuple(result)


def typelib_paths() -> tuple[Path, ...]:
    """Return bundled GI typelib directories, when this package provides them."""
    package_dir = _package_dir()
    return _existing_directories(
        [
            package_dir / "typelib",
            package_dir / "girepository-1.0",
        ]
    )


def library_paths() -> tuple[Path, ...]:
    """Return bundled shared-library directories, when meson-python provides them."""
    package_dir = _package_dir()
    site_packages = package_dir.parent
    candidates = [
        package_dir / "lib",
        site_packages / ".pipewire_gobject.mesonpy.libs",
        site_packages / ".pipewire-gobject.mesonpy.libs",
    ]
    candidates.extend(sorted(site_packages.glob(".pipewire*gobject*.mesonpy.libs")))
    return _existing_directories(candidates)


def _import_girepository():
    try:
        import gi
    except ImportError as exc:
        raise RuntimeError(
            "pipewire-gobject requires PyGObject to load the Pwg GI namespace. "
            "Install your distribution's python3-gi/python3-gobject package."
        ) from exc

    loaded = False
    for version in ("3.0", "2.0", "1.0"):
        try:
            gi.require_version("GIRepository", version)
        except (AttributeError, ValueError):
            continue
        loaded = True
        break

    with warnings.catch_warnings():
        if not loaded:
            warnings.simplefilter("ignore")
        from gi.repository import GIRepository

    return GIRepository


def _prepend_paths(prepend, paths: tuple[Path, ...]) -> None:
    # GIRepository prepends each path, so iterate in reverse to preserve the
    # caller-facing order returned by typelib_paths() and library_paths().
    for path in reversed(paths):
        prepend(str(path))


def _preload_bundled_library(paths: tuple[Path, ...]) -> None:
    candidates: list[Path] = []
    for path in paths:
        candidates.extend(
            [
                path / "libpwg-0.1.so.0",
                path / "libpwg-0.1.so",
            ]
        )
        candidates.extend(sorted(path.glob("libpwg-0.1.so.*")))

    candidates = [path for path in candidates if path.is_file()]
    if not candidates:
        return

    import ctypes

    errors: list[OSError] = []
    mode = getattr(ctypes, "RTLD_GLOBAL", 0)
    for candidate in candidates:
        try:
            handle = ctypes.CDLL(str(candidate), mode=mode)
        except OSError as exc:
            errors.append(exc)
            continue
        _PRELOADED_LIBRARIES.append(handle)
        return

    message = "; ".join(str(error) for error in errors)
    raise RuntimeError(f"Could not load bundled libpwg-0.1 shared library: {message}")


def configure() -> None:
    """Register bundled Pwg typelib and library directories with GIRepository.

    Call this before ``gi.require_version("Pwg", "0.1")`` when using a
    pip-installed local wheel. Importing this module calls it automatically.
    """
    global _CONFIGURED

    if _CONFIGURED:
        return

    GIRepository = _import_girepository()
    repository = GIRepository.Repository
    libraries = library_paths()
    _prepend_paths(repository.prepend_search_path, typelib_paths())
    _prepend_paths(repository.prepend_library_path, libraries)
    _preload_bundled_library(libraries)
    _CONFIGURED = True


configure()

__all__ = ["configure", "library_paths", "typelib_paths"]
