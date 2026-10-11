"""Run INSIDE Unreal 5.6 Editor; Blender FBX conversion must run first."""
from pathlib import Path
import unreal
root=Path(__file__).resolve().parents[1]
source=root/'ArtSource/EnhancedFBX_V19'
if not source.is_dir():raise RuntimeError('Run Blender export first')
tools=unreal.AssetToolsHelpers.get_asset_tools()
for file in sorted(source.glob('SM_*.fbx')):
 task=unreal.AssetImportTask();task.filename=str(file)
 task.destination_path='/Game/Askir/Enhanced3D_V19';task.destination_name=file.stem
 task.automated=True;task.replace_existing=False;task.save=True
 options=unreal.FbxImportUI();options.import_mesh=True;options.import_as_skeletal=False
 options.static_mesh_import_data.combine_meshes=True
 task.options=options;tools.import_asset_tasks([task])
 unreal.log(f'{file.name}: {task.imported_object_paths}')
