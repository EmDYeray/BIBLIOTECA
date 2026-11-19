#!/usr/bin/env python3
"""Publish an editor ZIP as a versioned, static GitHub content feed."""
import json
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile

REPO = "https://github.com/EmDYeray/Library-of-Stars.git"
BRANCH = "library-content"


def run(*args, cwd=None):
    return subprocess.run(args, cwd=cwd, check=True, text=True, capture_output=True).stdout.strip()


def publish(archive):
    archive = Path(archive).resolve()
    manifest = json.loads((archive.parent / "manifest.json").read_text())
    from editor.pack import hashlib
    assert archive.name == manifest["file"] and hashlib.sha256(archive.read_bytes()).hexdigest() == manifest["sha256"]
    run("gh", "auth", "status")
    with tempfile.TemporaryDirectory(prefix="starry-publish-") as tmp:
        root = Path(tmp)
        run("git", "init", str(root))
        run("git", "remote", "add", "origin", REPO, cwd=root)
        found = subprocess.run(["git", "fetch", "--depth=1", "origin", BRANCH], cwd=root, capture_output=True).returncode == 0
        if found:
            run("git", "checkout", "-b", BRANCH, "FETCH_HEAD", cwd=root)
        else:
            run("git", "checkout", "--orphan", BRANCH, cwd=root)
        target = root / "packs" / (manifest["sha256"] + ".zip")
        target.parent.mkdir(exist_ok=True)
        shutil.copyfile(archive, target)
        manifest["file"] = "packs/" + target.name
        (root / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n")
        run("git", "add", "manifest.json", str(target.relative_to(root)), cwd=root)
        if subprocess.run(["git", "diff", "--cached", "--quiet"], cwd=root).returncode == 0:
            return "Sin cambios"
        run("git", "-c", "user.name=Starry content editor", "-c", "user.email=editor@users.noreply.github.com",
            "commit", "-m", "Update library content " + manifest["sha256"][:12], cwd=root)
        run("git", "push", "origin", "HEAD:refs/heads/" + BRANCH, cwd=root)
        return "Publicado: " + manifest["sha256"]


if __name__ == "__main__":
    if len(sys.argv) != 2:
        sys.exit("Uso: python3 tools/publish-content.py paquete.zip")
    try:
        print(publish(sys.argv[1]))
    except (OSError, ValueError, AssertionError, subprocess.CalledProcessError) as error:
        sys.exit("No se publicó: " + str(error))
