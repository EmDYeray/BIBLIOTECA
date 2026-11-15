"""Small deterministic OBJ builder for the library's authored pop-up meshes."""
from math import cos, pi, sin, sqrt
from pathlib import Path


def add(a, b):
    return tuple(x + y for x, y in zip(a, b))


def sub(a, b):
    return tuple(x - y for x, y in zip(a, b))


def mul(v, k):
    return tuple(x * k for x in v)


def cross(a, b):
    return (a[1]*b[2] - a[2]*b[1], a[2]*b[0] - a[0]*b[2], a[0]*b[1] - a[1]*b[0])


def unit(v):
    return mul(v, 1 / sqrt(sum(x*x for x in v)))


def rgb(hex_color):
    return tuple(int(hex_color[i:i+2], 16) / 255 for i in (0, 2, 4))


def blend(a, b, t):
    return add(mul(a, 1-t), mul(b, t))


class Mesh:
    def __init__(self):
        self.faces = []
        self.actors = []
        # Bounds of authored pieces already placed in a paper spread.
        self.occupied = []

    def face(self, material, points, normals=None, colors=None):
        vertices = [(p, normals[i] if normals else None, colors[i] if colors else None)
                    for i, p in enumerate(points)]
        for i in range(1, len(vertices)-1):
            self.faces.append((material, (vertices[0], vertices[i], vertices[i+1])))

    def box(self, center, size, material, bevel=.012):
        """Rounded low-poly box with a soft silhouette and recessed end caps.

        The old single chamfer made every panel read as a sharp block in close-up.
        Four samples per corner keep the authored paper-pop language while giving
        lighting a continuous-looking highlight around housings, wheels and bases.
        """
        x, y, z = (d/2 for d in size)
        g = min(bevel, x*.46, y*.46, z*.46)
        corner_centers = ((x-g,z-g),(-x+g,z-g),(-x+g,-z+g),(x-g,-z+g))
        corners = []
        for cx, cz in corner_centers:
            # Counter-clockwise perimeter, eight points per quarter arc.
            angle = 0 if cx > 0 and cz > 0 else 0
            if cx < 0 and cz > 0: angle = pi/2
            elif cx < 0 and cz < 0: angle = pi
            elif cx > 0 and cz < 0: angle = 3*pi/2
            for j in range(8):
                a = angle + j*pi/14
                corners.append((cx + g*cos(a), cz + g*sin(a)))
        # top/bottom are inset slightly, making the edge catch light without a
        # separate bevel mesh or texture seam.
        rings = [[add(center, (u*sx, h, v*sz)) for u, v in corners]
                 for h, sx, sz in ((-y,1-g/x,1-g/z),(-y+g,1,1),(y-g,1,1),(y,1-g/x,1-g/z))]
        self.face(material, rings[-1], [(0,1,0)]*len(rings[-1]))
        self.face(material, list(reversed(rings[0])), [(0,-1,0)]*len(rings[0]))
        for bottom, top in zip(rings, rings[1:]):
            for i, a in enumerate(bottom):
                j = (i+1) % len(bottom)
                u, v = corners[i]
                length=sqrt(u*u+v*v)
                normal=(u/length,0,v/length)
                self.face(material, (a, bottom[j], top[j], top[i]), (normal,normal,normal,normal))

    def tube(self, rings, material, n=36, caps=True, smooth=True):
        """Turn a profile around the axis through its first and last ring."""
        axis = unit(sub(rings[-1][0], rings[0][0]))
        right = unit(cross(axis, (0, 1, 0) if abs(axis[1]) < .9 else (1, 0, 0)))
        front = cross(axis, right)
        radial = [add(mul(right, cos(i*2*pi/n)), mul(front, sin(i*2*pi/n))) for i in range(n)]
        points = [[add(c, mul(v, r)) for v in radial] for c, r in rings]
        for k, (inner, outer) in enumerate(zip(points, points[1:])):
            a, ra = rings[k]
            b, rb = rings[k+1]
            delta = sub(b, a)
            length = sum(x*y for x, y in zip(delta, axis))
            normals = [unit(add(mul(v, length), mul(axis, ra-rb))) for v in radial]
            for i in range(n):
                j = (i+1)%n
                self.face(material, (inner[i], inner[j], outer[j], outer[i]),
                          (normals[i], normals[j], normals[j], normals[i]) if smooth else None)
        if caps:
            self.face(material, list(reversed(points[0])))
            self.face(material, points[-1])

    def rod(self, a, b, r, material, n=18):
        self.tube(((a, r), (b, r)), material, n)

    def sphere(self, center, radius, material, sample=None, bands=36, segments=72, pole=-pi/2):
        grid = []
        for i in range(bands+1):
            lat = pole + (pi/2-pole)*i/bands
            row = []
            for j in range(segments):
                lon = j*2*pi/segments-pi
                v = (cos(lat)*sin(lon), sin(lat), cos(lat)*cos(lon))
                relief, color = sample(lat, lon, v) if sample else (0, None)
                row.append((add(center, mul(v, radius*(1+relief))), v, color))
            grid.append(row)
        for i in range(bands):
            for j in range(segments):
                k = (j+1)%segments
                points = (grid[i][j], grid[i][k], grid[i+1][k], grid[i+1][j])
                p, n, c = zip(*points)
                self.face(material, p, n, c if sample else None)

    def include(self, mesh, at=(0,0,0), scale=1, yaw=0):
        c, s = cos(yaw), sin(yaw)
        offset = len(self.faces)
        for material, vertices in mesh.faces:
            transformed = []
            for (x,y,z), normal, color in vertices:
                point = add(at, mul((x*c+z*s,y,z*c-x*s), scale))
                if normal:
                    nx, ny, nz = normal
                    normal = (nx*c+nz*s,ny,nz*c-nx*s)
                transformed.append((point, normal, color))
            self.faces.append((material, tuple(transformed)))
        for start, stop, kind, (x,y,z) in mesh.actors:
            pivot=add(at,mul((x*c+z*s,y,z*c-x*s),scale))
            self.actors.append((offset+start,offset+stop,kind,pivot))

    def animate(self, start, kind, pivot):
        self.actors.append((start,len(self.faces),kind,pivot))

    def save(self, path):
        points, normals, faces = {}, {}, []
        for source_index, (material, vertices) in enumerate(self.faces):
            a, b, c = (v[0] for v in vertices)
            ab = sub(b, a)
            ac = sub(c, a)
            cross_area = cross(ab, ac)
            if sum(value*value for value in cross_area) < 1e-9:
                # Pole caps and zero-width decorative seams can generate a
                # mathematically empty triangle. Do not ship it to the OBJ;
                # it only wastes memory and can trigger depth flicker on iPad.
                continue
            indices = []
            for point, normal, color in vertices:
                key = tuple(round(x,5) for x in point+(color or ()))
                i = points.setdefault(key, len(points)+1)
                if normal:
                    n = normals.setdefault(tuple(round(x,5) for x in normal), len(normals)+1)
                    indices.append(f'{i}//{n}')
                else:
                    indices.append(str(i))
            faces.append((source_index, material, ' '.join(indices)))
        with path.open('w') as f:
            f.write(f'# Biblioteca · Modelos 3D para libro pop-up\nmtllib palette.mtl\no {path.stem}\n')
            for v in points:
                f.write('v '+' '.join(f'{x:.5f}' for x in v)+'\n')
            for n in normals:
                f.write('vn '+' '.join(f'{x:.5f}' for x in n)+'\n')
            actors = iter(sorted(self.actors))
            actor = next(actors, None)
            previous = None
            last = None
            for source_index, mat, face in faces:
                while actor is not None and source_index >= actor[1]:
                    actor = next(actors, None)
                current = actor if actor is not None and source_index >= actor[0] else None
                if current != previous:
                    if current is None:
                        f.write('# pop_still\n')
                    else:
                        _, _, kind, pivot = current
                        f.write('# pop_motion '+kind+' '+' '.join(f'{v:.5f}' for v in pivot)+'\n')
                    previous = current
                if mat != last:
                    f.write(f'usemtl {mat}\n')
                    last = mat
                f.write(f'f {face}\n')
        return {material for _, material, _ in faces}


