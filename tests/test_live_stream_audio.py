from __future__ import annotations

import faulthandler
import math
import os
import shutil
import struct
import subprocess
import sys
import tempfile
import threading
import time
from pathlib import Path

SINK_NAME = "pwg-test-sink"
PLAYBACK_NAME = "pwg-test-playback"
RATE = 48000
CHANNELS = 2
PEAK_THRESHOLD = 0.05


def require_commands(commands: list[str]) -> None:
    missing = [command for command in commands if shutil.which(command) is None]
    if missing:
        raise SystemExit(f"missing required command(s): {', '.join(missing)}")


def reexec_with_session_bus() -> None:
    if os.environ.get("PWG_LIVE_STREAM_AUDIO_DBUS") == "1":
        return
    if os.environ.get("DBUS_SESSION_BUS_ADDRESS"):
        return

    dbus_run_session = shutil.which("dbus-run-session")
    if dbus_run_session is None:
        raise SystemExit("missing required command: dbus-run-session")

    env = os.environ.copy()
    env["PWG_LIVE_STREAM_AUDIO_DBUS"] = "1"
    os.execvpe(
        dbus_run_session,
        [dbus_run_session, "--", sys.executable, __file__, *sys.argv[1:]],
        env,
    )


def pipewire_config() -> str:
    return f"""
context.objects = [
  {{ factory = adapter
    args = {{
      factory.name = support.null-audio-sink
      node.name = {SINK_NAME}
      node.description = {SINK_NAME}
      media.class = Audio/Sink
      audio.rate = {RATE}
      audio.channels = {CHANNELS}
      audio.position = [ FL FR ]
      monitor.passthrough = true
      adapter.auto-port-config = {{ mode = dsp monitor = true position = preserve }}
    }}
  }}
]
"""


def run_text(args: list[str], env: dict[str, str]) -> subprocess.CompletedProcess[str]:
    return subprocess.run(args, check=False, env=env, text=True, capture_output=True)


def wait_for(description: str, predicate, timeout: float = 5.0) -> None:
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        if predicate():
            return
        time.sleep(0.1)
    raise RuntimeError(f"timed out waiting for {description}")


def pw_link_iol(env: dict[str, str]) -> str:
    result = run_text(["pw-link", "-iol"], env)
    return result.stdout


def link_ports(source: str, sink: str, env: dict[str, str]) -> None:
    last_error = "pw-link failed"

    def try_link() -> bool:
        nonlocal last_error
        result = run_text(["pw-link", source, sink], env)
        if result.returncode == 0:
            return True
        last_error = result.stderr.strip() or last_error
        return False

    try:
        wait_for(f"{source} -> {sink}", try_link)
    except RuntimeError as exc:
        raise RuntimeError(
            last_error
            + "\n\npw-link -iol:\n"
            + pw_link_iol(env)
        ) from exc


def print_log(label: str, path: Path) -> None:
    print(f"{label}:")
    if not path.exists():
        print("(missing)")
        return
    text = path.read_text(errors="replace")
    print(text[-8000:] if text else "(empty)")


def terminate(process: subprocess.Popen | None) -> None:
    if process is None or process.poll() is not None:
        return
    process.terminate()
    try:
        process.wait(timeout=3)
    except subprocess.TimeoutExpired:
        process.kill()
        process.wait(timeout=3)


def start_playback(env: dict[str, str], stderr_path: Path) -> tuple[subprocess.Popen, threading.Event]:
    stderr_file = stderr_path.open("wb")
    process = subprocess.Popen(
        [
            "pw-cat",
            "--playback",
            "--target",
            "0",
            "--raw",
            "--rate",
            str(RATE),
            "--channels",
            str(CHANNELS),
            "--format",
            "f32",
            "-P",
            f"node.name={PLAYBACK_NAME}",
            "-P",
            "adapter.auto-port-config={ mode = dsp position = preserve }",
            "-",
        ],
        env=env,
        stdin=subprocess.PIPE,
        stdout=subprocess.DEVNULL,
        stderr=stderr_file,
    )
    stderr_file.close()
    stop_event = threading.Event()

    def feed_sine() -> None:
        phase = 0.0
        step = 2.0 * math.pi * 440.0 / RATE
        chunk_frames = 1024
        while not stop_event.is_set() and process.poll() is None:
            samples: list[float] = []
            for _ in range(chunk_frames):
                value = 0.30 * math.sin(phase)
                phase += step
                if phase >= 2.0 * math.pi:
                    phase -= 2.0 * math.pi
                samples.extend([value, value])
            try:
                assert process.stdin is not None
                process.stdin.write(struct.pack("<" + "f" * len(samples), *samples))
                process.stdin.flush()
            except (BrokenPipeError, OSError):
                return
            time.sleep(chunk_frames / RATE)

    threading.Thread(target=feed_sine, name="pwg-test-sine", daemon=True).start()
    return process, stop_event


