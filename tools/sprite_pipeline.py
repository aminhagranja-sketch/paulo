#!/usr/bin/env python3
"""Inspect irregular transparent sheets, then pack reviewed frames without resampling."""
import argparse
import hashlib
import json
import math
import sys
from pathlib import Path
from PIL import Image

SOURCES = {
    'chests': 'e2b20b93-8799-4092-a400-14b2df516e3a.png',
    'buildings': '3e7f225e-5979-4f0f-b6cb-e4f7544e7836.png',
    'trees': '4f1a5112-b608-47c7-aaf7-6be980fe74d9.png',
    'environment': '591f28fd-06f3-46d7-bce3-11dfd7e2e488.png',
    'enemies/snake': '15681c87-bdbc-446b-a3c2-c1d3853e999a.png',
    'enemies/fox': 'bef336d9-407a-4ad2-a747-f4ff3616e2ba.png',
    'characters/chicks': 'ca70ca01-1579-450b-9d26-5dd3eab5ecb5.png',
    'characters/chicken': '5462a363-df7e-4da6-b2ce-1a715d75e117.png',
}


def load_sheet(path):
    with Image.open(path) as source:
        if source.format != 'PNG':
            raise ValueError(f'{path}: esperado PNG')
        return source.convert('RGBA')


def regions(image, threshold=1, minimum=16):
    """8-connected alpha components; detached tails/effects remain reviewable proposals."""
    width, height = image.size
    alpha = image.getchannel('A')
    if alpha.getextrema()[0] == 255:
        raise ValueError('Imagem opaca: forneça recortes manuais; o fundo não será removido automaticamente.')
    data = alpha.tobytes()
    seen = bytearray(width * height)
    boxes = []
    for start, value in enumerate(data):
        if value < threshold or seen[start]:
            continue
        seen[start] = 1
        stack = [start]
        x0 = x1 = start % width
        y0 = y1 = start // width
        count = 0
        while stack:
            index = stack.pop()
            x, y = index % width, index // width
            count += 1
            x0, x1, y0, y1 = min(x0, x), max(x1, x), min(y0, y), max(y1, y)
            for ny in range(max(0, y-1), min(height, y+2)):
                for nx in range(max(0, x-1), min(width, x+2)):
                    neighbor = ny * width + nx
                    if not seen[neighbor] and data[neighbor] >= threshold:
                        seen[neighbor] = 1
                        stack.append(neighbor)
        # Only detection filters small components: original pixels are never erased.
        if count >= minimum:
            boxes.append([x0, y0, x1-x0+1, y1-y0+1])
    return sorted(boxes, key=lambda box: (box[1], box[0]))


def inspect(source_dir, output):
    document = {'version': 1, 'sheets': []}
    missing = []
    for category, filename in SOURCES.items():
        path = source_dir / filename
        if not path.exists():
            missing.append(filename)
            continue
        image = load_sheet(path)
        entry = {'category': category, 'source': filename,
                 'sha256': hashlib.sha256(path.read_bytes()).hexdigest(),
                 'size': list(image.size), 'reviewed': False, 'frames': [], 'animations': {}}
        try:
            entry['frames'] = [{'id': f'frame_{i:03}', 'rect': box,
                                'origin': [box[2]/2, box[3]]}
                               for i, box in enumerate(regions(image))]
        except ValueError as error:
            entry['notice'] = str(error)
        document['sheets'].append(entry)
    document['missing'] = missing
    output.parent.mkdir(parents=True, exist_ok=True)
    if output.exists():
        raise ValueError(f'{output} já existe; use outro caminho para preservar sua revisão.')
    output.write_text(json.dumps(document, ensure_ascii=False, indent=2)+'\n', encoding='utf-8')
    print(f'{len(document["sheets"])} folhas encontradas; {len(missing)} ausentes. Proposta: {output}')
    return 2 if missing else 0


