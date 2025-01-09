#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Window.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <string>
#include <sstream>
#include <iostream> // Для отладочных сообщений

class Bullet {
public:
    sf::CircleShape shape;
    float speed;

    Bullet(float x, float y) {
        shape.setRadius(5);
        shape.setFillColor(sf::Color::Yellow);
        shape.setPosition(x, y);
        speed = 10.0f;
    }

    void move() {
        shape.move(0, -speed);
    }

    bool isOffScreen() {
        return shape.getPosition().y < 0;
    }
};
class Cargo {
public:
    sf::CircleShape shape;
    float speed;

    Cargo(float x, float y) {
        shape.setRadius(10);
        shape.setFillColor(sf::Color::White);
        shape.setPosition(x, y);
        speed = 1.7f; // Скорость падения груза
    }

    void move() {
        shape.move(0, speed);
    }

    bool isOffScreen(const sf::RenderWindow& window) {
        return shape.getPosition().y > window.getSize().y;
    }
};

class Enemy {
public:
    enum EnemyType {
        SMALL_SQUARE,
        SMALL_CIRCLE,
        SQUARE,
        CIRCLE,
        BIG_SQUARE,
        BIG_CIRCLE,
        BOSS
    };

    sf::Shape* shape = nullptr;
    sf::Shape* shapeboss = nullptr;
    float speed;
    int health;
    int maxHealth;

    Enemy(float x, float y, int hp, EnemyType type) : health(hp), maxHealth(hp) {
        speed = 2.0f;

        // Создание врагов в зависимости от типа
        switch (type) {
        case SMALL_SQUARE:
            shape = new sf::RectangleShape(sf::Vector2f(30, 30));
            shape->setFillColor(sf::Color::Yellow);
            break;
        case SMALL_CIRCLE:
            shape = new sf::CircleShape(15);
            shape->setFillColor(sf::Color::Cyan);
            break;
        case SQUARE:
            shape = new sf::RectangleShape(sf::Vector2f(50, 50));
            shape->setFillColor(sf::Color::Red);
            break;
        case CIRCLE:
            shape = new sf::CircleShape(25);
            shape->setFillColor(sf::Color::Green);
            break;
        case BIG_SQUARE:
            shape = new sf::RectangleShape(sf::Vector2f(70, 70));
            shape->setFillColor(sf::Color::Blue);
            break;
        case BIG_CIRCLE:
            shape = new sf::CircleShape(35);
            shape->setFillColor(sf::Color::Magenta);
            break;
        case BOSS:
            shape = new sf::RectangleShape(sf::Vector2f(200, 200));
            shape->setFillColor(sf::Color::Blue);
            break;
        }


        shape->setPosition(x, y);
    }

    virtual void move() {
        shape->move(0, speed);
    }

    virtual bool isOffScreen(const sf::RenderWindow& window) {
        return shape->getPosition().y > window.getSize().y;
    }

    void decreaseHealth(int damage) {
        health -= damage;
        if (health < 0) health = 0;
        updateColor();
    }

    void updateColor() {
        float healthPercentage = static_cast<float>(health) / maxHealth;
        if (healthPercentage > 0.66f) {
            shape->setFillColor(sf::Color::Yellow);
        }
        else if (healthPercentage > 0.33f) {
            shape->setFillColor(sf::Color::Magenta);
        }
        else {
            shape->setFillColor(sf::Color::Red);
        }
    }

    virtual ~Enemy() {
        delete shape;
    }
};

class Game {
public:

    std::vector<Cargo*> cargos; // Вектор для хранения грузов
    int pickedUpCargos = 0; // Счетчик поднятых грузов
    int deliveredCargos = 0; // Счетчик доставленных грузов
    int bestDeliveredCargos = 0; // Лучшее количество доставленных грузовъ

    sf::RenderWindow window;
    sf::RectangleShape player;
    std::vector<Bullet> bullets;
    std::vector<Enemy*> enemies;

    sf::Clock shootClock;
    const float initialShootCooldown = 0.1f;
    float shootCooldown;

