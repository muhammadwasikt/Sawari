# SAWARI — Real City Taxi Simulator

**Phase 0 playable prototype foundation · Unreal Engine 5.8 · Karachi, Pakistan**

Sawari follows the supplied Real City Taxi Simulator plan. This repository now contains a self-contained first-playable prototype foundation: a physics-driven taxi pawn, a procedural Karachi-style boulevard blockout, a visible passenger character, a ride HUD, and pickup/drop-off checkpoints that update the in-game wallet.

## Honest build status

The source has been committed to GitHub, but it has **not yet been compiled or launched in Unreal Editor** in this environment. Treat this as implementation source awaiting an Unreal 5.8 compile and hands-on physics tuning, not as a verified packaged game.

The current taxi is a self-contained arcade-physics fallback using a physics box chassis and modular visual body/wheel pieces. It can be controlled without a third-party vehicle mesh, but it is not yet a full Chaos four-wheel suspension simulation. The existing Chaos Vehicles base class remains available for a later configured skeletal-mesh taxi.

The current procedural city is a first visual/gameplay blockout, not the final imported real Karachi OSM map. Its buildings are blockout geometry and its main boulevard is laid out to test the demo ride distances. The Saddar OSM extraction and Blender city-massing pipeline remain separate development-time tools.

## What's implemented in source

- Unreal Engine 5.8 C++ project and game module.
- Taxi pawn with physics-driven acceleration, braking/reverse, steering, lateral grip, handbrake, speed limiting and three camera views.
- Native HUD with speed, wallet, ride state, fare and next-checkpoint distance.
- Procedural 3D blockout: ground, long boulevard, side roads, lane markings, sidewalks, low-rise city blocks, daylight sky, sun and atmospheric haze.
- Visible passenger built from human-shaped 3D components.
- Ride flow: request → accept → drive to passenger → automatic pickup → drive to destination → automatic drop-off.
- Fare calculation from the project's Economy rates, with 85% driver payout and 15% platform fee.
- Saddar OSM extraction, Blender massing export and Python converter test.

## Create the Unreal map once

Requirements:
- Unreal Engine 5.8
- Visual Studio 2022 with Desktop development with C++ and Game development with C++
- PythonScriptPlugin and EditorScriptingUtilities enabled (both are enabled in the project descriptor)

1. Open Sawari/Sawari.uproject in Unreal Engine 5.8.
2. Generate project files if prompted, then build the SawariEditor target in Visual Studio.
3. In Unreal Editor, open the Python console through Window → Developer Tools → Output Log. In the command line at the bottom, run the script using its absolute path:

       py "C:/FULL/PATH/TO/Sawari/Scripts/create_sawari_level.py"

4. The script creates and saves /Game/Maps/L_SawariPrototype, assigns SawariGameMode, and updates the project default map settings. Restart the editor once after the first run.

If the Python command is unavailable, verify that PythonScriptPlugin and EditorScriptingUtilities are enabled in the Plugins panel, then restart Unreal Editor.

## Play the prototype

Open L_SawariPrototype and press Play. GameMode spawns the taxi, city and passenger automatically. A sample Ahmed K. ride request is ready.

| Control | Action |
|---|---|
| W | Accelerate |
| S | Brake while moving forward; reverse when nearly stopped |
| A / D | Steer |
| Space | Handbrake |
| V | Cycle chase / bonnet / cockpit camera |
| E | Accept current ride |
| Q | Decline current request |
| R | Generate the next ride after completing or declining |

Ride flow:
1. Press E to accept the sample request.
2. Drive to the yellow pickup beacon beside the visible passenger in the Saddar blockout.
3. When the taxi is close enough, pickup happens automatically and the passenger boards.
4. Drive along the boulevard to the green Clifton destination beacon.
5. Drop-off happens automatically, fare is calculated and the driver's share is added to the wallet.

The sample ride uses the plan's 1.2 km pickup leg, 5 km passenger trip, and Economy fare values. The current prototype uses a straight boulevard and distance-to-checkpoint HUD guidance; turn-by-turn road routing is not integrated yet.

## Real Saddar OSM data pipeline

From the repository root:

    python Scripts/fetch_saddar_osm.py

This downloads a small starter test bounding box around Saddar, not the final playable district boundary. It writes roads.geojson, buildings.geojson, pois.geojson and manifest.json under SourceData/Saddar. Network access is required only for this development-time extraction. The final single-player game should use bundled city data and must not call Overpass while playing.

Then run Blender:

    blender --background --python Blender/build_saddar_city.py -- --input SourceData/Saddar --output Exports/Saddar_Blockout.fbx

Import the FBX into Unreal and validate scale, orientation, origin and collision. This is a first low-detail massing export, not photogrammetry or a photorealistic copy. Building heights without OSM height/levels tags are estimates. Major landmarks and facades require a separate manual art pass.

## Tests and acceptance

Run the Python converter test from the repository root:

    python -m unittest discover -s tests -v

This tests OSM conversion only. It does not compile Unreal C++ or certify driving physics. Phase 0 is not complete until the project compiles in UE 5.8, taxi handling is tuned in play, the OSM FBX imports at correct scale/collision, and the entire ride can be completed.

## OpenStreetMap attribution

Map data and derived map content must carry the appropriate attribution and comply with the ODbL terms for the specific distribution method.

© OpenStreetMap contributors — https://www.openstreetmap.org/copyright

Keep OSM source and license notes with any distributed OSM-derived database/content. These notes are not legal advice.
