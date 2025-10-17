#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#define CCFUNCS_IMPLEMENTATION
#include "CCFuncs.h"
#include "raylib.h"
#include "player.h"

char *read_str_from_stream(FILE *stream) {
    StringBuilder sb = {0};
    char c;
    while((c = fgetc(stream)) != EOF) {
        da_append(&sb, c);
    }
    char *str = sb_dump_str(&sb);
    da_free(&sb);
    return str;
}

// returns tag content as a string
char *get_music_str_tag(const char *filePath, char *tagOpt) {
    const char *cmd = TextFormat("exiftool -b %s %s", tagOpt, filePath);
    FILE *fp = popen(cmd, "r");
    if(fp == NULL) return NULL;

    char *content = read_str_from_stream(fp);

    pclose(fp);
    return content;
}

char *get_music_title(const char *filePath) {
    return get_music_str_tag(filePath, "-title");
}

char *get_music_artist(const char *filePath) {
    return get_music_str_tag(filePath, "-artist");
}

char *get_music_genre(const char *filePath) {
    return get_music_str_tag(filePath, "-genre");
}

char *get_music_album(const char *filePath) {
    return get_music_str_tag(filePath, "-album");
}

bool load_music_cover(const char *filePath, Texture2D *dst) {
    const char *cmd = TextFormat("exiftool -b -picture %s", filePath);
    FILE *fp = popen(cmd, "r");
    if(fp == NULL) return false;

    struct {
        unsigned char *items;
        size_t count;
        size_t capacity;
    } buffer = {0};

    int c;
    while((c = fgetc(fp)) != EOF) {
        da_append(&buffer, (unsigned char)c);
    }

    pclose(fp);

    Image image = LoadImageFromMemory(".jpg", buffer.items, buffer.count);
    *dst = LoadTextureFromImage(image);
    SetTextureFilter(*dst, TEXTURE_FILTER_BILINEAR);
    da_free(&buffer);
    UnloadImage(image);

    return true;
}

MusicTrack *load_music(const char *filePath) {
    MusicTrack *track = calloc(1, sizeof(MusicTrack));
    track->music = LoadMusicStream(filePath);
    track->title = get_music_title(filePath);
    track->artist = get_music_artist(filePath);
    track->genre = get_music_genre(filePath);
    track->album = get_music_album(filePath);

    if(!load_music_cover(filePath, &track->cover)) {
        log_error("Failed to load the cover from %s", filePath);
    }

    return track;
}

void unload_music(MusicTrack *track) {
    UnloadMusicStream(track->music);
    free(track->title);
    free(track->artist);
    free(track->genre);
    free(track->album);

    UnloadTexture(track->cover);
    free(track);
}

int main(void) {
    const char *musicPath = "./test.mp3";

    InitWindow(1280, 720, "C Music");
    SetTargetFPS(60);

    InitAudioDevice();

    Player player = {0};

    player.track = load_music(musicPath);

    while(!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);

        update_player(&player);

        EndDrawing();
    }

    unload_music(player.track);
    player.track = NULL;

    CloseAudioDevice();
    CloseWindow();

    return 0;
}
