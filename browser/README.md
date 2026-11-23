# Sawari Browser Game — first playable milestone

This is a standalone Three.js browser version of Sawari. It runs without Unreal Engine.

## Implemented
- Third-person 3D yellow taxi, chase camera, keyboard driving and touch steering wheel/pedals.
- Touch accelerator and brake stay active for as long as the player holds the pedal.
- The taxi spawns on the nearest downloaded OpenStreetMap road node.
- Saddar–Clifton roads and building footprints load from OpenStreetMap Overpass at runtime; a map failure is shown rather than replacing Karachi with fabricated streets.
- 3D road surfaces and extruded OSM building footprints. Buildings are batched by material to reduce draw calls; collision checks use the building footprint polygon.
- Basic ambient road traffic, road-graph shortest route, passenger pickup/drop-off and route markers.
- Fare calculation with 85% driver / 15% platform split, wallet, daily target, three upgrade tracks and local saved career.
- English/Urdu interface, simple synthesized engine sound, responsive desktop/mobile layout and pause-on-tab-hide.

## Run on a computer
From the repository root, run:
```bash
python -m http.server 8000
```
Open http://localhost:8000/browser/ in Chrome or another modern browser. The game uses ES modules, so opening the file directly with `file://` is not supported.

## Publish on GitHub Pages
This project repository is private at the moment. GitHub Free supports GitHub Pages for public repositories; private repository Pages needs a paid plan. After the owner changes this repository to public:
1. Open Settings → Pages and choose **GitHub Actions** as the publishing source.
2. Open Actions → **Deploy Sawari Browser Game** → **Run workflow**.
3. After the first successful deployment, open the Pages URL shown in the workflow's deployment environment. The root page redirects into `/browser/`.

The deployment workflow is manual-only to avoid unnecessary Actions runs. Run it again after publishing changes to `main`.

## Known limitations
This is a first playable vertical slice, not the finished realistic city simulator. OSM ways are used for road/building geometry; relation-only building footprints may be missing. Buildings are massing models, not detailed facades or Karachi landmarks. Traffic is a basic moving-car layer without signals, lane changes, overtaking, rickshaws/buses or advanced collision avoidance. There is no full turn-by-turn voice navigation or multi-vehicle garage/unlock tree yet. The map is downloaded at game launch from a public Overpass endpoint, so internet and a responsive endpoint are needed on first load; Three.js is imported from jsDelivr. Map attribution to OpenStreetMap contributors is displayed in-game; data is under ODbL: https://www.openstreetmap.org/copyright.
