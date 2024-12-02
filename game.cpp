#include <SFML/Graphics.hpp>
#include <vector>
#include <iostream>
#include <random>
#include "Enemy.cpp"
#include "Boss.cpp"

using namespace std;

// 화면 크기 정의
const int WINDOW_WIDTH = 1300;
const int WINDOW_HEIGHT = 800;
const int FLOOR_COUNT = 4;   // 층 개수
const float GRAVITY = 0.5f;
const float JUMP_FORCE = -15.0f;
const float DOWN_FORCE = 10.0f;
const float FLOOR_SPACING = 200.0f;  // 층 간 간격을 줄여서 화면에 다 들어오게 조정
const float FLOOR_THICKNESS = 40.0f;  // 층 두께 설정
int lastDirection = 1;  // 1: 오른쪽, -1: 왼쪽 (기본값은 오른쪽)   
int enemyCnt = 0;

class Player {
public:
    sf::Sprite sprite;
    sf::Texture texture;
    float velocityY = 0.0f;
    bool isJumping = false;
    bool isDown = false;
    int hp = 3;             // 플레이어 체력
    float moveSpeed = 10.0f; // 플레이어 이동 속도

    Player(const std::string& textureFile, float x, float y, float moveSpeed = 5.0f)
        : moveSpeed(moveSpeed) {
        if (!texture.loadFromFile(textureFile)) {
            std::cerr << "Failed to load player texture!" << std::endl;
            exit(-1);
        }
        sprite.setTexture(texture);
        sprite.setPosition(x, y);
    }   

    void jump() {
        if (!isJumping) {
            velocityY = JUMP_FORCE;
            isJumping = true;
        }
    }

    void moveDown() {
        if (!isDown) {
            velocityY = DOWN_FORCE;
            isDown = true;
        }
    }

    void moveLeft() {
        sprite.move(-moveSpeed, 0);
        lastDirection = -1; // 왼쪽으로 이동
    }

    void moveRight() {
        sprite.move(moveSpeed, 0);
        lastDirection = 1; // 오른쪽으로 이동
    }

    void applyGravity() {
        velocityY += GRAVITY;
        sprite.move(0, velocityY);
    }

    void checkCollision(const std::vector<float>& floorPositions) {
        for (float floorY : floorPositions) {
            if (sprite.getPosition().y + sprite.getGlobalBounds().height >= floorY &&
                sprite.getPosition().y + sprite.getGlobalBounds().height <= floorY + 10 &&
                velocityY >= 0) {
                sprite.setPosition(sprite.getPosition().x, floorY - sprite.getGlobalBounds().height);
                velocityY = 0;
                isJumping = false;
                isDown = false;
                break;
            }
        }
    }

    void takeDamage() {
        hp--;
        if (hp <= 0) {
            std::cout << "Game Over!" << std::endl;
            exit(0);  // 게임 오버 처리
        }
    }
};


class Missile {
public:
    sf::CircleShape shape;
    float speed;
    int direction; // 미사일의 방향 (-1: 왼쪽, 1: 오른쪽)

    // 생성자: 미사일 방향 초기화
    Missile(float x, float y, float speed, int direction)
        : speed(speed), direction(direction) {
        shape.setRadius(10.0f); // 미사일 크기
        shape.setFillColor(sf::Color::Red);
        shape.setPosition(x, y +50.0f); // y값을 조정하여 발사 위치 설정
    }

    // 미사일 이동
    void move() {
        shape.move(speed * direction, 0); // 미사일 방향에 따라 이동
    }

    // 화면 밖으로 나갔는지 확인
    bool isOffScreen() {
        return shape.getPosition().x < 0 || shape.getPosition().x > WINDOW_WIDTH;
    }

    // 적과 충돌했는지 확인
    bool isHitByMissile(const sf::Sprite& enemySprite) {
        return shape.getGlobalBounds().intersects(enemySprite.getGlobalBounds());
    }
};

class Floor {
public:
    sf::RectangleShape shape;

    Floor(float x, float y, float width, float height) {
        shape.setSize({ width, height });
        shape.setPosition(x, y);
        shape.setFillColor(sf::Color::White);
    }
};

