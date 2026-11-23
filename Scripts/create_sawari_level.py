"""Create and save the first playable Sawari Unreal map.

Run ONLY inside Unreal Editor after the Sawari C++ module has compiled.
Use Tools > Execute Python Script, or Unreal's Output Log Python console:
    py "C:/actual/path/to/Sawari/Scripts/create_sawari_level.py"

This is not standalone Python: the 'unreal' module is provided by the
running Unreal Editor and is unavailable in normal Python environments.

The GameMode creates the city, taxi, passenger and HUD at runtime. This script
creates the persistent map asset and assigns the C++ GameMode.
"""
from pathlib import Path
import re
import unreal

MAP_PACKAGE = "/Game/Maps/L_SawariPrototype"
MAP_OBJECT = MAP_PACKAGE + ".L_SawariPrototype"
GAME_MODE_PATH = "/Script/Sawari.SawariGameMode"


def main():
    if not unreal.EditorAssetLibrary.does_asset_exist(MAP_OBJECT):
        if not unreal.EditorLevelLibrary.new_level(MAP_PACKAGE):
            raise RuntimeError("Unreal could not create " + MAP_PACKAGE)
    else:
        if not unreal.EditorLevelLibrary.load_level(MAP_PACKAGE):
            raise RuntimeError("Unreal could not load " + MAP_PACKAGE)

    world = unreal.EditorLevelLibrary.get_editor_world()
    if not world:
        raise RuntimeError("No editor world is active")

    world_settings = world.get_world_settings()
    game_mode_class = unreal.load_class(None, GAME_MODE_PATH)
    if not game_mode_class:
        raise RuntimeError("SawariGameMode was not loaded. Compile the Sawari C++ module first.")
    world_settings.set_editor_property("default_game_mode", game_mode_class)

    # Add a PlayerStart for editor friendliness; the GameMode spawns and
    # possesses its taxi at the known boulevard position itself.
    player_starts = unreal.EditorLevelLibrary.get_all_level_actors()
    if not any(isinstance(actor, unreal.PlayerStart) for actor in player_starts):
        unreal.EditorLevelLibrary.spawn_actor_from_class(
            unreal.PlayerStart, unreal.Vector(-180000.0, 0.0, 150.0), unreal.Rotator(0.0, 0.0, 0.0)
        )

    unreal.EditorLevelLibrary.save_current_level()

    project_dir = Path(unreal.Paths.project_dir())
    config_path = project_dir / "Config" / "DefaultEngine.ini"
    config = config_path.read_text(encoding="utf-8") if config_path.exists() else ""
    section = "[/Script/EngineSettings.GameMapsSettings]"
    if section not in config:
        config = section + "\n" + config
    if re.search(r"^GameDefaultMap=.*$", config, flags=re.MULTILINE):
        config = re.sub(r"^GameDefaultMap=.*$", "GameDefaultMap=" + MAP_PACKAGE, config, flags=re.MULTILINE)
    else:
        config = config.replace(section, section + "\nGameDefaultMap=" + MAP_PACKAGE, 1)
    if re.search(r"^EditorStartupMap=.*$", config, flags=re.MULTILINE):
        config = re.sub(r"^EditorStartupMap=.*$", "EditorStartupMap=" + MAP_PACKAGE, config, flags=re.MULTILINE)
    else:
        config = config.replace(section, section + "\nEditorStartupMap=" + MAP_PACKAGE, 1)
    config_path.write_text(config, encoding="utf-8")

    unreal.log("Sawari map saved: " + MAP_PACKAGE)
    unreal.log("Restart Unreal Editor once so the new Editor Startup Map setting takes effect.")


if __name__ == "__main__":
    main()
