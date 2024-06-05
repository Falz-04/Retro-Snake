#include "raylib.h"
#include <iostream>
#include <deque>
#include <raymath.h>

using namespace std;

Color green = { 173, 204, 96, 255 };
Color darkGreen = { 43, 51, 25, 255 };

int cellSize = 30;
int cellCount = 25;
int offset = 75;

double lastUpdateTime = 0;

bool ElementInDeque(Vector2 element, deque<Vector2> deque) {
    for (unsigned int i = 0; i < deque.size(); i++) {
        if (Vector2Equals(element, deque[i])) {
            return true;
        }
    }
    return false;
}

bool eventTriggered(double interval) {
    double currentTime = GetTime();
    if (currentTime - lastUpdateTime >= interval) {
        lastUpdateTime = currentTime;
        return true;
    }
    return false;
}

class Snake {
public:
    deque<Vector2> body = { Vector2{6.0f, 9.0f} , Vector2{5.0f, 9.0f} , Vector2{4.0f, 9.0f} };
    Vector2 direction = { 1.0f, 0.0f };
    bool addSegment = false;

    void Draw() {
        for (unsigned int i = 0; i < body.size(); i++) {
            DrawRectangleRounded(Rectangle{ offset + body[i].x * cellSize, offset + body[i].y * cellSize, (float)cellSize, (float)cellSize }, 0.5, 6, darkGreen);
        }
    }

    void Update() {
        body.push_front(Vector2Add(body[0], direction));
        if (addSegment == true) {
            addSegment = false;
        }
        else {
            body.pop_back();
        }
    }

    void Reset() {
        body = { Vector2{6.0f, 9.0f} , Vector2{5.0f, 9.0f} , Vector2{4.0f, 9.0f} };
        direction = { 1.0f, 0.0f };
    }

};

class Food {
public:
    Vector2 pos;
    Texture2D texture;

    Food(deque<Vector2> snakeBody) {
        Image image = LoadImage("food.png");
        texture = LoadTextureFromImage(image);
        UnloadImage(image);
        pos = GenerateRandomPos(snakeBody);
        cout << "Initial food position: " << pos.x << ", " << pos.y << endl; // Debug
    }

    ~Food() {
        UnloadTexture(texture);
    }

    void Draw() {
        DrawTexture(texture, offset + pos.x * cellSize, offset + pos.y * cellSize, WHITE);
    }

    Vector2 GenerateRandomCell() {
        float x = GetRandomValue(0, cellCount - 1);
        float y = GetRandomValue(0, cellCount - 1);
        return Vector2{ x, y };
    }

    Vector2 GenerateRandomPos(deque<Vector2> snakeBody) {
        Vector2 position = GenerateRandomCell();
        while (ElementInDeque(position, snakeBody)) {
            position = GenerateRandomCell();
        }
        cout << "Generated food position: " << position.x << ", " << position.y << endl; // Debug
        return position;
    }

    void Reset(deque<Vector2> snakeBody) {
        pos = GenerateRandomPos(snakeBody);
        cout << "Reset food position: " << pos.x << ", " << pos.y << endl; // Debug
    }
};

class Game {
public:
    Snake snake = Snake();
    Food food = Food(snake.body);
    bool running = false;
    int score = 0;
    Sound eatSound;
    Sound wallSound;

    Game() {
        InitAudioDevice();
        eatSound = LoadSound("eat.mp3");
        wallSound = LoadSound("wall.mp3");
    }

    ~Game() {
        UnloadSound(eatSound);
        UnloadSound(wallSound);
        CloseAudioDevice();
    }

    void Draw() {
        food.Draw();
        snake.Draw();
    }

    void Update() {
        if (running) {
            snake.Update();
            EatFood();
            CheckCollisionEdges();
            CheckCollisionTail();
        }
    }

    void EatFood() {
        if (Vector2Equals(snake.body[0], food.pos)) {
            food.Reset(snake.body); // Correctly reset the food position
            snake.addSegment = true;
            score++;
            PlaySound(eatSound);
        }
    }

    void CheckCollisionEdges() {
        if (snake.body[0].x == cellCount || snake.body[0].x == -1 || snake.body[0].y == cellCount || snake.body[0].y == -1) {
            GameOver();
        }
    }

    void CheckCollisionTail() {
        deque<Vector2> headless = snake.body;
        headless.pop_front();
        if (ElementInDeque(snake.body[0], headless)) {
            GameOver();
        }
    }

    void GameOver() {
        snake.Reset();
        food.Reset(snake.body); // Correctly reset the food position
        running = false;
        score = 0;
        PlaySound(wallSound);
    }
};

int main() {
    int ScreenSize = 2 * offset + cellSize * cellCount;
    int FPS = 60;

    InitWindow(ScreenSize, ScreenSize, "Snake Game");
    SetTargetFPS(FPS);

    Game game = Game();

    while (!WindowShouldClose()) {
        BeginDrawing();

        // Updating
        if (eventTriggered(0.2) && game.running) {
            game.Update();
        }

        if (!game.running) {
            DrawText("Press SPACE to start", ScreenSize / 2 - 100, ScreenSize / 2, 20, WHITE);
        }

        if (IsKeyPressed(KEY_SPACE)) {
            game.running = true;
        }

        if ((IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP)) && game.snake.direction.y != 1.0f) {
            game.snake.direction = { 0.0f, -1.0f };
        }
        if ((IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN)) && game.snake.direction.y != -1.0f) {
            game.snake.direction = { 0.0f, 1.0f };
        }
        if ((IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT)) && game.snake.direction.x != -1.0f) {
            game.snake.direction = { 1.0f, 0.0f };
        }
        if ((IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT)) && game.snake.direction.x != 1.0f) {
            game.snake.direction = { -1.0f, 0.0f };
        }

        // Drawing
        ClearBackground(green);
        DrawRectangleLinesEx(Rectangle{ (float)offset - 5, (float)offset - 5, (float)cellSize * cellCount + 10, (float)cellSize * cellCount + 10 }, 5, darkGreen);
        DrawText("Retro Snake", offset - 5, 20, 40, darkGreen);
        DrawText(TextFormat("Score: %i", game.score), offset - 5, offset + cellSize * cellCount + 10, 40, darkGreen);
        game.Draw();

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
