# SAWARI — Real City Taxi Simulator

**Phase 0 / next-step engineering build · Unreal Engine 5.8 · Karachi, Pakistan**

Sawari is a single-player real-city taxi / ride-hailing simulator based on the supplied development plan. The first real-city prototype corridor runs from Saddar toward Clifton. The project uses bundled OpenStreetMap data so the single-player game does not need to contact a map server while playing.

## Important build status

The latest source changes are committed to GitHub, but this environment cannot launch Unreal Editor or compile the C++ project. The actual OSM extract has also **not yet been downloaded** into this repository. Treat this as implementation source and a local build pipeline awaiting UE 5.8 compilation, data staging, vehicle asset import and hands-on testing—not a verified playable binary.

The game selects the most complete local setup available:
- If OSM GeoJSON is staged, GameMode loads the real corridor roads/building footprints and uses their drivable road polylines for AI traffic. If the files are missing, the earlier procedural city blockout remains as a fallback.
- If the rigged taxi Skeletal Mesh and generated Physics Asset are imported, GameMode selects the Chaos four-wheel taxi. Otherwise, the self-contained arcade-physics taxi remains the fallback.
- The Chaos taxi still needs its Wheel Controller Animation Blueprint created in Unreal Editor so the four skeletal wheel bones visually follow the physics wheels.

## What this iteration adds in source

- Unreal Engine 5.8 Chaos taxi base with four wheel classes, wheel setup names, engine torque curve, automatic transmission, braking, reverse, steering, handbrake and three camera views.
- Blender generator for a taxi Skeletal Mesh with the exact wheel bone names required by the Chaos setup.
- Unreal Python import helper for the taxi FBX and generated Physics Asset validation.
- Runtime OSM actor that reads the staged manifest and GeoJSON, projects geographic coordinates into Unreal units, generates road and building meshes, creates road collision, and snaps taxi / ride checkpoints toward nearby road segments.
- Basic AI traffic vehicles that travel along extracted primary, secondary, tertiary and residential road polylines, with lane offsets, collision sweeps and slowdown when blocked.
- A fallback procedural city and arcade taxi so development can continue before the real data and vehicle assets are ready.
- HUD speed and checkpoint-distance values are now read from the active GameMode, so they follow the real city coordinates instead of hard-coded blockout coordinates.

## Requirements

- Unreal Engine 5.8
- Visual Studio 2022 with Desktop development with C++ and Game development with C++
- Blender 4.x
- Python 3.10+
- Unreal plugins: Chaos Vehicles, Procedural Mesh Component, Python Editor Script Plugin and Editor Scripting Utilities
- No paid map API key is required for the OSM prototype pipeline

The project descriptor enables the required plugins. Restart Unreal Editor after enabling plugins or generating project files.

## 1. Download and stage real Saddar–Clifton OSM data

From the repository root, run:

    python Scripts/fetch_saddar_osm.py

The script uses Overpass to extract roads, building footprints and amenity/shop/tourism/transport point features from the prototype corridor bounding box:

    South 24.785, West 66.985, North 24.890, East 67.095

It writes source files under SourceData/Saddar and copies the runtime data to:

    Sawari/Content/SawariData/Saddar/

Expected files:
- roads.geojson
- buildings.geojson
- pois.geojson
- manifest.json

The manifest includes the projection origin, bounding box, extraction time, counts and OSM attribution. This is a prototype corridor, **not all of Karachi**. The script makes network requests only during development-time extraction; the game loads the bundled files offline at runtime.

If Overpass is temporarily rate-limited, rerun the script later. Do not replace a failed or empty extract with fabricated map geometry and call it real city data.

## 2. Generate and import the rigged taxi

Create the Blender FBX:

    blender --background --python Blender/create_sawari_taxi.py

This produces:

    Exports/SK_SawariTaxi.fbx

Open the project in Unreal Engine 5.8. Build the SawariEditor target, then run the Unreal Python script from the Editor Output Log console:

    py "C:/FULL/PATH/TO/Sawari/Scripts/import_sawari_taxi.py"

The import helper places the Skeletal Mesh at:

    /Game/Vehicles/Taxi/SK_SawariTaxi

It requests Physics Asset creation and checks for these bones:
- Chassis
- Wheel_Front_Left
- Wheel_Front_Right
- Wheel_Rear_Left
- Wheel_Rear_Right

Open the generated Physics Asset and verify that the chassis has a sensible collision body covering the car body. Wheel bodies should not collide with the road; Chaos wheel suspension traces control ground contact. If the importer did not generate a usable Physics Asset, create/fix it in the Physics Asset Editor before testing.

## 3. Create the Chaos wheel animation blueprint

