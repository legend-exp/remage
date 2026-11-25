"""
export_changelog.py
Exports GitHub release notes as a Markdown changelog using the gh CLI.

Usage:
    python3 export_changelog.py [owner/repo] [output_file]
"""

from __future__ import annotations

import json
import shutil
import subprocess
import sys
from pathlib import Path


def run_gh(*args):
    """Run a gh CLI command and return parsed JSON output."""
    result = subprocess.run(  # noqa: PLW1510
        ["gh", *args],
        capture_output=True,
        text=True,
    )
    if result.returncode != 0:
        raise RuntimeError(result.stderr.strip())
    return json.loads(result.stdout)


def detect_repo() -> str | None:
    result = run_gh("repo", "view", "--json", "nameWithOwner")
    return result["nameWithOwner"] or None


def indent_headings(text: str) -> str:
    """Shift every Markdown heading in text one level deeper (# → ##, ## → ###, etc.)."""
    lines = []
    for line in text.splitlines():
        if line.startswith("#"):
            line = "#" + line  # noqa: PLW2901
        lines.append(line)
    return "\n".join(lines)


def build_changelog(repo: str, output: str) -> None:
    print(f"Fetching releases for {repo} …")  # noqa: T201

    releases = run_gh(
        "release",
        "list",
        "--repo",
        repo,
        "--limit",
        "500",
        "--json",
        "tagName,name,publishedAt,isPrerelease,isDraft",
    )

    # Filter out drafts
    releases = [r for r in releases if not r.get("isDraft")]

    if not releases:
        print(f"No published releases found for {repo}.")  # noqa: T201
        return

    print(f"Found {len(releases)} release(s). Building changelog …")  # noqa: T201

    lines = [
        "# Changelog",
        "",
    ]

    for i, r in enumerate(releases, 1):
        tag = r["tagName"]

        try:
            detail = run_gh(
                "release",
                "view",
                tag,
                "--repo",
                repo,
                "--json",
                "tagName,name,publishedAt,isPrerelease,isDraft,body",
            )
        except RuntimeError as e:
            print(  # noqa: T201
                f"  Warning: could not fetch release '{tag}', skipping. ({e})",
                file=sys.stderr,
            )
            continue

        title = (detail.get("name") or "").strip()
        date = (detail.get("publishedAt") or "")[:10]
        body = (detail.get("body") or "").strip()
        is_pre = detail.get("isPrerelease", False)

        label = tag
        if title and title != tag:
            label = f"{tag} — {title}"
        label += f" ({date})"
        if is_pre:
            label += " *(pre-release)*"

        lines += [
            f"## {label}",
            "",
            indent_headings(body) if body else "*No release notes provided.*",
            "",
        ]

        print(f"  [{i}/{len(releases)}] {tag}")  # noqa: T201

    with Path(output).open("w", encoding="utf-8") as f:
        f.write("\n".join(lines))

    print(f"\n✓ Wrote {len(releases)} release(s) to: {output}")  # noqa: T201


def main():
    if shutil.which("gh") is None:
        sys.exit("Error: 'gh' CLI not found. Install it from https://cli.github.com/")

    repo = sys.argv[1] if len(sys.argv) > 1 else None
    output = sys.argv[2] if len(sys.argv) > 2 else "CHANGELOG.md"

    if repo is None:
        repo = detect_repo()
        if not repo:
            sys.exit(
                "Error: Could not detect a GitHub repo. "
                "Run from inside a git repo or pass owner/repo as the first argument."
            )
        print(f"Detected repo: {repo}")  # noqa: T201

    build_changelog(repo, output)


if __name__ == "__main__":
    main()
