from __future__ import annotations

import sys
import xml.etree.ElementTree as ET
from pathlib import Path

GIR_URI = "http://www.gtk.org/introspection/core/1.0"
C_URI = "http://www.gtk.org/introspection/c/1.0"
GLIB_URI = "http://www.gtk.org/introspection/glib/1.0"

GIR_NS = {
    "gir": GIR_URI,
    "c": C_URI,
    "glib": GLIB_URI,
}


gir_path = Path(sys.argv[1])
root = ET.parse(gir_path).getroot()

package = root.find("gir:package", GIR_NS)
assert package is not None
assert package.attrib["name"] == "pwg-0.1"

namespace = root.find("gir:namespace", GIR_NS)
assert namespace is not None
assert namespace.attrib["name"] == "Pwg"
assert namespace.attrib["version"] == "0.1"
assert namespace.attrib[f"{{{C_URI}}}identifier-prefixes"] == "Pwg"
assert namespace.attrib[f"{{{C_URI}}}symbol-prefixes"] == "pwg"
assert namespace.attrib["shared-library"] == "libpwg-0.1.so.0"

init_function = namespace.find("gir:function[@name='init']", GIR_NS)
assert init_function is not None
assert init_function.attrib["version"] == "0.1"

version_function = namespace.find("gir:function[@name='get_library_version']", GIR_NS)
assert version_function is not None
assert version_function.find("gir:return-value/gir:type", GIR_NS).attrib["name"] == "utf8"

audio_format = namespace.find("gir:class[@name='AudioFormat']", GIR_NS)
assert audio_format is not None
assert audio_format.attrib[f"{{{C_URI}}}type"] == "PwgAudioFormat"
assert audio_format.attrib[f"{{{GLIB_URI}}}get-type"] == "pwg_audio_format_get_type"
audio_format_constructor = audio_format.find("gir:constructor[@name='new']", GIR_NS)
assert audio_format_constructor is not None
assert audio_format_constructor.attrib["version"] == "0.1"
assert audio_format_constructor.attrib["stability"] == "Unstable"
sample_format_param = audio_format_constructor.find(
    "gir:parameters/gir:parameter[@name='sample_format']",
    GIR_NS,
)
assert sample_format_param is not None
assert sample_format_param.attrib.get("nullable") is None
assert audio_format.find("gir:property[@name='bytes-per-frame']", GIR_NS) is not None

audio_block = namespace.find("gir:class[@name='AudioBlock']", GIR_NS)
assert audio_block is not None
assert audio_block.attrib[f"{{{C_URI}}}type"] == "PwgAudioBlock"
assert audio_block.attrib[f"{{{GLIB_URI}}}get-type"] == "pwg_audio_block_get_type"
audio_block_constructor = audio_block.find("gir:constructor[@name='new']", GIR_NS)
assert audio_block_constructor is not None
format_param = audio_block_constructor.find("gir:parameters/gir:parameter[@name='format']", GIR_NS)
assert format_param is not None
assert format_param.find("gir:type", GIR_NS).attrib["name"] == "AudioFormat"
data_param = audio_block_constructor.find("gir:parameters/gir:parameter[@name='data']", GIR_NS)
assert data_param is not None
assert data_param.find("gir:type", GIR_NS).attrib["name"] == "GLib.Bytes"
block_data = audio_block.find("gir:method[@name='get_data']", GIR_NS)
assert block_data is not None
assert block_data.find("gir:return-value/gir:type", GIR_NS).attrib["name"] == "GLib.Bytes"

stream = namespace.find("gir:class[@name='Stream']", GIR_NS)
assert stream is not None
assert stream.attrib[f"{{{C_URI}}}type"] == "PwgStream"
assert stream.attrib[f"{{{GLIB_URI}}}get-type"] == "pwg_stream_get_type"

stream_constructor = stream.find("gir:constructor[@name='new_audio_capture']", GIR_NS)
assert stream_constructor is not None
assert stream_constructor.attrib["version"] == "0.1"
assert stream_constructor.attrib["stability"] == "Unstable"
stream_target_param = stream_constructor.find("gir:parameters/gir:parameter[@name='target_object']", GIR_NS)
assert stream_target_param is not None
assert stream_target_param.attrib.get("nullable") == "1"

