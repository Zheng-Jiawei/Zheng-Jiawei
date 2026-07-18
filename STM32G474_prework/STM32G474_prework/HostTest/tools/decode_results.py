#!/usr/bin/env python3
"""Decode Keil SAVE files and independently validate the HRC bus timing."""

from __future__ import annotations

import csv
import json
import struct
import sys
from dataclasses import asdict, dataclass
from pathlib import Path


EVENT_SIZE = 12
SUMMARY_FIELDS = (
    "magic",
    "format_version",
    "overall_pass",
    "checks",
    "failures",
    "trace_count",
    "cycle_count",
    "protocol_errors",
    "unsupported_commands",
    "invalid_addresses",
    "adc_latency_ns",
    "octdc_latency_ns",
    "final_time_ns",
    "led2_level",
    "adc_seen",
    "adc_raw",
    "adc_code",
    "octdc_seen",
    "octdc_value",
)

STATE_NAMES = {
    0: "RESET",
    1: "IDLE",
    2: "WRITE_CFG_MODE",
    3: "WRITE_CFG_SINGLE_ADDR",
    4: "WRITE_CFG_SINGLE_VALUE",
    5: "WRITE_CFG_SINGLE_DONE",
    6: "WRITE_CFG_TOTAL",
    7: "READ_CFG_MODE",
    8: "READ_CFG_SINGLE_ADDR",
    9: "READ_CFG_SINGLE_DONE",
    10: "READ_CFG_TOTAL",
    11: "ADC_TEST",
    12: "OCTDC_TEST",
}

PHASE_NAMES = {
    0: "BOOT",
    1: "INITIAL_RESET_IDLE",
    2: "CFG_DEFAULT_READ_TOTAL",
    3: "CFG_SINGLE_WRITE_READ",
    4: "CFG_TOTAL_WRITE_READ",
    5: "ADC_ASYNC_LOCKED",
    6: "OCTDC_ASYNC_LOCKED",
    7: "DRIVER_PARAMETER_GUARDS",
}


@dataclass(frozen=True)
class Event:
    time_ns: int
    model_cycle: int
    operation_cycle: int
    clk: int
    rstn: int
    cmd: int
    valid_out: int
    data_in: int
    data_out: int
    state_id: int
    phase_id: int

    @property
    def state(self) -> str:
        return STATE_NAMES.get(self.state_id, f"UNKNOWN_{self.state_id}")

    @property
    def phase(self) -> str:
        return PHASE_NAMES.get(self.phase_id, f"PHASE_{self.phase_id}")


def read_ihex(path: Path) -> tuple[int, bytes]:
    memory: dict[int, int] = {}
    upper = 0

    for line_number, raw_line in enumerate(path.read_text(encoding="ascii").splitlines(), 1):
        line = raw_line.strip()
        if not line:
            continue
        if not line.startswith(":"):
            raise ValueError(f"{path}:{line_number}: invalid Intel HEX record")
        record = bytes.fromhex(line[1:])
        if (sum(record) & 0xFF) != 0:
            raise ValueError(f"{path}:{line_number}: checksum mismatch")
        length = record[0]
        address = (record[1] << 8) | record[2]
        record_type = record[3]
        data = record[4 : 4 + length]

        if record_type == 0x00:
            absolute = upper + address
            for offset, value in enumerate(data):
                memory[absolute + offset] = value
        elif record_type == 0x04:
            upper = int.from_bytes(data, "big") << 16
        elif record_type == 0x02:
            upper = int.from_bytes(data, "big") << 4
        elif record_type == 0x01:
            break

    if not memory:
        raise ValueError(f"{path}: no data records")
    start = min(memory)
    end = max(memory)
    return start, bytes(memory.get(address, 0) for address in range(start, end + 1))


def decode_event(block: bytes, index: int) -> Event:
    offset = index * EVENT_SIZE
    time_ns, cycle, operation_cycle, flags, data_in, data_out, state, phase = (
        struct.unpack_from("<IH6B", block, offset)
    )
    return Event(
        time_ns=time_ns,
        model_cycle=cycle,
        operation_cycle=operation_cycle,
        clk=flags & 0x01,
        rstn=(flags >> 1) & 0x01,
        cmd=(flags >> 2) & 0x01,
        valid_out=(flags >> 3) & 0x01,
        data_in=data_in,
        data_out=data_out,
        state_id=state,
        phase_id=phase,
    )


