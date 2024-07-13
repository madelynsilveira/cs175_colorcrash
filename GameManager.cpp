#include "GameManager.h"

#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"

GameManager *GameManager::_instance = nullptr;


/**
 * Static methods should be defined outside the class.
 */
GameManager *GameManager::GetInstance() {
    if (_instance == nullptr) {
        _instance = new GameManager();
    }
    return _instance;
}

void GameManager::initialize() {
    srand(time(nullptr));
    state = ST_MENU;
    currentTime = 0.0;
    timePerWallTextureCycle = 5.0;
    playerDist = 2.0f;
    playerScale = 0.2f;
    num_tex = 3;
    GLuint *tex_ids = new GLuint[4];
    tex_ids[0] = 0;
    tex_ids[1] = 1;
    tex_ids[2] = 2;
    tex_ids[3] = 3;
    timer = 2.0;
    lastTimeCreated = 0;
    grid = new Grid();
    activatedColors = 0b100;
    lives = 3;
}

void GameManager::ResetGame() {
    obs.clear();
    currentTime = 0.0;
    timer = 2.0;
    lastTimeCreated = 0;
    difficulty = 0.0;
    activatedColors = 0b100;
    lives = 3;
}

int GameManager::GameUpdate() {
    double newTime = GetTime();
    // frameTime indicates the amount of time since the previous frame
    double frameTime = newTime - currentTime;
    if (frameTime > 0.25)
        frameTime = 0.25;
    currentTime = newTime;

    switch (state) {
        case ST_MENU:
            // TODO draw menu
            return ST_MENU;
        case ST_GAMERUNNING:

            // Set game state here based on how much frameTime has passed

            // update player position based on mouse position
            updatePlayer(frameTime);
            // update wall position based on time
            updateObstacles(frameTime);
            // update texture increment based on time
            updateEnvironment(frameTime);
            return ST_GAMERUNNING;
        case ST_GAMEEND:
            // TODO draw game over
            return ST_GAMEEND;
        default:
            throw std::runtime_error("Invalid game state");
    }
}

void GameManager::UpdateGameState(States s) {
    state = s;
}

// TODO: PLAYER UPDATES PER AMOUNT FRAMETIME
void GameManager::updatePlayer(double frameTime) {

}

// TODO: UPDATE OBSTACLE POSITIONS PER AMOUNT FRAMETIME
void GameManager::updateObstacles(double frameTime) {
    timer += frameTime;
    for (auto i = obs.begin(); i != obs.end();) {
        auto o = i;
        o->depth += o->speed * (float) frameTime * (float) difficulty;
        o->pos = glm::vec3(o->pos.x, o->pos.y, o->depth);
        if (o->depth > 1.2f && !(o->intersected)) {
            o->intersected = true;
            if (intersectionCheck(*o)) {
                Colors c = o->color;
                CheckGameOver(c);
//                auto pos = obs.erase(i);
//                i = pos;
            }
        } else if (o->depth > 2.0f) {
            auto pos = obs.erase(i);
            i = pos;
        }
        if (i != obs.end())
            ++i;
    }
    if (timer > lastTimeCreated + (3.0 / difficulty)) {
        lastTimeCreated = timer;
        difficulty += 0.1;
        MakeWall();
    }
}

// TODO: UPDATE ENVIRONMENT PER AMOUNT FRAMETIME
void GameManager::updateEnvironment(double frameTime) {
    grid->update();
}

bool GameManager::intersectionCheck(obstacle &o) {
    //cout << "intersection checking" << endl;
    return SameSign(o.pos.x, getPlayerPos().x) && SameSign(o.pos.y, getPlayerPos().y);
}

void GameManager::CheckGameOver(Colors &c) {
    if (c != playerColor) {
        lives -= 1;
        if (lives == 0) {
            state = ST_GAMEEND;
            cout << "---------- GAME OVER ----------" << endl;
        }
    }
}

void GameManager::MakeWall() {
    bool allBlack = false;
    cout << "difficulty: " << difficulty << endl;
    allBlack &= MakeObstacle(glm::vec2(-0.5f, 0.5f)) == BLACK;
    allBlack &= MakeObstacle(glm::vec2(0.5f, 0.5f)) == BLACK;
    allBlack &= MakeObstacle(glm::vec2(-0.5f, -0.5f)) == BLACK;
    allBlack &= MakeObstacle(glm::vec2(0.5f, -0.5f)) == BLACK;
    if (allBlack) {
        obs.back().color = static_cast<Colors>(rand() % BLACK);
    }
}

GameManager::Colors GameManager::MakeObstacle(glm::vec2 pos) {
    obstacle o;
    o.poly = new ply("./data/obstacle.ply");
    o.pos = glm::vec3(pos.x, pos.y, -20.0f);
    if (difficulty < 2.0)
        o.color = static_cast<Colors>(rand() % GREEN);
    else
        o.color = static_cast<Colors>(rand() % LAST);
    o.poly->buildArrays();
    glc->reloadObstacleVBO();
    obs.push_back(o);
    return o.color;
    //cout << "Making an obstacle" << endl;
}

