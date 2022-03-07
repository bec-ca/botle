#!/usr/bin/env python3

import json
import sys


def filter(data):
    if isinstance(data, int):
        return data
    if isinstance(data, float):
        return data
    elif isinstance(data, str):
        return data
    elif isinstance(data, list):
        return [filter(el) for el in data]
    elif isinstance(data, dict):
        return {k: filter(v) for k, v in data.items() if k != "_"}
    else:
        raise (ValueError(f"Unexpected json value {data}"))


def main():
    for name in sys.argv[1:]:
        print(name)
        with open(name, "r") as f:
            data = f.read()
        parsed = json.loads(data)
        filtered = filter(parsed)
        with open(name, "w") as f:
            f.write(json.dumps(filtered, separators=(",", ":")))


if __name__ == "__main__":
    main()
