#!/usr/bin/env python3
"""Refresh the installed Linux content overlay without replacing the executable."""
import hashlib
import io
import json
import os
from pathlib import Path
import re
import shutil
import sys
from urllib.request import Request, urlopen
import zipfile

FEED = "https://raw.githubusercontent.com/EmDYeray/Library-of-Stars/library-content/"


def download(path, limit):
    with urlopen(Request(FEED + path, headers={"User-Agent": "Starry-Library"}), timeout=6) as response:
        data = response.read(limit + 1)
    if len(data) > limit:
        raise ValueError("Content size limit")
    return data


def update(assets):
    manifest = json.loads(download("manifest.json", 8192))
    digest = manifest["sha256"]
    if (manifest.get("schema") != 1 or not re.fullmatch(r"[0-9a-f]{64}", digest)
            or manifest.get("file") != f"packs/{digest}.zip"):
        raise ValueError("Invalid manifest")
    content = assets / "content"
    if (content / "version.txt").is_file() and (content / "version.txt").read_text().strip() == digest:
        return False
    size = manifest["bytes"]
    if not isinstance(size, int) or size <= 0 or size > 80_000_000:
        raise ValueError("Invalid pack size")
    payload = download(manifest["file"], size)
    if len(payload) != size or hashlib.sha256(payload).hexdigest() != digest:
        raise ValueError("Pack checksum mismatch")
    staging = assets / "content-next"
    if staging.exists():
        shutil.rmtree(staging)
    staging.mkdir()
    expanded = 0
    try:
        with zipfile.ZipFile(io.BytesIO(payload)) as archive:
            for entry in archive.infolist():
                if (entry.filename.startswith("/") or ".." in entry.filename or
                        not re.fullmatch(r"[a-zA-Z0-9_./-]+", entry.filename)):
                    raise ValueError("Unsafe filename")
                expanded += entry.file_size
                if expanded > 150_000_000:
                    raise ValueError("Pack too large")
                destination = staging / entry.filename
                if entry.is_dir():
                    destination.mkdir(parents=True, exist_ok=True)
                else:
                    destination.parent.mkdir(parents=True, exist_ok=True)
                    with archive.open(entry) as source, destination.open("wb") as target:
                        shutil.copyfileobj(source, target)
        (staging / "version.txt").write_text(digest)
        old = assets / "content-old"
        if old.exists():
            shutil.rmtree(old)
        if content.exists():
            content.rename(old)
        try:
            staging.rename(content)
        except Exception:
            if old.exists():
                old.rename(content)
            raise
        if old.exists():
            shutil.rmtree(old)
        return True
    finally:
        if staging.exists():
            shutil.rmtree(staging)


if __name__ == "__main__":
    try:
        print("Content updated" if update(Path(sys.argv[1])) else "Content already current")
    except Exception as error:
        print(f"Using saved content: {error}", file=sys.stderr)
