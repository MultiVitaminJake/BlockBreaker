#include <string>
#include <raylib.h>
#include <iostream>
#include <vector>

// Constants for screen size and grid properties
constexpr int TILE_SIZE = 30;      // Size of each tile in pixels
constexpr int TILE_AMOUNT = 30;    // Number of tiles along each axis
constexpr int GRID_SIZE = TILE_SIZE * TILE_AMOUNT; // Grid dimension (900px)
constexpr int SCREEN_WIDTH = GRID_SIZE;  // Screen width in pixels
constexpr int SCREEN_HEIGHT = GRID_SIZE; // Screen height in pixels

// Color scheme definitions for UI elements
Color colorScheme[] =
{
    { 57, 255, 20, 255 }  // Neon Green (Text)
};  

// Macro definitions for easier color referencing
#define TEXT_COLOR colorScheme[0]

// Enum to define different game/ball states
enum class GameState { Menu, Playing, Victory, Paused, GameOver, HighScores };
enum class BallState { Stuck, Moving, PowerUp };

// Structure for managing text rendering
struct TextManager
{
    // Draws text centered on the screen with adjustable padding
    void drawCenterText(const char* text, int fontSize, Color color, int paddingX, int paddingY) const
    {   
        int textWidth = MeasureText(text, fontSize);
        int centerX = (SCREEN_WIDTH / 2) - (textWidth / 2) + paddingX;
        int centerY = (SCREEN_HEIGHT / 2) - (fontSize / 2) + paddingY;

        DrawText(text, centerX, centerY, fontSize, color);
    }

    // Draws text aligned to the left with adjustable padding
    void drawLeftText(const char* text, int fontSize, Color color, int paddingX, int paddingY) const
    {   
        int centerY = (SCREEN_HEIGHT / 2) - (fontSize / 2) + paddingY;
        DrawText(text, paddingX, centerY, fontSize, color);
    }
};

// Structure for managing the paddle
struct Paddle
{
    Vector2 position; // Paddle position on screen
    float width = TILE_SIZE * 4, height = TILE_SIZE / 2; // Paddle dimensions
    float speed = 300.0f; // Paddle movement speed
    float velocityX = 0.0f; // Paddle horizontal velocity

    // Constructor initializes the paddle at the bottom center of the screen
    Paddle()
    {
        position = { (SCREEN_WIDTH / 2) - (width / 2), SCREEN_HEIGHT - height - 10 };
    }

    // Draws the paddle on the screen
    void draw()
    {
        DrawRectangleV(position, { width, height }, GREEN);
    }

    // Updates paddle movement based on user input
    void update()
    {
        float deltaTime = GetFrameTime();

        if (IsKeyDown(KEY_LEFT) && position.x > 0) 
        { 
            position.x -= speed * deltaTime; 
            velocityX = -speed; 
        }
        if (IsKeyDown(KEY_RIGHT) && position.x + width < SCREEN_WIDTH) 
        { 
            position.x += speed * deltaTime; 
            velocityX = speed; 
        }
    }
};

// Structure for managing the ball
struct Ball
{
    BallState state = BallState::Stuck; // Ball starts in a stuck state
    TextManager textManager; // Handles text rendering

    Vector2 position; // Ball position on screen
    Vector2 velocity; // Ball velocity

    float radius = 10.0f; // Ball radius
    float speed = 250.0f; // Ball movement speed
    int lives = 3; // Player's remaining lives

    // Constructor initializes the ball above the paddle in a stationary state
    Ball(const Paddle& paddle) 
    {
        position = { paddle.position.x + (paddle.width / 2), paddle.position.y - radius * 2 };
        velocity = { 0, 0 };
    }

    // Updates the ball's position and state based on gameplay events
    void update(const Paddle& paddle)
    {
        float deltaTime = GetFrameTime();

        switch (state)
        {
        case BallState::Stuck:
            position.x = paddle.position.x + (paddle.width / 2);
            position.y = paddle.position.y - radius * 2;

            if (IsKeyPressed(KEY_SPACE)) 
            {
                velocity = { speed, -speed };
                state = BallState::Moving;
            }
            break;

        case BallState::Moving:
            position.x += velocity.x * deltaTime;
            position.y += velocity.y * deltaTime;

            if (position.x - radius <= 0 || position.x + radius >= SCREEN_WIDTH) 
                velocity.x *= -1; 
            else if (position.y - radius <= 0) 
                velocity.y *= -1; 
            
            if (position.y + radius >= SCREEN_HEIGHT)
            {
                lives--;
                state = BallState::Stuck;
            }

            if (position.y + radius >= paddle.position.y &&
                position.x >= paddle.position.x &&
                position.x <= paddle.position.x + paddle.width)
            {
                float hitPosition = (position.x - paddle.position.x) / paddle.width; // Normalize 0-1
                velocity.y *= -1;
                velocity.x = (hitPosition - 0.5f) * speed; // ✅ Ball bounces based on where it hits paddle
            }            
            break;
        }
    }

