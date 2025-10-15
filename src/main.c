#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#define CCFUNCS_IMPLEMENTATION
#include "CCFuncs.h"
#include "raylib.h"

#define MUSIC_PLAYER_WIDTH 600
#define MUSIC_PLAYER_COVER_SIZE 400 // width and height of the cover

bool button() {
    Vector2 mousePos = GetMousePosition();
    Rectangle rec = {0, 0, 100, 30};

    DrawRectangleRec(rec, BLUE);

    return CheckCollisionPointRec(mousePos, rec) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}

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
    da_free(&buffer);
    UnloadImage(image);

    return true;
}

typedef struct {
    Music music;
    char *title;
    char *artist;
    char *genre;
    char *album;
    Texture2D cover;
} MusicTrack;

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

void draw_player(MusicTrack *track) {
    int screenWidth = GetScreenWidth();
    int posY = 0;

    float center = screenWidth / 2 - MUSIC_PLAYER_WIDTH / 2;

    float scale = MUSIC_PLAYER_COVER_SIZE / (float)track->cover.width;
    float coverWidth = scale * (float)track->cover.width;
    Vector2 coverPos = {screenWidth / 2 - coverWidth / 2, 0};
    DrawTextureEx(track->cover, coverPos, 0, scale, WHITE);
    posY += track->cover.height * scale;

    int pad = 20;
    posY += pad;
    DrawText(track->title, center + pad, posY, 40, WHITE);
    posY += 50;

    // posY += pad;
    DrawText(track->artist, center + pad, posY, 30, GRAY);
}

int main(void) {
    const char *musicPath = "./test.mp3";

    InitWindow(1280, 720, "C Music");
    SetTargetFPS(60);

    InitAudioDevice();

    MusicTrack *track = load_music(musicPath);

    while(!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);
        UpdateMusicStream(track->music);
        draw_player(track);

        if(button()) {
            if(IsMusicStreamPlaying(track->music)) {
                PauseMusicStream(track->music);
            } else {
                PlayMusicStream(track->music);
            }
        }

        EndDrawing();
    }

    unload_music(track);

    CloseAudioDevice();
    CloseWindow();

    return 0;
}
