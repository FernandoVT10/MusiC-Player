#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include "raylib.h"

bool button() {
    Vector2 mousePos = GetMousePosition();
    Rectangle rec = {0, 0, 100, 30};

    DrawRectangleRec(rec, BLUE);

    return CheckCollisionPointRec(mousePos, rec) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}

typedef struct {
    Music music;
    Texture2D cover;
    char *title;
} MusicFile;

bool load_music(MusicFile *musicFile, const char *path) {
    musicFile->music = LoadMusicStream(path);

    const char *cmd = TextFormat("exiftool -binary -picture %s", path);
    FILE *fp = popen(cmd, "r");
    if(fp == NULL) {
        return false;
    }
    size_t size = 200 * 1024;
    void *buffer = malloc(size);
    fread(buffer, size, 1, fp);
    pclose(fp);

    Image image = LoadImageFromMemory(".jpg", buffer, size);
    musicFile->cover = LoadTextureFromImage(image);
}

int main(void) {
    const char *music_path = "test.mp3";

    FILE *fp = popen("exiftool -binary -picture ./test.mp3", "r");

    if(fp == NULL) return 1;

    InitWindow(1280, 720, "C Music");
    SetTargetFPS(60);

    InitAudioDevice();

    Music music = LoadMusicStream(music_path);
    Image image = LoadImageFromMemory(".jpg", buffer, size);
    Texture2D t = LoadTextureFromImage(image);

    while(!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);
        UpdateMusicStream(music);
        DrawTexture(t, 0, 0, WHITE);

        if(button()) {
            if(IsMusicStreamPlaying(music)) {
                PauseMusicStream(music);
            } else {
                PlayMusicStream(music);
            }
        }

        EndDrawing();
    }

    UnloadMusicStream(music);
    CloseAudioDevice();
    CloseWindow();

    return 0;
}
