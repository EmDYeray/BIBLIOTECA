#!/usr/bin/env python3
"""Translate the book prose in UTF-8 C++ string literals without touching C++ identifiers.

This is an automated English *draft*. Review literary quality and regressions
before merging into the default branch.
"""
from __future__ import annotations

import json
import pathlib
import re
from collections import Counter

import argostranslate.package
import argostranslate.translate

ROOT = pathlib.Path(__file__).resolve().parents[1]
FILES = (
    "src/stories.cpp",
    "src/child_text.cpp",
    "src/adult_books.cpp",
    "src/imported_books.cpp",
    "src/technical_notes.cpp",
)
LITERAL = re.compile(r'"(?:\\.|[^"\\])*"', re.DOTALL)
SPANISH = re.compile(r"\b(el|la|los|las|un|una|unos|unas|del|al|que|quien|cuando|como|tambien|sobre|desde|tras|siguio|habia|tenia|estaba|para|con|sin|entre|hacia|donde|fue|eran|podian|podia|poco|despues|misma|aunque|porque|mientras|sin|hasta|miró|vio|dijo|pidio|hizo)\b", re.I)
EXACT = {"Marte": "Mars", "Luna": "Moon", "El Sol": "The Sun", "Tierra": "Earth", "Órbita": "Orbit", "Observación": "Observation"}
EXT = re.compile(r"^[\w./\\+-]+\.(?:cpp|h|hpp|inc|obj|png|jpg|ppm|ttf|md|txt|frag|vert|so|dll|exe)$", re.I)
CACHE: dict[str,str] = {}
COUNTS = Counter()

def should_translate(value: str) -> bool:
    value = value.strip()
    if not value or value.startswith(("http://","https://","#")):
        return False
    if EXT.fullmatch(value):
        return False
    if value in EXACT:
        return True
    if len(value) < 5:
        return False
    if re.search(r"[áéíóúñÁÉÍÓÚ¿¡]", value):
        return True
    if len(value) >= 18 and len(value.split()) >= 3:
        return True
    return bool(SPANISH.search(value))

def english(text: str) -> str:
    if text in EXACT:
        return EXACT[text]
    if text not in CACHE:
        # Preserve explicit line breaks in book paragraphs.
        parts = text.split("\n")
        translated = [argostranslate.translate.translate(p, "es", "en") if p.strip() else p for p in parts]
        result = "\n".join(translated).strip()
        CACHE[text] = result or text
    return CACHE[text]

def translate_file(relative: str) -> None:
    path = ROOT / relative
    original = path.read_text(encoding="utf-8")
    changed = 0
    def replace(match: re.Match[str]) -> str:
        nonlocal changed
        token = match.group(0)
        try:
            value = json.loads(token)
        except (ValueError, TypeError):
            return token
        if not isinstance(value, str) or not should_translate(value):
            return token
        translated = english(value)
        if translated == value:
            COUNTS["unchanged"] += 1
            return token
        changed += 1
        COUNTS["translated"] += 1
        return json.dumps(translated, ensure_ascii=False)
    updated = LITERAL.sub(replace, original)
    if len(LITERAL.findall(updated)) != len(LITERAL.findall(original)):
        raise RuntimeError(f"String token count changed for {relative}")
    if updated != original:
        path.write_text(updated, encoding="utf-8")
    print(f"{relative}: {changed} translated string literals")

def main() -> None:
    print("Installing the Spanish-to-English translation model...")
    argostranslate.package.update_package_index()
    candidates = [p for p in argostranslate.package.get_available_packages()
                  if p.from_code == "es" and p.to_code == "en"]
    if not candidates:
        raise RuntimeError("No es-to-en Argos model was available")
    argostranslate.package.install_from_path(candidates[0].download())
    for relative in FILES:
        translate_file(relative)
    print("Total:", dict(COUNTS), "unique translations:", len(CACHE))

if __name__ == "__main__":
    main()