stream_start = stream.find("gir:method[@name='start']", GIR_NS)
assert stream_start is not None
assert stream_start.attrib.get("throws") == "1"

stream_level = stream.find("glib:signal[@name='level']", GIR_NS)
assert stream_level is not None
assert stream_level.attrib["version"] == "0.1"
assert stream_level.attrib["stability"] == "Unstable"
stream_peak_param = stream_level.find("gir:parameters/gir:parameter[@name='peak']", GIR_NS)
assert stream_peak_param is not None
assert stream_peak_param.find("gir:type", GIR_NS).attrib["name"] == "gdouble"

stream_audio_block = stream.find("glib:signal[@name='audio-block']", GIR_NS)
assert stream_audio_block is not None
assert stream_audio_block.attrib["version"] == "0.1"
assert stream_audio_block.attrib["stability"] == "Unstable"
stream_block_param = stream_audio_block.find("gir:parameters/gir:parameter[@name='block']", GIR_NS)
assert stream_block_param is not None
assert stream_block_param.find("gir:type", GIR_NS).attrib["name"] == "AudioBlock"

stream_audio_format = stream.find("gir:method[@name='get_audio_format']", GIR_NS)
assert stream_audio_format is not None
assert stream_audio_format.find("gir:return-value/gir:type", GIR_NS).attrib["name"] == "AudioFormat"
assert stream.find("gir:property[@name='deliver-audio-blocks']", GIR_NS) is not None
assert stream.find("gir:property[@name='audio-format']", GIR_NS) is not None

audio_capture = namespace.find("gir:class[@name='AudioCapture']", GIR_NS)
assert audio_capture is not None
assert audio_capture.attrib[f"{{{C_URI}}}type"] == "PwgAudioCapture"
assert audio_capture.attrib[f"{{{GLIB_URI}}}get-type"] == "pwg_audio_capture_get_type"

constructor = audio_capture.find("gir:constructor[@name='new']", GIR_NS)
assert constructor is not None
assert constructor.attrib["version"] == "0.1"
assert constructor.attrib["stability"] == "Unstable"
target_param = constructor.find("gir:parameters/gir:parameter[@name='target_object']", GIR_NS)
assert target_param is not None
assert target_param.attrib.get("nullable") == "1"

start_method = audio_capture.find("gir:method[@name='start']", GIR_NS)
assert start_method is not None
assert start_method.attrib.get("throws") == "1"

level_signal = audio_capture.find(
    "glib:signal[@name='level']",
    GIR_NS,
)
assert level_signal is not None
assert level_signal.attrib["version"] == "0.1"
assert level_signal.attrib["stability"] == "Unstable"
peak_param = level_signal.find("gir:parameters/gir:parameter[@name='peak']", GIR_NS)
assert peak_param is not None
assert peak_param.find("gir:type", GIR_NS).attrib["name"] == "gdouble"

core = namespace.find("gir:class[@name='Core']", GIR_NS)
assert core is not None
assert core.attrib[f"{{{C_URI}}}type"] == "PwgCore"
assert core.attrib[f"{{{GLIB_URI}}}get-type"] == "pwg_core_get_type"

connect_method = core.find("gir:method[@name='connect']", GIR_NS)
assert connect_method is not None
assert connect_method.attrib.get("throws") == "1"

global_class = namespace.find("gir:class[@name='Global']", GIR_NS)
assert global_class is not None
assert global_class.attrib[f"{{{C_URI}}}type"] == "PwgGlobal"
assert global_class.attrib[f"{{{GLIB_URI}}}get-type"] == "pwg_global_get_type"
assert global_class.find("gir:method[@name='get_properties']", GIR_NS) is not None
assert global_class.find("gir:method[@name='dup_property']", GIR_NS) is not None
for method_name in (
    "dup_name",
    "dup_description",
    "dup_media_class",
    "dup_object_serial",
):
    global_dup = global_class.find(f"gir:method[@name='{method_name}']", GIR_NS)
    assert global_dup is not None
    assert global_dup.find("gir:return-value", GIR_NS).attrib["transfer-ownership"] == "full"
    assert global_dup.find("gir:return-value", GIR_NS).attrib.get("nullable") == "1"
