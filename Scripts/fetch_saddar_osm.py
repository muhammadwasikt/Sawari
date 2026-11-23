#!/usr/bin/env python3
"""Fetch a small Saddar, Karachi OSM extract and split it into GeoJSON layers."""
from __future__ import annotations
import json
import time
import urllib.parse
import urllib.request
from pathlib import Path
from typing import Any

# Starter test box only; review the boundary before calling this the full district.
SOUTH, WEST, NORTH, EAST = 24.845, 67.003, 24.872, 67.038
OUT_DIR = Path(__file__).resolve().parents[1] / "SourceData" / "Saddar"
ENDPOINTS = [
    "https://overpass-api.de/api/interpreter",
    "https://overpass.kumi.systems/api/interpreter",
]

def overpass_query() -> str:
    bbox = f"{SOUTH},{WEST},{NORTH},{EAST}"
    return f"""[out:json][timeout:120];
(
  way["highway"]({bbox});
  way["building"]({bbox});
  node["amenity"]({bbox});
  node["shop"]({bbox});
  node["tourism"]({bbox});
  node["public_transport"]({bbox});
);
out geom;"""

def fetch_osm() -> dict[str, Any]:
    encoded = urllib.parse.urlencode({"data": overpass_query()})
    last_error: Exception | None = None
    for endpoint in ENDPOINTS:
        request = urllib.request.Request(
            f"{endpoint}?{encoded}",
            headers={"User-Agent": "SawariPrototype/0.1 (OSM research/build pipeline)"},
        )
        for attempt in range(3):
            try:
                with urllib.request.urlopen(request, timeout=150) as response:
                    payload = json.loads(response.read().decode("utf-8"))
                if "elements" not in payload:
                    raise ValueError("Overpass response did not contain an elements array")
                return payload
            except Exception as exc:
                last_error = exc
                time.sleep(2 ** attempt)
    raise RuntimeError(f"Unable to fetch OSM data from Overpass: {last_error}")

def feature(element: dict[str, Any], geometry_type: str, coordinates: Any) -> dict[str, Any]:
    return {
        "type": "Feature",
        "id": f"{element['type']}/{element['id']}",
        "properties": {"osm_id": element["id"], "osm_type": element["type"], **element.get("tags", {})},
        "geometry": {"type": geometry_type, "coordinates": coordinates},
    }

def convert(payload: dict[str, Any]) -> tuple[dict[str, Any], dict[str, Any], dict[str, Any]]:
    roads: list[dict[str, Any]] = []
    buildings: list[dict[str, Any]] = []
    pois: list[dict[str, Any]] = []
    for element in payload.get("elements", []):
        tags = element.get("tags", {})
        if element["type"] == "way":
            points = element.get("geometry") or []
            coords = [[p["lon"], p["lat"]] for p in points if "lon" in p and "lat" in p]
            if len(coords) < 2:
                continue
            if "building" in tags and len(coords) >= 4 and coords[0] == coords[-1]:
                buildings.append(feature(element, "Polygon", [coords]))
            elif "highway" in tags:
                roads.append(feature(element, "LineString", coords))
        elif element["type"] == "node":
            if {"amenity", "shop", "tourism", "public_transport"}.intersection(tags):
                pois.append(feature(element, "Point", [element["lon"], element["lat"]]))

    def collection(items: list[dict[str, Any]]) -> dict[str, Any]:
        return {"type": "FeatureCollection", "features": items}
    return collection(roads), collection(buildings), collection(pois)

def main() -> None:
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    print("Fetching OSM ways/nodes for the Saddar starter bbox...")
    raw = fetch_osm()
    roads, buildings, pois = convert(raw)
    outputs = {"roads.geojson": roads, "buildings.geojson": buildings, "pois.geojson": pois}
    for filename, data in outputs.items():
        path = OUT_DIR / filename
        path.write_text(json.dumps(data, ensure_ascii=False), encoding="utf-8")
        print(f"Wrote {path} ({len(data['features'])} features)")
    manifest = {
        "source": "OpenStreetMap contributors via Overpass API",
        "license": "ODbL 1.0",
        "bbox_south_west_north_east": [SOUTH, WEST, NORTH, EAST],
        "features": {name.removesuffix(".geojson"): len(data["features"]) for name, data in outputs.items()},
        "attribution": "© OpenStreetMap contributors",
        "runtime_network_required": False,
    }
    (OUT_DIR / "manifest.json").write_text(json.dumps(manifest, indent=2), encoding="utf-8")
    print("Done. Keep the attribution with distributed OSM-derived content.")

if __name__ == "__main__":
    main()
