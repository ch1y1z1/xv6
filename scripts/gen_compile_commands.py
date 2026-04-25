#!/usr/bin/env python3
"""Generate compile_commands.json from `make -n` output (matches current Makefile / lab)."""

import argparse
import json
import os
import re
import shlex
import subprocess
import sys


def parse_c_compile(tokens):
    """Return source path for a gcc line that contains -c, or None."""
    try:
        i = tokens.index("-c")
    except ValueError:
        return None
    rest = tokens[i + 1 :]
    if not rest:
        return None
    if rest[0] == "-o" and len(rest) >= 3:
        return rest[2]
    if "-o" in rest:
        oi = rest.index("-o")
        if oi >= 1:
            return rest[0]
    return rest[-1]


def synthesize_mkfs_compile(line):
    """mkfs is built as `gcc ... -o mkfs/mkfs mkfs/mkfs.c` without -c."""
    line = line.strip()
    if "mkfs/mkfs.c" not in line or "-c" in line:
        return None
    if not re.match(r"^gcc\s", line):
        return None
    tokens = shlex.split(line)
    try:
        cidx = tokens.index("mkfs/mkfs.c")
    except ValueError:
        return None
    prefix = tokens[:cidx]
    cmd = prefix + ["-c", "-o", "mkfs/mkfs.o", "mkfs/mkfs.c"]
    return cmd


def collect_entries(root, make_stdout):
    entries = []
    seen_files = set()

    for raw in make_stdout.splitlines():
        line = raw.strip()
        if not line or line.startswith("#"):
            continue
        driver = None
        if "riscv64-unknown-elf-gcc" in line or "riscv64-linux-gnu-gcc" in line:
            driver = "cross"
        elif re.match(r"^gcc\s", line) or re.match(r"^\S*gcc\s", line):
            driver = "host"
        else:
            continue

        cmd = None
        src = None

        try:
            tokens = shlex.split(line)
        except ValueError:
            continue

        if "-c" in tokens:
            src = parse_c_compile(tokens)
            cmd = tokens
        elif driver == "host":
            syn = synthesize_mkfs_compile(line)
            if syn:
                cmd = syn
                src = "mkfs/mkfs.c"

        if not cmd or not src:
            continue
        if not any(src.endswith(ext) for ext in (".c", ".S")):
            continue

        rel = os.path.normpath(src)
        key = os.path.normpath(os.path.join(root, rel))
        if key in seen_files:
            continue
        seen_files.add(key)

        entries.append(
            {
                "directory": root,
                "arguments": cmd,
                "file": rel,
            }
        )

    entries.sort(key=lambda e: e["file"])
    return entries


def main():
    ap = argparse.ArgumentParser(
        description="Generate compile_commands.json using make dry-run output."
    )
    ap.add_argument(
        "-o",
        "--output",
        default="compile_commands.json",
        help="output path (default: compile_commands.json in cwd)",
    )
    ap.add_argument(
        "make_args",
        nargs="*",
        default=["-n", "kernel/kernel", "fs.img"],
        help="arguments for make (default: -n kernel/kernel fs.img)",
    )
    args = ap.parse_args()

    root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    make_cmd = ["make"] + args.make_args
    proc = subprocess.run(
        make_cmd,
        cwd=root,
        capture_output=True,
        text=True,
    )
    if proc.returncode != 0:
        sys.stderr.write(proc.stderr or proc.stdout or "make failed\n")
        return proc.returncode

    entries = collect_entries(root, proc.stdout + "\n" + proc.stderr)
    out_path = (
        args.output
        if os.path.isabs(args.output)
        else os.path.join(root, args.output)
    )
    with open(out_path, "w", encoding="utf-8") as f:
        json.dump(entries, f, indent=2)
        f.write("\n")

    print(f"Wrote {len(entries)} entries to {out_path}", file=sys.stderr)
    return 0


if __name__ == "__main__":
    sys.exit(main())
