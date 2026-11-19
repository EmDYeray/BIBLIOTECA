# Runtime content format

`scene.json` is the source of truth shared by the Linux editor and the C++ OpenGL application. Schema 2 uses metres, degrees and right-handed world coordinates: X runs left/right, Y runs vertically, and Z runs through the room.

```json
{
  "schema": 2,
  "rooms": [{
    "id": "children",
    "name": "Niños",
    "builtin": true,
    "origin": [0.0, -2.26, 6.4],
    "size": [34.1, 8.9, 17.6],
    "height": 8.9,
    "floor": "wood",
    "walls": "plaster",
    "ceiling": "plaster"
  }],
  "connections": [{"id":"children-adult","from":"children","to":"adult","kind":"door"}],
  "objects": [{
    "id": "reading-table-1",
    "name": "Mesa de lectura",
    "kind": "table",
    "position": [3.0, -2.26, 7.0],
    "rotation": [0.0, 90.0, 0.0],
    "scale": [1.0, 1.0, 1.0],
    "color": "#92745f",
    "material": "wood",
    "solid": true,
    "animated": false,
    "interactive": false
  }]
}
```

Rooms with `builtin: true` describe the two legacy rooms without generating a second shell. A room with `builtin: false` generates its floor, ceiling and walls at runtime. Objects currently supported by the runtime loader are `shelf`, `fire`, `lantern`, `table`, `chair`, `bench`, `book`, `box`, `spawn` and `portal`. A book also carries `book`, the existing catalog id from 0 to 58.

The content ZIP stores this document as `scene.json`, book text below `books/<id>/<locale>/`, and covers below `covers/`. `manifest.json` identifies schema 2, exact byte length and SHA-256. Android and Linux stage a complete download, verify it, then switch the active content directory atomically. If validation or the network fails, the last valid content remains active.

## Compatibility boundary

Schema changes require a new app build. Content changes within schema 2 do not. Unknown object kinds and invalid transforms are rejected. The current runtime draws new room shells and basic objects, applies solid-object collisions, and accepts book position overrides. Door openings between generated rooms, portal behaviour, arbitrary imported meshes, LODs and the final 3D gizmo editor are later phases.
