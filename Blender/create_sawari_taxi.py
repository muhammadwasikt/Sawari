"""Generate a rigged low-poly Karachi taxi for the Sawari Chaos Vehicle.

Run from the repository root:
  blender --background --python Blender/create_sawari_taxi.py

The mesh is exported in centimetre-scaled FBX units with chassis and four
wheel deform bones. It is a clean engineering vehicle asset, not photoreal art.
"""
import bpy
import math
from pathlib import Path
from mathutils import Vector

ROOT = Path(__file__).resolve().parents[1]
OUTPUT = ROOT / "Exports" / "SK_SawariTaxi.fbx"
OUTPUT.parent.mkdir(parents=True, exist_ok=True)

def material(name, color, metallic=0.0, roughness=0.55):
    mat = bpy.data.materials.new(name)
    mat.diffuse_color = (*color, 1.0)
    mat.use_nodes = True
    p = mat.node_tree.nodes.get("Principled BSDF")
    if p:
        p.inputs["Base Color"].default_value = (*color, 1.0)
        p.inputs["Metallic"].default_value = metallic
        p.inputs["Roughness"].default_value = roughness
    return mat

YELLOW = material("Sawari Taxi Yellow", (0.95, 0.58, 0.035), 0.12, 0.38)
GLASS = material("Smoked Blue Glass", (0.035, 0.12, 0.16), 0.35, 0.2)
BLACK = material("Rubber Black", (0.018, 0.022, 0.026), 0.0, 0.88)
SILVER = material("Wheel Alloy", (0.45, 0.48, 0.50), 0.72, 0.28)
WHITE = material("Warm White", (0.88, 0.90, 0.87), 0.1, 0.35)
RED = material("Rear Lamp Red", (0.68, 0.035, 0.025), 0.05, 0.3)
DARK = material("Taxi Trim", (0.035, 0.04, 0.045), 0.2, 0.5)

bpy.ops.object.select_all(action="SELECT")
bpy.ops.object.delete(use_global=False)
scene = bpy.context.scene
scene.unit_settings.system = "METRIC"
scene.unit_settings.scale_length = 1.0

def bevel_box(name, location, dimensions, mat, bone_name="Chassis", bevel=0.035):
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=location)
    obj = bpy.context.object
    obj.name = name
    obj.dimensions = dimensions
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    if bevel > 0:
        mod = obj.modifiers.new("Soft body edges", "BEVEL")
        mod.width = bevel
        mod.segments = 3
        bpy.context.view_layer.objects.active = obj
        bpy.ops.object.modifier_apply(modifier=mod.name)
    obj.data.materials.append(mat)
    vg = obj.vertex_groups.new(name=bone_name)
    if obj.data.vertices:
        vg.add([v.index for v in obj.data.vertices], 1.0, "REPLACE")
    return obj

def cylinder(name, location, radius, depth, mat, bone_name, vertices=32):
    bpy.ops.mesh.primitive_cylinder_add(
        vertices=vertices,
        radius=radius,
        depth=depth,
        end_fill_type="NGON",
        location=location,
        rotation=(0.0, math.radians(90.0), 0.0)
    )
    obj = bpy.context.object
    obj.name = name
    obj.data.materials.append(mat)
    for poly in obj.data.polygons:
        poly.use_smooth = True
    vg = obj.vertex_groups.new(name=bone_name)
    vg.add([v.index for v in obj.data.vertices], 1.0, "REPLACE")
    return obj

# Four conventional wheel joints. Blender Y is vehicle-forward; the FBX axis
# conversion below maps that to Unreal X-forward.
wheel_specs = [
    ("Wheel_Front_Left", -0.88, 1.36, 0.36),
    ("Wheel_Front_Right", 0.88, 1.36, 0.36),
    ("Wheel_Rear_Left", -0.88, -1.34, 0.36),
    ("Wheel_Rear_Right", 0.88, -1.34, 0.36),
]

bpy.ops.object.armature_add(enter_editmode=True, location=(0.0, 0.0, 0.0))
arm = bpy.context.object
arm.name = "SK_SawariTaxi_Armature"
arm.data.name = "SK_SawariTaxi_Skeleton"
root = arm.data.edit_bones[0]
root.name = "Chassis"
root.head = (0.0, 0.0, 0.55)
root.tail = (0.0, 0.0, 1.05)

for bone_name, x, y, z in wheel_specs:
    bone = arm.data.edit_bones.new(bone_name)
    bone.head = (x, y, z)
    # Bone length is along the wheel axle; this orientation must be validated
    # against the Wheel Controller node after FBX import.
    bone.tail = (x + (0.28 if x < 0 else -0.28), y, z)
    bone.parent = root
    bone.use_connect = False

bpy.ops.object.mode_set(mode="OBJECT")
arm.data.display_type = "OCTAHEDRAL"
arm.show_in_front = True