The Skeletal Mesh and Physics Asset are not enough to animate the visible wheel bones. Epic's Chaos vehicle setup also uses a Vehicle Animation Blueprint based on VehicleAnimationInstance and Wheel Controller nodes.

In Unreal Editor:
1. Create an Animation Blueprint for the taxi skeleton.
2. Set its parent class to VehicleAnimationInstance.
3. In the AnimGraph, connect Reference Pose through Wheel Controller nodes for all four wheel bones, then into Output Pose.
4. Set each node's bone to the exact imported name: Wheel_Front_Left, Wheel_Front_Right, Wheel_Rear_Left and Wheel_Rear_Right.
5. Save it as:

       /Game/Vehicles/Taxi/ABP_SawariTaxi

The C++ taxi pawn automatically assigns this Animation Blueprint when it exists. Without it, the physics movement may be configured but the skeletal wheels will not receive the intended Wheel Controller animation.

## 4. Open the playable prototype

Open:

    Sawari/Sawari.uproject

Build the SawariEditor target in Visual Studio 2022. In Unreal Editor, run the level creation helper once from the Python console:

    py "C:/FULL/PATH/TO/Sawari/Scripts/create_sawari_level.py"

The script creates and saves /Game/Maps/L_SawariPrototype and assigns SawariGameMode. Restart the editor once after the map has been created, open L_SawariPrototype and press Play.

At runtime:
- With the staged OSM data, the real Saddar–Clifton road/building meshes, pickup/drop-off markers and road-based traffic load.
- With the imported taxi Skeletal Mesh and Physics Asset, the Chaos four-wheel taxi is selected.
- If either asset/data stage is missing, the corresponding prototype fallback is used and the Output Log states which path was selected.

## Controls

| Key | Action |
|---|---|
| W | Accelerate |
| S | Reverse; acts as brake while the taxi is still moving forward |
| A / D | Steering |
| Left Ctrl | Service brake |
| Space | Handbrake |
| V | Cycle chase / bonnet / cockpit camera |
| E | Accept passenger request |
| Q | Decline passenger request |
| R | Generate next ride after completion or decline |

Ride loop:
1. Press E to accept the sample request.
2. Drive to the yellow pickup beacon and the visible passenger.
3. Pickup triggers automatically when the taxi is close enough.
4. Drive to the green Clifton drop-off beacon.
5. Drop-off triggers automatically and the Economy fare is distributed 85% to the driver and 15% to the platform.

The HUD shows active vehicle speed, wallet balance, ride state and distance to the current checkpoint. The current navigation indicator is straight-line distance; turn-by-turn route planning and route recalculation are not yet implemented.

## AI traffic — current scope

When real OSM data is loaded, the traffic manager selects extracted drivable road polylines and spawns a capped set of moving NPC cars. Cars follow extracted two-way road polylines with left-hand lane offsets, sweep against collisions and slow down when blocked. One-way-tagged roads are excluded from this first bidirectional traffic follower so NPCs are not sent against one-way direction. At the end of an individual OSM way, the NPC restarts at that way's beginning; connected junction routing is still future work.

This is the first traffic simulation layer—not a finished Karachi traffic model. Junction priority, lane-changing, overtaking, traffic lights, bus/rickshaw behaviour and a fully connected road-graph routing system remain future tasks.

## Offline packaging and OSM attribution

The project stages Content/SawariData as Unreal's Additional UFS directory so the bundled GeoJSON/manifest can be read by the Unreal file system in packaged builds. Do not add runtime Overpass calls.

OSM data is available under the Open Database License (ODbL). Preserve this attribution in the game credits/loading/settings screens and with any redistributed OSM-derived data:

**© OpenStreetMap contributors** — https://www.openstreetmap.org/copyright

## Validation checklist

Run the existing Python converter tests:

    python -m unittest discover -s tests -v

Then in Unreal 5.8 verify:
1. SawariEditor compiles successfully with the enabled plugins.
2. The OSM manifest and road/building feature files load and produce non-empty mesh sections.
3. The imported road mesh is aligned with the pickup/drop-off points; taxi checkpoints snap to a nearby drivable road.
4. The Chaos taxi reports a valid Skeletal Mesh, Physics Asset and four wheel setups; all four Wheel Controller nodes animate their wheel bones.
5. The taxi accelerates, brakes, reverses, steers, handbrakes and changes all three cameras.
6. NPC traffic spawns only on valid OSM route polylines and moves without becoming stuck permanently or blocking the player spawn.
7. A complete pickup-to-drop-off ride succeeds and the driver/platform wallet split is correct.
8. A packaged build can run the single-player corridor without an internet connection after OSM data has been staged.

Do not mark the milestone complete until these runtime gates have been tested in Unreal. This repository currently contains source and build tooling, not a compiled or QA-certified game build.
