"""Generate the tiling grain atlas used by the room and by the paper models."""
from math import cos, floor, pi, sin
from pathlib import Path
from random import Random


def value_field(cols, rows, seed):
    rng = Random(seed)
    cells = [[rng.random() for _ in range(cols)] for _ in range(rows)]

    def sample(x, y):
        u, v = x * cols, y * rows
        i, j = floor(u), floor(v)
        u, v = u - i, v - j
        u, v = u*u*(3-2*u), v*v*(3-2*v)
        a, b = cells[j % rows], cells[(j+1) % rows]
        left = (1-v)*a[i % cols] + v*b[i % cols]
        right = (1-v)*a[(i+1) % cols] + v*b[(i+1) % cols]
        return (1-u)*left + u*right

    return sample


def main():
    output = Path(__file__).resolve().parents[1] / 'assets' / 'materials'
    output.mkdir(exist_ok=True)
    size = 512
    veins = value_field(4, 37, 28)
    pores = value_field(24, 141, 43)
    long_fiber = value_field(21, 117, 218)
    short_fiber = value_field(130, 91, 315)
    grit = value_field(128, 117, 311)
    glaze = value_field(8, 9, 57)
    pixels = bytearray()
    for j in range(size):
        y = j / size
        for i in range(size):
            x = i / size
            bend = .018*sin(2*pi*x) + .004*sin(2*pi*(2*x+y))
            growth = .5 + .5*cos(2*pi*46*(y+bend) + 3.1*veins(x,y))
            wood = .49*veins(x,y+bend) + .32*growth + .19*pores(x,y)
            weave = .57*long_fiber(x,y) + .43*short_fiber(x,y)
            fleck = max(0.,(grit(x,y)-.71)*3.1)
            tile = .78*glaze(x,y) + .22*fleck
            pixels.extend(round(v*255) for v in (wood,weave,tile))
    (output / 'grain.ppm').write_bytes(f'P6\n{size} {size}\n255\n'.encode() + pixels)


if __name__ == '__main__':
    main()
