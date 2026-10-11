"""Create native-parent Blueprint assets in Unreal Editor 5.6.
DRY_RUN defaults to True. Requires compiled Askir C++ classes.
Missing parents are reported; no fake .uasset files are written.
"""
import json
from pathlib import Path
import unreal

DRY_RUN = True
OVERWRITE_EXISTING = False
ROOT = Path(__file__).resolve().parent.parent
MANIFEST = ROOT / 'Production' / 'Blueprints' / 'blueprint_manifest_v50.json'

def run():
    items = json.loads(MANIFEST.read_text(encoding='utf-8'))['blueprints']
    assets = unreal.AssetToolsHelpers.get_asset_tools()
    lib = unreal.EditorAssetLibrary
    result = {'created': [], 'existing': [], 'missing_parent': [], 'failed': [], 'preview': []}
    for item in items:
        path = item['asset_path']
        if lib.does_asset_exist(path):
            result['existing'].append(path)
            continue
        if DRY_RUN:
            result['preview'].append(path)
            continue
        native = unreal.load_class(None, item['native_path'])
        if native is None:
            result['missing_parent'].append(item['native_path'])
            continue
        try:
            folder = path.rsplit('/', 1)[0]
            if not lib.does_directory_exist(folder):
                lib.make_directory(folder)
            factory = unreal.BlueprintFactory()
            factory.set_editor_property('parent_class', native)
            bp = assets.create_asset(item['name'], folder, unreal.Blueprint, factory)
            if bp is None:
                result['failed'].append(path)
            elif lib.save_loaded_asset(bp):
                result['created'].append(path)
            else:
                result['failed'].append(path + ' (save failed)')
        except Exception as exc:
            result['failed'].append(path + ': ' + str(exc))
    unreal.log('[Askir Blueprint V50] ' + json.dumps(result, ensure_ascii=False))
    return result

if __name__ == '__main__':
    run()
