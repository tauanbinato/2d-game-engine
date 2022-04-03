#include "./AssetStore.h"
#include "../Logger/Logger.h"
#include <SDL2/SDL_image.h>

AssetStore::AssetStore() {
    Logger::Log("AssetStore constructor called");
}

AssetStore::~AssetStore(){
    ClearAssets();
    Logger::Log("AssetStore destructor called");
}

void AssetStore::ClearAssets(){
    for (auto texture : m_textures) {
        SDL_DestroyTexture(texture.second);
    }
    m_textures.clear();

    for (auto font : m_fonts) {
        
    }
    m_textures.clear();
}

void AssetStore::FreeFont(TTF_Font* font) {
    TTF_CloseFont(font);
}

void AssetStore::AddTexture(SDL_Renderer* renderer, const std::string& assetId, const std::string& filePath){
    SDL_Surface* surface = IMG_Load(filePath.c_str());
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    // Add the texture to the map
    m_textures.emplace(assetId, texture);

    Logger::Log("New texture added to the Asset Store with id = " + assetId);
}

SDL_Texture* AssetStore::GetTexture(const std::string& assetId){
    return m_textures[assetId];
}
std::map<std::string, SDL_Texture*> AssetStore::GetAllTextures() {
    return m_textures;
}

void AssetStore::AddFont(const std::string& assetId, const std::string& filePath, int fontSize) {
    TTF_Font* font = TTF_OpenFont(filePath.c_str(), fontSize);
    if (!font) {
        Logger::Error("Unable to add font!");
        std::exit(1);
    }
    m_fonts.emplace(assetId, font);
}

TTF_Font* AssetStore::GetFont(const std::string& assetId) {
    return m_fonts[assetId];
}