from __future__ import annotations

import subprocess
import sys
from pathlib import Path

EXPECTED_PUBLIC_SYMBOLS = {
    "pwg_audio_block_get_data",
    "pwg_audio_block_get_format",
    "pwg_audio_block_get_n_frames",
    "pwg_audio_block_get_peak",
    "pwg_audio_block_get_sequence",
    "pwg_audio_block_get_type",
    "pwg_audio_block_new",
    "pwg_audio_capture_get_channels",
    "pwg_audio_capture_get_monitor",
    "pwg_audio_capture_get_peak",
    "pwg_audio_capture_get_rate",
    "pwg_audio_capture_get_running",
    "pwg_audio_capture_get_target_object",
    "pwg_audio_capture_get_type",
    "pwg_audio_capture_new",
    "pwg_audio_capture_start",
    "pwg_audio_capture_stop",
    "pwg_audio_format_get_bytes_per_frame",
    "pwg_audio_format_get_bytes_per_sample",
    "pwg_audio_format_get_channels",
    "pwg_audio_format_get_rate",
    "pwg_audio_format_get_sample_format",
    "pwg_audio_format_get_type",
    "pwg_audio_format_new",
    "pwg_client_info_dup_access",
    "pwg_client_info_dup_api",
    "pwg_client_info_dup_app_id",
    "pwg_client_info_dup_app_name",
    "pwg_client_info_dup_name",
    "pwg_client_info_dup_object_serial",
    "pwg_client_info_dup_process_binary",
    "pwg_client_info_dup_process_id",
    "pwg_client_info_get_global",
    "pwg_client_info_get_id",
    "pwg_client_info_get_type",
    "pwg_client_info_new_from_global",
    "pwg_core_connect",
    "pwg_core_disconnect",
    "pwg_core_get_connected",
    "pwg_core_get_library_version",
    "pwg_core_get_type",
    "pwg_core_new",
    "pwg_error_quark",
    "pwg_get_library_version",
    "pwg_get_pipewire_library_version",
    "pwg_global_dup_property",
    "pwg_global_dup_description",
    "pwg_global_dup_media_class",
    "pwg_global_dup_name",
    "pwg_global_dup_object_serial",
    "pwg_global_get_id",
    "pwg_global_get_interface_type",
    "pwg_global_get_permissions",
    "pwg_global_get_properties",
    "pwg_global_get_type",
    "pwg_global_get_version",
    "pwg_global_is_client",
    "pwg_global_is_interface",
    "pwg_global_is_metadata",
    "pwg_global_is_node",
    "pwg_global_is_port",
    "pwg_init",
    "pwg_metadata_clear",
    "pwg_metadata_dup_configured_audio_sink_name",
    "pwg_metadata_dup_configured_audio_source_name",
    "pwg_metadata_dup_default_audio_sink_name",
    "pwg_metadata_dup_default_audio_source_name",
    "pwg_metadata_dup_value",
    "pwg_metadata_dup_value_type",
    "pwg_metadata_get_bound",
    "pwg_metadata_get_core",
    "pwg_metadata_get_name",
    "pwg_metadata_get_running",
    "pwg_metadata_get_type",
    "pwg_metadata_new",
    "pwg_metadata_set",
    "pwg_metadata_start",
    "pwg_metadata_stop",
    "pwg_node_info_dup_description",
    "pwg_node_info_dup_media_class",
    "pwg_node_info_dup_name",
    "pwg_node_info_dup_object_serial",
    "pwg_node_info_get_global",
    "pwg_node_info_get_id",
    "pwg_node_info_get_type",
    "pwg_node_info_new_from_global",
    "pwg_port_info_dup_alias",
    "pwg_port_info_dup_audio_channel",
    "pwg_port_info_dup_description",
    "pwg_port_info_dup_direction",
    "pwg_port_info_dup_media_class",
    "pwg_port_info_dup_name",
    "pwg_port_info_dup_object_serial",
    "pwg_port_info_get_global",
    "pwg_port_info_get_id",
    "pwg_port_info_get_physical",
    "pwg_port_info_get_type",
    "pwg_port_info_new_from_global",
    "pwg_registry_get_core",
    "pwg_registry_get_globals",
    "pwg_registry_get_running",
    "pwg_registry_get_type",
    "pwg_registry_dup_globals_by_interface",
    "pwg_registry_dup_globals_by_media_class",
    "pwg_registry_dup_globals_by_property",
    "pwg_registry_lookup_global",
    "pwg_registry_lookup_global_by_object_serial",
    "pwg_registry_lookup_global_by_property",
    "pwg_registry_new",
    "pwg_registry_start",
    "pwg_registry_stop",
    "pwg_stream_get_audio_format",
    "pwg_stream_get_channels",
    "pwg_stream_get_deliver_audio_blocks",
    "pwg_stream_get_monitor",
    "pwg_stream_get_peak",
    "pwg_stream_get_rate",
    "pwg_stream_get_running",
    "pwg_stream_get_target_object",
    "pwg_stream_get_type",
    "pwg_stream_new_audio_capture",
    "pwg_stream_set_deliver_audio_blocks",
    "pwg_stream_start",
    "pwg_stream_stop",
}


build_dir = Path(sys.argv[1])
libraries = sorted(
    (path for path in build_dir.glob("libpwg-0.1.so*") if path.is_file()),
    key=lambda path: len(path.name),
    reverse=True,
)
assert libraries, "Could not find built libpwg-0.1 shared library"

output = subprocess.check_output(
    ["nm", "-D", "--defined-only", str(libraries[0])],
    text=True,
)
symbols = {
    fields[-1]
    for line in output.splitlines()
    if (fields := line.split())
}

missing = EXPECTED_PUBLIC_SYMBOLS - symbols
assert not missing, f"Missing public symbols: {sorted(missing)}"

unexpected_pwg_symbols = {
    symbol
    for symbol in symbols
    if symbol.startswith("pwg_") and symbol not in EXPECTED_PUBLIC_SYMBOLS
}
assert not unexpected_pwg_symbols, (
    f"Unexpected public pwg_ symbols: {sorted(unexpected_pwg_symbols)}"
)
