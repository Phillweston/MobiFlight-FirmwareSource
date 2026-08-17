import json
import re
from collections import Counter
from pathlib import Path

ROOT = Path(__file__).resolve().parents[3]
PROJECT = ROOT / "docs" / "FENIX A320 OVHD 自锁版 V1_0_2.mfproj"
OUTPUT = Path(__file__).with_name("entities.generated.yaml")

INPUT_MODULES = {
    "InputShifter": 0, "Shifter APU": 1, "Shifter APU 2": 2,
    "Shifter LEFT": 3, "Shifter RIGHT": 4,
    "Multiplexer": 0, "Multiplexer 1": 1,
}
OUTPUT_MODULES = {
    "Shift LEFT": 0, "Shift RIGHT": 1, "Shift CENTER": 2,
    "Shift APU": 3, "Shift FIRE": 4,
}
OUTPUT_GPIOS = {
    "backlight": 0, "LED LIGHT": 1, "LEDV": 2, "WHITE STROBE": 3,
    "GREEN LED|RED LED": 4, "WHTIE RWY": 5, "WHITE NOSE": 6,
}


def slug(value: str) -> str:
    value = re.sub(r"^FENIX_A320_OVHD_", "", value).lower()
    return re.sub(r"[^a-z0-9]+", "_", value).strip("_")


def quoted(value: str) -> str:
    return json.dumps(value, ensure_ascii=False)


def main() -> None:
    project = json.loads(PROJECT.read_text(encoding="utf-8-sig"))
    items = [item for item in project["ConfigFiles"][0]["ConfigItems"] if item.get("Active")]
    inputs = [item for item in items if item.get("DeviceType") in {"InputShiftRegister", "InputMultiplexer"}]
    analog = [item for item in items if item.get("DeviceType") == "AnalogInput"]
    outputs = [item for item in items if item.get("DeviceType") in {"ShiftRegister", "Output"}]

    input_names = Counter(
        item["Name"].replace("FENIX_A320_OVHD_", "").replace("_", " ").strip()
        for item in inputs
    )
    output_names = Counter(
        item["Name"].replace("FENIX_A320_OVHD_", "").replace("_", " ").strip()
        for item in outputs
    )

    lines = ["# Generated from FENIX A320 OVHD V1_0_2.mfproj. Do not edit.", "mobiflight_bridge:", "  id: a320_bridge", "  sda_pin: 2", "  scl_pin: 1", "  address: 0x42", "  analog_raw_max: 1023", "  segment_module: 0", "  setup_ap_password: !secret fallback_ap_password", "  bat1_display:", "    name: BAT1 Display", "    id: bat1_display", "  bat2_display:", "    name: BAT2 Display", "    id: bat2_display", "  digital_inputs:"]
    for item in inputs:
        is_shifter = item["DeviceType"] == "InputShiftRegister"
        action = item["inputShiftRegister"] if is_shifter else item["inputMultiplexer"]
        channel = int(action["ExtPin"] if is_shifter else action["DataPin"])
        object_id = f"{slug(item['Name'])}_{slug(item['DeviceName'])}_{channel}"
        name = item["Name"].replace("FENIX_A320_OVHD_", "").replace("_", " ").strip()
        if input_names[name] > 1:
            name = f"{name} [{item['DeviceName']} {channel}]"
        lines += [
            f"    - name: {quoted(name)}",
            f"      id: input_{object_id}",
            f"      event_type: {'0x03' if is_shifter else '0x04'}",
            f"      module: {INPUT_MODULES[item['DeviceName']]}",
            f"      channel: {channel}",
            f"      press_only: {'true' if action.get('onRelease') is None else 'false'}",
    ]

    lines += ["  analog_inputs:"]
    for module, item in enumerate(analog):
        object_id = slug(item["Name"])
        name = item["Name"].replace("FENIX_A320_OVHD_", "").replace("_", " ")
        lines += [
            f"    - name: {quoted(name + ' Position')}",
            f"      id: analog_{object_id}",
            f"      module: {module}",
            "      raw_sensor:",
            f"        name: {quoted(name + ' Raw')}",
            f"        id: analog_{object_id}_raw",
        ]

    lines += ["  digital_outputs:"]
    for item in outputs:
        is_shifter = item["DeviceType"] == "ShiftRegister"
        if is_shifter:
            channel = int(item["Device"]["Pin"].removeprefix("Output "))
            module = OUTPUT_MODULES[item["DeviceName"]]
            event_type = "0x06"
        else:
            channel = OUTPUT_GPIOS[item["DeviceName"]]
            module = 0
            event_type = "0x02"
        object_id = f"{slug(item['Name'])}_{slug(item['DeviceName'])}_{channel}"
        name = item["Name"].replace("FENIX_A320_OVHD_", "").replace("_", " ").strip()
        if output_names[name] > 1:
            name = f"{name} [{item['DeviceName']} {channel}]"
        lines += [
            f"    - name: {quoted(name + ' Output')}",
            f"      id: output_{object_id}",
            f"      event_type: {event_type}",
            f"      module: {module}",
            f"      channel: {channel}",
        ]

    OUTPUT.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(f"Generated {len(inputs)} inputs, {len(analog)} analog inputs and {len(outputs)} outputs in {OUTPUT}")


if __name__ == "__main__":
    main()