    int playerHealth = 5;
    int score = 0;
    int highScore = 0;
    int highLevel = 1;
    bool isGameOver = false;
    bool isMenu = true;
    bool isPaused = false;

    sf::Font font;
    sf::Text gameOverText;
    sf::Text scoreText;
    sf::Text highScoreText;
    sf::Text highLevelText;
    sf::Text healthText;
    sf::Text startGameText;
    sf::Text difficultyLevelText;
    sf::Text fpsText;
    sf::Text cargoCountText; // Текст для отображения количества грузов
    std::vector<sf::Text> titleLetters;

    // Кнопки для улучшений
    sf::Text damageButton;
    sf::Text fireRateButton;
    sf::Text healthButton;

    // Пауза меню
    sf::Text pauseMenuText;
    sf::Text continueButton;
    sf::Text menuButton;
    sf::Text restartButton;
    sf::Text exitButton;

    sf::SoundBuffer hitBuffer;
    sf::Sound hitSound;
    sf::SoundBuffer pikmiBuffer;
    sf::Sound pikmi;
    sf::SoundBuffer delivedBuffer;
    sf::Sound delived;
    sf::SoundBuffer hpHitBuffer;
    sf::Sound hpHit;
    sf::Music backgroundMusic;

    int difficultyLevel = 1;
    sf::Clock difficultyClock;
    int bulletDamage = 1;

    // Переменная для отслеживания выбранного пункта меню
    int menuSelection = 0; // 0 - Start Game, 1 - High Score, 2 - Exit
    int upgradeSelection = 0; // 0 - Damage, 1 - Fire Rate, 2 - Health