global_is_interface = global_class.find("gir:method[@name='is_interface']", GIR_NS)
assert global_is_interface is not None
interface_type_param = global_is_interface.find(
    "gir:parameters/gir:parameter[@name='interface_type']",
    GIR_NS,
)
assert interface_type_param is not None
assert interface_type_param.attrib.get("nullable") is None
assert global_class.find("gir:method[@name='is_client']", GIR_NS) is not None
assert global_class.find("gir:method[@name='is_device']", GIR_NS) is not None
assert global_class.find("gir:method[@name='is_link']", GIR_NS) is not None
assert global_class.find("gir:method[@name='is_node']", GIR_NS) is not None
assert global_class.find("gir:method[@name='is_port']", GIR_NS) is not None
assert global_class.find("gir:method[@name='is_metadata']", GIR_NS) is not None
assert global_class.find("gir:property[@name='interface-type']", GIR_NS) is not None

client_info = namespace.find("gir:class[@name='ClientInfo']", GIR_NS)
assert client_info is not None
assert client_info.attrib[f"{{{C_URI}}}type"] == "PwgClientInfo"
assert client_info.attrib[f"{{{GLIB_URI}}}get-type"] == "pwg_client_info_get_type"
client_info_constructor = client_info.find("gir:constructor[@name='new_from_global']", GIR_NS)
assert client_info_constructor is not None
assert client_info_constructor.find("gir:return-value", GIR_NS).attrib["transfer-ownership"] == "full"
assert client_info_constructor.find("gir:return-value", GIR_NS).attrib.get("nullable") == "1"
client_info_global_param = client_info_constructor.find(
    "gir:parameters/gir:parameter[@name='global']",
    GIR_NS,
)
assert client_info_global_param is not None
assert client_info_global_param.find("gir:type", GIR_NS).attrib["name"] == "Global"
client_info_get_global = client_info.find("gir:method[@name='get_global']", GIR_NS)
assert client_info_get_global is not None
assert client_info_get_global.find("gir:return-value", GIR_NS).attrib["transfer-ownership"] == "none"
assert client_info_get_global.find("gir:return-value/gir:type", GIR_NS).attrib["name"] == "Global"
assert client_info.find("gir:method[@name='get_id']", GIR_NS) is not None
for method_name in (
    "dup_name",
    "dup_app_name",
    "dup_app_id",
    "dup_api",
    "dup_access",
    "dup_process_binary",
    "dup_process_id",
    "dup_object_serial",
):
    client_dup = client_info.find(f"gir:method[@name='{method_name}']", GIR_NS)
    assert client_dup is not None
    assert client_dup.find("gir:return-value", GIR_NS).attrib["transfer-ownership"] == "full"
    assert client_dup.find("gir:return-value", GIR_NS).attrib.get("nullable") == "1"

device_info = namespace.find("gir:class[@name='DeviceInfo']", GIR_NS)
assert device_info is not None
assert device_info.attrib[f"{{{C_URI}}}type"] == "PwgDeviceInfo"
assert device_info.attrib[f"{{{GLIB_URI}}}get-type"] == "pwg_device_info_get_type"
device_info_constructor = device_info.find("gir:constructor[@name='new_from_global']", GIR_NS)
assert device_info_constructor is not None
assert device_info_constructor.find("gir:return-value", GIR_NS).attrib["transfer-ownership"] == "full"
assert device_info_constructor.find("gir:return-value", GIR_NS).attrib.get("nullable") == "1"
device_info_global_param = device_info_constructor.find(
    "gir:parameters/gir:parameter[@name='global']",
    GIR_NS,
)
assert device_info_global_param is not None
assert device_info_global_param.find("gir:type", GIR_NS).attrib["name"] == "Global"
device_info_get_global = device_info.find("gir:method[@name='get_global']", GIR_NS)
assert device_info_get_global is not None
assert device_info_get_global.find("gir:return-value", GIR_NS).attrib["transfer-ownership"] == "none"
assert device_info_get_global.find("gir:return-value/gir:type", GIR_NS).attrib["name"] == "Global"
assert device_info.find("gir:method[@name='get_id']", GIR_NS) is not None
for method_name in (
    "dup_name",
    "dup_description",
    "dup_nick",
    "dup_api",
    "dup_class",
    "dup_form_factor",
    "dup_bus",
    "dup_vendor_name",
    "dup_product_name",
    "dup_media_class",
    "dup_object_serial",
):
    device_dup = device_info.find(f"gir:method[@name='{method_name}']", GIR_NS)
    assert device_dup is not None
    assert device_dup.find("gir:return-value", GIR_NS).attrib["transfer-ownership"] == "full"
    assert device_dup.find("gir:return-value", GIR_NS).attrib.get("nullable") == "1"

