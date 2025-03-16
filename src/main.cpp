#include <string>
#include <raylib.h>
#include <iostream>

// Constants for screen size and grid properties
constexpr int TILE_SIZE = 30;      // Size of each tile in pixels
constexpr int TILE_AMOUNT = 30;    // Number of tiles along each axis
constexpr int GRID_SIZE = TILE_SIZE * TILE_AMOUNT; // Grid dimension (900px)
constexpr int SCREEN_WIDTH = GRID_SIZE;  // Screen width in pixels
constexpr int SCREEN_HEIGHT = GRID_SIZE; // Screen height in pixels

int score = 0; // Initializes the score
int lives = 100; // initialized the lives

// Enum to define different game states
enum e_GameState { MENU, PLAYING, VICTORY, PAUSED };
enum e_BallState { STUCK, MOVING, POWER_UP };

// Structure for managing text rendering
struct TextManager
{
    /**
     * Draws centered text/to the left/to the right with optional padding adjustments.
     *
     * @param text The text to display.
     * @param fontSize The font size for the text.
     * @param color The color of the text.
     * @param paddingX Horizontal padding for fine-tuning position.
     * @param paddingY Vertical padding for fine-tuning position.
     */
    void drawCenterText(const char* text, int fontSize, Color color, int paddingX, int paddingY) const
    {   
        int textWidth = MeasureText(text, fontSize); // Calculate text width in pixels
        int centerX = (SCREEN_WIDTH / 2) - (textWidth / 2) + paddingX; // Center X coordinate
        int centerY = (SCREEN_HEIGHT / 2) - (fontSize / 2) + paddingY; // Center Y coordinate

        DrawText(text, centerX, centerY, fontSize, color); // Render the text
    }
    void drawLeftText(const char* text, int fontSize, Color color, int paddingX, int paddingY) const
    {   
        int centerY = (SCREEN_HEIGHT / 2) - (fontSize / 2) + paddingY; // Center Y coordinate
        DrawText(text, paddingX, centerY, fontSize, color); // Render text at fixed X position
    }
};

// Structure for managing the paddle
struct Paddle
{
    Vector2 position;
    float width = TILE_SIZE * 4, height = TILE_SIZE / 2;
    float speed = 6.0f;

    // Constructor initializes the paddle at the bottom center of the screen
    Paddle()
    {
        position = { (SCREEN_WIDTH / 2) - (width / 2), SCREEN_HEIGHT - height - 10 };
    }

    // Draws the paddle
    void draw()
    {
        DrawRectangleV(position, { width, height }, GREEN);
    }

    // Updates paddle movement based on user input
    void update()
    {
        if(IsKeyDown(KEY_LEFT) && position.x > 0) // Moves left if within bounds
            position.x -= speed;
        if(IsKeyDown(KEY_RIGHT) && position.x + width < SCREEN_WIDTH) // Moves right if within bounds
            position.x += speed;
    }
};

// Structure for managing the ball
struct Ball
{
    e_BallState ballState = STUCK;

    Vector2 position;
    Vector2 velocity;

    float radius = 10.0f;
    float speed = 8.0f;

    // Initializes the ball above the paddle in a stationary state.
    Ball(const Paddle& paddle) 
    {
        position = { paddle.position.x + (paddle.width / 2), paddle.position.y - radius * 2 };
        velocity = { 0, 0 };
    }

    // Updates the ball's position and state.
    void update(const Paddle& paddle)
    {
        switch (ballState)
        {
        case STUCK:
            position.x = paddle.position.x + (paddle.width / 2); // Stick to paddle
            position.y = paddle.position.y - radius * 2;

            if(IsKeyPressed(KEY_SPACE)) // Launch the ball when space is pressed
            {
                velocity = { speed, -speed };
                ballState = MOVING;
            }
            break;

        case MOVING:
            position.x += velocity.x;
            position.y += velocity.y;

            if (position.x - radius <= 0 || position.x + radius >= SCREEN_WIDTH ) // Ball wall collision (LEFT / RIGHT)
                velocity.x *= -1; // Reverse X direction
            else if (position.y - radius <= 0) // Ball hits the ceiling
                velocity.y *= -1; // Reverse Y direction

            break;
        }
    }

    // Draws the ball
    void draw()
    {
        DrawCircleV(position, radius, WHITE);
    }
};

// Structure for managing game states and transitions
struct GameState
{
    e_GameState gameState = MENU;  // Default state: Menu
    TextManager textManager;       // Handles text rendering

    Paddle paddle;
    Ball ball;

    std::string scoreText = "Score: " + std::to_string(score); // Converts score text to string
    std::string livesText = "Lives: " + std::to_string(lives); // Converts lives text to string



    // Constructor initializes the game state and resets the game
    GameState() : ball(paddle)
    {
        resetGame();
    }

    // Resets the game elements to their initial state
    void resetGame()
    {
        paddle = Paddle();
        ball = Ball(paddle);
        ball.ballState = STUCK;

        score = 0;
        lives = 3;
        scoreText = "Score: " + std::to_string(score);
        livesText = "Lives: " + std::to_string(lives);
    }

    // Updates the game state and handles state transitions.
    void update()
    {
        switch (gameState)
        {
            case MENU:
                textManager.drawCenterText("Block Breaker Game", 30, GREEN, 0, 0);
                textManager.drawCenterText("Press 'Enter' To Play...", 30, GREEN, 0, 50);
                if (IsKeyPressed(KEY_ENTER)) gameState = PLAYING;
                break;

            case VICTORY:
                textManager.drawCenterText("You won!!!", 30, GREEN, 0, 0);
                textManager.drawCenterText("Press 'Q' to return to menu", 25, GREEN, 0, 50);
                textManager.drawCenterText("Or press 'R' to play again", 25, GREEN, 0, 25);
                if (IsKeyPressed(KEY_Q)) gameState = MENU;
                else if (IsKeyPressed(KEY_R)) resetGame();
                break;

            case PLAYING:
                textManager.drawLeftText(scoreText.c_str(), 25, GREEN, 20, -400);
                textManager.drawLeftText(livesText.c_str(), 25, GREEN, 20, -370);
                paddle.update();
                paddle.draw();
                ball.update(paddle);
                ball.draw();
                if (IsKeyPressed(KEY_V)) gameState = VICTORY;
                else if (IsKeyPressed(KEY_O)) gameState = PAUSED;
                break;

            case PAUSED:
                textManager.drawCenterText("Paused", 30, GREEN, 0, 0);
                textManager.drawCenterText("Press 'Q' to return to menu", 25, GREEN, 0, 20);
                textManager.drawCenterText("Press 'R' to reset the game", 25, GREEN, 0, 50);
                textManager.drawCenterText("Press 'O' to return to the game", 25, GREEN, 0, 100);
                if (IsKeyPressed(KEY_Q)) gameState = MENU;
                else if (IsKeyPressed(KEY_R)) { resetGame(); gameState = PLAYING; }
                else if (IsKeyPressed(KEY_O)) gameState = PLAYING;
                break;  
        }
    }
};

// Main function - initializes and runs the game
int main()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Block Breaker");
    SetTargetFPS(60);

    GameState gameState;

    // Main game loop
    while (!WindowShouldClose()) 
    {
        BeginDrawing();
        ClearBackground(BLACK);
        gameState.update();
        EndDrawing();
    }

    CloseWindow();
}