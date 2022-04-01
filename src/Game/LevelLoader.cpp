#include "LevelLoader.h"
#include <fstream>
#include "../Components/TransformComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/SpriteComponent.h"
#include "../Components/AnimationComponent.h"
#include "../Components/BoxColliderComponent.h"
#include "../Components/KeyboardControlledComponent.h"
#include "../Components/CameraFollowComponent.h"
#include "../Components/ProjectileEmitterComponent.h"
#include "../Components/HealthComponent.h"
#include "../Components/TextLabelComponent.h"
#include "../Components/LuaScriptComponent.h"
#include "./Game.h"
#include "../Logger/Logger.h"
#include <sol/sol.hpp>
#include <string>

LevelLoader::LevelLoader() {
    Logger::Log("Level Loader constructor called");
};

LevelLoader::~LevelLoader() {
    Logger::Log("Level Loader destructor called");
};

void LevelLoader::LoadLevel(sol::state& m_lua, const std::unique_ptr<Registry>& m_registry, const std::unique_ptr<AssetStore>& m_assetStore, SDL_Renderer* m_ptrRenderer, int level) {

    sol::load_result script = m_lua.load_file("assets/scripts/Level" + std::to_string(level) + ".lua");

    // Check the syntax but do not execute the script.
    // in case not valid
    if (!script.valid()) {
        sol::error err = script;
        std::string errorMessage = err.what();
        Logger::Error("Error loading Lua Script: " + errorMessage);
        return;
    }

    // Executes the script using the Sol state
    m_lua.script_file("assets/scripts/Level" + std::to_string(level) + ".lua");

    // Read the big table level
    sol::table tlevel = m_lua["Level"];

    //////////////////////////////////////////////////////////////////////////////////////////////
    // Read the level assets
    //////////////////////////////////////////////////////////////////////////////////////////////

    sol::table assets = tlevel["assets"];

    int i = 0;
    while (true) {
        // Tries to get the asset from asset table into the script
        sol::optional<sol::table> hasAsset = assets[i];

        // Leaves the while
        if(hasAsset == sol::nullopt) {
            break;
        }
        sol::table asset = assets[i];
        std::string assetType = asset["type"];
        std::string assetId = asset["id"];

        if (assetType == "texture") {
            m_assetStore->AddTexture(m_ptrRenderer, asset["id"], asset["file"]);
            Logger::Log("A new texture was added via level-loader and lua script, id: " + assetId);
        }

        if (assetType == "font") {
            m_assetStore->AddFont(asset["id"], asset["file"], asset["font_size"]);
            Logger::Log("A new font was added via level-loader and lua script, id: " + assetId);
        }
        i++;
    };

    //////////////////////////////////////////////////////////////////////////////////////////////
    // Read the level tilemap information
    //////////////////////////////////////////////////////////////////////////////////////////////
    sol::table map = tlevel["tilemap"];
    std::string mapFilePath = map["map_file"];
    std::string mapTextureAssetId = map["texture_asset_id"];
    int mapNumRows = map["num_rows"];
    int mapNumCols = map["num_cols"];
    int tileSize = map["tile_size"];
    double mapScale = map["scale"];

    // Opening the tilemap
    std::fstream mapFile;
    mapFile.open(mapFilePath);
    for (int y = 0; y < mapNumRows; y++) {
        for (int x = 0; x < mapNumCols; x++) {
            char ch;
            mapFile.get(ch);
            int srcRectY = std::atoi(&ch) * tileSize;
            mapFile.get(ch);
            int srcRectX = std::atoi(&ch) * tileSize;
            mapFile.ignore();

            Entity tile = m_registry->CreateEntity();
            tile.Group("tiles");
            tile.AddComponent<TransformComponent>(glm::vec2(x * (mapScale * tileSize), y * (mapScale * tileSize)), glm::vec2(mapScale, mapScale), 0.0);
            tile.AddComponent<SpriteComponent>(mapTextureAssetId, tileSize, tileSize, srcRectX, srcRectY, 0, false);
        }
    }
    mapFile.close();
    Game::m_mapWidth = mapNumCols * tileSize * mapScale;
    Game::m_mapHeight = mapNumRows * tileSize * mapScale;

    //////////////////////////////////////////////////////////////////////////////////////////////
    // Read the level entities and components
    //////////////////////////////////////////////////////////////////////////////////////////////
    sol::table entities = tlevel["entities"];
    i = 0;
    while (true) {

        sol::optional<sol::table> hasEntity = entities[i];
        if (hasEntity == sol::nullopt) {
            break;
        }

        sol::table entity = entities[i];

        Entity newEntity = m_registry->CreateEntity();

        // Tag
        sol::optional<std::string> tag = entity["tag"];
        if (tag != sol::nullopt) {
            newEntity.Tag(entity["tag"]);
        }

        // Group
        sol::optional<std::string> group = entity["group"];
        if (group != sol::nullopt) {
            newEntity.Group(entity["group"]);
        }

        // Components
        sol::optional<sol::table> hasComponents = entity["components"];
        // if has component in script
        if (hasComponents != sol::nullopt) {
            // Transform
            sol::optional<sol::table> transform = entity["components"]["transform"];
            if (transform != sol::nullopt) {
                newEntity.AddComponent<TransformComponent>(
                    glm::vec2(
                        entity["components"]["transform"]["position"]["x"],
                        entity["components"]["transform"]["position"]["y"]
                    ),
                    glm::vec2(
                        entity["components"]["transform"]["scale"]["x"].get_or(1.0),
                        entity["components"]["transform"]["scale"]["y"].get_or(1.0)
                    ),
                    entity["components"]["transform"]["rotation"].get_or(0.0)
                );
                /* ex:
                chopper.AddComponent<TransformComponent>(glm::vec2(500.0, 250.0), glm::vec2(2.0, 2.0), 0.0);
                */
            }

            // RigidBody
            sol::optional<sol::table> rigidbody = entity["components"]["rigidbody"];
            if (rigidbody != sol::nullopt) {
                newEntity.AddComponent<RigidBodyComponent>(
                    glm::vec2(
                        entity["components"]["rigidbody"]["velocity"]["x"].get_or(0.0),
                        entity["components"]["rigidbody"]["velocity"]["y"].get_or(0.0)
                    )
                );
                /* ex:
                chopper.AddComponent<RigidBodyComponent>(glm::vec2(0.0, 0.0));
                */
            }

            // Sprite
            sol::optional<sol::table> sprite = entity["components"]["sprite"];
            if (sprite != sol::nullopt) {
                newEntity.AddComponent<SpriteComponent>(
                    entity["components"]["sprite"]["texture_asset_id"],
                    entity["components"]["sprite"]["width"],
                    entity["components"]["sprite"]["height"],
                    entity["components"]["sprite"]["src_rect_x"].get_or(0),
                    entity["components"]["sprite"]["src_rect_y"].get_or(0),
                    entity["components"]["sprite"]["z_index"].get_or(1),
                    entity["components"]["sprite"]["fixed"].get_or(false)
                );
                /*
                chopper.AddComponent<SpriteComponent>("chopper-image", 32, 32, 0, 0, 2, false);
                */
            }

            // Animation
            sol::optional<sol::table> animation = entity["components"]["animation"];
            if (animation != sol::nullopt) {
                newEntity.AddComponent<AnimationComponent>(
                    entity["components"]["animation"]["num_frames"].get_or(1),
                    entity["components"]["animation"]["speed_rate"].get_or(1),
                    entity["components"]["animation"]["should_loop"].get_or(true)
                );
                /* ex:
                chopper.AddComponent<AnimationComponent>(2, 15, true);
                */
            }

            // BoxCollider
            sol::optional<sol::table> collider = entity["components"]["boxcollider"];
            if (collider != sol::nullopt) {
                newEntity.AddComponent<BoxColliderComponent>(
                    entity["components"]["boxcollider"]["width"],
                    entity["components"]["boxcollider"]["height"],
                    glm::vec2(
                        entity["components"]["boxcollider"]["offset"]["x"].get_or(0),
                        entity["components"]["boxcollider"]["offset"]["y"].get_or(0)
                    )
                );
                /* ex:
                chopper.AddComponent<BoxColliderComponent>(32, 32);
                */
            }
            
            // Health
            sol::optional<sol::table> health = entity["components"]["health"];
            if (health != sol::nullopt) {
                newEntity.AddComponent<HealthComponent>(
                    static_cast<int>(entity["components"]["health"]["health_percentage"].get_or(100))
                );
                /* ex:
                chopper.AddComponent<HealthComponent>(100);
                */
            }
            
            // ProjectileEmitter
            sol::optional<sol::table> projectileEmitter = entity["components"]["projectile_emitter"];
            if (projectileEmitter != sol::nullopt) {
                newEntity.AddComponent<ProjectileEmitterComponent>(
                    glm::vec2(
                        entity["components"]["projectile_emitter"]["projectile_velocity"]["x"],
                        entity["components"]["projectile_emitter"]["projectile_velocity"]["y"]
                    ),
                    static_cast<int>(entity["components"]["projectile_emitter"]["repeat_frequency"].get_or(1)) * 1000,
                    static_cast<int>(entity["components"]["projectile_emitter"]["projectile_duration"].get_or(10)) * 1000,
                    static_cast<int>(entity["components"]["projectile_emitter"]["hit_percentage_damage"].get_or(10)),
                    entity["components"]["projectile_emitter"]["friendly"].get_or(false)
                );
                /* ex:
                chopper.AddComponent<ProjectileEmitterComponent>(glm::vec2(500.0, 500.0), 300, 5000, 10, true);
                */
            }

            // CameraFollow
            sol::optional<sol::table> cameraFollow = entity["components"]["camera_follow"];
            if (cameraFollow != sol::nullopt) {
                newEntity.AddComponent<CameraFollowComponent>();
            }

            // KeyboardControlled
            sol::optional<sol::table> keyboardControlled = entity["components"]["keyboard_controller"];
            if (keyboardControlled != sol::nullopt) {
                newEntity.AddComponent<KeyboardControlledComponent>(
                    glm::vec2(
                        entity["components"]["keyboard_controller"]["up_velocity"]["x"],
                        entity["components"]["keyboard_controller"]["up_velocity"]["y"]
                    ),
                    glm::vec2(
                        entity["components"]["keyboard_controller"]["right_velocity"]["x"],
                        entity["components"]["keyboard_controller"]["right_velocity"]["y"]
                    ),
                    glm::vec2(
                        entity["components"]["keyboard_controller"]["down_velocity"]["x"],
                        entity["components"]["keyboard_controller"]["down_velocity"]["y"]
                    ),
                    glm::vec2(
                        entity["components"]["keyboard_controller"]["left_velocity"]["x"],
                        entity["components"]["keyboard_controller"]["left_velocity"]["y"]
                    )
                );
                /* ex:
                chopper.AddComponent<KeyboardControlledComponent>(glm::vec2(0.0, -120.0), glm::vec2(120.0, 00.0), glm::vec2(00.0, 120.0), glm::vec2(-120.0, 00.0));
                */
            }

            // LuaScriptComponent
            sol::optional<sol::table> script = entity["components"]["on_update_script"];
            if (script != sol::nullopt) {
                // Fetch the lua script function
                sol::function func = entity["components"]["on_update_script"][0];
                // Add the sol func to the component
                newEntity.AddComponent<LuaScriptComponent>(func);
            }
        }
        i++;
    }
    /*
    // Adding assets to the asset store
    m_assetStore->AddTexture(m_ptrRenderer, "tank-image", "./assets/images/tank-panther-right.png");
    m_assetStore->AddTexture(m_ptrRenderer, "chopper-image", "./assets/images/chopper-spritesheet.png");
    m_assetStore->AddTexture(m_ptrRenderer, "radar-image", "./assets/images/radar.png");
    m_assetStore->AddTexture(m_ptrRenderer, "truck-image", "./assets/images/truck-ford-right.png");
    m_assetStore->AddTexture(m_ptrRenderer, "tilemap-image", "./assets/tilemaps/jungle.png");
    m_assetStore->AddTexture(m_ptrRenderer, "bullet-image", "./assets/images/bullet.png");
    m_assetStore->AddTexture(m_ptrRenderer, "tree-image", "./assets/images/tree.png");

    //Adding fonts
    m_assetStore->AddFont("charriot-font", "./assets/fonts/charriot.ttf", 20);
    m_assetStore->AddFont("pico8-font-5", "./assets/fonts/pico8.ttf", 5);
    m_assetStore->AddFont("pico8-font-10", "./assets/fonts/pico8.ttf", 10);
   

    // Load the tilemap
    int tileSize = 32;
    double tileScale = 4;
    int mapNumCols = 25;
    int mapNumRows = 20;

    std::fstream mapFile;
    mapFile.open("./assets/tilemaps/jungle.map");

    for (int y = 0; y < mapNumRows; y++) {
        for (int x = 0; x < mapNumCols; x++) {
            char ch;
            mapFile.get(ch);
            int srcRectY = std::atoi(&ch) * tileSize;
            mapFile.get(ch);
            int srcRectX = std::atoi(&ch) * tileSize;
            mapFile.ignore();

            Entity tile = m_registry->CreateEntity();
            tile.Group("tiles");
            tile.AddComponent<TransformComponent>(glm::vec2(x * (tileScale * tileSize), y * (tileScale * tileSize)), glm::vec2(tileScale, tileScale), 0.0);
            tile.AddComponent<SpriteComponent>("tilemap-image", tileSize, tileSize, srcRectX, srcRectY, 0, false);
        }
    }
    mapFile.close();
    Game::m_mapWidth = mapNumCols * tileSize * tileScale;
    Game::m_mapHeight = mapNumRows * tileSize * tileScale;

    // We need to load the tilemap from ./assets/tilemaps/jungle.png
    // We need to load the file ./assets/tilemaps/jungle.map

    // Create some entities
    Entity chopper = m_registry->CreateEntity();
    chopper.Tag("player");
    chopper.AddComponent<TransformComponent>(glm::vec2(500.0, 250.0), glm::vec2(2.0, 2.0), 0.0);
    chopper.AddComponent<RigidBodyComponent>(glm::vec2(0.0, 0.0));
    chopper.AddComponent<SpriteComponent>("chopper-image", 32, 32, 0, 0, 2, false);
    chopper.AddComponent<AnimationComponent>(2, 15, true);
    chopper.AddComponent<KeyboardControlledComponent>(glm::vec2(0.0, -120.0), glm::vec2(120.0, 00.0), glm::vec2(00.0, 120.0), glm::vec2(-120.0, 00.0));
    chopper.AddComponent<CameraFollowComponent>();
    chopper.AddComponent<BoxColliderComponent>(32, 32);
    chopper.AddComponent<ProjectileEmitterComponent>(glm::vec2(500.0, 500.0), 300, 5000, 10, true);
    chopper.AddComponent<HealthComponent>(100);

    Entity radar = m_registry->CreateEntity();
    radar.AddComponent<TransformComponent>(glm::vec2(Game::m_windowWidth - 75, 10.0), glm::vec2(1.0, 1.0), 0.0);
    radar.AddComponent<RigidBodyComponent>(glm::vec2(0.0, 0.0));
    radar.AddComponent<SpriteComponent>("radar-image", 64, 64, 0, 0, 3, true);
    radar.AddComponent<AnimationComponent>(8, 6, true);


    Entity tank = m_registry->CreateEntity();
    tank.Group("enemies");
    tank.AddComponent<TransformComponent>(glm::vec2(1550.0, 332.0), glm::vec2(1.0, 1.0), 0.0);
    tank.AddComponent<RigidBodyComponent>(glm::vec2(20.0, 0.0));
    tank.AddComponent<SpriteComponent>("tank-image", 32, 32, 0, 0, 2);
    tank.AddComponent<BoxColliderComponent>(32, 32);
    // tank.AddComponent<ProjectileEmitterComponent>(glm::vec2(300.0, 0.0), 1000, 5000, 40, false);
    tank.AddComponent<HealthComponent>(100);

    Entity truck = m_registry->CreateEntity();
    truck.Group("enemies");
    truck.AddComponent<TransformComponent>(glm::vec2(1600.0, 835.0), glm::vec2(1.0, 1.0), 0.0);
    truck.AddComponent<RigidBodyComponent>(glm::vec2(0.0, 0.0));
    truck.AddComponent<SpriteComponent>("truck-image", 32, 32, 0, 0, 1);
    truck.AddComponent<BoxColliderComponent>(32, 32);
    truck.AddComponent<ProjectileEmitterComponent>(glm::vec2(300.0, 0.0), 1000, 5000, 20, false);
    truck.AddComponent<HealthComponent>(100);

    Entity treeA = m_registry->CreateEntity();
    treeA.Group("obstacles");
    treeA.AddComponent<TransformComponent>(glm::vec2(1700.0, 332.0), glm::vec2(1.0, 1.0), 0.0);
    treeA.AddComponent<BoxColliderComponent>(16, 32);
    treeA.AddComponent<SpriteComponent>("tree-image", 16, 32, 0, 0, 2);

    Entity treeB = m_registry->CreateEntity();
    treeB.Group("obstacles");
    treeB.AddComponent<TransformComponent>(glm::vec2(1500.0, 332.0), glm::vec2(1.0, 1.0), 0.0);
    treeB.AddComponent<BoxColliderComponent>(16, 32);
    treeB.AddComponent<SpriteComponent>("tree-image", 16, 32, 0, 0, 2);

    Entity label = m_registry->CreateEntity();
    SDL_Color green = {0, 255, 0};
    label.AddComponent<TextLabelComponent>(glm::vec2(Game::m_windowWidth / 2 - 40, 10), "CHOPPER 1.0", "charriot-font", green, true);
    */
}