"""Validate and export a small, versioned content overlay for the native library."""
import hashlib
import io
import json
from pathlib import Path
import re
import zipfile

BOOKS = 59
PAGES = 16
KINDS = {"shelf", "fire", "lantern", "table", "chair", "bench", "book", "box", "spawn", "portal"}
EXTENTS = {"shelf": (4.2, .85), "fire": (2.5, 2.5), "lantern": (.42, .42),
           "table": (3.08, 1.52), "chair": (.91, .83), "bench": (4.48, 1.06),
           "book": (.4, .88), "box": (1, 1), "spawn": (.2, .2), "portal": (1.2, .18)}


def validate(data):
    if data.get("schema") != 2 or not isinstance(data.get("rooms"), list):
        raise ValueError("Expected schema 2 and a rooms list")
    ids = set()
    for room in data["rooms"]:
        name = room.get("id")
        if not isinstance(name, str) or not re.fullmatch(r"[a-z][a-z0-9_-]{0,31}", name) or name in ids:
            raise ValueError("Room ids must be unique ASCII identifiers")
        ids.add(name)
        if not isinstance(room.get("name"), str) or not room["name"].strip():
            raise ValueError("Room name required")
        if not all(isinstance(value, (int, float)) for value in room.get("origin", [])) or len(room.get("origin", [])) != 3:
            raise ValueError("Room origin must contain x, y and z")
        if not all(isinstance(value, (int, float)) and value > 0 for value in room.get("size", [])) or len(room.get("size", [])) != 3:
            raise ValueError("Room size must contain three positive values")
        if not isinstance(room.get("height"), (int, float)) or room["height"] < 2.4:
            raise ValueError("Room height must be at least 2.4 metres")
    object_ids = set()
    for item in data.get("objects", []):
        if item.get("kind") not in KINDS:
            raise ValueError("Unknown object")
        ident = item.get("id")
        if not isinstance(ident, str) or not re.fullmatch(r"[a-z][a-z0-9_-]{0,47}", ident) or ident in object_ids:
            raise ValueError("Object ids must be unique ASCII identifiers")
        object_ids.add(ident)
        for field in ("position", "rotation", "scale"):
            value = item.get(field)
            if not isinstance(value, list) or len(value) != 3 or not all(isinstance(number, (int, float)) for number in value):
                raise ValueError(f"Object {field} must contain x, y and z")
        if any(value <= 0 for value in item["scale"]):
            raise ValueError("Object scale must be positive")
        if item["kind"] == "book" and not 0 <= item.get("book", -1) < BOOKS:
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
    room_ids = {room["id"] for room in data["rooms"]}
    for connection in data.get("connections", []):
        if connection.get("from") not in room_ids or connection.get("to") not in room_ids:
            raise ValueError("Connection references an unknown room")
    failures = scene_problems(data)
    if failures:
        raise ValueError("Scene validation failed:\n- " + "\n- ".join(failures))


def scene_problems(data):
    problems = []
    for item in data.get("objects", []):
        x, _y, z = item["position"]
        inside = any(abs(x-room["origin"][0]) <= room["size"][0]/2
                     and abs(z-room["origin"][2]) <= room["size"][2]/2 for room in data["rooms"])
        if not inside:
            problems.append(f"{item['name']} is outside every room")
    solids = [item for item in data.get("objects", []) if item.get("solid") and item["kind"] not in ("spawn", "portal")]
    for index, first in enumerate(solids):
        fw, fd = EXTENTS[first["kind"]][0]*first["scale"][0], EXTENTS[first["kind"]][1]*first["scale"][2]
        for second in solids[index+1:]:
            sw, sd = EXTENTS[second["kind"]][0]*second["scale"][0], EXTENTS[second["kind"]][1]*second["scale"][2]
            if (abs(first["position"][0]-second["position"][0]) < (fw+sw)/2-.05
                    and abs(first["position"][2]-second["position"][2]) < (fd+sd)/2-.05):
                problems.append(f"{first['name']} overlaps {second['name']}")
    for book in data.get("books", []):
        for locale in ("es", "en"):
            version = book.get(locale, {})
            if not version.get("title", "").strip() or not any(page.strip() for page in version.get("pages", [])):
                problems.append(f"Book {book['id']} has incomplete {locale.upper()} text")
    return problems


def export(data, covers, target):
    validate(data)
    payload = io.BytesIO()
    with zipfile.ZipFile(payload, "w", zipfile.ZIP_DEFLATED) as archive:
        scene = {key: data.get(key, []) for key in ("rooms", "connections", "objects")}
        scene["schema"] = 2
        archive.writestr("scene.json", json.dumps(scene, ensure_ascii=False, indent=2))
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
    return {"schema": 2, "version": hashlib.sha256(content).hexdigest(), "file": target.name,
            "sha256": hashlib.sha256(content).hexdigest(), "bytes": len(content)}
