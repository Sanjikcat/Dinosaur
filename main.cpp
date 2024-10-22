#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <sstream>

using namespace std;
using namespace sf;

const int WINDOW_WIDTH = 1000;
const int WINDOW_HEIGHT = 800;
const int LANE_WIDTH = WINDOW_WIDTH / 3;
const int PLAYER_WIDTH = 40;
const int PLAYER_HEIGHT = 40;
const int OBSTACLE_WIDTH = 80;
const int OBSTACLE_HEIGHT = 30;
const float OBSTACLE_SPEED = 0.6f;
const float SPAWN_INTERVAL = 2.0f;
const float MOVE_COOLDOWN = 0.3f;
const int POINTS_PER_TIME = 1;

enum GameState { PLAYING, GAME_OVER };

int main() {
    srand(static_cast<unsigned>(time(0)));
    RenderWindow window(VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Dino");

    RectangleShape player(Vector2f(PLAYER_WIDTH, PLAYER_HEIGHT));
    player.setFillColor(Color::Green);

    RectangleShape road(Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
    road.setFillColor(Color::Black);

    vector<RectangleShape> laneMarks;
    for (int i = 1; i < 3; ++i) {
        RectangleShape laneMark(Vector2f(5, WINDOW_HEIGHT));
        laneMark.setFillColor(Color::White);
        laneMark.setPosition(i * LANE_WIDTH, 0);
        laneMarks.push_back(laneMark);
    }

    int lane = 1;
    player.setPosition(LANE_WIDTH * lane + LANE_WIDTH / 2 - PLAYER_WIDTH / 2, WINDOW_HEIGHT - PLAYER_HEIGHT - 50);

    vector<RectangleShape> obstacles;
    float spawnTimer = 0.0f;
    float moveTimer = 0.0f;

    int score = 0;
    Font font;
    Text scoreText;

    Music backgroundMusic;
    if (!backgroundMusic.openFromFile("untitled.wav")) {
        return -1;
    }
    backgroundMusic.setLoop(true);
    backgroundMusic.play();

    if (!font.loadFromFile("Far Cry Cyr.ttf")) {
        return -1;
    }
    scoreText.setFont(font);
    scoreText.setCharacterSize(24);
    scoreText.setFillColor(Color::White);
    scoreText.setPosition(WINDOW_WIDTH - 150, 10);

    GameState gameState = PLAYING;
    Text gameOverText;
    gameOverText.setFont(font);
    gameOverText.setCharacterSize(30);
    gameOverText.setFillColor(Color::White);
    gameOverText.setString("Game Over! Press Space to restart.");
    gameOverText.setPosition(WINDOW_WIDTH / 2 - 300, WINDOW_HEIGHT / 2 - 50);

    Clock clock;
    int lastObstacleLane = -1;

    while (window.isOpen()) {
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed) {
                window.close();
            }
        }

        float deltaTime = clock.restart().asSeconds();
        spawnTimer += deltaTime;
        moveTimer += deltaTime;

        if (gameState == GAME_OVER) {
            if (Keyboard::isKeyPressed(Keyboard::Space)) {
                gameState = PLAYING;
                score = 0;
                obstacles.clear();
                lane = 1;
                player.setPosition(LANE_WIDTH * lane + LANE_WIDTH / 2 - PLAYER_WIDTH / 2, WINDOW_HEIGHT - PLAYER_HEIGHT - 50);
                lastObstacleLane = -1;
            }
            continue;
        }

        if (gameState == PLAYING) {
            score += POINTS_PER_TIME;

            static float targetPosition = player.getPosition().x;
            static float currentPosition = player.getPosition().x;
            float movementSpeed = 400.0f * deltaTime;

            if (Keyboard::isKeyPressed(Keyboard::Left) && moveTimer >= MOVE_COOLDOWN && lane > 0) {
                lane--;
                targetPosition = LANE_WIDTH * lane + LANE_WIDTH / 2 - PLAYER_WIDTH / 2;
                moveTimer = 0.0f;
            }
            if (Keyboard::isKeyPressed(Keyboard::Right) && moveTimer >= MOVE_COOLDOWN && lane < 2) {
                lane++;
                targetPosition = LANE_WIDTH * lane + LANE_WIDTH / 2 - PLAYER_WIDTH / 2;
                moveTimer = 0.0f;
            }

            if (currentPosition != targetPosition) {
                currentPosition += (targetPosition - currentPosition) * movementSpeed;
                player.setPosition(currentPosition, player.getPosition().y);
            }

            if (spawnTimer > SPAWN_INTERVAL) {
                spawnTimer = 0.0f;
                int obstacleLane;
                do {
                    obstacleLane = rand() % 3;
                } while (obstacleLane == lastObstacleLane);
                lastObstacleLane = obstacleLane;

                RectangleShape newObstacle(Vector2f(OBSTACLE_WIDTH, OBSTACLE_HEIGHT));
                newObstacle.setFillColor(Color::Red);
                newObstacle.setPosition(LANE_WIDTH * obstacleLane + LANE_WIDTH / 2 - OBSTACLE_WIDTH / 2, -OBSTACLE_HEIGHT);
                obstacles.push_back(newObstacle);
            }

            for (auto& obst : obstacles) {
                obst.move(0, OBSTACLE_SPEED);
            }

            for (auto& obst : obstacles) {
                if (player.getGlobalBounds().intersects(obst.getGlobalBounds())) {
                    gameState = GAME_OVER;
                    break;
                }
            }

            obstacles.erase(remove_if(obstacles.begin(), obstacles.end(),
                [](RectangleShape& obst) { return obst.getPosition().y > WINDOW_HEIGHT; }),
                obstacles.end());
        }

        stringstream ss;
        ss << "Score: " << score;
        scoreText.setString(ss.str());

        window.clear();
        window.draw(road);
        for (const auto& laneMark : laneMarks) {
            window.draw(laneMark);
        }
        window.draw(player);
        for (auto& obst : obstacles) {
            window.draw(obst);
        }
        window.draw(scoreText);
        if (gameState == GAME_OVER) {
            window.draw(gameOverText);
        }
        window.display();
    }

    return 0;
}
