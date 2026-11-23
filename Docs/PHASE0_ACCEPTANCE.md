# Sawari — Phase 0 acceptance checklist

Phase 0 proves the risky foundation before the scope expands: a Karachi OSM-to-Unreal pipeline, one taxi, a basic ride loop and offline routing groundwork.

## Included in this repository

- Unreal Engine 5.8 C++ project shell.
- Chaos Vehicles taxi pawn base with throttle, brake, steering, reverse, handbrake and three camera modes.
- Blueprint-callable ride state machine: request → accept → pickup → drop-off.
- Economy-class sample rates from the Sawari plan and 85/15 driver/platform split.
- Saddar OSM fetcher with separate roads/buildings/POIs GeoJSON layers.
- Blender LoD1 massing exporter for roads, building footprints and temporary POI validation markers.

## Not yet included / must be completed

- Vehicle Skeletal Mesh, Physics Asset, wheel Blueprints, torque curve and wheel-bone setup.
- Blueprint child BP_SawariTaxi based on ASawariTaxiPawn.
- Saved playable Unreal level with imported Saddar FBX and collision settings.
- HUD widgets, passenger NPC character/animation, pedestrian and traffic AI.
- Road graph compilation and offline turn-by-turn navigation.
- Signature landmark models and final facade materials.
- Runtime QA and packaged executable.

## Gate tests

1. Project compiles in Unreal 5.8 with Chaos Vehicles enabled.
2. A configured taxi Blueprint drives, brakes, reverses, steers and changes Chase/Bonnet/Cockpit cameras.
3. Saddar OSM extract produces non-empty roads and building layers.
4. Blender export opens in Unreal at correct scale/orientation; roads have collision and buildings do not block drivable lanes.
5. One complete ride can be requested, accepted, picked up and completed; 85% of final fare is added to driver wallet and 15% is platform fee.
6. The packaged single-player prototype can complete a ride with internet disconnected after city data is imported.

Do not declare Phase 0 passed merely because the map or car appears in the editor. All six gates must be tested.
