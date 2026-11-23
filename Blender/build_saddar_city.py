"""Build a first stylized-real OSM massing model for Saddar in Blender."""
import bpy
import json
import math
import sys
from pathlib import Path

def args_after_separator():
    args = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    input_dir = Path(args[args.index("--input") + 1]) if "--input" in args else Path("SourceData/Saddar")
    output_file = Path(args[args.index("--output") + 1]) if "--output" in args else Path("Exports/Saddar_Blockout.fbx")
    return input_dir, output_file

def read_geojson(path):
    if not path.exists():
        return []
    with path.open("r", encoding="utf-8") as handle:
        return json.load(handle).get("features", [])

def parse_height(props):
    raw_height = props.get("height")
    if raw_height:
        try:
            return max(3.0, min(100.0, float(str(raw_height).replace("m", "").strip())))
        except ValueError:
            pass
    levels = props.get("building:levels") or props.get("levels")
    if levels:
        try:
            return max(3.0, min(100.0, float(levels) * 3.2))
        except ValueError:
            pass
    kind = str(props.get("building", "yes")).lower()
    if kind in {"commercial", "office", "retail", "industrial"}:
        return 15.0
    if kind in {"apartments", "residential", "house", "terrace"}:
        return 9.0
    return 7.0  # estimated when OSM has no height value

def make_material(name, color, roughness=0.88):
    mat = bpy.data.materials.get(name) or bpy.data.materials.new(name)
    mat.diffuse_color = (*color, 1.0)
    mat.use_nodes = True
    bsdf = mat.node_tree.nodes.get("Principled BSDF")
    if bsdf:
        bsdf.inputs["Base Color"].default_value = (*color, 1.0)
        bsdf.inputs["Roughness"].default_value = roughness
    return mat

def project(lon, lat, origin_lon, origin_lat):
    x = (lon - origin_lon) * 111320.0 * math.cos(math.radians(origin_lat))
    y = (lat - origin_lat) * 110540.0
    return x, y

def build_buildings(features, origin_lon, origin_lat, material):
    verts, faces = [], []
    for item in features:
        geom = item.get("geometry") or {}
        if geom.get("type") != "Polygon":
            continue
        rings = geom.get("coordinates") or []
        if not rings or len(rings[0]) < 4:
            continue
        ring = rings[0]
        ring = ring[:-1] if ring[0] == ring[-1] else ring
        if len(ring) < 3:
            continue
        base = len(verts)
        footprint = [project(p[0], p[1], origin_lon, origin_lat) for p in ring]
        height = parse_height(item.get("properties", {}))
        verts.extend([(x, y, 0.0) for x, y in footprint])
        verts.extend([(x, y, height) for x, y in footprint])
        n = len(footprint)
        faces.append(tuple(base + n + i for i in range(n)))
        for i in range(n):
            nxt = (i + 1) % n
            faces.append((base + i, base + nxt, base + n + nxt, base + n + i))
    mesh = bpy.data.meshes.new("Saddar_Building_Massing_Mesh")
    mesh.from_pydata(verts, [], faces)
    mesh.materials.append(material)
    mesh.update()
    obj = bpy.data.objects.new("Saddar_Buildings_OSM_LOD1", mesh)
    bpy.context.collection.objects.link(obj)
    return obj

def road_width(highway):
    widths = {
        "motorway": 16.0, "trunk": 14.0, "primary": 11.0, "secondary": 9.0,
        "tertiary": 7.0, "residential": 6.0, "unclassified": 5.0, "service": 3.5,
        "living_street": 4.5, "pedestrian": 3.0, "footway": 1.8, "path": 1.5,
    }
    return widths.get(str(highway), 4.0)

def build_roads(features, origin_lon, origin_lat, material):
    verts, faces = [], []
    for item in features:
        geom = item.get("geometry") or {}
        if geom.get("type") != "LineString":
            continue
        coords = geom.get("coordinates") or []
        if len(coords) < 2:
            continue
        width = road_width(item.get("properties", {}).get("highway"))
        for a, b in zip(coords, coords[1:]):
            ax, ay = project(a[0], a[1], origin_lon, origin_lat)
            bx, by = project(b[0], b[1], origin_lon, origin_lat)
            dx, dy = bx - ax, by - ay
            length = math.hypot(dx, dy)
            if length < 0.5:
                continue
            nx, ny = -dy / length * width / 2.0, dx / length * width / 2.0
            base = len(verts)
            verts.extend([(ax+nx, ay+ny, 0.12), (ax-nx, ay-ny, 0.12),
                          (bx-nx, by-ny, 0.12), (bx+nx, by+ny, 0.12)])
            faces.append((base, base+1, base+2, base+3))
    mesh = bpy.data.meshes.new("Saddar_Road_Corridors_Mesh")
    mesh.from_pydata(verts, [], faces)
    mesh.materials.append(material)
    mesh.update()
    obj = bpy.data.objects.new("Saddar_Roads_OSM", mesh)
    bpy.context.collection.objects.link(obj)
    return obj

def build_pois(features, origin_lon, origin_lat, material):
    markers = []
    for index, item in enumerate(features):
        coords = (item.get("geometry") or {}).get("coordinates")
        if not coords:
            continue
        x, y = project(coords[0], coords[1], origin_lon, origin_lat)
        bpy.ops.mesh.primitive_cube_add(size=1.0, location=(x, y, 2.0))
        obj = bpy.context.object
        obj.name = f"OSM_POI_{index:04d}"
        obj.dimensions = (1.4, 1.4, 4.0)
        obj.data.materials.append(material)
        markers.append(obj)
    return markers

def main():
    input_dir, output_file = args_after_separator()
    if not input_dir.is_absolute():
        input_dir = Path.cwd() / input_dir
    if not output_file.is_absolute():
        output_file = Path.cwd() / output_file
    output_file.parent.mkdir(parents=True, exist_ok=True)
    manifest_path = input_dir / "manifest.json"
    if not manifest_path.exists():
        raise FileNotFoundError(f"Missing {manifest_path}. Run Scripts/fetch_saddar_osm.py first.")
    manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    south, west, north, east = manifest["bbox_south_west_north_east"]
    origin_lat, origin_lon = (south+north)/2.0, (west+east)/2.0
    roads = read_geojson(input_dir / "roads.geojson")
    buildings = read_geojson(input_dir / "buildings.geojson")
    pois = read_geojson(input_dir / "pois.geojson")
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)
    building_mat = make_material("Sawari_Warm_Concrete", (0.66, 0.60, 0.49))
    road_mat = make_material("Sawari_Asphalt", (0.12, 0.14, 0.16))
    poi_mat = make_material("Sawari_POI_Marker", (0.05, 0.70, 0.62))
    building_obj = build_buildings(buildings, origin_lon, origin_lat, building_mat)
    road_obj = build_roads(roads, origin_lon, origin_lat, road_mat)
    poi_objects = build_pois(pois, origin_lon, origin_lat, poi_mat)
    bpy.ops.object.select_all(action="DESELECT")
    export_objects = [building_obj, road_obj, *poi_objects]
    for obj in export_objects:
        obj.select_set(True)
    bpy.context.view_layer.objects.active = building_obj
    bpy.ops.export_scene.fbx(filepath=str(output_file), use_selection=True,
        apply_unit_scale=True, global_scale=100.0, axis_forward="-Z", axis_up="Y",
        object_types={"MESH"})
    print(f"Exported OSM blockout to {output_file}")
    print("Missing building heights are estimated. POI cubes are temporary validation markers.")

if __name__ == "__main__":
    main()