link_info = namespace.find("gir:class[@name='LinkInfo']", GIR_NS)
assert link_info is not None
assert link_info.attrib[f"{{{C_URI}}}type"] == "PwgLinkInfo"
assert link_info.attrib[f"{{{GLIB_URI}}}get-type"] == "pwg_link_info_get_type"
link_info_constructor = link_info.find("gir:constructor[@name='new_from_global']", GIR_NS)
assert link_info_constructor is not None
assert link_info_constructor.find("gir:return-value", GIR_NS).attrib["transfer-ownership"] == "full"
assert link_info_constructor.find("gir:return-value", GIR_NS).attrib.get("nullable") == "1"
link_info_global_param = link_info_constructor.find(
    "gir:parameters/gir:parameter[@name='global']",
    GIR_NS,
)
assert link_info_global_param is not None
assert link_info_global_param.find("gir:type", GIR_NS).attrib["name"] == "Global"
link_info_get_global = link_info.find("gir:method[@name='get_global']", GIR_NS)
assert link_info_get_global is not None
assert link_info_get_global.find("gir:return-value", GIR_NS).attrib["transfer-ownership"] == "none"
assert link_info_get_global.find("gir:return-value/gir:type", GIR_NS).attrib["name"] == "Global"
for method_name in (
    "get_id",
    "get_passive",
    "get_feedback",
):
    assert link_info.find(f"gir:method[@name='{method_name}']", GIR_NS) is not None
for method_name in (
    "dup_link_id",
    "dup_output_node_id",
    "dup_output_port_id",
    "dup_input_node_id",
    "dup_input_port_id",
    "dup_object_serial",
):
    link_dup = link_info.find(f"gir:method[@name='{method_name}']", GIR_NS)
    assert link_dup is not None
    assert link_dup.find("gir:return-value", GIR_NS).attrib["transfer-ownership"] == "full"
    assert link_dup.find("gir:return-value", GIR_NS).attrib.get("nullable") == "1"

node_info = namespace.find("gir:class[@name='NodeInfo']", GIR_NS)
assert node_info is not None
assert node_info.attrib[f"{{{C_URI}}}type"] == "PwgNodeInfo"
assert node_info.attrib[f"{{{GLIB_URI}}}get-type"] == "pwg_node_info_get_type"
node_info_constructor = node_info.find("gir:constructor[@name='new_from_global']", GIR_NS)
assert node_info_constructor is not None
assert node_info_constructor.find("gir:return-value", GIR_NS).attrib["transfer-ownership"] == "full"
assert node_info_constructor.find("gir:return-value", GIR_NS).attrib.get("nullable") == "1"
node_info_global_param = node_info_constructor.find(
    "gir:parameters/gir:parameter[@name='global']",
    GIR_NS,
)
assert node_info_global_param is not None
assert node_info_global_param.find("gir:type", GIR_NS).attrib["name"] == "Global"
node_info_get_global = node_info.find("gir:method[@name='get_global']", GIR_NS)
assert node_info_get_global is not None
assert node_info_get_global.find("gir:return-value", GIR_NS).attrib["transfer-ownership"] == "none"
assert node_info_get_global.find("gir:return-value/gir:type", GIR_NS).attrib["name"] == "Global"
assert node_info.find("gir:method[@name='get_id']", GIR_NS) is not None
for method_name in (
    "dup_name",
    "dup_description",
    "dup_media_class",
    "dup_object_serial",
):
    node_dup = node_info.find(f"gir:method[@name='{method_name}']", GIR_NS)
    assert node_dup is not None
    assert node_dup.find("gir:return-value", GIR_NS).attrib["transfer-ownership"] == "full"
    assert node_dup.find("gir:return-value", GIR_NS).attrib.get("nullable") == "1"

