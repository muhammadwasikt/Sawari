# Sawari Browser Game — first playable milestone

This is an independent browser-native Three.js version of Sawari. It does not require Unreal Engine or a local game-engine install.

## Implemented in this milestone
- Third-person yellow taxi with throttle, brake, steering, chase camera and basic driving handling.
- Desktop keyboard controls (WASD / arrow keys) and mobile touch steering wheel, accelerator and brake pedals.
- Saddar–Clifton prototype map loaded at runtime from OpenStreetMap Overpass road ways and building footprints; the game shows a map error rather than substituting a fabricated Karachi map if the data service is unavailable.
- Procedural 3D road surfaces, extruded building footprints and lightweight ambient moving traffic.
- Road-graph shortest-path route line, passenger pickup/drop-off interaction and markers.
- In-game fare calculation with 85% driver / 15% platform split.
- Wallet, daily earnings target, engine/handling/brake upgrades and locally saved career progress.
- English/Urdu UI switch and synthesized engine sound with a sound toggle.
- Responsive desktop/mobile layout and pause-on-tab-hide behavior.

## Run locally
Serve the repository over HTTP; ES modules do not reliably run from a `file://` URL:
```bash
python -m http.server 8000
```
Then open http://localhost:8000/browser/ in a browser.

## Hosting note
The Sawari repository is currently private. GitHub Free provides GitHub Pages for public repositories; private-repository Pages requires a paid plan. This change deliberately does not publish the private source or trigger a Pages workflow. A hosted public link therefore still needs an explicitly chosen hosting route. See GitHub's current plan documentation: https://docs.github.com/en/get-started/learning-about-github/githubs-plans

## Current limitations
This is a first playable vertical slice, not the finished city simulator. OSM ways are used for roads/building footprints; relation-only buildings may be absent. Buildings are massing models rather than detailed facades or landmarks. Traffic is a basic ambient road-following layer and does not yet have junction signals, lane changes, overtaking, rickshaws/buses or advanced collision avoidance. Multiple selectable taxis and the full vehicle-unlock career tree are not implemented yet. Route guidance is a road line/marker, not voice turn-by-turn navigation. Fare rate and daily target are prototype balancing values, not real commercial rates.

The map is downloaded from a public Overpass endpoint when the game starts, so first launch needs internet and a responsive endpoint. Three.js is loaded from jsDelivr. OpenStreetMap attribution is shown in-game; map data is available under the Open Database License (ODbL): https://www.openstreetmap.org/copyright.
