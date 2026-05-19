from __future__ import annotations

import os

RUN_ENV = "PWG_RUN_REGISTRY_CHURN"


def skip(message: str) -> int:
    print(message)
    return 77


if os.environ.get(RUN_ENV) != "1":
    raise SystemExit(
        skip(
            f"registry churn regression probe skipped; set {RUN_ENV}=1 to run it"
        )
    )

os.environ.setdefault("G_DEBUG", "fatal-criticals")

import gi  # noqa: E402

gi.require_version("Pwg", "0.1")
from gi.repository import GLib, Pwg  # noqa: E402

failures: list[BaseException] = []


def inspect_global(global_) -> None:
    global_id = global_.get_id()
    interface_type = global_.get_interface_type()
    assert interface_type
    assert global_.get_properties() is not None

    if global_.is_node():
        info = new_info_from_global(Pwg.NodeInfo, global_, global_id, "node", interface_type)
    elif global_.is_device():
        info = new_info_from_global(Pwg.DeviceInfo, global_, global_id, "device", interface_type)
    elif global_.is_link():
        info = new_info_from_global(Pwg.LinkInfo, global_, global_id, "link", interface_type)
    elif global_.is_client():
        info = new_info_from_global(Pwg.ClientInfo, global_, global_id, "client", interface_type)
    elif global_.is_port():
        info = new_info_from_global(Pwg.PortInfo, global_, global_id, "port", interface_type)
    else:
        info = None

    if info is not None:
        assert info.get_global().get_id() == global_id
        assert info.get_id() == global_id


def new_info_from_global(info_type, global_, global_id: int, label: str, interface_type: str):
    try:
        info = info_type.new_from_global(global_)
    except TypeError as exc:
        raise AssertionError(
            f"{label} global {global_id} ({interface_type}) was reported as {label} "
            f"but could not build {info_type.__name__}"
        ) from exc

    assert info is not None, (
        f"{label} global {global_id} ({interface_type}) was reported as {label} "
        f"but returned no {info_type.__name__}"
    )
    return info


def record_global(_registry, global_) -> None:
    try:
        inspect_global(global_)
    except Exception as exc:
        failures.append(exc)


def raise_signal_failures() -> None:
    if failures:
        raise failures[0]


def drain_main_context() -> None:
    context = GLib.MainContext.default()
    while context.pending():
        context.iteration(False)


def sync_object(obj, timeout_ms: int = 2000) -> bool:
    if hasattr(obj, "sync"):
        return bool(obj.sync(timeout_ms))

    deadline = GLib.get_monotonic_time() + (timeout_ms * 1000)
    while GLib.get_monotonic_time() < deadline:
        drain_main_context()
        GLib.usleep(10_000)
    drain_main_context()
    return True


def inspect_registry(registry) -> None:
    model = registry.get_globals()
    for index in range(model.get_n_items()):
        global_ = model.get_item(index)
        inspect_global(global_)


def make_probe_stream(cycle: int, index: int):
    stream = Pwg.Stream.new_audio_capture(None, True)
    stream.set_pipewire_property("application.name", "pwg-registry-churn")
    stream.set_pipewire_property("node.name", f"pwg-registry-churn-{cycle}-{index}")
    stream.set_pipewire_property("state.restore-props", "false")
    stream.set_pipewire_property("state.restore-target", "false")
    return stream


def main() -> int:
    Pwg.init()

    core = Pwg.Core.new()
    try:
        connected = core.connect()
    except GLib.GError as exc:
        return skip(f"PipeWire core unavailable: {exc.message}")
    if not connected:
        return skip("PipeWire core unavailable")

    assert sync_object(core)

    registry = Pwg.Registry.new(core)
    assert registry.start()
    assert sync_object(registry)

    registry.connect("global-added", record_global)
    registry.connect("global-removed", record_global)

    for cycle in range(80):
        streams = [make_probe_stream(cycle, index) for index in range(3)]
        for stream in streams:
            assert stream.start()

        assert sync_object(registry)
        drain_main_context()
        raise_signal_failures()
        inspect_registry(registry)

        for stream in reversed(streams):
            stream.stop()

        assert sync_object(registry)
        drain_main_context()
        raise_signal_failures()
        inspect_registry(registry)

    registry.stop()
    core.disconnect()
    print("registry churn regression probe completed without a PwgGlobal critical")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