node = namespace.find("gir:class[@name='Node']", GIR_NS)
assert node is not None
assert node.attrib[f"{{{C_URI}}}type"] == "PwgNode"
assert node.attrib[f"{{{GLIB_URI}}}get-type"] == "pwg_node_get_type"
node_constructor = node.find("gir:constructor[@name='new']", GIR_NS)
assert node_constructor is not None
assert node_constructor.find("gir:return-value", GIR_NS).attrib["transfer-ownership"] == "full"
assert node_constructor.find("gir:return-value", GIR_NS).attrib.get("nullable") == "1"
node_core_param = node_constructor.find("gir:parameters/gir:parameter[@name='core']", GIR_NS)
assert node_core_param is not None
assert node_core_param.find("gir:type", GIR_NS).attrib["name"] == "Core"
node_global_param = node_constructor.find("gir:parameters/gir:parameter[@name='global']", GIR_NS)
assert node_global_param is not None
assert node_global_param.find("gir:type", GIR_NS).attrib["name"] == "Global"
node_start = node.find("gir:method[@name='start']", GIR_NS)
assert node_start is not None
assert node_start.attrib.get("throws") == "1"
node_enum_params = node.find("gir:method[@name='enum_params']", GIR_NS)
assert node_enum_params is not None
assert node_enum_params.attrib.get("throws") == "1"
node_enum_all_params = node.find("gir:method[@name='enum_all_params']", GIR_NS)
assert node_enum_all_params is not None
assert node_enum_all_params.attrib.get("throws") == "1"
for method_name in (
    "get_core",
    "get_global",
    "get_running",
    "get_bound",
    "get_param_infos",
    "get_params",
):
    assert node.find(f"gir:method[@name='{method_name}']", GIR_NS) is not None
for property_name in (
    "running",
    "bound",
    "param-infos",
    "params",
):
    assert node.find(f"gir:property[@name='{property_name}']", GIR_NS) is not None
node_param_signal = node.find("glib:signal[@name='param']", GIR_NS)
assert node_param_signal is not None
node_param_signal_param = node_param_signal.find("gir:parameters/gir:parameter[@name='param']", GIR_NS)
assert node_param_signal_param is not None
assert node_param_signal_param.find("gir:type", GIR_NS).attrib["name"] == "Param"

param = namespace.find("gir:class[@name='Param']", GIR_NS)
assert param is not None
assert param.attrib[f"{{{C_URI}}}type"] == "PwgParam"
assert param.attrib[f"{{{GLIB_URI}}}get-type"] == "pwg_param_get_type"
for method_name in (
    "get_seq",
    "get_id",
    "dup_name",
    "get_index",
    "get_next",
    "get_pod_type",
    "dup_pod_type_name",
    "get_object_type",
    "dup_object_type_name",
    "get_object_id",
    "dup_object_id_name",
    "get_format_media_type",
    "dup_format_media_type_name",
    "get_format_media_subtype",
    "dup_format_media_subtype_name",
    "dup_audio_format",
    "get_bytes",
    "dup_summary",
):
    assert param.find(f"gir:method[@name='{method_name}']", GIR_NS) is not None
param_audio_format = param.find("gir:method[@name='dup_audio_format']", GIR_NS)
assert param_audio_format.find("gir:return-value", GIR_NS).attrib["transfer-ownership"] == "full"
assert param_audio_format.find("gir:return-value", GIR_NS).attrib.get("nullable") == "1"
assert param_audio_format.find("gir:return-value/gir:type", GIR_NS).attrib["name"] == "AudioFormat"
param_bytes = param.find("gir:method[@name='get_bytes']", GIR_NS)
assert param_bytes.find("gir:return-value", GIR_NS).attrib["transfer-ownership"] == "full"
assert param_bytes.find("gir:return-value/gir:type", GIR_NS).attrib["name"] == "GLib.Bytes"
for method_name in (
    "dup_name",
    "dup_pod_type_name",
    "dup_object_type_name",
    "dup_object_id_name",
    "dup_format_media_type_name",
    "dup_format_media_subtype_name",
    "dup_summary",
):
    param_dup = param.find(f"gir:method[@name='{method_name}']", GIR_NS)
    assert param_dup.find("gir:return-value", GIR_NS).attrib["transfer-ownership"] == "full"
