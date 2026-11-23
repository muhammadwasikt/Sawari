import importlib.util
import json
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location("fetch_saddar_osm", ROOT / "Scripts" / "fetch_saddar_osm.py")
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)


class OsmConverterTests(unittest.TestCase):
    def test_split_roads_buildings_and_pois(self):
        payload = json.loads((ROOT / "tests" / "fixtures" / "tiny_osm.json").read_text())
        roads, buildings, pois = MODULE.convert(payload)
        self.assertEqual(len(roads["features"]), 1)
        self.assertEqual(len(buildings["features"]), 1)
        self.assertEqual(len(pois["features"]), 1)
        self.assertEqual(roads["features"][0]["geometry"]["type"], "LineString")
        self.assertEqual(buildings["features"][0]["geometry"]["type"], "Polygon")
        self.assertEqual(pois["features"][0]["properties"]["amenity"], "fuel")


if __name__ == "__main__":
    unittest.main()
