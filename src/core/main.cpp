// sdl2
#include <SDL2/SDL.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_scancode.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>

// OpenGl
#include <GL/glew.h>

// glm
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <cmath> // for sin()

#include "globals.hpp" // defines WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT, FOV, PLAYER_FLY, default_level_path
#include "../graphics/texture.hpp"
#include "../levels/load.hpp"
#include "../fonts/fonts.hpp"
#include "../console/console.hpp"
#include "../shaders/shaders.hpp"

// player headers
#include "../player/input.hpp"
#include "../player/keyboard.hpp"
#include "../player/mouse.hpp"

// error handler
#include "../errors/error.hpp"

int S_CurrentVolume = MIX_MAX_VOLUME / 2;
std::string R_CurrentTextureKey = "1";
std::vector<std::string> C_CommandHistory;
int C_HistoryIndex = -1;

Spawn playerSpawn;

glm::vec3 computeLevelCenter(const std::vector<Wall> &walls) {
    if (walls.empty())
        return glm::vec3(0.0f, 0.0f, 0.0f);

    float minX = walls[0].x1, maxX = walls[0].x1;
    float minY = walls[0].y1, maxY = walls[0].y1;
    float minZ = walls[0].z1, maxZ = walls[0].z1;

    for (const Wall &w : walls) {
        minX = std::min({minX, w.x1, w.x2});
        maxX = std::max({maxX, w.x1, w.x2});
        minY = std::min({minY, w.y1, w.y2});
        maxY = std::max({maxY, w.y1, w.y2});
        minZ = std::min({minZ, w.z1, w.z2});
        maxZ = std::max({maxZ, w.z1, w.z2});
    }

    return glm::vec3((minX + maxX) * 0.5f, (minY + maxY) * 0.5f, (minZ + maxZ) * 0.5f);
}

std::vector<float> buildLevelVertices(const std::vector<Sector> &sectors, const std::vector<Wall> &walls) {
    std::vector<float> vertices;
    size_t totalTriangles = 0;
    for (const Sector &sector : sectors)
        totalTriangles += sector.wallCount * 2;
    vertices.reserve(totalTriangles * 5 * 3);

    const float textureScale = 2.5f;

    for (const Sector &sector : sectors) {
        for (int i = 0; i < sector.wallCount; ++i) {
            int wallIndex = sector.startWall + i;
            if (wallIndex < 0 || wallIndex >= walls.size())
                continue;
            const Wall &wall = walls[wallIndex];
            float bl[3] = {wall.x1, sector.floor, wall.y1};
            float br[3] = {wall.x2, sector.floor, wall.y2};
            float tr[3] = {wall.x2, sector.ceiling, wall.y2};
            float tl[3] = {wall.x1, sector.ceiling, wall.y1};

            float dx = wall.x2 - wall.x1;
            float dz = wall.y2 - wall.y1;
            float wallLength = sqrt(dx * dx + dz * dz);
            float wallHeight = sector.ceiling - sector.floor;

            float texBL[2] = {0.0f, 0.0f};
            float texBR[2] = {wallLength / textureScale, 0.0f};
            float texTR[2] = {wallLength / textureScale, wallHeight / textureScale};
            float texTL[2] = {0.0f, wallHeight / textureScale};

            vertices.insert(vertices.end(), {bl[0], bl[1], bl[2], texBL[0], texBL[1],
                                             br[0], br[1], br[2], texBR[0], texBR[1],
                                             tr[0], tr[1], tr[2], texTR[0], texTR[1]});
            vertices.insert(vertices.end(), {bl[0], bl[1], bl[2], texBL[0], texBL[1],
                                             tr[0], tr[1], tr[2], texTR[0], texTR[1],
                                             tl[0], tl[1], tl[2], texTL[0], texTL[1]});
        }
    }
    return vertices;
}

std::string vertex_shader_source_STR = readShaderFile("shaders/vertex_source.vert");
const char *vertex_shader_source = vertex_shader_source_STR.c_str();

std::string fragment_shader_source_STR = readShaderFile("shaders/fragment_source.frag");
const char *fragment_shader_source = fragment_shader_source_STR.c_str();

