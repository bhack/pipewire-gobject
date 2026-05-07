FROM debian:trixie-slim

ARG RUFF_VERSION=0.15.12

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
 && apt-get install -y --no-install-recommends \
      build-essential \
      ca-certificates \
      clang-format \
      clang-tidy \
      dbus \
      gi-docgen \
      git \
      gobject-introspection \
      libgirepository1.0-dev \
      libglib2.0-dev \
      libpipewire-0.3-dev \
      libspa-0.2-modules \
      meson \
      ninja-build \
      pipewire \
      pkg-config \
      python3 \
      python3-gi \
      python3-venv \
      wireplumber \
 && rm -rf /var/lib/apt/lists/*

RUN python3 -m venv /opt/ruff \
 && /opt/ruff/bin/pip install --no-cache-dir --upgrade pip \
 && /opt/ruff/bin/pip install --no-cache-dir "ruff==${RUFF_VERSION}" \
 && ln -s /opt/ruff/bin/ruff /usr/local/bin/ruff

WORKDIR /work