def main() -> int:
    require_commands(["pipewire", "wireplumber", "pw-cat", "pw-link"])
    reexec_with_session_bus()
    faulthandler.enable()
    faulthandler.dump_traceback_later(30.0, exit=True)

    with tempfile.TemporaryDirectory(prefix="pwg-live-stream-") as temp_name:
        temp_dir = Path(temp_name)
        runtime_dir = temp_dir / "runtime"
        config_dir = temp_dir / "config"
        pipewire_config_dir = config_dir / "pipewire" / "pipewire.conf.d"
        runtime_dir.mkdir()
        runtime_dir.chmod(0o700)
        pipewire_config_dir.mkdir(parents=True)
        (pipewire_config_dir / "99-pwg-live-stream-audio.conf").write_text(pipewire_config())

        env = os.environ.copy()
        env["XDG_RUNTIME_DIR"] = str(runtime_dir)
        env["XDG_CONFIG_HOME"] = str(config_dir)
        os.environ.update(env)

        pipewire_log = temp_dir / "pipewire.log"
        wireplumber_log = temp_dir / "wireplumber.log"
        playback_log = temp_dir / "playback.log"

        pipewire = None
        wireplumber = None
        playback = None
        stop_playback = threading.Event()
        stream = None

        try:
            pipewire_log_file = pipewire_log.open("wb")
            pipewire = subprocess.Popen(
                ["pipewire", "-c", "pipewire.conf"],
                env=env,
                stdout=pipewire_log_file,
                stderr=subprocess.STDOUT,
            )
            pipewire_log_file.close()
            wireplumber_log_file = wireplumber_log.open("wb")
            wireplumber = subprocess.Popen(
                ["wireplumber"],
                env=env,
                stdout=wireplumber_log_file,
                stderr=subprocess.STDOUT,
            )
            wireplumber_log_file.close()

            wait_for(
                "test sink ports",
                lambda: f"{SINK_NAME}:playback_FL" in pw_link_iol(env)
                and f"{SINK_NAME}:monitor_FL" in pw_link_iol(env),
            )

            import gi

            gi.require_version("Pwg", "0.1")
            from gi.repository import GLib, Pwg

            Pwg.init()
            stream = Pwg.Stream.new_audio_capture(SINK_NAME, True)
            stream.set_deliver_audio_blocks(True)
            seen = {"level": 0, "block": 0, "format": False, "nonzero": False}
            loop = GLib.MainLoop()

            def on_level(_stream, peak):
                seen["level"] += 1
                if peak > PEAK_THRESHOLD:
                    print(f"level {peak:.6f}")

            def on_audio_block(_stream, block):
                peak = block.get_peak()
                seen["block"] += 1
                if peak > PEAK_THRESHOLD:
                    seen["nonzero"] = True
                    print(
                        "audio-block "
                        f"frames={block.get_n_frames()} peak={peak:.6f} "
                        f"sequence={block.get_sequence()}"
                    )
                    loop.quit()

            def on_audio_format(*_args):
                audio_format = stream.get_audio_format()
                if audio_format is None:
                    return
                seen["format"] = True
                print(
                    "format "
                    f"{audio_format.get_sample_format()} "
                    f"{audio_format.get_rate()}Hz "
                    f"{audio_format.get_channels()}ch"
                )

            stream.connect("level", on_level)
            stream.connect("audio-block", on_audio_block)
            stream.connect("notify::audio-format", on_audio_format)
            if not stream.start():
                raise RuntimeError("Pwg.Stream did not start")

            playback, stop_playback = start_playback(env, playback_log)
            wait_for(
                "playback ports",
                lambda: f"{PLAYBACK_NAME}:output_FL" in pw_link_iol(env),
            )

            for channel in ("FL", "FR"):
                link_ports(
                    f"{PLAYBACK_NAME}:output_{channel}",
                    f"{SINK_NAME}:playback_{channel}",
                    env,
                )

            GLib.timeout_add_seconds(10, loop.quit)
            loop.run()

            if not seen["format"]:
                raise RuntimeError("stream format was not negotiated")
            if not seen["nonzero"]:
                raise RuntimeError("stream did not receive non-silent audio")

            print(
                "live-stream-audio-ok "
                f"levels={seen['level']} blocks={seen['block']}"
            )
            return 0
        except Exception:
            print_log("pipewire log", pipewire_log)
            print_log("wireplumber log", wireplumber_log)
            print_log("playback log", playback_log)
            raise
        finally:
            stop_playback.set()
            terminate(playback)
            if stream is not None:
                stream.stop()
            terminate(wireplumber)
            terminate(pipewire)


if __name__ == "__main__":
    raise SystemExit(main())