void processCommand(const std::string &cmd, bool &G_Running, std::string &commandInput,
                    std::vector<Sector> &sectors, std::vector<Wall> &walls,
                    std::vector<float> &levelVertices, GLuint VBO,
                    std::unordered_map<std::string, GLuint> &textureLevel,
                    bool &paused, float &mouseSensitivity, int &currentVolume,
                    std::string &currentTextureKey, SDL_Window *window,
                    std::vector<std::string> &commandHistory, int &historyIndex) {
    std::istringstream iss(cmd);
    std::string token;
    iss >> token;
    if (token.empty()) {
        commandInput = "";
        return;
    }
    commandHistory.push_back(cmd);
    historyIndex = commandHistory.size();

    if (token == "clear") {
        commandInput = "";
    }
    else if (token == "exit" || token == "quit") {
        G_Running = false;
    }
    else if (token == "close") {
        consoleActive = false;
    }
    else if (token == "fly") {
        M_fly = !M_fly;
        commandInput = "";
    }
    else if (token == "load") {

        std::string filename;
        iss >> filename;
        if (!filename.empty()) {
            std::vector<Sector> newSectors;
            std::vector<Wall> newWalls;
            std::unordered_map<std::string, GLuint> newTextureMap;
            if (L_loadLevel(filename.c_str(), newSectors, newWalls, newTextureMap, playerSpawn)) {
                sectors = newSectors;
                walls = newWalls;
                levelVertices = buildLevelVertices(sectors, walls);
                glBindBuffer(GL_ARRAY_BUFFER, VBO);
                glBufferData(GL_ARRAY_BUFFER, levelVertices.size() * sizeof(float), levelVertices.data(), GL_STATIC_DRAW);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                textureLevel = newTextureMap;
                info("Map loaded", filename.c_str());
            }
            else {
                warn("Failed to load map!");
            }
        }
        commandInput = "";
    }
    else if (token == "reload") {
        std::vector<Sector> newSectors;
        std::vector<Wall> newWalls;
        std::unordered_map<std::string, GLuint> newTextureMap;
        if (L_loadLevel(WTF_map, newSectors, newWalls, newTextureMap, playerSpawn)) {
            sectors = newSectors;
            walls = newWalls;
            levelVertices = buildLevelVertices(sectors, walls);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, levelVertices.size() * sizeof(float), levelVertices.data(), GL_STATIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            textureLevel = newTextureMap;
            info("Map reloaded", WTF_map);
        }
        else {
            warn("Failed to reload map", WTF_map);
        }
        commandInput = "";
    }
    else if (token == "fullscreen") {
        Uint32 flags = SDL_GetWindowFlags(window);
        if (flags & SDL_WINDOW_FULLSCREEN)
            SDL_SetWindowFullscreen(window, 0);
        else
            SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
        commandInput = "";
    }
    else if (token == "pause") {
        paused = !paused;
        std::cout << "Paused: " << (paused ? "true" : "false") << std::endl;
        commandInput = "";
    }
    else if (token == "sens") {
        float value;
        if (iss >> value) {
            mouseSensitivity = value;
            info("Mouse sensitivity set to", std::to_string(mouseSensitivity).c_str());
        }
        else {
            error("Invalid sensitivity value", std::to_string(value).c_str());
        }
        commandInput = "";
    }
    else if (token == "volume") {
        int value;
        if (iss >> value) {
            currentVolume = std::max(0, std::min(128, value));
            Mix_VolumeMusic(currentVolume);
            info("Volume set to", std::to_string(currentVolume).c_str());
        }
        else {
            error("Invalid volume value", std::to_string(value).c_str());
        }
        commandInput = "";
    }
    else {
        commandInput = "";
    }
}

int playMusic(const char *filePath) {
    Mix_Music *music = Mix_LoadMUS(filePath);
    if (!music)
    {
        fprintf(stderr, "Failed to load music: %s\n", Mix_GetError());
        return -1;
    }
    if (Mix_PlayMusic(music, -1) == -1)
    {
        fprintf(stderr, "Failed to play music: %s\n", Mix_GetError());
        Mix_FreeMusic(music);
        return -1;
    }
    return 0;
}

int init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        critical("Failed to initialize SDL", SDL_GetError());
        return -1;
    }
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
    {
        critical("Failed to initialize audio", Mix_GetError());
        return -1;
    }
    if (TTF_Init() < 0)
    {
        critical("Failed to initialize TTF", TTF_GetError());
        return -1;
    }
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG))
    {
        critical("Failed to initialize SDL_image", IMG_GetError());
        return -1;
    }
    return 0;
}