for method_name in (
    "dup_format_media_type_name",
    "dup_format_media_subtype_name",
):
    param_dup = param.find(f"gir:method[@name='{method_name}']", GIR_NS)
    assert param_dup.find("gir:return-value", GIR_NS).attrib.get("nullable") == "1"
for property_name in (
    "seq",
    "id",
    "name",
    "index",
    "next",
    "pod-type",
    "pod-type-name",
    "object-type",
    "object-type-name",
    "object-id",
    "object-id-name",
    "bytes",
    "summary",
):
    assert param.find(f"gir:property[@name='{property_name}']", GIR_NS) is not None

param_info = namespace.find("gir:class[@name='ParamInfo']", GIR_NS)
assert param_info is not None
assert param_info.attrib[f"{{{C_URI}}}type"] == "PwgParamInfo"
assert param_info.attrib[f"{{{GLIB_URI}}}get-type"] == "pwg_param_info_get_type"
for method_name in (
    "get_id",
    "dup_name",
    "get_flags",
    "get_readable",
    "get_writable",
    "get_serial",
):
    assert param_info.find(f"gir:method[@name='{method_name}']", GIR_NS) is not None
param_info_name = param_info.find("gir:method[@name='dup_name']", GIR_NS)
assert param_info_name.find("gir:return-value", GIR_NS).attrib["transfer-ownership"] == "full"
assert param_info_name.find("gir:return-value", GIR_NS).attrib.get("nullable") == "1"
for property_name in (
    "id",
    "name",
    "flags",
    "readable",
    "writable",
    "serial",
):
    assert param_info.find(f"gir:property[@name='{property_name}']", GIR_NS) is not None

port_info = namespace.find("gir:class[@name='PortInfo']", GIR_NS)
assert port_info is not None
assert port_info.attrib[f"{{{C_URI}}}type"] == "PwgPortInfo"
assert port_info.attrib[f"{{{GLIB_URI}}}get-type"] == "pwg_port_info_get_type"
port_info_constructor = port_info.find("gir:constructor[@name='new_from_global']", GIR_NS)
assert port_info_constructor is not None
assert port_info_constructor.find("gir:return-value", GIR_NS).attrib["transfer-ownership"] == "full"
assert port_info_constructor.find("gir:return-value", GIR_NS).attrib.get("nullable") == "1"
port_info_global_param = port_info_constructor.find(
    "gir:parameters/gir:parameter[@name='global']",
    GIR_NS,
)
assert port_info_global_param is not None
assert port_info_global_param.find("gir:type", GIR_NS).attrib["name"] == "Global"
port_info_get_global = port_info.find("gir:method[@name='get_global']", GIR_NS)
assert port_info_get_global is not None
assert port_info_get_global.find("gir:return-value", GIR_NS).attrib["transfer-ownership"] == "none"
assert port_info_get_global.find("gir:return-value/gir:type", GIR_NS).attrib["name"] == "Global"
assert port_info.find("gir:method[@name='get_id']", GIR_NS) is not None
assert port_info.find("gir:method[@name='get_physical']", GIR_NS) is not None
for method_name in (
    "dup_name",
    "dup_description",
    "dup_alias",
    "dup_direction",
    "dup_audio_channel",
    "dup_media_class",
    "dup_object_serial",
):
    port_dup = port_info.find(f"gir:method[@name='{method_name}']", GIR_NS)
    assert port_dup is not None
    assert port_dup.find("gir:return-value", GIR_NS).attrib["transfer-ownership"] == "full"
    assert port_dup.find("gir:return-value", GIR_NS).attrib.get("nullable") == "1"

registry = namespace.find("gir:class[@name='Registry']", GIR_NS)
assert registry is not None
assert registry.attrib[f"{{{C_URI}}}type"] == "PwgRegistry"
assert registry.attrib[f"{{{GLIB_URI}}}get-type"] == "pwg_registry_get_type"

registry_constructor = registry.find("gir:constructor[@name='new']", GIR_NS)
assert registry_constructor is not None
core_param = registry_constructor.find("gir:parameters/gir:parameter[@name='core']", GIR_NS)
assert core_param is not None
assert core_param.find("gir:type", GIR_NS).attrib["name"] == "Core"

