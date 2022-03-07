#!/usr/bin/env python3

import os
import sys
import json


class Node:
    def __init__(self):
        self.patterns = {}
        self.count = 0
        self.longest_path = 0

    def get_pattern(self, word, pattern):
        self.count += 1
        key = (word, pattern)
        out = self.patterns.get(key, None)
        if out is not None:
            return out
        out = Node()
        self.patterns[key] = out
        return out

    def get_longest_path(self):
        out = 0
        for (key, child) in self.patterns.items():
            out = max(out, child.get_longest_path() + 1)
        self.longest_path = out
        return out

    def to_json(self):

        if len(self.patterns) == 0:
            return None

        words = set()

        children = []
        for (key, child) in self.patterns.items():
            word, pattern = key
            words.add(word)
            children.append(
                {
                    "p": pattern,
                    "n": child.to_json(),
                }
            )

        children.sort(key=lambda v: v["p"])

        if len(words) > 1:
            raise RuntimeError(f"Multiple choices {words}")

        return {
            "w": words.pop(),
            "s": self.count,
            "l": self.longest_path,
            "p": children,
        }


class Tree:
    def __init__(self):
        self.root = Node()

    def add_path(self, path):
        node = self.root
        for (word, pattern) in path:
            node = node.get_pattern(word, pattern)

    def to_json(self):
        self.root.get_longest_path()
        return json.dumps(self.root.to_json(), separators=(",", ":"))


def main():
    path = []
    tree = Tree()

    with open(sys.argv[1], "r") as f:
        for line in f:
            line = line.strip()
            if line.startswith("Found secret"):
                tree.add_path(path)
                path = []
            elif line.startswith("Making guess"):
                parts = line.split()
                guess = parts[2]
                pattern = parts[4]
                path.append((guess, pattern))

    print(tree.to_json())


if __name__ == "__main__":
    main()
