
#include "render.h"

namespace Letvezi {
    namespace Render {
        bool collide(const SDL_Rect *r1, const SDL_Rect *r2) {
            return (SDL_HasIntersection(r1,r2) == SDL_TRUE);
        };

        Game::LSit Eng::render_QuitGame(std::shared_ptr<GameState::S_QuitGame>) {
            return Game::BreakLoop;
        };
        Game::LSit Eng::render_Running(std::shared_ptr<GameState::S_Running> run) {

            std::shared_ptr<Entity::Type> player_pointer = run->ent_mp[Entity::PlayerID()];

            /* draw entities on the world */
            for (auto curr = run->ent_mp.begin() ; curr != run->ent_mp.end() ;) {
                if (curr->second->killed) {
                    curr = run->ent_mp.erase(curr);
                } else {
                    curr++;
                };
            };

            for (auto& curr : run->ent_mp) {
                apply_velocity(curr.second->pos, curr.second->vel, s.common.res, fps_relation);
                render_pic(curr.second->pos, curr.second->txt_name);
                curr.second->extra_render(run, sdl_inf, fps_relation);
                if ((curr.second->pos.x > s.common.res.width  || curr.second->pos.x < 0) ||
                    (curr.second->pos.y > s.common.res.height || curr.second->pos.y < 0) ) {
                        curr.second->out_of_screen(run);
                };
            }

            // collision detection
            for (auto& curr : run->ent_mp) {
                if (curr.second->killed) continue;
                sdl_inf->with(curr.second->txt_name, [&](Game::TextureInfo text) {
                    SDL_Rect curr_rect { curr.second->pos.x, curr.second->pos.y , text.width, text.height };
                    for (auto& other : run->ent_mp) {
                        if (curr.second->killed)       break;    // ^ and this are used for avoiding extra `duplication`
                                                                 //   x when more than entity collide with the same enemy or so
                        if (other.first <= curr.first) continue; // we ignore ourselves (and previous checks)
                        if (other.second->killed)      continue; // we ignore dead entities
                        sdl_inf->with(other.second->txt_name, [&](Game::TextureInfo text2) {
                            SDL_Rect other_rect { other.second->pos.x, other.second->pos.y , text2.width , text2.height };
                            if (collide(&curr_rect, &other_rect)) {
                                Entity::collision(run, curr.second, other.second);
                            };
                        });
                    };
                });
            };

            {
                int32_t shield = dynamic_cast<Entity::Player*>(run->ent_mp[Entity::PlayerID()].get())->shield;
                Hud hud;
                SDL_Color txt_color = {200, 200, 200, 255}; // hud color
                hud.start_hud = s.common.res.height - 64;
                hud.add_text(s.common.res.width - 256 - 32 , s.common.res.height - 48, txt_color, "Points:  " + std::to_string(run->points));
                hud.add_text(s.common.res.width - 512 - 32 , s.common.res.height - 48, txt_color, "Shield: " + std::to_string(shield));
                hud.add_text(48, s.common.res.height - 48, txt_color, "Lives: ");
                Position pos(48 + 20 * 7, s.common.res.height - 48);
                for (int32_t i=0; i < run->lives; i++) {
                    hud.add_image("player_life", pos);
                    pos.x += 40;
                };

                render_hud(hud);
            };

            return Game::KeepLooping;
        };

        Game::LSit Eng::render_HighScores(std::shared_ptr<GameState::S_HighScores> hs) {
            {
                Hud hud;
                hud.start_hud = s.common.res.height - 64;
                {
                    SDL_Color player_name {200,   0, 200, 255};
                    SDL_Color points      {200, 200, 200, 255};
                    hud.add_text(Position(48, 64+16), points, "High scores");
                    Position pos(64, 64+48);
                    for (auto& r : s.common.persistent->high_scores.table()) {
                        hud.add_text(pos, player_name , r.name);
                        hud.add_text(pos + Position(s.common.res.width/3, 0) , points, std::to_string(r.points));
                        pos += Position(0,48);
                    };

                };
                SDL_Color txt_color {200, 200, 200, 255}; // hud color
                if (hs->points) {
                    hud.add_text(s.common.res.width - 256 - 32 , s.common.res.height - 48, txt_color, "Points:  " + std::to_string(*(hs->points)));
                } else {
                    hud.add_text(s.common.res.width - 256 - 64 , s.common.res.height - 48, txt_color, "Name: " + s.common.persistent->user_data.player_name);
                };
                SDL_Color game_over_c {255, 0, 0, 255};
                hud.add_text(48, s.common.res.height - 100, game_over_c, "GAME OVER");
                SDL_Color game_over_c2 = {0,0,255,255};
                hud.add_text(s.common.res.width/3, s.common.res.height - 48, game_over_c2, "Press [Return] to go back to the main menu");
                render_hud(hud);
            };
            return Game::KeepLooping;
        };

