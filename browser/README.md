# Sawari Browser Game — Karachi taxi-driving prototype

This is an independent Three.js browser build. It runs in Chrome without Unreal Engine.

## What this build contains
- Third-person 3D taxi driving; chase camera forward direction is derived from the taxi mesh orientation, with three camera distances (C), desktop WASD/arrow keys and mobile touch steering/pedals.
- Three selectable original game-made body silhouettes: Suzuki Mehran-style compact hatchback, Suzuki Alto-style tall hatchback and Toyota Corolla-style sedan. These are procedural meshes for gameplay, not licensed manufacturer CAD models.
- Saddar–Clifton roads and building ways from OpenStreetMap, shown in the correct geographic corridor.
- Procedural 3D roads with lane dashes and sidewalks; OSM building ways and stitched outer rings from building multipolygon relations are extruded into solid 3D walls/roofs, with procedural windows and rooftop tanks. Buildings without surveyed height tags use an estimated height.
- A few road-side streetlight fixtures and simple ambient traffic.
- A mini-map with the taxi, green pickup marker (P), yellow drop-off marker (D), current route and named OSM road guidance where a road name exists.
- Passenger pickup/drop-off, passenger NPC, 85% driver / 15% platform fare split, local wallet, daily target, vehicle upgrades and locally saved career.
- English / Urdu interface, engine sound toggle and responsive desktop/mobile layout.

## Map download behaviour
The first visit downloads the Saddar–Clifton OSM extract from a public Overpass endpoint. The successful extract is stored in the browser's IndexedDB cache for up to 90 days; normal reloads read the saved copy and do not request the full map again. This build increments the cache key to include building multipolygon relations, so the first launch after this update downloads the expanded extract once; later reloads use the cached version. Internet is required for the first download. In Settings → Karachi map data → Refresh map, the player can explicitly clear the cache and download a fresh copy. If the cache is cleared by the browser or the user opens another browser/device, that browser needs its own first download.

## Run locally
From the repository root:
```bash
python -m http.server 8000
```
Open http://localhost:8000/browser/ in Chrome. Do not open the HTML file directly with `file://`; the game uses JavaScript modules.

## Publish
The GitHub Pages workflow is manual-only to avoid unnecessary Actions runs. The repository owner can select GitHub Actions under Settings → Pages, then run **Deploy Sawari Browser Game** from the Actions tab. The root URL redirects into `/browser/`.

## Known gaps — this is not yet the finished simulator
- This is still a first playable vertical slice, not a photorealistic or fully verified production game.
- OSM provides geographic road and building data, but not complete photographed building facades, interiors, shop signs, local street clutter or every landmark. Buildings are procedural representations based on footprints and often estimated heights, not exact architectural replicas.
- The Mehran/Alto/Corolla choices are original game-made body approximations, not exact manufacturer CAD models. Exact vehicle assets with detailed interiors, rigged wheel meshes and realistic damage physics still need suitable licensed models and integration.
- Traffic is a basic moving-car layer without junction signals, lane-changing, overtaking, rickshaw/bus behaviours or advanced collision avoidance.
- Navigation uses the local road graph and a mini-map; it does not yet include full voice turn-by-turn navigation or a production routing engine.
- The initial OSM download can fail when public Overpass endpoints are busy or unavailable. A previously cached extract remains usable offline in the same browser.

OpenStreetMap attribution is displayed in-game. Map data is available under ODbL: https://www.openstreetmap.org/copyright.
