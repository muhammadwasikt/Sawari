# Sawari Browser Game — first playable milestone

This is an independent browser-native Three.js version of Sawari. It does not require Unreal Engine or a local game-engine install.

## Implemented in this milestone
- 3D third-person yellow taxi with throttle, brake, steering, chase camera and simple vehicle handling.
- Desktop keyboard controls (WASD / arrow keys) and mobile touch steering wheel, accelerator and brake pedals.
- Real Saddar–Clifton prototype map loaded at runtime from OpenStreetMap Overpass road ways and building footprints; no fake Karachi-map fallback is shown if the data service is unavailable.
- Procedural 3D road surfaces and extruded building footprints, plus lightweight ambient moving traffic.
- Road-graph shortest-path line, passenger pickup/drop-off interaction and route markers.
- In-game fare calculation with the established 85% driver / 15% platform split.
- Wallet, daily earnings target, engine/handling/brake upgrades and locally saved career progress.
- English/Urdu UI switch and a simple synthesized engine sound with a sound toggle.
- Responsive desktop/mobile layout and pause-on-tab-hide behavior.

## Run locally
Serve the repository over HTTP (ES modules do not reliably run from a file:// URL):
```bash
python -m http.server 8000
```
Open http://localhost:8000/browser/ in a browser.

## Publish with GitHub Pages
The workflow `.github/workflows/deploy-browser.yml` publishes the repository root after changes are merged to `main`. In repository Settings → Pages, select **GitHub Actions** as the deployment source once. The workflow then deploys the site.

## Current limitations — not the finished simulator
This is a first playable vertical slice, not the full Sawari city simulator. OSM ways are used for roads/building footprints; relation-only buildings may be absent. Buildings are massing models rather than detailed facades or landmarks. Traffic is a basic ambient road-following layer: it does not yet have junction signals, lane changes, overtaking, rickshaws/buses or advanced collision avoidance. There are not yet multiple selectable taxis or a full taxi-unlock career tree. The turn guidance is a route line/marker rather than full voice turn-by-turn navigation. Fare rate and the daily target are prototype game-balancing values, not real commercial rates.

The map is downloaded from an Overpass endpoint when the game starts, so first launch needs internet and a responsive public endpoint. Three.js is loaded from jsDelivr. OSM attribution is shown in-game and the data is available under the Open Database License (ODbL): https://www.openstreetmap.org/copyright.