        Game::LSit Eng::render_Credits(std::shared_ptr<GameState::S_Credits> credits) {
            Position pos = credits->text_pos;
            Hud      hud;
            SDL_Color txt_color = {200, 200, 200, 255};
            if (pos.y < -200) *s.ms = credits->previous;
            hud.start_hud = 64;
            std::vector<std::pair<Game::FontID,std::string>> entries {
                {Game::Huge  , "Credits"                         },
                {Game::Normal, ""                                },
                {Game::Huge  , "LETVEZI"                         },
                {Game::Huge  , "Programming work and design"     },
                {Game::Normal, "      Name1"                     },
                {Game::Normal, "      Name2"                     },
                {Game::Normal, "      Name3"                     },
                {Game::Normal, ""                                },
                {Game::Normal, "Art"                             },
                {Game::Normal, "      Kenney  http://kenney.nl/" },
                {Game::Normal, "      neocrey http://neocrey.com"}
            };
            for (auto& i : entries) {
                hud.add_text(pos, txt_color, i.second, i.first);
                if (pos.x >= s.common.res.width || pos.y >= s.common.res.height) break;
                Position rel = Position(0,0);
                switch(i.first) {
                    case Game::Small:
                            rel = Position(0,64);
                        break;
                    case Game::Normal:
                            rel = Position(0,96);
                        break;
                    case Game::Huge:
                            rel = Position(0,128);
                        break;
                };
                pos += rel;
            };
            render_hud(hud);
            apply_velocity(credits->text_pos, Velocity(0, -30), s.common.res, fps_relation);
            return Game::KeepLooping;
        };

        Game::LSit Eng::render_Menu(std::shared_ptr<GameState::S_Menu> s_menu) {
            {
                Hud hud;
                SDL_Color txt_color = {200, 200, 200, 255}; // hud color
                hud.start_hud = s.common.res.height - (256+128);;
                hud.add_text(Position(48, hud.start_hud + 32), txt_color, s_menu->menu.title, Game::Huge);
                SDL_Color pressed_option {255, 0,   0, 255};
                SDL_Color current_option {255, 0, 255, 255};
                SDL_Color other_option   {0,   0, 255, 255};
                // TODO: see how we could deal with the menu w/o using a normal loop
                // and still have an easy way to select the next/prev menu entry easily
                Position pos(128, hud.start_hud + 128); // start pos
                for (uint16_t i=0; i<s_menu->menu.opts.size(); i++) {
                    auto c = s_menu->menu.opts[i];
                    if (i == s_menu->menu.current) {
                        if (s_menu->menu.pressed > 0) {
                            hud.add_text(pos, pressed_option, c.text);
                            s_menu->menu.pressed -= fps_relation;
                            if (s_menu->menu.pressed <= 0) {
                                s_menu->menu.pressed = 0;
                                c.callback(s);
                            };
                        } else {
                            hud.add_text(pos, current_option, c.text);
                        }
                    } else {
                            hud.add_text(pos, other_option,   c.text);
                    };
                    pos += Position(c.text.length() * (16+8) ,0);
                };
                hud.add_text(Position(s.common.res.width - (512+256) , hud.start_hud + 256     ), pressed_option, "Left/Right keys = Move between options");
                hud.add_text(Position(s.common.res.width - (512+256) , hud.start_hud + 256 + 32), pressed_option, "Space key       = Select current option");
                render_hud(hud);
            };
            return Game::KeepLooping;
        };