def find_command(records: list[Event], phase_id: int, opcode: int, start: int = 0) -> int:
    for index in range(start, len(records)):
        event = records[index]
        if event.phase_id == phase_id and event.cmd == 1 and event.data_in == opcode:
            return index
    raise AssertionError(f"phase {phase_id}: command 0x{opcode:02X} not found")


def bits(value: int, width: int) -> str:
    return format(value & ((1 << width) - 1), f"0{width}b")


def write_events_csv(path: Path, events: list[Event]) -> None:
    with path.open("w", newline="", encoding="utf-8-sig") as handle:
        writer = csv.writer(handle)
        writer.writerow(
            (
                "index",
                "time_ns",
                "time_us",
                "phase",
                "model_cycle",
                "operation_cycle",
                "clk",
                "rstn",
                "cmd",
                "data_in",
                "valid_out",
                "data_out",
                "state",
            )
        )
        for index, event in enumerate(events):
            writer.writerow(
                (
                    index,
                    event.time_ns,
                    f"{event.time_ns / 1000:.3f}",
                    event.phase,
                    event.model_cycle,
                    event.operation_cycle,
                    event.clk,
                    event.rstn,
                    event.cmd,
                    f"0x{event.data_in:02X}",
                    event.valid_out,
                    f"0x{event.data_out:02X}",
                    event.state,
                )
            )


def write_vcd(path: Path, events: list[Event]) -> None:
    signal_defs = (
        ("!", 1, "CLK"),
        ('"', 1, "RSTN"),
        ("#", 1, "CMD"),
        ("$", 8, "DATA_IN"),
        ("%", 1, "VALID_OUT"),
        ("&", 8, "DATA_OUT"),
        ("'", 4, "MODEL_STATE"),
        ("(", 8, "TEST_PHASE"),
        (")", 16, "MODEL_CYCLE"),
        ("*", 8, "OP_CYCLE"),
    )

    with path.open("w", encoding="ascii", newline="\n") as handle:
        handle.write("$date generated by HostTest/tools/decode_results.py $end\n")
        handle.write("$version HRC C behavior model + production STM32 driver $end\n")
        handle.write("$timescale 1ns $end\n$scope module hrc $end\n")
        for identifier, width, name in signal_defs:
            suffix = f" [{width - 1}:0]" if width > 1 else ""
            handle.write(f"$var wire {width} {identifier} {name}{suffix} $end\n")
        handle.write("$upscope $end\n$enddefinitions $end\n")

        previous: tuple[int, ...] | None = None
        for event in events:
            values = (
                event.clk,
                event.rstn,
                event.cmd,
                event.data_in,
                event.valid_out,
                event.data_out,
                event.state_id,
                event.phase_id,
                event.model_cycle,
                event.operation_cycle,
            )
            handle.write(f"#{event.time_ns}\n")
            for signal_index, ((identifier, width, _), value) in enumerate(
                zip(signal_defs, values)
            ):
                if previous is not None and previous[signal_index] == value:
                    continue
                if width == 1:
                    handle.write(f"{value}{identifier}\n")
                else:
                    handle.write(f"b{bits(value, width)} {identifier}\n")
            previous = values