void GameManager::handleKeyPress(char c) {
    switch (c) {
        case '\r':
        case '\n':
            if (state == ST_MENU) {
                std::cout << "Starting game" << std::endl;
                state = ST_GAMERUNNING;
            } else if (state == ST_GAMEEND) {
                std::cout << "Restarting game" << std::endl;
                ResetGame();
                state = ST_GAMERUNNING;
            }
            break;
        case 'A':
            if (state == ST_GAMERUNNING)
                activatedColors ^= 0b100;
            updateColor();
            break;
        case 'S':
            if (state == ST_GAMERUNNING)
                activatedColors ^= 0b010;
            updateColor();
            break;
        case 'D':
            if (state == ST_GAMERUNNING)
                activatedColors ^= 0b001;
            updateColor();
            break;
    }
}


glm::vec3 GameManager::generateRay(int pixelX, int pixelY) {
    glm::vec3 point((pixelX / (float) sizeX) * 2 - 1, (pixelY / (float) sizeY) * -2 + 1, -1.0f);

    //std::cout << point.x << ", " << point.y << std::endl;
    //std::cout << sizeX << ", " << sizeY << std::endl;
    glm::vec4 ray_clip = glm::vec4(point, 1.0);
    glm::vec3 ray_eye = glm::inverse(perspectiveMatrix) * ray_clip;
    glm::vec4 ray_wor = glm::vec4(inverse(modelViewMatrix) * glm::vec4(ray_eye, 0.0));

    return ray_wor;
}

void GameManager::resize(int x, int y) {
    sizeX = x;
    sizeY = y;
}

// -------------- Getter & Setter -----------------

GameManager::States GameManager::GetGameState() {
    return state;
}

void GameManager::setPlayerPos(int x, int y) {
    // calculate player point here
    glm::vec3 ray_wor = generateRay(x, y);
    playerPos = eyeP + playerDist * ray_wor;
}

glm::vec3 GameManager::getPlayerPos() {
    return playerPos;
}

void GameManager::setEyePoint(glm::vec3 p) {
    eyeP = p;
}

void GameManager::updateColor() {
    switch (activatedColors) {
        case 0b000:
            playerColor = BLACK;
            break;
        case 0b001:
            playerColor = YELLOW;
            break;
        case 0b010:
            playerColor = BLUE;
            break;
        case 0b011:
            playerColor = GREEN;
            break;
        case 0b100:
            playerColor = RED;
            break;
        case 0b101:
            playerColor = ORANGE;
            break;
        case 0b110:
            playerColor = PURPLE;
            break;
        case 0b111:
            playerColor = WHITE;
            break;
    }
}

glm::vec3 GameManager::getPlayerColor() {
    return enumToColor(playerColor);
}


double GameManager::GetTime() {
    clock_t cycles = clock();
    double time = (double) cycles / CLOCKS_PER_SEC;
    return time;
}

glm::vec3 GameManager::enumToColor(Colors c) {
    // switch (c) {
    //     case WHITE:
    //         return {1.0f, 1.0f, 1.0f};
    //     case RED:
    //         return {1.0f, 0.0f, 0.0f};
    //     case BLUE:
    //         return {0.0f, 0.0f, 1.0f};
    //     case GREEN:
    //         return {0.0f, 1.0f, 0.0f};
    //     case YELLOW:
    //         return {1.0f, 1.0f, 0.0f};
    //     case PURPLE:
    //         return {0.5f, 0.0f, 0.5f};
    //     case ORANGE:
    //         return {1.0f, 0.5f, 0.0f};
    //     default:
    //         return glm::vec3(0.0f);
    // }
}

GameManager::Colors GameManager::colorToEnum(glm::vec3 v) {
    // TODO fp equality
    if (v.x == 1.0f && v.y == 0.0f && v.z == 0.0f) { return RED; }
    if (v.x == 0.0f && v.y == 1.0f && v.z == 0.0f) { return GREEN; }
    if (v.x == 0.0f && v.y == 0.0f && v.z == 1.0f) { return BLUE; }
    if (v.x == 1.0f && v.y == 1.0f && v.z == 0.0f) { return YELLOW; }
    if (v.x == 1.0f && v.y == 0.0f && v.z == 1.0f) { return PURPLE; }
    if (v.x == 1.0f && v.y == 0.5f && v.z == 0.0f) { return ORANGE; }
    if (v.x == 1.0f && v.y == 1.0f && v.z == 1.0f) { return WHITE; }
    return BLACK;
}

void GameManager::loadTexture() {
    GLuint *textures = new GLuint[num_tex];
    glGenTextures(num_tex, textures);
    loadTexture("./data/butterfly.png", GL_TEXTURE0, textures[0]);
    loadTexture("./data/wall-diffuse.png", GL_TEXTURE1, textures[1]);
    loadTexture("./data/wall-normal.png", GL_TEXTURE2, textures[2]);
    //loadTexture("./data/red.png", GL_TEXTURE1, textures[1]);
    //loadTexture("./data/green.png", GL_TEXTURE2, textures[2]);
    //loadTexture("./data/blue.png", GL_TEXTURE3, textures[3]);
    //loadTexture("./data/black.png", GL_TEXTURE4, textures[4]);

    tex_ids = textures;
}

void GameManager::loadTexture(const char *path, int textureInd, GLuint &tex_id) {
    int w;
    int h;
    int comp;
    std::cout << "Reading in " << path << " from data" << std::endl;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *image = stbi_load(path, &w, &h, &comp, STBI_rgb_alpha);

    if (image == nullptr) {
        throw std::runtime_error("Failed to load texture");
    }
    glActiveTexture(textureInd);

    glBindTexture(GL_TEXTURE_2D, tex_id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(image);
}


bool GameManager::SameSign(float a, float b) {
    return a * b >= 0.0f;
}