        void Eng::render_pic(Position pos, std::string txt_name, uint8_t alpha) {
            sdl_inf->with(txt_name, [&](Game::TextureInfo text) {
                SDL_Rect r;
                r.x = pos.x;
                r.y = pos.y;
                r.w = text.width;
                r.h = text.height;
                SDL_SetTextureAlphaMod(text.texture, alpha);
                SDL_RenderCopy(sdl_inf->win_renderer, text.texture, NULL, &r);
            });
        };

        void Eng::render_hud(const Hud& hud) {
                // hud
                SDL_SetRenderDrawColor(sdl_inf->win_renderer, 0, 0, 0, 255);
                {
                    SDL_Rect rect {0, hud.start_hud, s.common.res.width, s.common.res.height - hud.start_hud};
                    SDL_RenderFillRect(sdl_inf->win_renderer, &rect);
                };
                for (auto& txt : hud.txts) {
                    sdl_inf->render_text(txt.pos.x, txt.pos.y, txt.f_id, txt.col, txt.text);
                };
                for (auto& spr : hud.items) {
                    render_pic(Position(spr.pos.x, spr.pos.y), spr.texture);
                };
        };

        void Eng::render_background() {
                // apply background
                SDL_SetRenderDrawColor(sdl_inf->win_renderer, 75, 0, 60, 255);
                SDL_RenderClear(sdl_inf->win_renderer);

                sdl_inf->with("bg_star", [&](Game::TextureInfo bg_star) {
                    for(auto p = s.common.bg_particles.begin(); p != s.common.bg_particles.end() ;) {
                        apply_velocity(p->pos, p->vel, s.common.res, fps_relation);
                        SDL_Rect r {p->pos.x, p->pos.y, bg_star.width, bg_star.height};
                        SDL_RenderCopyEx(sdl_inf->win_renderer, bg_star.texture, NULL, &r, p->angle, NULL, SDL_FLIP_NONE);
                        p->angle += fps_relation/2;
                        if (p->pos.y >= s.common.res.height || p->pos.x <= 0 || p->pos.x >= s.common.res.width) {
                            p = s.common.bg_particles.erase(p);
                            s.common.add_bg_particle();
                        } else {
                            p++;
                        }
                    }
                });

                /**/
        }
    };
    namespace Entity {
        void Player::extra_render(std::shared_ptr<GameState::S_Running> gs, std::shared_ptr<Game::sdl_info> sdl_inf, int fps_rel) {
            if (shield > 0) {
                Render::Eng eng(sdl_inf, gs->parent, fps_rel);
                int alpha = 255;
                if (shield < 1000) alpha = shield/4;
                eng.render_pic(pos - Position(10,16), "player_shield", alpha);
                shield -= fps_rel;
                shield = std::max(0, shield);
            };
        };
        void PlayerBullet::extra_render(std::shared_ptr<GameState::S_Running>, std::shared_ptr<Game::sdl_info>, int) {
        };
        void EnemyBullet::extra_render(std::shared_ptr<GameState::S_Running>, std::shared_ptr<Game::sdl_info>, int) {
        };
        void PowerUp::extra_render(std::shared_ptr<GameState::S_Running>, std::shared_ptr<Game::sdl_info> , int) {
        };
        void Enemy::extra_render(std::shared_ptr<GameState::S_Running>, std::shared_ptr<Game::sdl_info>, int) {
        };


        void PlayerBullet::out_of_screen(std::shared_ptr<GameState::S_Running>) {
            this->kill();
        };
        void EnemyBullet::out_of_screen(std::shared_ptr<GameState::S_Running>) {
            this->kill();
        };
        void PowerUp::out_of_screen(std::shared_ptr<GameState::S_Running>) {
            this->kill();
        };
        void Enemy::out_of_screen(std::shared_ptr<GameState::S_Running> run) {
            if (boss) {
                vel = -1 * vel;
            } else {
                this->kill();
                run->add_enemy();
            }
        };
    };
        /* ^ those are here because the virtual definitions need to be all in the same compilation unit (.o) */
}