# SAWARI — Real City Taxi Simulator

**Phase 0 engineering foundation · Unreal Engine 5.8 · Karachi, Pakistan**

Sawari is a real-city taxi / ride-hailing simulator based on the supplied game development plan. The first milestone is deliberately small: validate a Saddar Karachi OpenStreetMap (OSM) data pipeline, establish the Chaos Vehicles taxi pawn, and implement a complete single-player passenger ride state flow.

## Current repository status

This repository contains source scaffolding and build-time tooling, not a compiled or playable game build. Unreal C++ compilation, Editor launch, vehicle-asset setup and end-to-end gameplay testing have not yet been completed.

Included:
- Unreal Engine 5.8 C++ project skeleton and project descriptor
- Chaos Vehicles taxi pawn base with keyboard input bindings and Chase / Bonnet / Cockpit camera switching
- Single-player ride subsystem: request → accept → pickup → drop-off, fare calculation and 85% driver / 15% platform split
- Saddar OSM extractor producing roads, building footprints and POI GeoJSON layers
- Blender LoD1 city-massing exporter for roads, buildings and validation POI markers
- Automated test for OSM JSON conversion

Not included yet:
- Configured taxi Skeletal Mesh, Physics Asset, Chaos wheel assets or torque curve
- Playable Unreal map, finished HUD/navigation widgets, pedestrian/traffic AI or detailed landmark art
- Bundled processed Karachi city assets, packaged Windows/Android builds or multiplayer services

## Project structure

- Sawari/ — Unreal project, configuration and C++ gameplay foundation
- Scripts/ — development-time OSM download and GeoJSON conversion
- Blender/ — first 3D city blockout exporter
- Docs/ — Phase 0 acceptance criteria
- tests/ — OSM conversion test and fixture

## Requirements

- Unreal Engine 5.8
- Visual Studio 2022 with Desktop development with C++ and Game development with C++
- Blender 4.x
- Python 3.10+
- Unreal Chaos Vehicles plugin enabled
- Cesium for Unreal 2.31.0 for UE 5.8 is an optional city-streaming integration; install its matching package before enabling it

## Build the Saddar data blockout

From the repository root:

    python Scripts/fetch_saddar_osm.py

The script downloads a small starter test bounding box around Saddar, not the final playable district boundary. It writes roads.geojson, buildings.geojson, pois.geojson and manifest.json under SourceData/Saddar. Network access is required only for this development-time extraction. The final single-player game should use bundled city data and must not call Overpass while playing.

Then run Blender:

    blender --background --python Blender/build_saddar_city.py -- --input SourceData/Saddar --output Exports/Saddar_Blockout.fbx

Import the FBX into Unreal and validate scale, orientation, origin and collision. This creates a first low-detail massing blockout, not photogrammetry or a photorealistic copy. Building heights without OSM height/levels tags are estimates. Major landmarks and facades require a separate manual art pass.

## Configure the taxi pawn

Open Sawari/Sawari.uproject with Unreal Engine 5.8 and generate IDE project files if prompted. Build the SawariEditor target in Visual Studio.

Before the taxi can drive, create a Blueprint child of ASawariTaxiPawn and assign:
1. A vehicle Skeletal Mesh and matching Physics Asset.
2. Chaos wheel Blueprint classes with the correct wheel-bone names and front/rear setup.
3. A torque curve and vehicle movement tuning.
4. A playable map, Player Start and GameMode default pawn.
5. A basic HUD / Widget Blueprint that calls the ride subsystem functions.

Keyboard mappings:
| Action | Key |
|---|---|
| Accelerate | W |
| Brake | Space |
| Steering | A / D |
| Reverse gear | Hold S |
| Handbrake | Left Shift |
| Camera cycle | V |

The taxi mesh, wheel assets and driving tuning are not fabricated as placeholders; they must be configured as real assets before a playable vehicle test.

## Ride subsystem

Retrieve USawariRideSubsystem from the Game Instance and call:
1. GenerateDemoRide
2. AcceptRide
3. MarkPassengerPickedUp
4. CompleteRide

The sample request uses the plan's example passenger Ahmed K. (4.8), Saddar pickup and Clifton destination. Economy-class values use the supplied plan's base fare 70 Coins, 15 Coins/km and 2 Coins/minute. The driver receives 85% and the in-game platform retains 15%.

## Tests

From the repository root:

    python -m unittest discover -s tests -v

This validates only the OSM converter. It does not compile Unreal C++ or certify the game as playable.

## OpenStreetMap attribution

Map data and derived map content must carry the appropriate attribution and comply with the ODbL terms for the specific distribution method.

© OpenStreetMap contributors — https://www.openstreetmap.org/copyright

Keep the OSM source extract, attribution and relevant license notes with distributed map content. These notes are not legal advice.
