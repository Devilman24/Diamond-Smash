#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>

// Game states
enum class GameState {
    Welcome,
    Playing,
    GameOver
};

class Player {
public:
    sf::RectangleShape paddle;
    float speed = 6.f;
    Player(float x, float y) {
        paddle.setSize({10, 100});
        paddle.setPosition(x, y);
        paddle.setFillColor(sf::Color::White);
    }
    void move(float dy) {
        float newY = paddle.getPosition().y + dy;
        if (newY < 0) newY = 0;
        if (newY > 600 - paddle.getSize().y) newY = 600 - paddle.getSize().y;
        paddle.setPosition(paddle.getPosition().x, newY);
    }
};

class Ball {
public:
    sf::CircleShape ball;
    sf::Vector2f velocity;
    Ball(float x, float y) {
        ball.setRadius(10);
        ball.setPosition(x, y);
        ball.setFillColor(sf::Color::White);
        velocity = {-5.f, 5.f};
    }
    void move() {
        ball.move(velocity);
        if (ball.getPosition().y <= 0 || ball.getPosition().y + ball.getRadius() * 2 >= 600)
            velocity.y = -velocity.y;
    }
    void reset(float x, float y) {
        ball.setPosition(x, y);
        velocity = {(rand() % 2 == 0 ? -5.f : 5.f), (rand() % 2 == 0 ? -5.f : 5.f)};
    }
};

int main() {
    sf::RenderWindow window({800, 600}, "Diamond Smash");
    window.setFramerateLimit(60);

    sf::Font font;
    if (!font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf")) {
        std::cerr << "Error loading font\n";
        return -1;
    }

    GameState state = GameState::Welcome;

    std::string playerName;
    bool enterNameComplete = false;
    bool vsAI = true; // Enforce AI mode

    Player player1(50, 250);
    Player player2(740, 250);
    Ball ball(400, 300);

    int score1 = 0, score2 = 0;
    bool gameOver = false;

    // Dark blue background
    sf::RectangleShape background({800, 600});
    background.setFillColor(sf::Color(20, 30, 70));

    sf::Text title("Welcome to Diamond Smash", font, 40);
    title.setPosition(150, 50);

    // Title shadow for 3D effect
    sf::Text titleShadow = title;
    titleShadow.setFillColor(sf::Color(10, 10, 30));
    titleShadow.setPosition(title.getPosition().x + 3, title.getPosition().y + 3);

    sf::Text promptName("Enter your name: ", font, 30);
    promptName.setPosition(200, 200);

    sf::Text nameText("", font, 30);
    nameText.setPosition(200, 250);

    sf::Text infoText("Press Enter to continue", font, 20);
    infoText.setPosition(280, 380);

    sf::Text scoreText("", font, 40);
    scoreText.setPosition(370, 10);

    sf::Text winnerText("", font, 50);
    winnerText.setPosition(200, 250);
    winnerText.setFillColor(sf::Color::Yellow);

    sf::Clock clock; // For blinking effect

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (state == GameState::Welcome) {
                // Player name input
                if (!enterNameComplete) {
                    if (event.type == sf::Event::TextEntered) {
                        if (event.text.unicode == 8) { // Backspace
                            if (!playerName.empty()) playerName.pop_back();
                        } else if (event.text.unicode < 128 && playerName.size() < 15) {
                            char entered = static_cast<char>(event.text.unicode);
                            if (isalnum(entered) || entered == ' ')
                                playerName += entered;
                        }
                        nameText.setString(playerName);
                    } else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Enter) {
                        if (!playerName.empty()) {
                            enterNameComplete = true;
                            state = GameState::Playing; // Start the game directly against AI
                        }
                    }
                }
            }
        }

        window.clear();

        if (state == GameState::Welcome) {
            window.draw(background);

            // Draw shadow first for 3D effect
            window.draw(titleShadow);
            window.draw(title);

            // Dynamically center text
            float promptWidth = promptName.getLocalBounds().width;
            promptName.setPosition((800 - promptWidth) / 2, 200);

            float nameWidth = nameText.getLocalBounds().width;
            nameText.setPosition((800 - nameWidth) / 2, 250);

            window.draw(promptName);
            window.draw(nameText);

            // Blinking instruction text
            if (!enterNameComplete) {
                float t = clock.getElapsedTime().asSeconds();
                if (static_cast<int>(t * 2) % 2 == 0) { // Blinks twice per second
                    window.draw(infoText);
                }
            }
        }
        else if (state == GameState::Playing) {
            if (!gameOver) {
                // Player 1 control
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) player1.move(-player1.speed);
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) player1.move(player1.speed);

                // Simple AI for Player 2
                if (vsAI) {
                    float paddleCenter = player2.paddle.getPosition().y + player2.paddle.getSize().y / 2;
                    float ballCenter = ball.ball.getPosition().y + ball.ball.getRadius();
                    if (ball.velocity.x > 0) {
                        if (ballCenter < paddleCenter - 10) player2.move(-player2.speed * 0.8f);
                        else if (ballCenter > paddleCenter + 10) player2.move(player2.speed * 0.8f);
                    }
                }

                ball.move();

                // Paddle collisions
                if (ball.ball.getGlobalBounds().intersects(player1.paddle.getGlobalBounds())) {
                    ball.velocity.x = std::abs(ball.velocity.x);
                    float paddleCenter = player1.paddle.getPosition().y + player1.paddle.getSize().y / 2;
                    float ballCenter = ball.ball.getPosition().y + ball.ball.getRadius();
                    ball.velocity.y = (ballCenter - paddleCenter) * 0.3f;
                }
                if (ball.ball.getGlobalBounds().intersects(player2.paddle.getGlobalBounds())) {
                    ball.velocity.x = -std::abs(ball.velocity.x);
                    float paddleCenter = player2.paddle.getPosition().y + player2.paddle.getSize().y / 2;
                    float ballCenter = ball.ball.getPosition().y + ball.ball.getRadius();
                    ball.velocity.y = (ballCenter - paddleCenter) * 0.3f;
                }

                // Score management
                if (ball.ball.getPosition().x < 0) {
                    score2++;
                    ball.reset(400, 300);
                }
                if (ball.ball.getPosition().x > 800) {
                    score1++;
                    ball.reset(400, 300);
                }

                int maxScore = 5;
                if (score1 >= maxScore) {
                    gameOver = true;
                    winnerText.setString(playerName + " wins!");
                }
                else if (score2 >= maxScore) {
                    gameOver = true;
                    winnerText.setString("AI wins!");
                }

                scoreText.setString(std::to_string(score1) + " - " + std::to_string(score2));
            } else {
                // Game over, press space to replay
                sf::Text replay("Press SPACE to replay", font, 25);
                replay.setPosition(250, 350);
                window.draw(replay);

                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
                    score1 = 0; score2 = 0; gameOver = false;
                    ball.reset(400, 300);
                    player1.paddle.setPosition(50, 250);
                    player2.paddle.setPosition(740, 250);
                }
            }

            window.draw(player1.paddle);
            window.draw(player2.paddle);
            window.draw(ball.ball);
            window.draw(scoreText);
            if (gameOver) window.draw(winnerText);
        }

        window.display();
    }

    return 0;
}