// render scene bruh
void render(GLuint shaderProgram, GLint ourTextureLoc,
            std::unordered_map<std::string, GLuint> &textureLevel,
            glm::vec3 &mapCenter, GLint modelLoc, GLint viewLoc, GLint projLoc,
            GLuint VAO, std::vector<float> &levelVertices, float M_headbob_offset) {
    // render scene
    glClearColor(0.15f, 0.35f, 0.50f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shaderProgram);
    glUniform1i(ourTextureLoc, 0);
    glActiveTexture(GL_TEXTURE0);
    GLuint wallTexture = textureLevel[R_CurrentTextureKey];
    glBindTexture(GL_TEXTURE_2D, wallTexture);

    glm::mat4 model = glm::translate(glm::mat4(1.0f),
                                     glm::vec3(-mapCenter.x, 0.0f, -mapCenter.y));

    // apply head bob offset to the camera's Y position
    glm::vec3 bobbedCameraPos = cameraPos;
    bobbedCameraPos.y += M_headbob_offset;

    glm::mat4 view = glm::lookAt(bobbedCameraPos, bobbedCameraPos + cameraFront, cameraUp);
    glm::mat4 projection = glm::perspective(glm::radians(FOV),
                                            static_cast<float>(WINDOW_WIDTH) / WINDOW_HEIGHT,
                                            0.1f, 100.0f);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, levelVertices.size() / 5);
    glBindVertexArray(0);
}

