#ifndef ASSETSTORE_H
#define ASSETSTORE_H

#include <map>
#include <string>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

class AssetStore {
    private:
        std::map<std::string, SDL_Texture*> m_textures;
        std::map<std::string, TTF_Font*> m_fonts;

    public:
        AssetStore();
        ~AssetStore();

        //Textures handling 
        void ClearAssets();
        void AddTexture(SDL_Renderer* renderer, const std::string& assetId, const std::string& filePath);
        SDL_Texture* GetTexture(const std::string& assetId);
        std::map<std::string, SDL_Texture*> GetAllTextures();

        // Fonts Handling
        void AddFont(const std::string& assetId, const std::string& filePath, int fontSize);
        TTF_Font* GetFont(const std::string& assetId);
        void FreeFont(TTF_Font* font) ;

};

#endif