def validate(summary: dict[str, int], events: list[Event], cycles: list[Event]) -> list[dict[str, object]]:
    checks: list[dict[str, object]] = []

    def check(name: str, condition: bool, detail: str) -> None:
        checks.append({"name": name, "pass": bool(condition), "detail": detail})

    check("summary magic", summary["magic"] == 0x48524331, f"0x{summary['magic']:08X}")
    check("C assertions", summary["overall_pass"] == 1 and summary["failures"] == 0,
          f"checks={summary['checks']}, failures={summary['failures']}")
    check("model diagnostics",
          summary["protocol_errors"] == 0
          and summary["unsupported_commands"] == 0
          and summary["invalid_addresses"] == 0,
          "protocol=0, unsupported=0, invalid_address=0")

    falling_times: set[int] = set()
    rising_edges: list[tuple[int, int]] = []
    input_changes: list[tuple[int, Event]] = []
    previous = events[0]
    for event_index, event in enumerate(events[1:], 1):
        if previous.clk == 1 and event.clk == 0:
            falling_times.add(event.time_ns)
        if previous.clk == 0 and event.clk == 1:
            rising_edges.append((event_index, event.time_ns))
        if event.cmd != previous.cmd or event.data_in != previous.data_in:
            input_changes.append((event_index, event))
        previous = event

    setup_times: list[int] = []
    input_setup_ok = bool(input_changes)
    for index, (event_index, event) in enumerate(input_changes):
        next_rising_edge = next(
            (edge for edge in rising_edges if edge[0] > event_index),
            None,
        )
        next_change_index = (
            input_changes[index + 1][0]
            if index + 1 < len(input_changes)
            else None
        )
        if next_rising_edge is None:
            input_setup_ok = False
            continue
        next_rising_index, next_rising = next_rising_edge
        setup_times.append(next_rising - event.time_ns)
        input_setup_ok = input_setup_ok and (
            event.clk == 0
            and event.time_ns in falling_times
            and next_rising - event.time_ns >= 1000
            and (next_change_index is None or next_change_index >= next_rising_index)
        )

    minimum_setup = min(setup_times, default=0)
    check(
        "falling-edge input preparation",
        input_setup_ok,
        f"{len(input_changes)} CMD/DATA_IN transitions on CLK falling; "
        f"minimum setup to next rising={minimum_setup} ns",
    )

    reset_cycles = [event for event in cycles if event.phase_id == 1 and event.rstn == 0]
    reset_ok = len(reset_cycles) >= 8 and all(
        event.valid_out == 0 and event.data_out == 0xA5 for event in reset_cycles[:8]
    )
    check("asynchronous reset signature", reset_ok,
          f"{len(reset_cycles)} reset cycles, first 8 expected VALID=0 DATA=0xA5")

    default_cmd = find_command(cycles, 2, 0x0F)
    default_seq = cycles[default_cmd : default_cmd + 49]
    default_ok = (
        len(default_seq) >= 49
        and default_seq[1].data_out == 0xF0
        and default_seq[1].valid_out == 0
        and all(event.valid_out == 1 and event.data_out == 0 for event in default_seq[2:47])
        and default_seq[47].valid_out == 0
        and default_seq[48].valid_out == 0
        and default_seq[48].data_out == 0
    )
    check("READ_CFG total default timing", default_ok,
          "c1=F0; c2..c46 VALID=1/00; c47 VALID=0; c48 IDLE")

    single_write = find_command(cycles, 3, 0x0E)
    write_seq = cycles[single_write : single_write + 5]
    write_ok = (
        len(write_seq) == 5
        and [event.data_out for event in write_seq[1:4]] == [0xE0, 0xE1, 0xE2]
        and write_seq[2].data_in == 0
        and write_seq[3].data_in == 0x5A
        and write_seq[4].valid_out == 0
        and write_seq[4].data_out == 0
    )
    check("WRITE_CFG single timing", write_ok, "c1=E0, c2=E1, c3=E2, c4=IDLE")

    single_read = find_command(cycles, 3, 0x0F, single_write + 1)
    read_seq = cycles[single_read : single_read + 4]
    read_ok = (
        len(read_seq) == 4
        and read_seq[1].data_out == 0xF0
        and read_seq[2].valid_out == 1
        and read_seq[2].data_out == 0x5A
        and read_seq[3].valid_out == 0
        and read_seq[3].data_out == 0
    )
    check("READ_CFG single timing", read_ok, "c1=F0, c2 VALID=1/DATA=5A, c3=IDLE")

    total_write = find_command(cycles, 4, 0x0E)
    total_write_seq = cycles[total_write : total_write + 49]
    total_write_ok = (
        len(total_write_seq) == 49
        and total_write_seq[1].data_out == 0xE0
        and [event.data_out for event in total_write_seq[2:47]] == list(range(45))
        and total_write_seq[47].data_out == 44
        and total_write_seq[48].data_out == 0
    )
    check("WRITE_CFG total timing", total_write_ok,
          "c2..c46 address 0..44; c47 holds 44; c48 IDLE")

    total_read = find_command(cycles, 4, 0x0F, total_write + 1)
    total_read_seq = cycles[total_read : total_read + 49]
    expected_pattern = [0x55 if (index & 1) == 0 else 0xAA for index in range(45)]
    total_read_ok = (
        len(total_read_seq) == 49
        and total_read_seq[1].data_out == 0xF0
        and [event.data_out for event in total_read_seq[2:47]] == expected_pattern
        and all(event.valid_out == 1 for event in total_read_seq[2:47])
        and total_read_seq[47].valid_out == 0
        and total_read_seq[48].valid_out == 0
        and total_read_seq[48].data_out == 0
    )
    check("READ_CFG total timing", total_read_ok,
          "45 values 55/AA with VALID=1; c47 VALID=0; c48 IDLE")

    adc_events = [event for event in events if event.phase_id == 5 and event.state_id == 11]
    adc_start = min((event.time_ns for event in adc_events), default=-1)
    adc_change = min((event.time_ns for event in adc_events if event.data_out == 0x15), default=-1)
    adc_ok = (
        adc_start >= 0
        and adc_change - adc_start == summary["adc_latency_ns"]
        and all(event.valid_out == 0 for event in adc_events)
        and summary["adc_seen"] == 1
        and summary["adc_raw"] == 0x15
        and summary["adc_code"] == 42
    )
    check("ADC asynchronous timing and bit order", adc_ok,
          f"DATA_OUT 00->15 after {adc_change - adc_start} ns; logical code={summary['adc_code']}")

    octdc_events = [event for event in events if event.phase_id == 6 and event.state_id == 12]
    octdc_start = min((event.time_ns for event in octdc_events), default=-1)
    octdc_change = min((event.time_ns for event in octdc_events if event.data_out == 1), default=-1)
    octdc_ok = (
        octdc_start >= 0
        and octdc_change - octdc_start == summary["octdc_latency_ns"]
        and all(event.valid_out == 0 for event in octdc_events)
        and summary["octdc_seen"] == 1
        and summary["octdc_value"] == 1
    )
    check("OCTDC asynchronous timing", octdc_ok,
          f"DATA_OUT 00->01 after {octdc_change - octdc_start} ns")

    final_event = events[-1]
    check("final IDLE", final_event.state_id == 1 and final_event.valid_out == 0 and final_event.data_out == 0,
          f"state={final_event.state}, VALID={final_event.valid_out}, DATA=0x{final_event.data_out:02X}")
    return checks