    // Draws the ball on the screen
    void draw()
    {
        DrawCircleV(position, radius, WHITE);
    }
};

struct Brick
{
    Vector2 position;
    Vector2 size = { TILE_SIZE * 4, TILE_SIZE / 2 };
    std::vector<Brick> bricks;
    bool active = true;

    Brick() : position({ 0, 0 }) {};
    Brick(float x, float y) 
    {
        position = { x, y };
    }

    void draw()
    {
        if (active)
            DrawRectangleV(position, size, BLUE);
    }
};

// Structure for managing the game and its loop
struct Game
{
    GameState state = GameState::Menu;
    TextManager textManager;
    Paddle paddle;
    Ball ball;
    Brick brick;

    int score = 0;
    std::vector<Brick> bricks; // ✅ Game stores all bricks

    Game() : ball(paddle)
    {
        InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Block Breaker");
        SetTargetFPS(60);
        
        while (!WindowShouldClose())
        {
            update();

            BeginDrawing();
            ClearBackground(BLACK);
            draw();
            EndDrawing();
        }

        CloseWindow();
    }

    void initBricks()
    {
        bricks.clear();

        int rows = 5;
        int cols = 10;
        float brickWidth = 80.0f;
        float brickHeight = 30.0f;
        float padding = 10.0f;

        float totalGridWidth = (cols * brickWidth) + ((cols - 1) * padding);
        float startX = (SCREEN_WIDTH - totalGridWidth) / 2;
        float startY = 150.0f;

        for (int row = 0; row < rows; row++)  
        {
            for (int col = 0; col < cols; col++) 
            {
                float x = startX + col * (brickWidth + padding);
                float y = startY + row * (brickHeight + padding);
                bricks.emplace_back(x, y);
            }
        }
    }

    void drawBricks()
    {
        for (Brick& brick : bricks)
        {
            if (brick.active)
                brick.draw();
        }
    }

    void reset()
    {
        paddle = Paddle();
        ball = Ball(paddle);
        ball.state = BallState::Stuck;
        score = 0;
        ball.lives = 3;

        if (state == GameState::Menu || state == GameState::GameOver) state = GameState::Menu; 
        else state = GameState::Playing;
    }

    void update()
    {
        switch (state)
        {
            case GameState::Menu:
                if (IsKeyPressed(KEY_ENTER)) state = GameState::Playing;
                else if (IsKeyPressed(KEY_H)) state = GameState::HighScores;
                break;
            case GameState::HighScores:
                if (IsKeyPressed(KEY_B)) state = GameState::Menu; 
                break;
            case GameState::Playing:
                paddle.update();
                ball.update(paddle);

                if (IsKeyPressed(KEY_B)) state = GameState::Paused;
                if (ball.lives == 0) state = GameState::GameOver;
                break;
            case GameState::Paused:
                if (IsKeyPressed(KEY_ENTER)) state = GameState::Playing;
                else if (IsKeyPressed(KEY_B)) { state = GameState::Menu; reset(); }
                else if (IsKeyPressed(KEY_R)) reset();
                break;
                
        }
    }

    void draw()
    {
        drawUI();
        if (state == GameState::Playing)
        {
            initBricks();
            paddle.draw();
            ball.draw();
            drawBricks();
        }
    }

    void drawUI()
    {
        std::string scoreText = "Score: " + std::to_string(score);
        std::string livesText = "Lives: " + std::to_string(ball.lives);

        Color livesColor = TEXT_COLOR;
        if (ball.lives == 2) livesColor = ORANGE;
        else if (ball.lives == 1) livesColor = RED;

        switch (state)
        {
            case GameState::Menu:
                textManager.drawCenterText("Block Breaker Game", 50, TEXT_COLOR, 0, -300);
                textManager.drawCenterText("Press 'Enter' To Start...", 25, TEXT_COLOR, 0, -250);
                textManager.drawCenterText("Press 'H' To Go To High Scores", 25, TEXT_COLOR, 0, -220);
                break;
            case GameState::HighScores:
                textManager.drawCenterText("High Scores:", 50, TEXT_COLOR, 0, -300);
                break;
            case GameState::Playing:
                textManager.drawLeftText(scoreText.c_str(), 25, TEXT_COLOR, 20, -400);
                textManager.drawLeftText(livesText.c_str(), 25, livesColor, 20, -370);
                break;
            case GameState::Paused:
                textManager.drawCenterText("Paused", 50, TEXT_COLOR, 0, -300);
                textManager.drawCenterText("Press 'Enter' to return back to game", 25, TEXT_COLOR, 20, -250);
                textManager.drawCenterText("Press 'B' To Go Back To Menu", 25, TEXT_COLOR, 20, -200);
                textManager.drawCenterText("Press 'R' To Reset The Game", 25, TEXT_COLOR, 20, -165);
                break;
        }
    }
};

// Main function - initializes and runs the game
int main()
{
    Game game;
}