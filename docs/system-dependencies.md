# System Dependencies

`pipewire-gobject` can be installed from PyPI as a source distribution, but pip
does not install the native GLib, GObject-Introspection, PipeWire, or PyGObject
packages that the build and runtime need. Install the matching distribution
packages before running `python3 -m pip install pipewire-gobject`.

The minimum supported native build dependencies are:

- GLib/GObject/GIO >= 2.66
- GObject-Introspection tools and development headers
- libpipewire-0.3 >= 1.0.2
- pkg-config
- a C compiler and standard build tools

Pip build isolation installs the required Python build backend packages from
`pyproject.toml`, including Meson, meson-python, Ninja, and setuptools. If you
build with pip isolation disabled, make sure those Python packages are
available for the interpreter that runs the build.

Runtime users also need:

- PyGObject for the Python interpreter that imports `Pwg`
- libglib-2.0, libgobject-2.0, libgio-2.0, and libpipewire-0.3 shared libraries
- a running PipeWire daemon for live PipeWire operations

## Debian And Ubuntu

```bash
sudo apt install \
  build-essential \
  gobject-introspection \
  libgirepository1.0-dev \
  libglib2.0-dev \
  libpipewire-0.3-dev \
  pkg-config \
  python3-gi
```

Runtime-only installs generally need:

```bash
sudo apt install \
  libglib2.0-0 \
  libpipewire-0.3-0 \
  pipewire \
  python3-gi
```

Package names vary by release. Newer Debian/Ubuntu releases may split
GObject-Introspection development files under newer `libgirepository` package
names.

## Fedora

```bash
sudo dnf install \
  gcc \
  gobject-introspection-devel \
  glib2-devel \
  pipewire-devel \
  pkgconf-pkg-config \
  python3-gobject
```

Runtime-only installs generally need:

```bash
sudo dnf install \
  glib2 \
  pipewire \
  python3-gobject
```

## Arch Linux

```bash
sudo pacman -S \
  base-devel \
  gobject-introspection \
  glib2 \
  pipewire \
  pkgconf \
  python-gobject
```

`pipewire-gobject` is a binding layer. Applications that use it may have
additional desktop/audio dependencies, such as GTK, Libadwaita, WirePlumber,
PipeWire modules, or JACK compatibility libraries.