def main() -> int:
    results = Path(__file__).resolve().parents[1] / "results"
    _, trace_block = read_ihex(results / "trace.hex")
    _, cycle_block = read_ihex(results / "cycles.hex")
    _, summary_block = read_ihex(results / "summary.hex")

    summary_values = struct.unpack_from("<19I", summary_block, 0)
    summary = dict(zip(SUMMARY_FIELDS, summary_values))
    events = [decode_event(trace_block, index) for index in range(summary["trace_count"])]
    cycles = [decode_event(cycle_block, index) for index in range(summary["cycle_count"])]

    write_events_csv(results / "hrc_trace.csv", events)
    write_events_csv(results / "hrc_cycles.csv", cycles)
    write_vcd(results / "hrc_simulation.vcd", events)

    validation = validate(summary, events, cycles)
    validation_pass = all(item["pass"] for item in validation)
    report = {
        "summary": summary,
        "validation_pass": validation_pass,
        "validation": validation,
        "phase_names": PHASE_NAMES,
        "state_names": STATE_NAMES,
    }
    (results / "hrc_summary.json").write_text(
        json.dumps(report, ensure_ascii=False, indent=2), encoding="utf-8"
    )

    lines = [
        f"overall={'PASS' if summary['overall_pass'] else 'FAIL'}",
        f"independent_validation={'PASS' if validation_pass else 'FAIL'}",
    ]
    lines.extend(f"{key}={value}" for key, value in summary.items())
    for item in validation:
        lines.append(
            f"validation.{item['name']}={'PASS' if item['pass'] else 'FAIL'} | {item['detail']}"
        )
    (results / "hrc_summary.txt").write_text("\n".join(lines) + "\n", encoding="utf-8")
    (results / "hrc_test.log").write_text(
        "\n".join(lines[0:2] + lines[2:7] + lines[21:]) + "\n", encoding="utf-8"
    )
    (results / "hrc_done.flag").write_text(
        "PASS\n" if validation_pass and summary["overall_pass"] else "FAIL\n",
        encoding="ascii",
    )

    print(json.dumps({
        "overall": bool(summary["overall_pass"]),
        "independent_validation": validation_pass,
        "checks": summary["checks"],
        "failures": summary["failures"],
        "trace_events": len(events),
        "cycles": len(cycles),
    }, ensure_ascii=False))
    return 0 if validation_pass and summary["overall_pass"] else 1


if __name__ == "__main__":
    sys.exit(main())