def load_open_model(path, target_height=.62):
    """Load a converted open OBJ and normalize it for a paper-pop scene.

    Imports geometry, smooth normals, baked vertex colors and remapped
    materials, with no runtime dependency on external texture files.
    """
    positions=[]
    colors=[]
    normals=[]
    faces=[]
    material='7A8790_metal'
    for line in Path(path).read_text(errors='ignore').splitlines():
        if not line or line.startswith('#'):
            continue
        parts=line.split()
        if parts[0]=='v' and len(parts)>=4:
            positions.append(tuple(float(v) for v in parts[1:4]))
            colors.append(tuple(float(v) for v in parts[4:7]) if len(parts)==7 else None)
        elif parts[0]=='vn' and len(parts)>=4:
            raw=tuple(float(v) for v in parts[1:4])
            normals.append(unit(raw) if sum(v*v for v in raw)>1e-12 else (0,1,0))
        elif parts[0]=='usemtl' and len(parts)>=2:
            material=parts[1]
        elif parts[0]=='f' and len(parts)>=4:
            refs=[]
            for token in parts[1:]:
                fields=token.split('/')
                vi=int(fields[0]); vi=vi-1 if vi>0 else len(positions)+vi
                ni=None
                if len(fields)>=3 and fields[-1]:
                    ni=int(fields[-1]); ni=ni-1 if ni>0 else len(normals)+ni
                refs.append((vi,ni))
            for i in range(1,len(refs)-1):
                faces.append((material,(refs[0],refs[i],refs[i+1])))
    if not positions or not faces:
        raise ValueError(f'Modelo abierto vacío: {path}')
    minx=min(p[0] for p in positions); maxx=max(p[0] for p in positions)
    miny=min(p[1] for p in positions); maxy=max(p[1] for p in positions)
    minz=min(p[2] for p in positions); maxz=max(p[2] for p in positions)
    scale=target_height/max(maxy-miny,maxx-minx,maxz-minz,1e-6)
    cx=(minx+maxx)*.5; cz=(minz+maxz)*.5
    mesh=Mesh()
    for mat,refs in faces:
        points=[]; ns=[]; pigments=[]
        for vi,ni in refs:
            x,y,z=positions[vi]
            points.append(((x-cx)*scale,(y-miny)*scale,(z-cz)*scale))
            ns.append(normals[ni] if ni is not None and ni<len(normals) else None)
            pigments.append(colors[vi])
        mesh.face(mat,points,ns if all(ns) else None,pigments if any(pigments) else None)
    return mesh
