/* main.cpp*/


#include <cstdint>
#include <SDL2/SDL.h>
#include <stdio.h>
#include <list>
#include <random>
#include <algorithm>
#include "game.h"
#include "letvezi.h"
#include "render.h"

#ifdef _WIN32
int WinMain() {
#else
int main() {
#endif

    try {
        if(SDL_Init(SDL_INIT_VIDEO) < 0) { throw Game::SDLError(); }
        { int imgFlags = IMG_INIT_PNG;
          if(!(IMG_Init(imgFlags) & imgFlags)) { throw Game::SDLError(IMG_GetError()); }
        }
        if (TTF_Init() < 0) { throw Game::SDLError(TTF_GetError()); }
        if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
            printf("Error starting SDL_Mix: %s\n", Mix_GetError());
        }
        auto gs = std::make_shared<Game::sdl_info>("Letvezi", "art/font.ttf");
        gs->loading_screen([](auto& ch) {
            auto set_bg   = [&](std::string file) {
                                ch.push( boost::optional<std::tuple<std::string,std::function<void(Game::sdl_info&)>>>(
                                    std::tuple<std::string,std::function<void(Game::sdl_info&)>>(
                                        "background music",
                                        [file](auto& gs) { gs.set_background(file); }
                                    )
                                ));
                            };
            auto load_sfx = [&](std::string key, std::string file) {
                                ch.push( boost::optional<std::tuple<std::string,std::function<void(Game::sdl_info&)>>>(
                                    std::tuple<std::string,std::function<void(Game::sdl_info&)>>(
                                        key,
                                        [key,file](auto& gs) { gs.load_sfx(key,file); }
                                    )
                                ));
                            };
            auto load_png = [&](std::string key, std::string file) {
                                ch.push( boost::optional<std::tuple<std::string,std::function<void(Game::sdl_info&)>>>(
                                    std::tuple<std::string,std::function<void(Game::sdl_info&)>>(
                                        key,
                                        [key,file](auto& gs) { gs.load_png(key,file); }
                                    )
                                ));
                            };
            auto end = [&]() {
                                ch.push( boost::none );
                            };


            set_bg("art/background.ogg");

            load_png("bg_star"         , "art/img/bg_star.png"            );

            load_png("player"          , "art/img/player.png"             );
            load_png("player_laser"    , "art/img/player_laser.png"       );
            load_png("player_life"     , "art/img/player_life.png"        );
            load_png("player_shield"   , "art/img/player_shield.png"      );

            load_png("enemy_1"         , "art/img/enemy_1.png"            );
            load_png("enemy_2"         , "art/img/enemy_2.png"            );
            load_png("enemy_3"         , "art/img/enemy_3.png"            );
            load_png("enemy_boss"      , "art/img/enemy_boss.png"         );
            load_png("enemy_laser"     , "art/img/enemy_laser.png"        );
            load_png("enemy_boss_squad", "art/img/enemy_boss_squad.png"   );

            load_png("powerup_shield"  , "art/img/powerup_shield.png"     );
            load_png("powerup_bolt"    , "art/img/powerup_bolt.png"       );

            load_sfx("player_laser"    , "art/sfx/player_laser.ogg"       );
            load_sfx("shield_enabled"  , "art/sfx/player_laser.ogg"       );

            end();
        });

        auto persistent = std::make_shared<Persistent>("user_info.dat");
        std::cout << "Loading user data from user_info.dat " << std::endl;
        persistent->load();

        Game::Resolution res = gs->get_current_res();
        Letvezi::GameState::Type start_state =
                   Letvezi::GameState::Type(persistent, gs, res,
                            Letvezi::Position(res.width/2, res.height-70-gs->textures().at("player").height));

        std::function<void(Conc::Chan<SDL_Event>&,Conc::Chan<Letvezi::Events::Type>&)> event_fn =
                Letvezi::event_handler;
        std::function<void(Conc::Chan<Letvezi::Events::Type>&,Conc::VarL<Letvezi::GameState::Type>&)> game_fn =
                Letvezi::game_handler;
        std::function<Game::LSit(Conc::VarL<Letvezi::GameState::Type>&,uint16_t)> render_fn =
                [gs](Conc::VarL<Letvezi::GameState::Type>& typ,uint16_t fps_rel) {
                            return Letvezi::Render::handler_game(gs,typ,fps_rel);
                        };

        gs->loop(event_fn, game_fn, render_fn, start_state);
        persistent->save();
        printf("Game over!\n");


    } catch (Game::SDLError& err) {
        printf("SDL Error: %s\n", err.what());
    }
    SDL_Quit();

    return 0;
}