def validate_entry(entry, image):
    category = Path(entry['category'])
    if str(category) not in SOURCES:
        raise ValueError(f'Categoria desconhecida: {category}')
    if entry.get('reviewed') is not True:
        raise ValueError(f'{category}: reveja os recortes e as sequências antes de marcar reviewed=true')
    frames = entry['frames']
    if not frames:
        raise ValueError(f'{category}: nenhum quadro')
    ids = set()
    for frame in frames:
        name = frame['id']
        if not isinstance(name, str) or not name or any(c not in 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-' for c in name) or name in ids:
            raise ValueError('IDs de quadros precisam ser únicos e seguros para nomes de arquivos')
        ids.add(name)
        box = frame['rect']
        if len(box) != 4 or any(type(n) is not int for n in box):
            raise ValueError(f'{name}: rect precisa de quatro inteiros')
        x, y, width, height = box
        if min(x, y) < 0 or min(width, height) <= 0 or x+width > image.width or y+height > image.height:
            raise ValueError(f'{name}: recorte fora da imagem')
        origin = frame['origin']
        if len(origin) != 2 or any(type(n) not in (int, float) or not math.isfinite(n) for n in origin):
            raise ValueError(f'{name}: origem inválida')
    for name, animation in entry['animations'].items():
        if not name or not animation.get('frames') or any(frame not in ids for frame in animation['frames']):
            raise ValueError(f'Animação {name}: referência inválida')
        fps = animation.get('fps', 0)
        if type(fps) not in (int, float) or not math.isfinite(fps) or not 0 < fps <= 60:
            raise ValueError(f'Animação {name}: fps deve estar entre 0 e 60')
    return category


def remove_border_black(crop, alpha_threshold=0):
    """Remove only near-black background connected to crop edges; retain black eyes."""
    width, height = crop.size
    pixels = crop.load()
    pending = [(x,y) for x in range(width) for y in (0,height-1)]
    pending += [(x,y) for y in range(height) for x in (0,width-1)]
    seen = set()
    while pending:
        x,y = pending.pop()
        if (x,y) in seen or not (0 <= x < width and 0 <= y < height):
            continue
        seen.add((x,y))
        r,g,b,a = pixels[x,y]
        if alpha_threshold:
            if a >= alpha_threshold:
                continue
        elif max(r,g,b) > 18:
            continue
        pixels[x,y] = (r,g,b,0)
        pending.extend(((x-1,y),(x+1,y),(x,y-1),(x,y+1)))
    return crop


def pack(source_dir, manifest, assets, size=2048):
    if not 64 <= size <= 4096:
        raise ValueError('Tamanho do atlas deve estar entre 64 e 4096')
    document = json.loads(manifest.read_text(encoding='utf-8'))
    if document.get('version') != 1 or document.get('missing'):
        raise ValueError('Manifesto incompleto: forneça todas as folhas do catálogo e gere uma nova proposta')
    entries = document['sheets']
    if len(entries) != len(SOURCES) or {e['category'] for e in entries} != set(SOURCES):
        raise ValueError('São necessárias todas as categorias, sem duplicatas')
    prepared = []
    # Validate all inputs before writing any output.
    for entry in entries:
        filename = SOURCES[entry['category']]
        if entry['source'] != filename:
            raise ValueError('Nome da fonte diferente do catálogo')
        path = source_dir / filename
        if hashlib.sha256(path.read_bytes()).hexdigest() != entry['sha256']:
            raise ValueError(f'{filename}: a fonte mudou desde a revisão')
        image = load_sheet(path)
        category = validate_entry(entry, image)
        atlas = Image.new('RGBA', (size, size))
        pages, crops, metadata = [], [], {}
        x = y = row_height = 2
        for frame in sorted(entry['frames'], key=lambda f: (-f['rect'][3], f['id'])):
            sx, sy, width, height = frame['rect']
            if width+4 > size or height+4 > size:
                raise ValueError(f'{frame["id"]}: quadro maior que o atlas; aumente --size')
            if x+width+2 > size:
                x, y, row_height = 2, y+row_height+4, 0
            if y+height+2 > size:
                pages.append(atlas)
                atlas = Image.new('RGBA', (size, size))
                x = y = 2
                row_height = 0
            crop = image.crop((sx, sy, sx+width, sy+height))
            if entry.get('removeBorderBlack'):
                crop = remove_border_black(crop)
            elif entry.get('cleanBorderAlpha'):
                crop = remove_border_black(crop, entry['cleanBorderAlpha'])
            # Paste without a mask preserves alpha exactly, including translucent fringes.
            atlas.paste(crop, (x, y))
            # Extrude edge pixels into padding to avoid linear-filter seams.
            atlas.paste(crop.crop((0,0,width,1)), (x,y-1))
            atlas.paste(crop.crop((0,height-1,width,height)), (x,y+height))
            atlas.paste(crop.crop((0,0,1,height)), (x-1,y))
            atlas.paste(crop.crop((width-1,0,width,height)), (x+width,y))
            metadata[frame['id']] = {'page': len(pages), 'rect': [x,y,width,height],
                                      'sourceRect': frame['rect'], 'origin': frame['origin']}
            crops.append((frame['id'], crop))
            x += width+4
            row_height = max(row_height, height)
        pages.append(atlas)
        prepared.append((category, entry, pages, crops, metadata))
    for category, entry, pages, crops, metadata in prepared:
        processed = assets/'processed'/category
        atlas_dir = assets/'atlases'/category
        animation_path = assets/'animations'/category.with_suffix('.json')
        for path in (processed, atlas_dir, animation_path.parent):
            path.mkdir(parents=True, exist_ok=True)
        for name, crop in crops:
            crop.save(processed/f'{name}.png', optimize=True)
        files = []
        for number, page in enumerate(pages):
            filename = f'atlas-{number}.png'
            page.save(atlas_dir/filename, optimize=True)
            files.append(str((category/filename).as_posix()))
        output = {'version': 1, 'source': entry['source'], 'sha256': entry['sha256'],
                  'pages': files, 'frames': metadata, 'animations': entry['animations']}
        animation_path.write_text(json.dumps(output, ensure_ascii=False, indent=2)+'\n', encoding='utf-8')
        print(f'{category}: {len(crops)} quadros, {len(pages)} atlas')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--assets', type=Path, default=Path('assets'))
    commands = parser.add_subparsers(dest='command', required=True)
    scan = commands.add_parser('inspect')
    scan.add_argument('--output', type=Path, required=True)
    build = commands.add_parser('pack')
    build.add_argument('--manifest', type=Path, required=True)
    build.add_argument('--size', type=int, default=2048)
    args = parser.parse_args()
    try:
        if args.command == 'inspect':
            return inspect(args.assets/'source', args.output)
        pack(args.assets/'source', args.manifest, args.assets, args.size)
        return 0
    except (ValueError, KeyError, OSError, TypeError) as error:
        print(f'Erro: {error}', file=sys.stderr)
        return 1

if __name__ == '__main__':
    sys.exit(main())
