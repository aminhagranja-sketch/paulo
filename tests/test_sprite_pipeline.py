import hashlib
import importlib.util
import json
import tempfile
import unittest
from pathlib import Path
from PIL import Image

spec = importlib.util.spec_from_file_location('pipeline', Path(__file__).parents[1]/'tools/sprite_pipeline.py')
pipeline = importlib.util.module_from_spec(spec)
spec.loader.exec_module(pipeline)


class PipelineTests(unittest.TestCase):
    def test_irregular_components_and_opaque_sheet(self):
        sheet = Image.new('RGBA', (90, 70))
        sheet.paste((20, 100, 200, 128), (3, 5, 16, 29))
        sheet.paste((255, 50, 20, 255), (57, 42, 83, 67))
        self.assertEqual(pipeline.regions(sheet), [[3, 5, 13, 24], [57, 42, 26, 25]])
        with self.assertRaisesRegex(ValueError, 'opaca'):
            pipeline.regions(Image.new('RGBA', (10, 10), (10, 10, 10, 255)))

    def fixture(self, root):
        source = root/'source'
        source.mkdir()
        entries = []
        for category, filename in pipeline.SOURCES.items():
            sheet = Image.new('RGBA', (80, 60))
            sheet.paste((20, 100, 200, 127), (3, 5, 48, 30))
            path = source/filename
            sheet.save(path)
            entries.append({'category': category, 'source': filename,
                            'sha256': hashlib.sha256(path.read_bytes()).hexdigest(),
                            'reviewed': True,
                            'frames': [{'id': 'one', 'rect': [3, 5, 45, 25], 'origin': [22, 25]},
                                       {'id': 'two', 'rect': [3, 5, 45, 25], 'origin': [22, 25]},
                                       {'id': 'three', 'rect': [3, 5, 45, 25], 'origin': [22, 25]}],
                            'animations': {'idle': {'frames': ['one'], 'fps': 4, 'loop': True}}})
        manifest = root/'review.json'
        manifest.write_text(json.dumps({'version': 1, 'missing': [], 'sheets': entries}))
        return source, manifest, entries

    def test_packing_preserves_alpha_and_origin_across_pages(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source, manifest, entries = self.fixture(root)
            pipeline.pack(source, manifest, root, 64)
            for entry in entries:
                category = Path(entry['category'])
                output = json.loads((root/'animations'/category.with_suffix('.json')).read_text())
                self.assertEqual(len(output['pages']), 2)
                self.assertEqual(output['frames']['one']['origin'], [22, 25])
                original = pipeline.load_sheet(source/entry['source']).crop((3, 5, 48, 30))
                record = output['frames']['one']
                page = pipeline.load_sheet(root/'atlases'/output['pages'][record['page']])
                x, y, width, height = record['rect']
                self.assertEqual(page.crop((x, y, x+width, y+height)).tobytes(), original.tobytes())
                self.assertEqual(pipeline.load_sheet(root/'processed'/category/'one.png').tobytes(), original.tobytes())

    def test_rejects_unreviewed_or_changed_source_before_writing(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source, manifest, entries = self.fixture(root)
            entries[-1]['reviewed'] = False
            manifest.write_text(json.dumps({'version': 1, 'missing': [], 'sheets': entries}))
            with self.assertRaisesRegex(ValueError, 'reveja'):
                pipeline.pack(source, manifest, root, 64)
            self.assertFalse((root/'processed').exists())
            entries[-1]['reviewed'] = True
            manifest.write_text(json.dumps({'version': 1, 'missing': [], 'sheets': entries}))
            (source/entries[-1]['source']).write_bytes(b'changed')
            with self.assertRaisesRegex(ValueError, 'mudou'):
                pipeline.pack(source, manifest, root, 64)

    def test_manual_opaque_crop_and_bad_animation(self):
        sheet = Image.new('RGBA', (20, 20), 'orange')
        entry = {'category': 'enemies/fox', 'reviewed': True,
                 'frames': [{'id': 'bite', 'rect': [1, 2, 10, 12], 'origin': [5, 15]}],
                 'animations': {'bite': {'frames': ['bite'], 'fps': 8}}}
        self.assertEqual(pipeline.validate_entry(entry, sheet), Path('enemies/fox'))
        entry['animations']['bite']['frames'] = ['missing']
        with self.assertRaisesRegex(ValueError, 'referência'):
            pipeline.validate_entry(entry, sheet)
        entry['animations'] = {}
        entry['frames'][0]['rect'] = [10, 10, 20, 20]
        with self.assertRaisesRegex(ValueError, 'fora'):
            pipeline.validate_entry(entry, sheet)

    def test_missing_inputs_reported_and_review_not_overwritten(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            manifest = root/'review.json'
            self.assertEqual(pipeline.inspect(root, manifest), 2)
            self.assertEqual(len(json.loads(manifest.read_text())['missing']), 7)
            with self.assertRaisesRegex(ValueError, 'preservar'):
                pipeline.inspect(root, manifest)


if __name__ == '__main__':
    unittest.main()