    Game() : window(sf::VideoMode::getFullscreenModes()[0], "Space Shooter", sf::Style::Fullscreen), shootCooldown(initialShootCooldown) {
        sf::Image icon;
        if (!icon.loadFromFile("icon.png"))
        {
            return;
        }
        window.setIcon(32, 32, icon.getPixelsPtr());
        window.setFramerateLimit(60);
        player.setSize(sf::Vector2f(50, 50));
        player.setFillColor(sf::Color::Cyan);
        player.setPosition(window.getSize().x / 2 - 25, window.getSize().y - 100);

        if (!font.loadFromFile("Fonts/arial.ttf")) {}
        if (!hitBuffer.loadFromFile("Audio/bullet.wav")) {}
        hitSound.setBuffer(hitBuffer);
        if (!pikmiBuffer.loadFromFile("Audio/cargo.wav")) {}
        pikmi.setBuffer(pikmiBuffer);
        if (!delivedBuffer.loadFromFile("Audio/delived.wav")) {}
        delived.setBuffer(delivedBuffer);
        if (!hpHitBuffer.loadFromFile("Audio/hit.wav")) {}
        hpHit.setBuffer(hpHitBuffer);

        if (!backgroundMusic.openFromFile("Audio/music.ogg")) {}
        backgroundMusic.setLoop(true);
        backgroundMusic.play();

        std::string title = "SPACE SHOOTER";
        std::vector<sf::Color> colors = { sf::Color::Blue, sf::Color::Green, sf::Color::Red };
        for (size_t i = 0; i < title.size(); ++i) {
            sf::Text letter;
            letter.setFont(font);
            letter.setString(std::string(1, title[i]));
            letter.setCharacterSize(100);
            letter.setFillColor(colors[i % colors.size()]);
            letter.setPosition(window.getSize().x / 2 - title.length() * 40 + i * 80, 20);
            titleLetters.push_back(letter);
        }

        gameOverText.setFont(font);
        gameOverText.setString("Game Over\nPress Down to Restart");
        gameOverText.setCharacterSize(60);
        gameOverText.setFillColor(sf::Color::White);
        gameOverText.setPosition(window.getSize().x / 2 - 200, window.getSize().y / 2 - 100);

        scoreText.setFont(font);
        scoreText.setCharacterSize(24);
        scoreText.setFillColor(sf::Color::Magenta);
        scoreText.setPosition(10, 10);

        healthText.setFont(font);
        healthText.setCharacterSize(30);
        healthText.setFillColor(sf::Color::Red);
        healthText.setPosition(10, window.getSize().y - 50);

        cargoCountText.setFont(font);
        cargoCountText.setCharacterSize(24);
        cargoCountText.setFillColor(sf::Color::White);
        cargoCountText.setPosition(10, window.getSize().y - 80); // Позиция над здоровьем




        difficultyLevelText.setFont(font);
        difficultyLevelText.setCharacterSize(50); // Увеличиваем размер текста уровня
        difficultyLevelText.setFillColor(sf::Color::Cyan);
        difficultyLevelText.setPosition(window.getSize().x - 250, 10); // Смещаем текст ближе к правому верхнему углу

        startGameText.setFont(font);
        startGameText.setString("Press Enter to Start Game");
        startGameText.setCharacterSize(30);
        startGameText.setFillColor(sf::Color::Red);
        startGameText.setPosition(window.getSize().x / 2 - 150, window.getSize().y / 2 - 15);

        highScoreText.setFont(font);
        loadHighScoreAndLevel();
        highScoreText.setString("High Score: " + std::to_string(highScore));
        highScoreText.setCharacterSize(24);
        highScoreText.setFillColor(sf::Color::Cyan);
        highScoreText.setPosition(window.getSize().x / 2 - highScoreText.getGlobalBounds().width / 2, window.getSize().y - 50);

        highLevelText.setFont(font);
        highLevelText.setString("Best Level: " + std::to_string(highLevel));
        highLevelText.setCharacterSize(24);
        highLevelText.setFillColor(sf::Color::Red);
        highLevelText.setPosition(window.getSize().x / 2 - highLevelText.getGlobalBounds().width / 2, window.getSize().y - 80);

        fpsText.setFont(font);
        fpsText.setCharacterSize(24);
        fpsText.setFillColor(sf::Color::White);
        fpsText.setPosition(window.getSize().x - 150, window.getSize().y - 50);

        // Инициализация кнопок
        damageButton.setFont(font);
        damageButton.setString("Damage +1 (Cost: 5)");
        damageButton.setCharacterSize(20);
        damageButton.setFillColor(sf::Color::White);
        damageButton.setPosition(10, 100);

        fireRateButton.setFont(font);
        fireRateButton.setString("Fire Rate -0.0005 (Cost: 5)");
        fireRateButton.setCharacterSize(20);
        fireRateButton.setFillColor(sf::Color::White);
        fireRateButton.setPosition(10, 130);

        healthButton.setFont(font);
        healthButton.setString("Health +1 (Cost: 10)");
        healthButton.setCharacterSize(20);
        healthButton.setFillColor(sf::Color::White);
        healthButton.setPosition(10, 160);

        // Пауза меню
        pauseMenuText.setFont(font);
        pauseMenuText.setString("Paused");
        pauseMenuText.setCharacterSize(70);
        pauseMenuText.setFillColor(sf::Color::White);
        pauseMenuText.setPosition(window.getSize().x / 2 - 130, window.getSize().y / 2 - 300);

        continueButton.setFont(font);
        continueButton.setString("Continue");
        continueButton.setCharacterSize(30);
        continueButton.setFillColor(sf::Color::Red);
        continueButton.setPosition(window.getSize().x / 2 - 70, window.getSize().y / 2 - 50);

        menuButton.setFont(font);
        menuButton.setString("Menu");
        menuButton.setCharacterSize(30);
        menuButton.setFillColor(sf::Color::Green);
        menuButton.setPosition(window.getSize().x / 2 - 40, window.getSize().y / 2);

        restartButton.setFont(font);
        restartButton.setString("Restart");
        restartButton.setCharacterSize(30);
        restartButton.setFillColor(sf::Color::Cyan);
        restartButton.setPosition(window.getSize().x / 2 - 60, window.getSize().y / 2 + 50);

        exitButton.setFont(font);
        exitButton.setString("Exit");
        exitButton.setCharacterSize(30);
        exitButton.setFillColor(sf::Color::Yellow);
        exitButton.setPosition(window.getSize().x / 2 - 40, window.getSize().y / 2 + 100);

        // Добавляем текст управления в меню
        sf::Text controlsText;
        controlsText.setFont(font);
        controlsText.setString("Controls:\nMove: Arrow Keys\nShoot: Up Arrow\nPause: Escape\nUpgrade All: Space\nUpgrade: 1, 2, 3\nSelecting buttons in the pause menu: 1, 2, 3, 4");
        controlsText.setCharacterSize(20);
        controlsText.setFillColor(sf::Color::White);
        controlsText.setPosition(window.getSize().x / 2 - 150, window.getSize().y / 2 + 50);
        titleLetters.push_back(controlsText); // Добавляем текст управления в вектор
        loadBestDeliveredCargos(); // Загружаем лучшее количество доставленных грузов

    }
    int bosses = 1;
    void run() {
        sf::Clock clock;
        while (window.isOpen()) {
            handleEvents();
            if (!isGameOver && !isMenu && !isPaused) {
                update();
            }
            render();

            float deltaTime = clock.restart().asSeconds();
            int fps = static_cast<int>(1.0f / deltaTime);
            fpsText.setString("FPS: " + std::to_string(fps));
        }
    }

