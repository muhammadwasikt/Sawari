"""Import Blender's rigged Sawari taxi as an Unreal skeletal mesh.

Run after creating Exports/SK_SawariTaxi.fbx:
  py "C:/FULL/PATH/TO/Sawari/Scripts/import_sawari_taxi.py"

The script requests Physics Asset generation and verifies the required bone names.
The Wheel Controller Animation Blueprint must still be created in the Editor.
"""
from pathlib import Path
import unreal

ROOT = Path(unreal.Paths.project_dir())
FBX = ROOT / "Exports" / "SK_SawariTaxi.fbx"
DEST = "/Game/Vehicles/Taxi"
EXPECTED = DEST + "/SK_SawariTaxi.SK_SawariTaxi"

def set_property(obj, name, value):
    try:
        obj.set_editor_property(name, value)
        return True
    except Exception as exc:
        unreal.log_warning("Skipped FBX setting %s: %s" % (name, exc))
        return False

def main():
    if not FBX.exists():
        raise RuntimeError("Missing taxi FBX. Run Blender/create_sawari_taxi.py first: " + str(FBX))

    ui = unreal.FbxImportUI()
    set_property(ui, "import_mesh", True)
    set_property(ui, "import_as_skeletal", True)
    set_property(ui, "import_animations", False)
    set_property(ui, "import_materials", True)
    set_property(ui, "import_textures", False)
    if not set_property(ui, "create_physics_asset", True):
        raise RuntimeError("This Unreal version did not accept create_physics_asset on FbxImportUI.")

    try:
        set_property(ui, "mesh_type_to_import", unreal.FBXImportType.FBXIT_SKELETAL_MESH)
    except Exception:
        pass

    task = unreal.AssetImportTask()
    set_property(task, "filename", str(FBX))
    set_property(task, "destination_path", DEST)
    set_property(task, "destination_name", "SK_SawariTaxi")
    set_property(task, "replace_existing", True)
    set_property(task, "automated", True)
    set_property(task, "save", True)
    set_property(task, "options", ui)

    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    imported = task.get_editor_property("imported_object_paths")
    if not imported:
        raise RuntimeError("Unreal FBX importer did not create an asset. Check the Output Log.")

    mesh = unreal.load_asset(EXPECTED)
    if not isinstance(mesh, unreal.SkeletalMesh):
        raise RuntimeError("Expected Skeletal Mesh not found at " + EXPECTED)

    skeleton = mesh.get_editor_property("skeleton")
    bone_names = []
    if skeleton:
        try:
            bone_names = [str(name) for name in skeleton.get_reference_skeleton().get_raw_bone_names()]
        except Exception:
            pass

    required = {"Chassis", "Wheel_Front_Left", "Wheel_Front_Right", "Wheel_Rear_Left", "Wheel_Rear_Right"}
    missing = sorted(required.difference(set(bone_names)))
    if missing:
        raise RuntimeError("Imported taxi is missing required Chaos wheel bones: " + ", ".join(missing))

    physics_asset = mesh.get_editor_property("physics_asset")
    if not physics_asset:
        raise RuntimeError(
            "Skeletal Mesh imported but has no Physics Asset. Open the mesh and create/assign a Physics Asset before testing Chaos."
        )

    unreal.EditorAssetLibrary.save_loaded_asset(mesh)
    unreal.EditorAssetLibrary.save_loaded_asset(physics_asset)
    unreal.log("Sawari taxi Skeletal Mesh imported: " + EXPECTED)
    unreal.log("Physics Asset generated: " + physics_asset.get_path_name())
    unreal.log("All four wheel bones validated.")
    unreal.log_warning(
        "Next Editor step: create /Game/Vehicles/Taxi/ABP_SawariTaxi based on VehicleAnimationInstance and add four Wheel Controller nodes."
    )

if __name__ == "__main__":
    main()