int main(int argc, char *argv[]) {

    // initialization
    if (init() < 0)
        return -1;

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    SDL_Window *window = SDL_CreateWindow(
        WINDOW_TITLE,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    if (!window) {
        critical("Failed to create window", SDL_GetError());
        SDL_Quit();
        return -1;
    }
    SDL_SetWindowGrab(window, SDL_TRUE);
    bool currentRelativeMode = true;
    SDL_SetRelativeMouseMode(SDL_TRUE);

    SDL_Surface *iconSurface = IMG_Load("logo/window.png");
    if (!iconSurface) {
        critical("Failed to load window icon", IMG_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }
    SDL_SetWindowIcon(window, iconSurface);
    SDL_FreeSurface(iconSurface);

    SDL_GLContext context = SDL_GL_CreateContext(window);
    if (!context) {
        critical("Failed to create OpenGL context", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        critical("Failed to initialize GLEW!");
        SDL_GL_DeleteContext(context);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    // TEMP: musiclevel is only for tests, musiclevel will be on the map
    const char *musicLevel = "resources/musics/demo.ogg";
    Mix_Music *music = Mix_LoadMUS(musicLevel);
    if (!music) {
        fprintf(stderr, "Failed to load music: %s\n", Mix_GetError());
        SDL_GL_DeleteContext(context);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }
    Mix_VolumeMusic(S_CurrentVolume);
    Mix_PlayMusic(music, -1);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // in older versions, each shader compilation had a unique function name.
    // this caused unnecessary duplication across the codebase.
    // now, we use a single function, "createShaderProgram," for all shaders.
    GLuint shaderProgram = createShaderProgram(vertex_shader_source, fragment_shader_source);
    GLuint consoleShaderProgram = createShaderProgram(console_vertex_shader_source, console_fragment_shader_source);
    GLuint textShaderProgram = createShaderProgram(text_vertex_shader_source, text_fragment_shader_source);

    if (shaderProgram == 0 || consoleShaderProgram == 0 || textShaderProgram == 0) {
        critical("Failed to create shader programs.");
    }

    GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
    GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLint projLoc = glGetUniformLocation(shaderProgram, "projection");
    GLint ourTextureLoc = glGetUniformLocation(shaderProgram, "ourTexture");

    GLint consoleProjLoc = glGetUniformLocation(consoleShaderProgram, "projection");
    GLint consoleModelLoc = glGetUniformLocation(consoleShaderProgram, "model");
    GLint consoleColorLoc = glGetUniformLocation(consoleShaderProgram, "color");

    std::vector<Sector> sectors;
    std::vector<Wall> walls;
    std::unordered_map<std::string, GLuint> textureLevel;

    if (!L_loadLevel(default_level_path, sectors, walls, textureLevel, playerSpawn)) {
        SDL_GL_DeleteContext(context);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    std::vector<float> levelVertices = buildLevelVertices(sectors, walls);
    glm::vec3 mapCenter = computeLevelCenter(walls);

    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, levelVertices.size() * sizeof(float), levelVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    float consoleVertices[] = {
        0.0f, 0.0f,
        static_cast<float>(WINDOW_WIDTH), 0.0f,
        static_cast<float>(WINDOW_WIDTH), static_cast<float>(consoleHeight),
        0.0f, 0.0f,
        static_cast<float>(WINDOW_WIDTH), static_cast<float>(consoleHeight),
        0.0f, static_cast<float>(consoleHeight)};
    GLuint consoleVAO, consoleVBO;
    glGenVertexArrays(1, &consoleVAO);
    glGenBuffers(1, &consoleVBO);
    glBindVertexArray(consoleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, consoleVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(consoleVertices), consoleVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    SDL_GL_SetSwapInterval(1);

    TTF_Font *consoleFont = TTF_OpenFont("resources/fonts/console.ttf", 16);
    if (!consoleFont) {
        warn("Failed to load font", TTF_GetError());
    }

    extern std::string commandInput;
    extern bool consoleActive;
    extern float consoleAnim;
    extern const float consoleAnimSpeed;

    // head bob effect variables
    float headBobTimer = 0.0f;
    const float bobbingSpeed = 10.0f;
    const float bobbingAmplitude = 0.10f;

    G_Running = true;
    SDL_Event event;
    Uint32 lastTime = SDL_GetTicks();
    float fpsTimer = 0.0f;
    int fpsCount = 0;
    float currentFPS = 0.0f;

    extern glm::vec3 cameraPos;
    extern glm::vec3 cameraFront;
    extern glm::vec3 cameraUp;

    SDL_StartTextInput();

    // main loop
    while (G_Running) {
        // update relative mouse mode
        if (consoleActive && currentRelativeMode) {
            SDL_SetRelativeMouseMode(SDL_FALSE);
            currentRelativeMode = false;
        }
        else if (!consoleActive && !currentRelativeMode) {
            SDL_SetRelativeMouseMode(SDL_TRUE);
            currentRelativeMode = true;
        }

        Uint32 currentTime = SDL_GetTicks();
        float deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;

        fpsTimer += deltaTime;
        fpsCount++;
        if (fpsTimer >= 1.0f) {
            currentFPS = fpsCount / fpsTimer;
            fpsCount = 0;
            fpsTimer = 0.0f;
        }

        // process events
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                G_Running = false;
                break;
            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_QUOTE)
                    consoleActive = true;
                if (event.key.keysym.sym == SDLK_ESCAPE)
                    consoleActive = false;
                else if (consoleActive) {
                    if (event.key.keysym.sym == SDLK_BACKSPACE && !commandInput.empty())
                        commandInput.pop_back();
                    else if (event.key.keysym.sym == SDLK_RETURN) {
                        processCommand(commandInput, G_Running, commandInput, sectors, walls, levelVertices,
                                       VBO, textureLevel, G_Paused, M_MouseSensitivity, S_CurrentVolume,
                                       R_CurrentTextureKey, window, C_CommandHistory, C_HistoryIndex);
                        mapCenter = computeLevelCenter(walls);
                    }
                    else if (event.key.keysym.sym == SDLK_UP) {
                        if (!C_CommandHistory.empty() && C_HistoryIndex > 0) {
                            C_HistoryIndex--;
                            commandInput = C_CommandHistory[C_HistoryIndex];
                        }
                    }
                    else if (event.key.keysym.sym == SDLK_DOWN) {
                        if (!C_CommandHistory.empty() &&
                            C_HistoryIndex < static_cast<int>(C_CommandHistory.size()) - 1) {
                            C_HistoryIndex++;
                            commandInput = C_CommandHistory[C_HistoryIndex];
                        }
                        else {
                            commandInput.clear();
                        }
                    }
                }
                break;
            case SDL_TEXTINPUT:
                if (consoleActive)
                    commandInput += event.text.text;
                break;
            case SDL_MOUSEMOTION:
                if (!consoleActive) {
                    M_processMouseInput(
                        event.motion.xrel,
                        event.motion.yrel,
                        M_pitch,
                        M_yaw,
                        M_MouseSensitivity,
                        cameraFront);
                }
                break;
            default:
                break;
            }
        }

        if (!consoleActive) {
            const Uint8 *keyStates = SDL_GetKeyboardState(NULL);
            glm::vec3 forwardDirection = glm::normalize(glm::vec3(cameraFront.x, 0.0f, cameraFront.z));
            glm::vec3 rightDirection = glm::normalize(glm::cross(forwardDirection, cameraUp));

            if (keyStates[SDL_SCANCODE_W]) {
                M_move_forward(keyStates, forwardDirection, walls, mapCenter, deltaTime, cameraPos);
            }
            if (keyStates[SDL_SCANCODE_S]) {
                M_move_backward(keyStates, forwardDirection, walls, mapCenter, deltaTime, cameraPos);
            }
            if (keyStates[SDL_SCANCODE_A]) {
                M_move_left(keyStates, rightDirection, walls, mapCenter, deltaTime, cameraPos);
            }
            if (keyStates[SDL_SCANCODE_D]) {
                M_move_right(keyStates, rightDirection, walls, mapCenter, deltaTime, cameraPos);
            }

            if (M_fly) { // enable flying movement {
                if (keyStates[SDL_SCANCODE_SPACE]) {
                    cameraPos.y += M_cameraSpeed * deltaTime;
                }
                if (keyStates[SDL_SCANCODE_LCTRL]) {
                    cameraPos.y -= M_cameraSpeed * deltaTime;
                }
            }
        }

        // --- head Bob Calculation ---
        // check if movement keys are pressed
        float M_headbob_offset = 0.0f;
        if (!consoleActive) {
            bool M_isMoving = false; {
                const Uint8 *keyStates = SDL_GetKeyboardState(NULL);
                if (keyStates[SDL_SCANCODE_W] || keyStates[SDL_SCANCODE_S] ||
                    keyStates[SDL_SCANCODE_A] || keyStates[SDL_SCANCODE_D]) {
                    M_isMoving = true;
                }

                if (keyStates[SDL_SCANCODE_A] && keyStates[SDL_SCANCODE_D] && !(keyStates[SDL_SCANCODE_W] || keyStates[SDL_SCANCODE_S])) {
                    M_isMoving = false;
                }
                if (keyStates[SDL_SCANCODE_W] && keyStates[SDL_SCANCODE_S] && !(keyStates[SDL_SCANCODE_A] || keyStates[SDL_SCANCODE_D])) {
                    M_isMoving = false;
                }
            }

            if (M_isMoving) {
                headBobTimer += deltaTime * bobbingSpeed;
                M_headbob_offset = sin(headBobTimer) * bobbingAmplitude;
            }
            else {
                headBobTimer = 0.0f;
            }
        }

        // --- end Head Bob Calculation ---

        // animate console overlay
        float targetAnim = consoleActive ? 1.0f : 0.0f;
        if (consoleAnim < targetAnim) {
            consoleAnim += consoleAnimSpeed * deltaTime;
            if (consoleAnim > targetAnim)
                consoleAnim = targetAnim;
        }
        else if (consoleAnim > targetAnim) {
            consoleAnim -= consoleAnimSpeed * deltaTime;
            if (consoleAnim < targetAnim)
                consoleAnim = targetAnim;
        }

        // Pass M_headbob_offset to the render function
        render(shaderProgram, ourTextureLoc, textureLevel, mapCenter,
               modelLoc, viewLoc, projLoc, VAO, levelVertices, M_headbob_offset);

        // render console overlay if active
        if (consoleAnim > 0.0f) {
            glUseProgram(consoleShaderProgram);
            glm::mat4 ortho = glm::ortho(0.0f, static_cast<float>(WINDOW_WIDTH),
                                         static_cast<float>(WINDOW_HEIGHT), 0.0f);
            glUniformMatrix4fv(consoleProjLoc, 1, GL_FALSE, glm::value_ptr(ortho));
            glm::mat4 consoleModel = glm::translate(glm::mat4(1.0f),
                                                    glm::vec3(0.0f, -consoleHeight + consoleAnim * consoleHeight, 0.0f));
            glUniformMatrix4fv(consoleModelLoc, 1, GL_FALSE, glm::value_ptr(consoleModel));
            glUniform4f(consoleColorLoc, 0.0f, 0.0f, 0.0f, 0.8f);

            glBindVertexArray(consoleVAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glBindVertexArray(0);

            renderText(consoleFont, "> " + commandInput, 10.0f, 10.0f, textShaderProgram);
        }

        // render FPS and pause indicator
        renderText(consoleFont, "FPS: " + std::to_string(static_cast<int>(currentFPS)),
                   WINDOW_WIDTH - 100, 10.0f, textShaderProgram);
        if (G_Paused)
            renderText(consoleFont, "PAUSED", WINDOW_WIDTH / 2 - 50, 50, textShaderProgram);

        SDL_GL_SwapWindow(window);
    }

    // cleanup
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
    glDeleteVertexArrays(1, &consoleVAO);
    glDeleteBuffers(1, &consoleVBO);
    glDeleteProgram(consoleShaderProgram);
    glDeleteProgram(textShaderProgram);

    TTF_CloseFont(consoleFont);
    TTF_Quit();
    IMG_Quit();
    SDL_StopTextInput();
    Mix_FreeMusic(music);
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