    void handleEvents() {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        if (isMenu) {
            loadHighScoreAndLevel();
            
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
                menuSelection = (menuSelection - 1 + 3) % 3; // Циклический переход вверх
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
                menuSelection = (menuSelection + 1) % 3; // Циклический переход вниз
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
                menuSelection = (menuSelection + 1) % 3; // Циклический переход вниз
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Enter)) {
                if (menuSelection == 0) {
                    isMenu = false; // Начинаем игру
                }
                // Дополнительные действия для других пунктов меню
                else if (menuSelection == 1) {
                    // Действие для High Score (например, показать высокие очки)
                }
                else if (menuSelection == 2) {
                    window.close(); // Закрыть игру
                }
            }
            return;
        }

        if (isGameOver) {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
                restartGame();
            }
            return;
        }

        if (isPaused) {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num1)) {
                isPaused = false; // Возвращаемся в игру
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num3)) {
                restartGame();
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num2)) {
                isMenu = true; // Возвращаемся в меню
                isPaused = false; // Убираем паузу
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num4)) {
                window.close();
            }
            return;
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {
            isPaused = true; // Пауза игры
            return;
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) && player.getPosition().x > 0) {
            player.move(-5.0f, 0);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) && player.getPosition().x < window.getSize().x - player.getSize().x) {
            player.move(5.0f, 0);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) && player.getPosition().x > 0) {
            player.move(-5.0f, 0);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) && player.getPosition().x < window.getSize().x - player.getSize().x) {
            player.move(5.0f, 0);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) && shootClock.getElapsedTime().asSeconds() >= shootCooldown) {
            bullets.emplace_back(player.getPosition().x + 22.5f, player.getPosition().y);
            shootClock.restart();
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) && shootClock.getElapsedTime().asSeconds() >= shootCooldown) {
            bullets.emplace_back(player.getPosition().x + 22.5f, player.getPosition().y);
            shootClock.restart();
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
            upgradePlayer();
        }

        // Управление улучшениями с помощью клавиш
        if (!isMenu) {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num1)) {
                buyDamageUpgrade();
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num2)) {
                buyFireRateUpgrade();
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num3)) {
                buyHealthUpgrade();
            }
            //читы
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Numpad1)) {
                difficultyLevel += 1;
                std::cerr << "difficultyLevel + 1" << std::endl; // Отладочное сообщение
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Numpad2)) {
                difficultyLevel -= 1;
                std::cerr << "difficultyLevel - 1" << std::endl; // Отладочное сообщение
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Numpad3)) {
                score += 1;
                std::cerr << "score + 1" << std::endl; // Отладочное сообщение
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Numpad4)) {
                enemies.push_back(new Enemy(player.getPosition().x, -50, 50 + health, Enemy::BOSS));
                std::cerr << "Spawn boss" << std::endl; // Отладочное сообщение
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Numpad5)) {
                enemies.clear();
                std::cerr << "Clear" << std::endl; // Отладочное сообщение
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::O)) {
                std::cerr << "difficultyClock restart" << std::endl;
                float x = static_cast<float>(rand() % (static_cast<int>(window.getSize().x) - 20));
                cargos.push_back(new Cargo(x, -20)); // Создаем новый груз// Отладочное сообщение
                difficultyClock.restart();
            }
        }
    }

    int health = 0;

    void update() {
        cargoCountText.setString("Picked Up: " + std::to_string(pickedUpCargos) +
            " Delivered: " + std::to_string(deliveredCargos));

        // Проверка времени для перехода на следующий уровень
        if (difficultyClock.getElapsedTime().asSeconds() >= 30) {
            bosses = 1;
            deliveredCargos += pickedUpCargos; // Переносим поднятые грузы в доставленные
            pickedUpCargos = 0; // Обнуляем счетчик поднятых грузов
            difficultyLevel++; // Увеличиваем уровень сложности
            difficultyClock.restart(); // Сбрасываем таймер
            delived.play();
        }

        if (deliveredCargos > bestDeliveredCargos) {
            bestDeliveredCargos = deliveredCargos;
        }

        // Спавн грузов на основе уровня сложности
        if (rand() % 100 < 2) { // Вероятность появления грузов
            float x = static_cast<float>(rand() % (static_cast<int>(window.getSize().x) - 20));
            cargos.push_back(new Cargo(x, -20)); // Создаем новый груз
        }

        // Обновление грузов
        for (size_t i = 0; i < cargos.size(); ++i) {
            cargos[i]->move();
            if (cargos[i]->isOffScreen(window)) {
                delete cargos[i];
                cargos.erase(cargos.begin() + i);
                --i;
            }
        }

        // Проверка на столкновение с грузами
        for (size_t i = 0; i < cargos.size(); ++i) {
            if (player.getGlobalBounds().intersects(cargos[i]->shape.getGlobalBounds())) {
                pickedUpCargos++; // Увеличиваем счетчик поднятых грузов
                delete cargos[i];
                cargos.erase(cargos.begin() + i);
                pikmi.play();
                --i;
            }
        }

        // Обновление текста уровня
        difficultyLevelText.setString("Level: " + std::to_string(difficultyLevel));

        for (size_t i = 0; i < bullets.size(); ++i) {
            bullets[i].move();
            if (bullets[i].isOffScreen()) {
                bullets.erase(bullets.begin() + i);
                --i;
            }
        }

        // Спавн врагов на основе уровня сложности
        if (rand() % 100 < 10 + (difficultyLevel * 2)) {
            float x = static_cast<float>(rand() % (static_cast<int>(window.getSize().x) - 50)); // Вероятность появления врагов зависит от уровня
            Enemy::EnemyType enemyType;
            if (bosses == 1 && difficultyLevel >= 2) { // Спавн Босса
                enemies.push_back(new Enemy(window.getSize().x / 2 - 500, -50, 50 + health, Enemy::BOSS));
                enemies.push_back(new Enemy(window.getSize().x / 2 + 500, -50, 50 + health, Enemy::BOSS));
                enemies.push_back(new Enemy(window.getSize().x / 2, -50, 50 + health, Enemy::BOSS));
                enemies.push_back(new Enemy(0, -50, 50 + health, Enemy::BOSS));
                bosses += 1;
                std::cerr << "Boss." << std::endl; // Отладочное сообщение
            }
            // Определяем тип врага в зависимости от уровня сложности
            if (difficultyLevel == 1) {
                enemyType = Enemy::SQUARE; // На первом уровне только квадрат
            }
            else if (difficultyLevel == 2) {
                enemyType = static_cast<Enemy::EnemyType>(rand() % 2 + 2); // На втором уровне квадрат или круг
            }
            else if (difficultyLevel == 3) {
                enemyType = static_cast<Enemy::EnemyType>(rand() % 3 + 3);
            }
            else {
                enemyType = static_cast<Enemy::EnemyType>(rand() % 6);
            }

            health = (enemyType + 1) * 3 + difficultyLevel * 2;
            enemies.push_back(new Enemy(x, -50, health, enemyType));
        }

        for (size_t i = 0; i < enemies.size(); ++i) {
            enemies[i]->move();
            if (enemies[i]->isOffScreen(window)) {
                delete enemies[i];
                enemies.erase(enemies.begin() + i);
                --i;
            }
        }

        for (size_t i = 0; i < bullets.size(); ++i) {
            for (size_t j = 0; j < enemies.size(); ++j) {
                if (bullets[i].shape.getGlobalBounds().intersects(enemies[j]->shape->getGlobalBounds())) {
                    enemies[j]->decreaseHealth(bulletDamage); // Урон пуль
                    hitSound.play();
                    bullets.erase(bullets.begin() + i);

                    // Начисление очков
                    if (enemies[j]->shape->getFillColor() == sf::Color::Blue && enemies[j]->health <= 0) { // Если это босс
                        score += 20; // 20 очков за босса
                    }
                    else if (enemies[j]->health <= 0) {
                        score += 1; // 1 очко за обычного врага
                    }

                    if (enemies[j]->health <= 0) {
                        delete enemies[j];
                        enemies.erase(enemies.begin() + j);
                    }
                    break;
                }
            }
        }

        for (size_t i = 0; i < enemies.size(); ++i) {
            if (player.getGlobalBounds().intersects(enemies[i]->shape->getGlobalBounds())) {
                playerHealth--;
                hpHit.play();
                delete enemies[i];
                enemies.erase(enemies.begin() + i);
                if (playerHealth <= 0) {
                    isGameOver = true;
                    saveHighScore();
                    saveHighLevel();
                }
                break;
            }
        }

        healthText.setString("Health: " + std::to_string(playerHealth));
        scoreText.setString("Score: " + std::to_string(score));

        float healthPercentage = static_cast<float>(playerHealth) / 5;
        if (healthPercentage > 0.66f) {
            healthText.setFillColor(sf::Color::Green);
        }
        else if (healthPercentage > 0.33f) {
            healthText.setFillColor(sf::Color::Yellow);
        }
        else {
            healthText.setFillColor(sf::Color::Red);
        }
    }


    void render() {
        window.clear();
        if (isMenu) {
            for (const auto& letter : titleLetters) {
                window.draw(letter);
            }
            window.draw(startGameText);
            window.draw(highScoreText);
            window.draw(highLevelText);

            sf::Text bestDeliveredText;
            bestDeliveredText.setFont(font);
            bestDeliveredText.setString("Best Delivered Cargos: " + std::to_string(bestDeliveredCargos));
            bestDeliveredText.setFillColor(sf::Color::Green);
            bestDeliveredText.setCharacterSize(24);
            bestDeliveredText.setPosition(window.getSize().x / 2 - bestDeliveredText.getGlobalBounds().width / 2, window.getSize().y - 110);
            window.draw(bestDeliveredText);

        }
        else if (isPaused) {
            window.draw(pauseMenuText);
            window.draw(continueButton);
            window.draw(menuButton);
            window.draw(restartButton);
            window.draw(exitButton);
        }
        else {
            window.draw(player);
            for (const auto& bullet : bullets) {
                window.draw(bullet.shape);
            }
            for (const auto& enemy : enemies) {
                window.draw(*enemy->shape);
            }
            for (const auto& cargo : cargos) {
                window.draw(cargo->shape);
            }
            if (isGameOver) {
                window.draw(gameOverText);
            }
            window.draw(scoreText);
            window.draw(healthText);
            window.draw(cargoCountText); // Отображаем текст с количеством грузов
            window.draw(difficultyLevelText);
            window.draw(fpsText);
            // Рисуем кнопки
            window.draw(damageButton);
            window.draw(fireRateButton);
            window.draw(healthButton);
        }
        window.display();
    }

    void loadBestDeliveredCargos() {
        std::ifstream file("Saves/best_delivered_cargos.carg");
        if (file.is_open()) {
            file >> bestDeliveredCargos;
            file.close();
        }
        else {
            std::cerr << "Error opening best_delivered_cargos.carg for reading." << std::endl; // Отладочное сообщение
        }
    }

    void saveBestDeliveredCargos() {
        std::ofstream file("Saves/best_delivered_cargos.carg");
        if (file.is_open()) {
            file << bestDeliveredCargos;
            file.close();
            std::cout << "New best delivered cargos saved: " << bestDeliveredCargos << std::endl; // Отладочное сообщение
        }
        else {
            std::cerr << "Error opening best_delivered_cargos.carg for writing." << std::endl; // Отладочное сообщение
        }
    }


    void restartGame() {
        playerHealth = 5;
        score = 0;
        shootCooldown = initialShootCooldown;
        difficultyLevel = 1;
        bulletDamage = 1; // Сброс урона пуль
        bosses = 1;
        difficultyClock.restart();
        isGameOver = false;
        isPaused = false; // Сбрасываем состояние паузы
        bullets.clear();
        cargos.clear();
        for (auto enemy : enemies) {
            delete enemy;
        }
        enemies.clear();
        pickedUpCargos = 0; // Сбрасываем поднятые грузы
        deliveredCargos = 0; // Сбрасываем доставленные грузы
        std::cerr << "Restart game." << std::endl;
    }


    void loadHighScoreAndLevel() {
        std::ifstream file("Saves/highscore.score");
        if (file.is_open()) {
            file >> highScore;
            file.close();
        }
        else {
            std::cerr << "Error opening highscore.score for reading." << std::endl; // Отладочное сообщение
        }

        std::ifstream levelFile("Saves/highlevel.lvl");
        if (levelFile.is_open()) {
            levelFile >> highLevel;
            levelFile.close();
        }
        else {
            std::cerr << "Error opening highlevel.lvl for reading." << std::endl; // Отладочное сообщение
        }
    }



    void saveHighScore() {
        if (score > highScore) {
            highScore = score;
            std::ofstream file("Saves/highscore.score");
            if (file.is_open()) {
                file << highScore;
                file.close();
                std::cout << "New high score saved: " << highScore << std::endl; // Отладочное сообщение
            }
            else {
                std::cerr << "Error opening highscore.txt for writing." << std::endl; // Отладочное сообщение
            }
        }
        saveBestDeliveredCargos(); // Сохраняем лучшее количество доставленных грузов
    }


    void saveHighLevel() {
        if (difficultyLevel > highLevel) {
            highLevel = difficultyLevel;
            std::ofstream levelFile("Saves/highlevel.lvl");
            if (levelFile.is_open()) {
                levelFile << highLevel;
                levelFile.close();
                std::cout << "New high level saved: " << highLevel << std::endl; // Отладочное сообщение
            }
            else {
                std::cerr << "Error opening highlevel.txt for writing." << std::endl; // Отладочное сообщение
            }
        }
    }

    void upgradePlayer() {
        // Увеличение всез характеристик игрока при нажатии пробела
        buyDamageUpgrade();
        buyFireRateUpgrade();
        buyHealthUpgrade();
    }

    void buyDamageUpgrade() {
        if (score >= 5) {
            score -= 5;
            bulletDamage += 1; // Увеличение урона пуль
            std::cerr << "buy Damage Upgrade." << std::endl;
        }
        
    }
    void buyFireRateUpgrade() {
        if (score >= 5) {
            score -= 5;
            shootCooldown = std::max(0.01f, shootCooldown - 0.0005f); // Уменьшение задержки между выстрелами
            std::cerr << "buy Fire Rate Upgrade." << std::endl;
        }
        
    }

    void buyHealthUpgrade() {
        if (playerHealth < 10) { // Проверяем, что здоровье не максимальное
            if (score >= 10) {
                score -= 10;
                playerHealth++; // Увеличиваем здоровье
                std::cerr << "buy Health Upgrade." << std::endl;
            }
        }
        else {
            std::cerr << "Health is already full. No score deducted." << std::endl; // Сообщение об отсутствии улучшения
        }
    }



    ~Game() {
        for (auto enemy : enemies) {
            delete enemy;
        }
    }
};

int main() {
    srand(static_cast<unsigned>(time(0)));
    Game game;
    game.run();
    return 0;
}