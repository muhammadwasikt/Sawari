#!/usr/bin/env python3
"""Fetch a Karachi corridor OSM extract and stage it for Unreal runtime import.

The selected development corridor intentionally covers Saddar through Clifton
so the first playable pickup-to-drop-off route can use real street geometry.
No network call is made from the running game; generated GeoJSON is bundled.
"""
from __future__ import annotations

import json
import shutil
import time
import urllib.parse
import urllib.request
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

# Saddar-to-Clifton corridor. This is a playable prototype extract, not all Karachi.
SOUTH, WEST, NORTH, EAST = 24.785, 66.985, 24.890, 67.095
ORIGIN_LAT = (SOUTH + NORTH) / 2.0
ORIGIN_LON = (WEST + EAST) / 2.0

ROOT = Path(__file__).resolve().parents[1]
SOURCE_DIR = ROOT / "SourceData" / "Saddar"
UNREAL_DIR = ROOT / "Sawari" / "Content" / "SawariData" / "Saddar"
ENDPOINTS = [
    "https://overpass-api.de/api/interpreter",
    "https://overpass.kumi.systems/api/interpreter",
]

def overpass_query() -> str:
    bbox = f"{SOUTH},{WEST},{NORTH},{EAST}"
    return f"""[out:json][timeout:180];
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
    encoded = urllib.parse.urlencode({"data": overpass_query()}).encode("utf-8")
    last_error: Exception | None = None

    for endpoint in ENDPOINTS:
        request = urllib.request.Request(
            endpoint,
            data=encoded,
            headers={
                "User-Agent": "SawariPrototype/0.2 (Karachi OSM development pipeline)",
                "Content-Type": "application/x-www-form-urlencoded; charset=utf-8",
                "Accept": "application/json",
            },
            method="POST",
        )
        for attempt in range(4):
            try:
                with urllib.request.urlopen(request, timeout=210) as response:
                    payload = json.loads(response.read().decode("utf-8"))
                if "elements" not in payload or not isinstance(payload["elements"], list):
                    raise ValueError("Overpass response did not contain an elements array")
                return payload
            except Exception as exc:
                last_error = exc
                delay = min(20, 2 ** attempt)
                print(f"Overpass attempt failed ({endpoint}): {exc}; retrying in {delay}s")
                time.sleep(delay)

    raise RuntimeError(f"Unable to fetch Karachi OSM data from Overpass: {last_error}")

def make_feature(element: dict[str, Any], geometry_type: str, coordinates: Any) -> dict[str, Any]:
    return {
        "type": "Feature",
        "id": f"{element['type']}/{element['id']}",
        "properties": {
            "osm_id": element["id"],
            "osm_type": element["type"],
            **element.get("tags", {}),
        },
        "geometry": {"type": geometry_type, "coordinates": coordinates},
    }

def convert(payload: dict[str, Any]) -> tuple[dict[str, Any], dict[str, Any], dict[str, Any]]:
    roads: list[dict[str, Any]] = []
    buildings: list[dict[str, Any]] = []
    pois: list[dict[str, Any]] = []

    poi_keys = {"amenity", "shop", "tourism", "public_transport"}

    for element in payload.get("elements", []):
        tags = element.get("tags", {})
        element_type = element.get("type")

        if element_type == "way":
            points = element.get("geometry") or []
            coordinates = [
                [point["lon"], point["lat"]]
                for point in points
                if "lon" in point and "lat" in point
            ]
            if len(coordinates) < 2:
                continue

            is_closed = len(coordinates) >= 4 and coordinates[0] == coordinates[-1]
            if "building" in tags and is_closed:
                buildings.append(make_feature(element, "Polygon", [coordinates]))
            elif "highway" in tags:
                roads.append(make_feature(element, "LineString", coordinates))

        elif element_type == "node" and poi_keys.intersection(tags):
            if "lat" in element and "lon" in element:
                pois.append(make_feature(
                    element, "Point", [element["lon"], element["lat"]]
                ))

    def collection(items: list[dict[str, Any]]) -> dict[str, Any]:
        return {"type": "FeatureCollection", "features": items}

    return collection(roads), collection(buildings), collection(pois)

def write_dataset(destination: Path, outputs: dict[str, dict[str, Any]], manifest: dict[str, Any]) -> None:
    destination.mkdir(parents=True, exist_ok=True)
    for filename, data in outputs.items():
        (destination / filename).write_text(
            json.dumps(data, ensure_ascii=False, separators=(",", ":")),
            encoding="utf-8",
        )
    (destination / "manifest.json").write_text(
        json.dumps(manifest, indent=2, ensure_ascii=False),
        encoding="utf-8",
    )

def main() -> None:
    print("Fetching real OSM data for the Saddar-to-Clifton prototype corridor...")
    raw = fetch_osm()
    roads, buildings, pois = convert(raw)

    if not roads["features"]:
        raise RuntimeError("OSM extract returned no drivable roads; no files were staged.")
    if not buildings["features"]:
        raise RuntimeError("OSM extract returned no building footprints; no files were staged.")

    outputs = {
        "roads.geojson": roads,
        "buildings.geojson": buildings,
        "pois.geojson": pois,
    }
    manifest = {
        "source": "OpenStreetMap contributors via Overpass API",
        "license": "ODbL 1.0",
        "bbox_south_west_north_east": [SOUTH, WEST, NORTH, EAST],
        "origin_lat_lon": [ORIGIN_LAT, ORIGIN_LON],
        "features": {
            "roads": len(roads["features"]),
            "buildings": len(buildings["features"]),
            "pois": len(pois["features"]),
        },
        "attribution": "© OpenStreetMap contributors",
        "runtime_network_required": False,
        "extracted_at_utc": datetime.now(timezone.utc).isoformat(),
        "usage_note": "Prototype corridor covering Saddar and Clifton, not all Karachi.",
    }

    write_dataset(SOURCE_DIR, outputs, manifest)
    write_dataset(UNREAL_DIR, outputs, manifest)

    print("Source extract:", SOURCE_DIR)
    print("Unreal staged data:", UNREAL_DIR)
    print("Roads:", manifest["features"]["roads"])
    print("Buildings:", manifest["features"]["buildings"])
    print("POIs:", manifest["features"]["pois"])
    print("Keep © OpenStreetMap contributors attribution with distributed data.")

if __name__ == "__main__":
    main()
