#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#define CCFUNCS_IMPLEMENTATION
#include "CCFuncs.h"
#include "raylib.h"

#define MUSIC_PLAYER_WIDTH 600
#define MUSIC_PLAYER_COVER_SIZE 400 // width and height of the cover
#define MUSIC_PLAYER_SLIDER_THICKNESS 5
#define MUSIC_PLAYER_SLIDER_COLOR GRAY
#define MUSIC_PLAYER_SLIDER_PLAYED_COLOR BLUE

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

typedef struct {
    MusicTrack *current;
} MusicPlayer;

MusicPlayer player = {0};

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

void toggle_music() {
    if(player.current == NULL) return;
    Music music = player.current->music;

    if(IsMusicStreamPlaying(music)) {
        PauseMusicStream(music);
    } else {
        PlayMusicStream(music);
    }
}

void draw_player_button(void) {
    Vector2 mousePos = GetMousePosition();
    Rectangle rec = {0, 0, 100, 30};
    DrawRectangleRec(rec, BLUE);

    if(CheckCollisionPointRec(mousePos, rec) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        toggle_music();
    }
}

void set_music_time(float time) {
    if(player.current != NULL) SeekMusicStream(player.current->music, time);
}

float get_music_time() {
    return player.current == NULL ? 0 : GetMusicTimePlayed(player.current->music);
}

float get_music_length() {
    return player.current == NULL ? 0 : GetMusicTimeLength(player.current->music);
}

void draw_player_slider(Vector2 pos, float width) {
    // TODO: change this variable to a global state or something
    static bool sliding = false;

    float lineThickness = MUSIC_PLAYER_SLIDER_THICKNESS;

    float value = get_music_time() / get_music_length();
    // the center of the slider
    float centerX = (value * width) + pos.x;

    {
        Vector2 start = {pos.x, pos.y};
        Vector2 end = {centerX, pos.y};
        DrawLineEx(start, end, lineThickness, MUSIC_PLAYER_SLIDER_PLAYED_COLOR);
    }
    {
        Vector2 start = {centerX, pos.y};
        Vector2 end = {pos.x + width, pos.y};
        DrawLineEx(start, end, lineThickness, MUSIC_PLAYER_SLIDER_COLOR);
    }

    Vector2 circlePos = {centerX, pos.y};
    DrawCircleV(circlePos, 10, MUSIC_PLAYER_SLIDER_PLAYED_COLOR);

    Vector2 mousePos = GetMousePosition();

    // here we make the collider taller for better UX, in total is 2x taller than the line
    Rectangle sliderRec = {
        .x = pos.x,
        .y = pos.y - lineThickness, // center the rec with the line
        .width = width,
        .height = lineThickness * 2,
    };

    Vector2 mouseDelta = GetMouseDelta();
    bool mouseWasMoved = mouseDelta.x + mouseDelta.y != 0;

    if((IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mousePos, sliderRec)) || (sliding && mouseWasMoved)) {
        float relativePos = mousePos.x - pos.x;
        float amount = relativePos / width;

        if(amount < 0) amount = 0;
        else if(amount > 1) amount = 1;

        set_music_time(amount * get_music_length());
        sliding = true;
    }

    if(IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && sliding) {
        sliding = false;
    }
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

    DrawText(track->artist, center + pad, posY, 30, GRAY);

    draw_player_button();

    Vector2 sliderPos = {100, 600};
    float sliderWidth = 1080;
    draw_player_slider(sliderPos, sliderWidth);
}

int main(void) {
    const char *musicPath = "./test.mp3";

    InitWindow(1280, 720, "C Music");
    SetTargetFPS(60);

    InitAudioDevice();

    player.current = load_music(musicPath);

    while(!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);
        UpdateMusicStream(player.current->music);
        draw_player(player.current);

        if(IsKeyPressed(KEY_SPACE)) {
            toggle_music();
        }

        if(IsKeyPressed(KEY_RIGHT)) {
            set_music_time(get_music_time() + 5);
        } else if(IsKeyPressed(KEY_LEFT)) {
            set_music_time(get_music_time() - 5);
        }

        EndDrawing();
    }

    unload_music(player.current);
    player.current = NULL;

    CloseAudioDevice();
    CloseWindow();

    return 0;
}