registry_start = registry.find("gir:method[@name='start']", GIR_NS)
assert registry_start is not None
assert registry_start.attrib.get("throws") == "1"

globals_method = registry.find("gir:method[@name='get_globals']", GIR_NS)
assert globals_method is not None
assert globals_method.find("gir:return-value/gir:type", GIR_NS).attrib["name"] == "Gio.ListModel"
lookup_by_property = registry.find("gir:method[@name='lookup_global_by_property']", GIR_NS)
assert lookup_by_property is not None
assert lookup_by_property.find("gir:return-value", GIR_NS).attrib["transfer-ownership"] == "full"
assert lookup_by_property.find("gir:return-value", GIR_NS).attrib.get("nullable") == "1"
lookup_by_object_serial = registry.find("gir:method[@name='lookup_global_by_object_serial']", GIR_NS)
assert lookup_by_object_serial is not None
assert lookup_by_object_serial.find("gir:return-value", GIR_NS).attrib["transfer-ownership"] == "full"
assert lookup_by_object_serial.find("gir:return-value", GIR_NS).attrib.get("nullable") == "1"
for method_name in (
    "dup_globals_by_property",
    "dup_globals_by_interface",
    "dup_globals_by_media_class",
):
    registry_filter = registry.find(f"gir:method[@name='{method_name}']", GIR_NS)
    assert registry_filter is not None
    assert registry_filter.find("gir:return-value", GIR_NS).attrib["transfer-ownership"] == "full"
    assert registry_filter.find("gir:return-value/gir:type", GIR_NS).attrib["name"] == "Gio.ListModel"

global_added = registry.find("glib:signal[@name='global-added']", GIR_NS)
assert global_added is not None
global_param = global_added.find("gir:parameters/gir:parameter[@name='global']", GIR_NS)
assert global_param is not None
assert global_param.find("gir:type", GIR_NS).attrib["name"] == "Global"

metadata = namespace.find("gir:class[@name='Metadata']", GIR_NS)
assert metadata is not None
assert metadata.attrib[f"{{{C_URI}}}type"] == "PwgMetadata"
assert metadata.attrib[f"{{{GLIB_URI}}}get-type"] == "pwg_metadata_get_type"

metadata_constructor = metadata.find("gir:constructor[@name='new']", GIR_NS)
assert metadata_constructor is not None
assert metadata_constructor.find("gir:return-value", GIR_NS).attrib["transfer-ownership"] == "full"
metadata_core_param = metadata_constructor.find("gir:parameters/gir:parameter[@name='core']", GIR_NS)
assert metadata_core_param is not None
assert metadata_core_param.find("gir:type", GIR_NS).attrib["name"] == "Core"
assert metadata_core_param.attrib["transfer-ownership"] == "none"

metadata_start = metadata.find("gir:method[@name='start']", GIR_NS)
assert metadata_start is not None
assert metadata_start.attrib.get("throws") == "1"
metadata_dup_value = metadata.find("gir:method[@name='dup_value']", GIR_NS)
assert metadata_dup_value is not None
metadata_dup_value_return = metadata_dup_value.find("gir:return-value", GIR_NS)
assert metadata_dup_value_return.attrib["transfer-ownership"] == "full"
assert metadata_dup_value_return.attrib.get("nullable") == "1"
metadata_dup_value_type = metadata.find("gir:method[@name='dup_value_type']", GIR_NS)
assert metadata_dup_value_type is not None
metadata_dup_value_type_return = metadata_dup_value_type.find("gir:return-value", GIR_NS)
assert metadata_dup_value_type_return.attrib["transfer-ownership"] == "full"
assert metadata_dup_value_type_return.attrib.get("nullable") == "1"
for method_name in (
    "dup_default_audio_sink_name",
    "dup_default_audio_source_name",
    "dup_configured_audio_sink_name",
    "dup_configured_audio_source_name",
):
    default_node = metadata.find(f"gir:method[@name='{method_name}']", GIR_NS)
    assert default_node is not None
    assert default_node.find("gir:return-value", GIR_NS).attrib["transfer-ownership"] == "full"
    assert default_node.find("gir:return-value", GIR_NS).attrib.get("nullable") == "1"
