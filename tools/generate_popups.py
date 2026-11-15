"""Regenerate all book pop-up models without external dependencies."""
from pathlib import Path

from mesh_builder import rgb
from popup_scenes import FLIERS, SCENES, model, scene


def main():
    output = Path(__file__).resolve().parents[1] / 'assets' / 'popups'
    output.mkdir(parents=True, exist_ok=True)
    materials = set()
    for name in SCENES:
        materials.update(scene(name).save(output / (name+'.obj')))
    for name, prop in FLIERS.items():
        materials.update(model(prop).save(output / (name+'.obj')))
    with (output / 'palette.mtl').open('w') as f:
        for name in sorted(materials):
            f.write('newmtl %s\nKd %.5f %.5f %.5f\n' % (name, *rgb(name)))
            shine = {'metal':(.64,60),'solar':(.32,44),'lamp':(.20,24),'rubber':(.03,4)}
            response, power = shine.get(name.partition('_')[2],(.13,9))
            f.write('Ks %.3f %.3f %.3f\nNs %d\nillum 2\n\n' % (response,response,response,power))


if __name__ == '__main__':
    main()