int main() {
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "BurgerKing");
    window.setFramerateLimit(60);

    // 배경 텍스처 및 스프라이트
    sf::Texture backgroundTexture, backgroundTexture2, gameOverTexture, gameClearTexture;
    if (!backgroundTexture.loadFromFile("img/back.jpg") || !backgroundTexture2.loadFromFile("img/back2.jpg")) {
        std::cerr << "Failed to load background textures!" << std::endl;
        return -1;
    }
    sf::Sprite backgroundSprite(backgroundTexture);

    // 플레이어 생성
    Player player("img/burger0.png", WINDOW_WIDTH / 2, WINDOW_HEIGHT - FLOOR_SPACING - 70);

    // 층 생성
    vector<Floor> floors;
    vector<float> floorPositions;
    const float FLOOR_OFFSET = 150.0f;
    for (int i = 0; i < FLOOR_COUNT; i++) {
        float y = WINDOW_HEIGHT - (i + 1) * FLOOR_SPACING + FLOOR_OFFSET;
        floors.emplace_back(0, y, WINDOW_WIDTH, FLOOR_THICKNESS);
        floorPositions.push_back(y);
    }

    // 각 층에 고유한 적 생성
    vector<BuneEnemy> buneEnemies;
    vector<CheeseEnemy> cheeseEnemies;
    vector<LettuceEnemy> lettuceEnemies;
    vector<PattyEnemy> pattyEnemies;
   


    // 1층 번 적 생성
    buneEnemies.emplace_back("img/bune.png", rand() % (WINDOW_WIDTH - 50), WINDOW_HEIGHT - FLOOR_SPACING + FLOOR_OFFSET - 70, 2.0f);
    // 2층 치즈 적 생성
    cheeseEnemies.emplace_back("img/cheese.png", "img/cheesefloor.png", rand() % (WINDOW_WIDTH - 50), WINDOW_HEIGHT - (2 * FLOOR_SPACING) + FLOOR_OFFSET - 120, 3.0f);
    // 3층 상추 적 생성
    lettuceEnemies.emplace_back("img/lettuce.png", "img/lettuces.png", rand() % (WINDOW_WIDTH - 50), WINDOW_HEIGHT - (3 * FLOOR_SPACING) + FLOOR_OFFSET - 70, 4.0f);
    // 4층 패티 적 생성
    pattyEnemies.emplace_back("img/patty.png", rand() % (WINDOW_WIDTH - 50), WINDOW_HEIGHT - (4 * FLOOR_SPACING) + FLOOR_OFFSET - 70, 5.0f, 1);

    // 버거킹 보스 생성 
    Boss boss("img/kingboss.png","bacon.png", (WINDOW_WIDTH - 150) / 2, WINDOW_HEIGHT - (2 * FLOOR_SPACING) + FLOOR_OFFSET - 150, 10.0f);

    // 좀비보스 
    Boss zom1("img/zombie.png", (WINDOW_WIDTH - 150) / 2, WINDOW_HEIGHT - (2 * FLOOR_SPACING) + FLOOR_OFFSET - 180, 5.0f);
    // 좀비보스 
    Boss zom2("img/zombie.png", (WINDOW_WIDTH - 150) / 2, WINDOW_HEIGHT - (3 * FLOOR_SPACING) + FLOOR_OFFSET - 180, 10.0f);
    // 좀비보스 
    Boss zom3("img/zombie.png", (WINDOW_WIDTH - 150) / 2, WINDOW_HEIGHT - (4 * FLOOR_SPACING) + FLOOR_OFFSET - 180, 15.0f);


    // 미사일 
    vector<Missile> missiles;

    // 미사일 발사 제한 시간
    sf::Clock missileClock;
    const float MISSILE_COOLDOWN = 1.0f; //1초 

    //남은 적 텍스트로 띄우기
    sf::Font font;
    if (!font.loadFromFile("fonts/MaplestoryBold.ttf")) {
        std::cerr << "Failed to load font!" << std::endl;
        return -1;
    }
    //남은 적 텍스트 설정
    sf::Text enemyCountText;
    enemyCountText.setFont(font);
    enemyCountText.setCharacterSize(50);
    enemyCountText.setFillColor(sf::Color::Red);
    enemyCountText.setPosition(10, 60);

    // 적 추가 타이머 설정
    sf::Clock enemyClock; // 적을 5초마다 생성할 타이머

    // 게임 루프
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        // 플레이어 이동
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) && player.sprite.getPosition().x > 0)
            player.moveLeft(); // ← 함수 호출로 수정
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) &&
            player.sprite.getPosition().x + player.sprite.getGlobalBounds().width < WINDOW_WIDTH)
            player.moveRight(); // → 함수 호출로 수정
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
            player.jump();
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
            player.moveDown();
    

        player.applyGravity(); // 중력 적용  
        player.checkCollision(floorPositions);// 충돌 확인

        // 적과 플레이어 충돌 처리
        for (auto& enemy : buneEnemies) {
            if (player.sprite.getGlobalBounds().intersects(enemy.sprite.getGlobalBounds())) {
                player.takeDamage();
            }
        }
        for (auto& enemy : cheeseEnemies) {
            if (player.sprite.getGlobalBounds().intersects(enemy.sprite.getGlobalBounds()) || enemy.checkPlayerCollision(player.sprite)) {
                player.takeDamage();
            }
        }
        for (auto& enemy : lettuceEnemies) {
            // 본체와 충돌 확인
            if (player.sprite.getGlobalBounds().intersects(enemy.sprite.getGlobalBounds())) {
                player.takeDamage();
            }
      // 분리된 잎들과 충돌 확인
            for (auto& leaf : enemy.lettuceLeaves) {
                if (player.sprite.getGlobalBounds().intersects(leaf.sprite.getGlobalBounds())) {
                    player.takeDamage();
                    break; // 충돌이 발생하면 잎의 나머지 충돌 확인 생략
                }
            }
        }
        for (auto& enemy : pattyEnemies) {
            if (player.sprite.getGlobalBounds().intersects(enemy.sprite.getGlobalBounds())) {
                player.takeDamage();
            }
        }

        //스페이스 눌렀을때 미사일 발사 
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
            if (missileClock.getElapsedTime().asSeconds() > MISSILE_COOLDOWN) {
                // 미사일 생성 시 플레이어의 위치와 방향을 저장
                missiles.emplace_back(
                    player.sprite.getPosition().x + player.sprite.getGlobalBounds().width / 2,
                    player.sprite.getPosition().y,
                    10.0f, // 미사일 속도
                    lastDirection // 플레이어의 마지막 방향 저장
                );
                missileClock.restart();
            }
        }
        // 미사일 이동
        for (auto it = missiles.begin(); it != missiles.end();) {
            it->move(); // 각 미사일은 자신의 방향으로 이동
            if (it->isOffScreen()) {
                it = missiles.erase(it); // 화면 밖으로 나간 미사일 제거
            }
            else {
                ++it;
            }
        }


        // 적과 미사일 충돌 처리
        for (auto& missile : missiles) {
            for (auto it = cheeseEnemies.begin(); it != cheeseEnemies.end();) {
                if (missile.isHitByMissile(it->sprite)) {
                    missile.shape.setPosition(-100, -100); // 미사일 제거

                    // 기존 적을 먼저 제거
                    it = cheeseEnemies.erase(it);
                    enemyCnt--;  // 남은 적 갯수 줄어듬
                    //적이 죽으면 새로 생성
                }
                else {
                    ++it;
                }
            }
            // 나머지 적도 같은 방식으로 수정
            for (auto it = lettuceEnemies.begin(); it != lettuceEnemies.end();) {
                if (missile.isHitByMissile(it->sprite)) {
                    missile.shape.setPosition(-100, -100);
                    it = lettuceEnemies.erase(it);
                    enemyCnt--;  // 남은 적 갯수 줄어듬
                }
                else {
                    ++it;
                }
            }
            for (auto it = pattyEnemies.begin(); it != pattyEnemies.end();) {
                if (missile.isHitByMissile(it->sprite)) {
                    missile.shape.setPosition(-100, -100);
                    it->hp--;
                    if (it->hp == 0) {   //패티적은 체력이 0일때
                        it = pattyEnemies.erase(it);  enemyCnt--;
                    }
                }
                else {
                    ++it;
                }
            }
            for (auto it = buneEnemies.begin(); it != buneEnemies.end();) {
                if (missile.isHitByMissile(it->sprite)) {
                    missile.shape.setPosition(-100, -100);
                    it = buneEnemies.erase(it);
                    enemyCnt--;
                }
                else {
                    ++it;
                }
            }
         
        }

        // 적을 ３초마다 생성
        if (enemyClock.getElapsedTime().asSeconds() >= 3.0f && enemyCnt > 0) {
            // 각 층에 적을 추가
            if (buneEnemies.size() < 1 && enemyCnt > 0) {
                buneEnemies.emplace_back("img/bune.png", rand() % (WINDOW_WIDTH), WINDOW_HEIGHT - FLOOR_SPACING + FLOOR_OFFSET - 70, 2.0f);

            }
            if (cheeseEnemies.size() < 1 && enemyCnt > 0) {
                cheeseEnemies.emplace_back("img/cheese.png", "img/cheesefloor.png", rand() % (WINDOW_WIDTH), WINDOW_HEIGHT - (2 * FLOOR_SPACING) + FLOOR_OFFSET - 120, 3.0f);

            }
            if (lettuceEnemies.size() < 1 && enemyCnt > 0) {
                lettuceEnemies.emplace_back("img/lettuce.png", "img/lettuces.png", rand() % (WINDOW_WIDTH), WINDOW_HEIGHT - (3 * FLOOR_SPACING) + FLOOR_OFFSET - 70, 4.0f);

            }
            // 반반 확률로 트리플패티 & 일반 패티 생성
            if (pattyEnemies.size() < 1 && enemyCnt > 0) {
                // 50% 확률로 트리플 패티 또는 일반 패티 생성
                if (std::rand() % 2 == 0) { // 50% 확률
                    pattyEnemies.emplace_back("img/patty3.png", rand() % (WINDOW_WIDTH - 50), WINDOW_HEIGHT - (4 * FLOOR_SPACING) + FLOOR_OFFSET - 150, 5.0f, 3);// 체력: 3 (트리플 패티)
                }
                else {
                    pattyEnemies.emplace_back( // 일반 패티
                        "img/patty.png", rand() % (WINDOW_WIDTH - 50), WINDOW_HEIGHT - (4 * FLOOR_SPACING) + FLOOR_OFFSET - 70,
                        5.0f, 1); // 체력: 1 (일반 패티)
                }
            }
            // 타이머 리셋
            enemyClock.restart();
        }

        // 각 적 이동
        for (auto& enemy : cheeseEnemies) {
            enemy.move(0, WINDOW_WIDTH);      // 이동
            enemy.spawnCheeseFloor();        // 치즈 바닥 생성
            enemy.updateCheeseFloors();      // 오래된 치즈 바닥 제거
        }
        for (auto& enemy : pattyEnemies) {
            enemy.move(0, WINDOW_WIDTH);
        }
        for (auto& enemy : lettuceEnemies) {
            enemy.move(0, WINDOW_WIDTH);
            // 공격 처리 (5초마다 분리, 합치기)
            enemy.attack();
        }
        for (auto& enemy : buneEnemies) {
            enemy.move(0, WINDOW_WIDTH);
            enemy.jump();
            // 각 적 이동
        }


        if (enemyCnt == 0) {

            // 기존 적 제거 및 배경 변경
            buneEnemies.clear();
            cheeseEnemies.clear();
            lettuceEnemies.clear();
            pattyEnemies.clear();

            backgroundSprite.setTexture(backgroundTexture2); // 배경을 back2로 변경

            boss.move(0, WINDOW_WIDTH);
    

            zom1.move(0, WINDOW_WIDTH);
      

            zom2.move(0, WINDOW_WIDTH);
          
            zom3.move(0, WINDOW_WIDTH);


        }

  

        // 화면 갱신
        window.clear();
        window.draw(backgroundSprite);

        // 남은 적  텍스트 추가
        enemyCountText.setString("Remaining Enemies: " + std::to_string(enemyCnt));
        window.draw(enemyCountText);

        for (auto& floor : floors) {
            window.draw(floor.shape);
        }
        for (auto& enemy : buneEnemies) {
            window.draw(enemy.sprite);
        }
        for (auto& enemy : cheeseEnemies) {
            window.draw(enemy.sprite);
        }
        // 치즈 바닥 렌더링
        for (auto& enemy : cheeseEnemies) {
            enemy.drawCheeseFloors(window);
        }
        for (auto& enemy : lettuceEnemies) {
            enemy.draw(window);
        }
        for (auto& enemy : pattyEnemies) {
            window.draw(enemy.sprite);
        }


        for (auto& missile : missiles) {
            window.draw(missile.shape);
        }
        window.draw(player.sprite);

        // 보스와 좀비 렌더링
        if (enemyCnt == 0) {
            window.draw(zom1.sprite); // 좀비 그리기
            window.draw(zom2.sprite); // 좀비 그리기
            window.draw(zom3.sprite); // 좀비 그리기

            window.draw(boss.sprite); // 보스 그리기
        }

        window.display();
    }


    return 0;
}