parts = []
parts.append(bevel_box("Taxi lower shell", (0.0, 0.0, 0.66), (1.82, 4.28, 0.78), YELLOW, bevel=0.09))
parts.append(bevel_box("Taxi cabin roof", (0.0, -0.12, 1.38), (1.54, 2.14, 0.22), YELLOW, bevel=0.10))
parts.append(bevel_box("Front bonnet", (0.0, 1.50, 0.98), (1.70, 1.12, 0.22), YELLOW, bevel=0.07))
parts.append(bevel_box("Rear boot", (0.0, -1.58, 0.98), (1.66, 0.90, 0.22), YELLOW, bevel=0.07))

# Glazing panels and pillars give a clear taxi silhouette without pretending
# to be a finished production vehicle mesh.
parts.append(bevel_box("Front windshield", (0.0, 0.89, 1.27), (1.47, 0.10, 0.48), GLASS, bevel=0.025))
parts.append(bevel_box("Rear windshield", (0.0, -1.18, 1.27), (1.40, 0.10, 0.42), GLASS, bevel=0.025))
for x in (-0.79, 0.79):
    parts.append(bevel_box("Side front window", (x, 0.38, 1.31), (0.055, 0.86, 0.43), GLASS, bevel=0.02))
    parts.append(bevel_box("Side rear window", (x, -0.58, 1.31), (0.055, 0.78, 0.43), GLASS, bevel=0.02))
    parts.append(bevel_box("B pillar", (x, -0.08, 1.31), (0.085, 0.11, 0.52), YELLOW, bevel=0.02))
    parts.append(bevel_box("Front headlight", (x * 0.84, 2.13, 0.92), (0.22, 0.08, 0.15), WHITE, bevel=0.035))
    parts.append(bevel_box("Rear tail light", (x * 0.84, -2.13, 0.92), (0.22, 0.08, 0.17), RED, bevel=0.025))

parts.append(bevel_box("Front bumper", (0.0, 2.18, 0.54), (1.87, 0.14, 0.17), DARK, bevel=0.04))
parts.append(bevel_box("Rear bumper", (0.0, -2.18, 0.54), (1.87, 0.14, 0.17), DARK, bevel=0.04))
parts.append(bevel_box("Taxi roof sign", (0.0, -0.03, 1.55), (0.52, 0.32, 0.14), WHITE, bevel=0.04))

for bone_name, x, y, z in wheel_specs:
    parts.append(cylinder(bone_name + "_Tyre", (x, y, z), 0.36, 0.22, BLACK, bone_name, 40))
    outer_x = x + (-0.115 if x < 0 else 0.115)
    parts.append(cylinder(bone_name + "_Alloy", (outer_x, y, z), 0.205, 0.018, SILVER, bone_name, 32))
    parts.append(cylinder(bone_name + "_Hub", (outer_x + (-0.012 if x < 0 else 0.012), y, z), 0.075, 0.025, DARK, bone_name, 24))

# Join all skinned parts while retaining the per-bone vertex groups and material slots.
bpy.ops.object.select_all(action="DESELECT")
for obj in parts:
    obj.select_set(True)
bpy.context.view_layer.objects.active = parts[0]
bpy.ops.object.join()
mesh = bpy.context.object
mesh.name = "SK_SawariTaxi_Mesh"
mesh.data.name = "SK_SawariTaxi_Geometry"
modifier = mesh.modifiers.new("Taxi armature", "ARMATURE")
modifier.object = arm
mesh.parent = arm

# A bone vertex group must exist for each deform bone. Any unassigned body
# vertices are weighted to the chassis root.
group_names = {g.name for g in mesh.vertex_groups}
if "Chassis" not in group_names:
    root_group = mesh.vertex_groups.new(name="Chassis")
else:
    root_group = mesh.vertex_groups["Chassis"]

assigned = set()
for group in mesh.vertex_groups:
    for vertex in mesh.data.vertices:
        try:
            if any(g.group == group.index for g in vertex.groups):
                assigned.add(vertex.index)
        except Exception:
            pass
unassigned = [v.index for v in mesh.data.vertices if v.index not in assigned]
if unassigned:
    root_group.add(unassigned, 1.0, "REPLACE")

# Export mesh and armature only. Use Unreal-compatible axis conversion and
# metres-to-centimetres scaling.
bpy.ops.object.select_all(action="DESELECT")
arm.select_set(True)
mesh.select_set(True)
bpy.context.view_layer.objects.active = arm
bpy.ops.export_scene.fbx(
    filepath=str(OUTPUT),
    use_selection=True,
    object_types={"ARMATURE", "MESH"},
    add_leaf_bones=False,
    apply_unit_scale=True,
    apply_scale_options="FBX_SCALE_ALL",
    global_scale=100.0,
    axis_forward="-Z",
    axis_up="Y",
    use_mesh_modifiers=True,
    use_armature_deform_only=True,
    bake_anim=False
)
print("Created rigged Sawari taxi FBX:", OUTPUT)
print("Wheel bone names:", ", ".join(x[0] for x in wheel_specs))
