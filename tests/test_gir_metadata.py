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
assert global_class.find("gir:property[@name='interface-type']", GIR_NS) is not None

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

global_added = registry.find("glib:signal[@name='global-added']", GIR_NS)
assert global_added is not None
global_param = global_added.find("gir:parameters/gir:parameter[@name='global']", GIR_NS)
assert global_param is not None
assert global_param.find("gir:type", GIR_NS).attrib["name"] == "Global"

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

for elem in namespace.iter():
    c_type = elem.attrib.get(f"{{{C_URI}}}type", "")
    assert "struct pw_" not in c_type
    assert "struct spa_" not in c_type