metadata_set = metadata.find("gir:method[@name='set']", GIR_NS)
assert metadata_set is not None
assert metadata_set.attrib.get("throws") == "1"
metadata_type_param = metadata_set.find("gir:parameters/gir:parameter[@name='type']", GIR_NS)
assert metadata_type_param is not None
assert metadata_type_param.attrib.get("nullable") == "1"
metadata_value_param = metadata_set.find("gir:parameters/gir:parameter[@name='value']", GIR_NS)
assert metadata_value_param is not None
assert metadata_value_param.attrib.get("nullable") == "1"
metadata_clear = metadata.find("gir:method[@name='clear']", GIR_NS)
assert metadata_clear is not None
assert metadata_clear.attrib.get("throws") == "1"
metadata_changed = metadata.find("glib:signal[@name='changed']", GIR_NS)
assert metadata_changed is not None
metadata_key_param = metadata_changed.find("gir:parameters/gir:parameter[@name='key']", GIR_NS)
assert metadata_key_param is not None
assert metadata_key_param.attrib.get("nullable") == "1"
metadata_type_signal_param = metadata_changed.find("gir:parameters/gir:parameter[@name='type']", GIR_NS)
assert metadata_type_signal_param is not None
assert metadata_type_signal_param.attrib.get("nullable") == "1"
metadata_value_signal_param = metadata_changed.find("gir:parameters/gir:parameter[@name='value']", GIR_NS)
assert metadata_value_signal_param is not None
assert metadata_value_signal_param.attrib.get("nullable") == "1"

error_enum = namespace.find("gir:enumeration[@name='Error']", GIR_NS)
assert error_enum is not None
assert error_enum.attrib["version"] == "0.1"
assert error_enum.attrib["stability"] == "Unstable"
assert error_enum.attrib[f"{{{GLIB_URI}}}error-domain"] == "pwg-error-quark"
assert namespace.find("gir:function[@name='error_quark']", GIR_NS) is not None

for node in namespace.findall(".//*[@version]", GIR_NS):
    assert node.attrib["version"] == "0.1"
    assert node.attrib["stability"] == "Unstable"

for class_node in namespace.findall("gir:class", GIR_NS):
    method_names = {
        method.attrib["name"]
        for method in class_node.findall("gir:method", GIR_NS)
    }
    property_names = {
        prop.attrib["name"].replace("-", "_")
        for prop in class_node.findall("gir:property", GIR_NS)
    }
    assert property_names.isdisjoint(method_names)

disallowed_collection_types = {
    "GLib.Array",
    "GLib.PtrArray",
    "GLib.ByteArray",
    "GLib.List",
    "GLib.SList",
    "GLib.Queue",
    "GLib.HashTable",
}
disallowed_c_collection_types = {
    "GArray",
    "GPtrArray",
    "GByteArray",
    "GList",
    "GSList",
    "GQueue",
    "GHashTable",
}

for type_node in namespace.findall(".//gir:type", GIR_NS):
    assert type_node.attrib.get("name") not in disallowed_collection_types
    c_type = type_node.attrib.get(f"{{{C_URI}}}type", "")
    c_type = c_type.removeprefix("const ").replace("*", "").strip()
    assert c_type not in disallowed_c_collection_types

callable_tags = {"callback", "constructor", "function", "method"}
for callable_node in namespace.iter():
    tag = callable_node.tag.rsplit("}", 1)[-1]
    if tag not in callable_tags:
        continue

    out_parameters = 0
    for parameter in callable_node.findall("gir:parameters/gir:parameter", GIR_NS):
        direction = parameter.attrib.get("direction", "in")
        assert direction != "inout"
        if direction == "out":
            out_parameters += 1
    assert out_parameters <= 1

assert namespace.findall(".//gir:callback", GIR_NS) == []
assert namespace.findall(".//gir:varargs", GIR_NS) == []

for record_node in namespace.findall("gir:record", GIR_NS):
    assert record_node.attrib["name"].endswith("Class")
    field_names = [
        field.attrib["name"]
        for field in record_node.findall("gir:field", GIR_NS)
    ]
    assert field_names == ["parent_class"]

for elem in namespace.iter():
    c_type = elem.attrib.get(f"{{{C_URI}}}type", "")
    assert "struct pw_" not in c_type
    assert "struct spa_" not in c_type
