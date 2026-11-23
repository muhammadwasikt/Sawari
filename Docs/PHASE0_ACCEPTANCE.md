# Sawari — Phase 0 acceptance checklist

Phase 0 proves the risky foundation before the scope expands: a Karachi OSM-to-Unreal pipeline, a controllable taxi, a ride HUD, a visible passenger and a complete single-player ride loop.

## Implemented in source (not yet UE-compiled)

- Unreal Engine 5.8 C++ project and project descriptor.
- Physics-driven fallback taxi pawn with visible modular car body and wheel components.
- Acceleration, brake/reverse, steering, handbrake, speed cap and Chase / Bonnet / Cockpit camera switching.
- Procedural 3D boulevard/city blockout with road collision, sidewalks, buildings, road markings, sun, sky atmosphere and haze.
- Visible low-poly passenger character at the pickup.
- Native HUD for speed, wallet, state, fare and checkpoint distance.
- Ride request, accept, pickup, drop-off, fare calculation and 85/15 driver/platform split.
- Python editor script to create and save the playable prototype map.
- Saddar OSM fetcher and Blender LoD1 blockout exporter remain available for the real-map pipeline.

## Still required

- Compile the Unreal 5.8 Editor target and resolve any engine-version/compiler issues.
- Tune the fallback taxi physics in Play-in-Editor; the fallback is arcade physics, not a four-wheel Chaos suspension simulation.
- Configure the production Chaos vehicle with a real taxi Skeletal Mesh, Physics Asset, four wheel setups and torque curve.
- Import and validate the actual Saddar OSM export; replace the temporary city blockout with OSM-derived geometry and hand-built major landmarks.
- Add a road graph, turn-by-turn offline navigation and route recalculation.
- Add moving traffic, pedestrians, passenger animation, day/night/weather, fuel and vehicle damage.
- Package and test Windows first; assess mobile performance separately.

## Controls and ride test

1. W accelerates; S brakes while moving forward and reverses when nearly stopped.
2. A/D steer; Space applies handbrake; V cycles the three camera views.
3. E accepts the sample ride; Q declines; R creates the next request after a ride is complete or declined.
4. Drive near the yellow Saddar pickup marker; the passenger boards automatically.
5. Drive to the green Clifton marker; the passenger is dropped off automatically.

## Phase 0 gate tests

1. Project compiles in Unreal 5.8 with the declared plugins enabled.
2. Play-in-Editor opens L_SawariPrototype and automatically spawns the city, taxi, HUD and passenger.
3. Taxi can accelerate, brake, reverse, steer, handbrake and switch all three camera views without falling through the road.
4. A request can be accepted, pickup triggered by proximity, drop-off triggered by proximity, and wallet updated.
5. For the sample no-surge fare (181 Coins), driver payout is 153 Coins and platform fee is 28 Coins.
6. Saddar OSM extract produces non-empty roads and building layers; Blender export opens in Unreal with correct scale, orientation and collision.
7. Packaged single-player prototype can complete a ride with internet disconnected after city data is imported.

Do not declare Phase 0 passed merely because the project files exist. All gates must be tested in Unreal and a packaged build.
