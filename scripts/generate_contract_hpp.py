#!/usr/bin/env python3
"""
Generates the C++ validation-constants header (namespace ``contract``) from
contract/asyncapi.yaml.

The AsyncAPI constraint schemas are the single source of truth for validation
limits. This script is the backend counterpart to the frontend's
scripts/generate-schemas.js, which derives the same numbers for the Svelte
client — keeping C++ and TypeScript limits in lock-step with zero hand-editing.

Usage: generate_contract_hpp.py <asyncapi.yaml> <output.hpp>
"""
import os
import sys

import yaml

# Constraint schema (components/schemas) -> {schema field: C++ constant name}.
# Insertion order here is the order the constants are emitted.
CONSTRAINT_MAP = {
    "Username":  {"minLength": "kUsernameMin", "maxLength": "kUsernameMax"},
    "Password":  {"minLength": "kPasswordMin"},
    "LobbyName": {"maxLength": "kLobbyNameMax"},
    "LobbyCode": {"minLength": "kLobbyCodeLen"},
}

# MAX_LOBBY_MEMBERS is expressed as a maxItems constraint on the lobby members
# array inside the LobbyList server message rather than as a named schema.
MAX_MEMBERS_PATH = [
    "LobbyList", "payload", "properties", "lobbies",
    "items", "properties", "members", "maxItems",
]


def die(msg):
    sys.stderr.write(f"[generate-contract-hpp] error: {msg}\n")
    sys.exit(1)


def walk(node, path):
    for key in path:
        if not isinstance(node, dict):
            return None
        node = node.get(key)
        if node is None:
            return None
    return node


def main():
    if len(sys.argv) != 3:
        die("usage: generate_contract_hpp.py <asyncapi.yaml> <output.hpp>")
    src, out = sys.argv[1], sys.argv[2]

    with open(src, "r", encoding="utf-8") as f:
        data = yaml.safe_load(f)

    components = data.get("components") or {}
    schemas = components.get("schemas") or {}
    messages = components.get("messages") or {}

    consts = {}  # name -> int value (insertion-ordered)

    for schema_name, field_map in CONSTRAINT_MAP.items():
        schema = schemas.get(schema_name)
        if schema is None:
            die(f"constraint schema '{schema_name}' not found in {src}")
        for field, const_name in field_map.items():
            if field not in schema:
                die(f"'{schema_name}.{field}' not found in {src}")
            consts[const_name] = schema[field]

    max_members = walk(messages, MAX_MEMBERS_PATH)
    if max_members is None:
        die("could not locate maxItems for lobby members (kMaxLobbyMembers); "
            "expected at components/messages/" + "/".join(MAX_MEMBERS_PATH))
    consts["kMaxLobbyMembers"] = max_members

    width = max(len(name) for name in consts)
    lines = [
        "#pragma once",
        "",
        "// AUTO-GENERATED — do not edit by hand.",
        "// Source:     contract/asyncapi.yaml (constraint schemas)",
        "// Generator:  scripts/generate_contract_hpp.py (run by CMake target gen_contract_hpp)",
        "//",
        "// These are the same validation limits the frontend derives via",
        "// scripts/generate-schemas.js, so C++ and TypeScript never drift.",
        "namespace contract {",
    ]
    for name, value in consts.items():
        lines.append(f"    inline constexpr int {name.ljust(width)} = {int(value)};")
    lines.append("}  // namespace contract")
    lines.append("")
    text = "\n".join(lines)

    # Write only when the content actually changes so we don't bump the header's
    # mtime and force a needless recompile of everything that includes it.
    try:
        with open(out, "r", encoding="utf-8") as f:
            if f.read() == text:
                return
    except FileNotFoundError:
        pass

    os.makedirs(os.path.dirname(out) or ".", exist_ok=True)
    with open(out, "w", encoding="utf-8") as f:
        f.write(text)
    print(f"[generate-contract-hpp] Written {out}")


if __name__ == "__main__":
    main()
