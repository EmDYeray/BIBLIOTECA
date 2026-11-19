"""Validate and export a small, versioned content overlay for the native library."""
import hashlib
import io
import json
from pathlib import Path
import re
import zipfile

BOOKS = 59
PAGES = 16
ROOMS = {"children", "adult"}


def validate(data):
    if data.get("schema") != 1 or not isinstance(data.get("rooms"), list):
        raise ValueError("Expected schema 1 and a rooms list")
    ids = set()
    for room in data["rooms"]:
        name = room.get("id")
        if not isinstance(name, str) or not re.fullmatch(r"[a-z][a-z0-9_-]{0,31}", name) or name in ids:
            raise ValueError("Room ids must be unique ASCII identifiers")
        ids.add(name)
        if not isinstance(room.get("name"), str) or not room["name"].strip():
            raise ValueError("Room name required")
        for item in room.get("objects", []):
            if item.get("kind") not in ("shelf", "fire", "lantern", "table", "chair", "bench", "book"):
                raise ValueError("Unknown object")
            if not all(isinstance(item.get(axis), (int, float)) and 0 <= item[axis] <= 100 for axis in ("x", "y")):
                raise ValueError("Object coordinates must be 0..100")
            if item["kind"] == "book" and not 0 <= item.get("book_id", -1) < BOOKS:
                raise ValueError("Book id out of range")
    for book in data.get("books", []):
        if not 0 <= book.get("id", -1) < BOOKS:
            raise ValueError("Book id out of range")
        for locale in ("es", "en"):
            version = book.get(locale, {})
            if not isinstance(version, dict) or not isinstance(version.get("title", ""), str):
                raise ValueError("Invalid title")
            pages = version.get("pages", [])
            if not isinstance(pages, list) or len(pages) > PAGES or any(not isinstance(p, str) for p in pages):
                raise ValueError("Invalid pages")
        cover = book.get("cover", "")
        if cover and (not isinstance(cover, str) or not re.fullmatch(r"[a-zA-Z0-9_-]+\.png", cover)):
            raise ValueError("Cover must be a PNG filename")
    if len({b["id"] for b in data.get("books", [])}) != len(data.get("books", [])):
        raise ValueError("Repeated book id")


def export(data, covers, target):
    validate(data)
    payload = io.BytesIO()
    with zipfile.ZipFile(payload, "w", zipfile.ZIP_DEFLATED) as archive:
        archive.writestr("layout.json", json.dumps(data["rooms"], ensure_ascii=False, indent=2))
        for book in data.get("books", []):
            ident = book["id"]
            for locale in ("es", "en"):
                version = book.get(locale, {})
                if version.get("title"):
                    archive.writestr(f"books/{ident}/{locale}/title.txt", version["title"])
                for index, page in enumerate(version.get("pages", [])):
                    if page.strip():
                        archive.writestr(f"books/{ident}/{locale}/{index}.txt", page)
            cover = book.get("cover")
            if cover:
                source = Path(covers) / cover
                if source.stat().st_size > 6_000_000 or source.read_bytes()[:8] != b"\x89PNG\r\n\x1a\n":
                    raise ValueError(f"Invalid PNG: {cover}")
                archive.write(source, f"covers/{ident}.png")
    target = Path(target)
    target.parent.mkdir(parents=True, exist_ok=True)
    content = payload.getvalue()
    target.write_bytes(content)
    return {"schema": 1, "version": hashlib.sha256(content).hexdigest(), "file": target.name,
            "sha256": hashlib.sha256(content).hexdigest(), "bytes": len(content)}
