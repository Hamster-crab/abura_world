#include <raylib.h>

Rectangle windowSize = {0, 0, 800, 500};

int main(void)
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(windowSize.width, windowSize.height, "abura");

    Font customFont = LoadFont("resources/fonts/8bitRegular.ttf");

    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawTextEx(customFont, "Hello, Custom Font!", (Vector2){ 50, 50 }, 30, 2, BLACK);
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
