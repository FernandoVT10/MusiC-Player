#ifndef PLAYER_H
#define PLAYER_H

#include "raylib.h"

typedef struct {
    Music music;
    char *title;
    char *artist;
    char *genre;
    char *album;
    Texture2D cover;
} MusicTrack;

typedef struct {
    MusicTrack *track; // song playing currently
    bool sliding;
    float titleOffset; // used to animate the title when it's too big
} Player;

void update_player(Player *player);

#endif // PLAYER_H
