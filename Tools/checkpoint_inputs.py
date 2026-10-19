"""Hash the executable source inputs, excluding generated reports and bytecode."""
from pathlib import Path
import hashlib

INPUT_DIRECTORIES = ('Source', 'Tools', 'Config', 'Production', 'Tests', 'ArtSource', 'Scripts', '.github')
INPUT_FILES = ('AskirRPG.uproject', 'requirements-dev.txt', '.gitattributes')

def capture(root):
    root = Path(root)
    paths = {p for folder in INPUT_DIRECTORIES for p in (root / folder).rglob('*')
             if p.is_file() and '__pycache__' not in p.parts and p.suffix != '.pyc'}
    paths.update(root / name for name in INPUT_FILES if (root / name).is_file())
    return {str(p.relative_to(root)): hashlib.sha256(p.read_bytes()).hexdigest()
            for p in sorted(paths)}
