"""Release regression: verified source bytes must be the bytes delivered."""
import importlib.util
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch
import zipfile
ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'Tools'))
from checkpoint_inputs import capture
import package_checkpoint

class CheckpointTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.base = Path(self.temp.name)
        self.project = self.base / 'project'
        (self.project / 'Source').mkdir(parents=True)
        (self.project / 'Reports').mkdir()
        (self.project / 'Source/game.cpp').write_text('int value=7;')
        (self.project / 'AskirRPG.uproject').write_text('{}')
        for args in (['init', '-q'], ['add', '.'],
                     ['-c', 'user.name=Checkpoint Test', '-c', 'user.email=checkpoint@example.invalid', '-c', 'commit.gpgsign=false', 'commit', '-qm', 'baseline']):
            subprocess.run(['git'] + args, cwd=self.project, check=True, capture_output=True)
        self.report = self.project / 'Reports/check.json'
        self.output = self.base / 'checkpoint.zip'
        self.refresh()
    def tearDown(self):
        self.temp.cleanup()
    def refresh(self):
        self.report.write_text(json.dumps({'version':'V79', 'checks':{'production_rules':{'status':'Bestanden'}}, 'verified_inputs':capture(self.project)}))
    def build(self, version='V79_Checkpoint01'):
        with patch.object(package_checkpoint, 'ROOT', self.project):
            return package_checkpoint.build(version, self.output, self.report)
    def test_zip_delivers_verified_bytes(self):
        result=self.build()
        self.assertEqual(result['all_file_hashes'], 'Bestanden')
        with zipfile.ZipFile(self.output) as archive:
            self.assertIsNone(archive.testzip())
            self.assertEqual(archive.read('AskirRPG_V79_Checkpoint01/Source/game.cpp'), b'int value=7;')
    def test_changed_source_rejected(self):
        (self.project/'Source/game.cpp').write_text('int value=8;')
        with self.assertRaisesRegex(ValueError, 'changed since verification'):self.build()
        self.assertFalse(self.output.exists())
    def test_added_source_rejected(self):
        (self.project/'Source/untested.cpp').write_text('int new_value=3;')
        with self.assertRaisesRegex(ValueError, 'changed since verification'):self.build()
    def test_wrong_version_rejected(self):
        with self.assertRaisesRegex(ValueError, 'version must match'):self.build('V80')
    def test_unmaterialized_lfs_rejected(self):
        (self.project/'Source/mesh.fbx').write_text('version https://git-lfs.github.com/spec/v1\noid sha256:invalid\nsize 100\n')
        self.refresh()
        with self.assertRaisesRegex(ValueError, 'LFS object not materialized'):self.build()
    def test_failed_check_rejected(self):
        data=json.loads(self.report.read_text());data['checks']['production_rules']['status']='Fehlgeschlagen';self.report.write_text(json.dumps(data))
        with self.assertRaisesRegex(ValueError, 'Offline checks must pass'):self.build()
if __name__=='__main__':unittest.main()
