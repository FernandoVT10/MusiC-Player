#include <stdio.h>

#include "raylib.h"

bool button() {
    Vector2 mousePos = GetMousePosition();
    Rectangle rec = {0, 0, 100, 30};

    DrawRectangleRec(rec, BLUE);

    return CheckCollisionPointRec(mousePos, rec) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}

int main(void) {
    InitWindow(1280, 720, "C Music");
    SetTargetFPS(60);

    InitAudioDevice();

    const char *music_path = "test.mp3";
    Music music = LoadMusicStream(music_path);

    while(!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);
        UpdateMusicStream(music);

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
