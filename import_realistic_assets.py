# Run in Unreal Engine 5.6 editor Python console:
# exec(__import__("pathlib").Path(r"C:/path/to/AskirRPG/Tools/import_realistic_assets.py").read_text(encoding="utf-8"), {"__file__":r"C:/path/to/AskirRPG/Tools/import_realistic_assets.py"})
# Enable "Python Editor Script Plugin" in Edit > Plugins.
import unreal
from pathlib import Path

root = Path(__file__).resolve().parents[1] if "__file__" in globals() else None
if root is None:
    raise RuntimeError("Run as a .py file so the project-relative asset path is known.")
source = root / "ArtSource" / "Realistic3D_V1"
destination = "/Game/Askir/Realistic3D_V1"
asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
imported = []
for path in sorted(source.glob("SM_*.obj")):
    task = unreal.AssetImportTask()
    task.filename = str(path)
    task.destination_path = destination
    task.destination_name = path.stem
    task.automated = True
    task.replace_existing = False
    task.save = True
    task.options = None
    asset_tools.import_asset_tasks([task])
    imported.extend(task.get_editor_property("imported_object_paths"))
unreal.log("Askir 3D imported: " + str(imported))
