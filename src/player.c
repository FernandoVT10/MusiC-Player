#include <stddef.h>

#include "player.h"

#define MUSIC_PLAYER_WIDTH 600
#define MUSIC_PLAYER_COVER_SIZE 400 // width and height of the cover
#define MUSIC_PLAYER_SLIDER_THICKNESS 5
#define MUSIC_PLAYER_SLIDER_COLOR GRAY
#define MUSIC_PLAYER_SLIDER_PLAYED_COLOR BLUE

#define MUSIC_PLAYER_TITLE_SIZE 40
#define MUSIC_PLAYER_TITLE_COLOR RED

// returns cover height
static float draw_cover(Texture2D cover) {
    int screenWidth = GetScreenWidth();
    float scale = MUSIC_PLAYER_COVER_SIZE / (float)cover.width;
    float coverWidth = scale * (float)cover.width;
    Vector2 coverPos = {screenWidth / 2 - coverWidth / 2, 0};
    DrawTextureEx(cover, coverPos, 0, scale, WHITE);
    return scale * cover.height;
}

static void toggle_music(Player *player) {
    if(player->track == NULL) return;
    Music music = player->track->music;

    if(IsMusicStreamPlaying(music)) {
        PauseMusicStream(music);
    } else {
        PlayMusicStream(music);
    }
}

static void set_music_time(Player *player, float time) {
    if(player->track != NULL) SeekMusicStream(player->track->music, time);
}

static float get_music_time(Player *player) {
    return player->track == NULL ? 0 : GetMusicTimePlayed(player->track->music);
}

static float get_music_length(Player *player) {
    return player->track == NULL ? 0 : GetMusicTimeLength(player->track->music);
}

static void draw_player_button(Player *player) {
    Vector2 mousePos = GetMousePosition();
    Rectangle rec = {0, 0, 100, 30};
    DrawRectangleRec(rec, BLUE);

    if(CheckCollisionPointRec(mousePos, rec) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        toggle_music(player);
    }
}

static void draw_player_slider(Player *player, Vector2 pos, float width) {
    float lineThickness = MUSIC_PLAYER_SLIDER_THICKNESS;

    float value = get_music_time(player) / get_music_length(player);
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
    bool mouseWasMoved = mouseDelta.x != 0 || mouseDelta.y != 0;

    if((IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mousePos, sliderRec)) || (player->sliding && mouseWasMoved)) {
        float relativePos = mousePos.x - pos.x;
        float amount = relativePos / width;

        if(amount < 0) amount = 0;
        else if(amount > 1) amount = 1;

        set_music_time(player, amount * get_music_length(player));
        player->sliding = true;
    }

    if(IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && player->sliding) {
        player->sliding = false;
    }
}

static void draw_title(Player *player, char *title, Vector2 pos, float maxWidth) {
    float dt = GetFrameTime();
    float scrollingSpeed = 50;
    float paddingBetweenTitles = 200;
    int textWidth = MeasureText(title, MUSIC_PLAYER_TITLE_SIZE);

    if(textWidth <= maxWidth) {
        DrawText(title, pos.x, pos.y, MUSIC_PLAYER_TITLE_SIZE, WHITE);
        return;
    }

    BeginScissorMode(pos.x, pos.y, maxWidth, MUSIC_PLAYER_TITLE_SIZE);

    player->titleOffset += scrollingSpeed * dt;

    if(player->titleOffset >= textWidth + paddingBetweenTitles) {
        player->titleOffset = 0;
    }

    DrawText(title, pos.x - player->titleOffset, pos.y, MUSIC_PLAYER_TITLE_SIZE, MUSIC_PLAYER_TITLE_COLOR);

    DrawText(
        title,
        pos.x + textWidth + paddingBetweenTitles - player->titleOffset,
        pos.y,
        MUSIC_PLAYER_TITLE_SIZE,
        MUSIC_PLAYER_TITLE_COLOR
    );

    EndScissorMode();
}

static void draw_player(Player *player) {
    // TODO: make a placeholder
    if(player->track == NULL) return;

    int screenWidth = GetScreenWidth();
    int posY = 0;

    float center = screenWidth / 2 - MUSIC_PLAYER_WIDTH / 2;

    posY += draw_cover(player->track->cover);

    int padding = 20;
    posY += padding;

    // title
    Vector2 titlePos = {center + padding, posY};
    float titleMaxWidth = MUSIC_PLAYER_WIDTH - padding * 2;
    draw_title(player, player->track->title, titlePos, titleMaxWidth);
    posY += MUSIC_PLAYER_TITLE_SIZE;

    char *artist = player->track->artist;
    DrawText(artist, center + padding, posY, 30, GRAY);

    draw_player_button(player);

    Vector2 sliderPos = {100, 600};
    float sliderWidth = 1080;
    draw_player_slider(player, sliderPos, sliderWidth);
}

void update_player(Player *player) {
    if(player->track == NULL) return;

    UpdateMusicStream(player->track->music);

    if(IsKeyPressed(KEY_SPACE)) {
        toggle_music(player);
    }

    float time = get_music_time(player);

    if(IsKeyPressed(KEY_RIGHT)) {
        set_music_time(player, time + 5);
    } else if(IsKeyPressed(KEY_LEFT)) {
        set_music_time(player, time - 5);
    }

    draw_player(player);
}
