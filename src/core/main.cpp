// SDL2
#include <SDL2/SDL.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_scancode.h>
#include <SDL2/SDL_ttf.h>

// OpenGl
#include <GL/glew.h>

// glm
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// the boys
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

// core
#include "../audio/audio.hpp"
#include "../console/console.hpp"
#include "../fonts/fonts.hpp"
#include "../levels/load.hpp"
#include "../logsystem/log.hpp"
#include "../shaders/shaders.hpp"

#include "globals.hpp"
#include "main.hpp"

// player headers
#include "../player/input.hpp"
#include "../player/keyboard.hpp"
#include "../player/mouse.hpp"

// graphics
#include "../graphics/render.hpp"

int                      S_CurrentVolume     = MIX_MAX_VOLUME / 2;
std::string              R_CurrentTextureKey = "1";
std::vector<std::string> C_CommandHistory;
int                      C_HistoryIndex = -1;

// store the player spawn data
Spawn playerSpawn;

glm::vec3 computeLevelCenter(const std::vector<Wall> &walls) {
    if (walls.empty())
        return {0.0f, 0.0f, 0.0f};

    // initialize with valid data from the first wall.
    float minX = walls[0].x1, maxX = walls[0].x1;
    float minY = walls[0].y1, maxY = walls[0].y1;
    float minZ = walls[0].z1, maxZ = walls[0].z1;

    for (const Wall &w : walls) {
        // skip invalid walls (NaN or Inf)
        if (std::isnan(w.x1) || std::isnan(w.y1) || std::isnan(w.z1) ||
            std::isnan(w.x2) || std::isnan(w.y2) || std::isnan(w.z2) ||
            std::isinf(w.x1) || std::isinf(w.y1) || std::isinf(w.z1) ||
            std::isinf(w.x2) || std::isinf(w.y2) || std::isinf(w.z2)) {
            continue; // skip this wall if invalid
        }

        // update bounds using valid walls only
        minX = std::min({minX, w.x1, w.x2});
        maxX = std::max({maxX, w.x1, w.x2});
        minY = std::min({minY, w.y1, w.y2});
        maxY = std::max({maxY, w.y1, w.y2});
        minZ = std::min({minZ, w.z1, w.z2});
        maxZ = std::max({maxZ, w.z1, w.z2});
    }

    return {(minX + maxX) * 0.5f, (minY + maxY) * 0.5f,
            (minZ + maxZ) * 0.5f};
}

std::vector<float> buildLevelVertices(const std::vector<Sector> &sectors,
                                      const std::vector<Wall>   &walls) {
    std::vector<float> vertices;
    size_t             totalTriangles = 0;
    for (const Sector &sector : sectors)
        totalTriangles += sector.wallCount * 2;
    vertices.reserve(totalTriangles * 5 * 3);

    float RVT_textureScale = 2.5f;

    for (const Sector &sector : sectors) {
        for (int i = 0; i < sector.wallCount; ++i) {
            int wallIndex = sector.startWall + i;
            if (wallIndex < 0 || wallIndex >= static_cast<int>(walls.size()))
                continue;
            const Wall &wall  = walls[wallIndex];
            float       bl[3] = {wall.x1, sector.floor, wall.y1};
            float       br[3] = {wall.x2, sector.floor, wall.y2};
            float       tr[3] = {wall.x2, sector.ceiling, wall.y2};
            float       tl[3] = {wall.x1, sector.ceiling, wall.y1};

            float dx         = wall.x2 - wall.x1;
            float dz         = wall.y2 - wall.y1;
            float wallLength = sqrt(dx * dx + dz * dz);
            float wallHeight = sector.ceiling - sector.floor;

            float texBL[2] = {0.0f, 0.0f};
            float texBR[2] = {wallLength / RVT_textureScale, 0.0f};
            float texTR[2] = {wallLength / RVT_textureScale, wallHeight / RVT_textureScale};
            float texTL[2] = {0.0f, wallHeight / RVT_textureScale};

            vertices.insert(vertices.end(),
                            {bl[0], bl[1], bl[2], texBL[0], texBL[1], br[0], br[1],
                             br[2], texBR[0], texBR[1], tr[0], tr[1], tr[2], texTR[0],
                             texTR[1]});
            vertices.insert(vertices.end(),
                            {bl[0], bl[1], bl[2], texBL[0], texBL[1], tr[0], tr[1],
                             tr[2], texTR[0], texTR[1], tl[0], tl[1], tl[2], texTL[0],
                             texTL[1]});
        }
    }
    return vertices;
}

// -- main shaders --

std::string main_vertex_shader_source_STR =
    readShaderFile("shaders/main/vertex.vert");

std::string main_fragment_shader_source_STR =
    readShaderFile("shaders/main/fragment.frag");

// -- end main shaders --

// -- text shaders --

std::string text_vertex_shader_source_STR =
    readShaderFile("shaders/text/vertex.vert");

std::string text_fragment_shader_source_STR =
    readShaderFile("shaders/text/fragment.frag");

// -- end text shaders --

// -- console shaders --

std::string console_vertex_shader_source_STR =
    readShaderFile("shaders/console/vertex.vert");

std::string console_fragment_shader_source_STR =
    readShaderFile("shaders/console/fragment.frag");

// -- end console shaders --

// -- main convertion to --

const char *main_vertex_shader_source   = main_vertex_shader_source_STR.c_str();
const char *main_fragment_shader_source = main_fragment_shader_source_STR.c_str();

const char *text_vertex_shader_source   = text_vertex_shader_source_STR.c_str();
const char *text_fragment_shader_source = text_fragment_shader_source_STR.c_str();

const char *console_vertex_shader_source = console_vertex_shader_source_STR.c_str();

const char *console_fragment_shader_source = console_fragment_shader_source_STR.c_str();

// -- end main convertion --

int init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        panic("Failed to initialize SDL", SDL_GetError());
        return EXIT_FAILURE;
    }
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        panic("Failed to initialize audio", Mix_GetError());
        return EXIT_FAILURE;
    }
    if (TTF_Init() < 0) {
        panic("Failed to initialize TTF", TTF_GetError());
        return EXIT_FAILURE;
    }
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        panic("Failed to initialize SDL_image", IMG_GetError());
        return EXIT_FAILURE;
    }

    return 0;
}

int main() {
    if (G_debug) {
        info("G_debug is true: Means that messages that use debug() will now work.");
    }

    // initialization
    init();

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    SDL_Window *window = SDL_CreateWindow(
        window_title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        screen_width, screen_height, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    if (!window) {
        panic("Failed to create window", SDL_GetError());
        SDL_Quit();
        return EXIT_FAILURE;
    }

    SDL_SetWindowGrab(window, SDL_TRUE);
    SDL_SetRelativeMouseMode(SDL_TRUE);
    bool currentRelativeMode = true;

    SDL_Surface *iconSurface = IMG_Load(window_icon);
    if (!iconSurface) {
        panic("Failed to load window icon", IMG_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return EXIT_FAILURE;
    }

    SDL_SetWindowIcon(window, iconSurface);
    SDL_FreeSurface(iconSurface);

    SDL_GLContext context = SDL_GL_CreateContext(window);
    if (!context) {
        panic("Failed to create OpenGL context", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return EXIT_FAILURE;
    }

    // initialize GLEW
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        panic("Failed to initialize GLEW");
        SDL_GL_DeleteContext(context);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return EXIT_FAILURE;
    }

    // setup basic OpenGL state
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // enable alpha blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // in older versions, each shader compilation had a unique function name.
    // this caused unnecessary duplication across the codebase.
    // now, we use a single function, "createShaderProgram," for all shaders.
    GLuint main_shader_program = createShaderProgram(
        main_vertex_shader_source, main_fragment_shader_source);
    GLuint console_shader_program = createShaderProgram(
        console_vertex_shader_source, console_fragment_shader_source);
    GLuint text_shader_program = createShaderProgram(text_vertex_shader_source,
                                                     text_fragment_shader_source);

    // checks if shader compilation failed
    if (main_shader_program == 0 || console_shader_program == 0 ||
        text_shader_program == 0) {
        panic("Failed to create shader programs.");
    }

    GLint modelLoc      = glGetUniformLocation(main_shader_program, "model");
    GLint viewLoc       = glGetUniformLocation(main_shader_program, "view");
    GLint projLoc       = glGetUniformLocation(main_shader_program, "projection");
    GLint ourTextureLoc = glGetUniformLocation(main_shader_program, "ourTexture");

    // console
    GLint consoleProjLoc =
        glGetUniformLocation(console_shader_program, "projection");
    GLint consoleModelLoc = glGetUniformLocation(console_shader_program, "model");
    GLint consoleColorLoc = glGetUniformLocation(console_shader_program, "color");

    std::vector<Sector>                     sectors;
    std::vector<Wall>                       walls;
    std::unordered_map<std::string, GLuint> textureLevel;
    LevelMetadata                           levelMetadata;

    if (!L_LoadLevel(default_level_path, sectors, walls, textureLevel,
                     playerSpawn, levelMetadata)) {
        SDL_GL_DeleteContext(context);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return EXIT_FAILURE;
    }

    // playAudio("resources/songs/demo.ogg", S_CurrentVolume);

    std::vector<float> levelVertices = buildLevelVertices(sectors, walls);
    glm::vec3          mapCenter     = computeLevelCenter(walls);

    // initialize player position using spawn point
    if (G_debug) {
        // not using debug() because it's not a string
        std::cout << "Player spawn: x=" << playerSpawn.x << ", y=" << playerSpawn.y
                  << ", size=" << playerSpawn.size << std::endl;
        std::cout << "Map center: x=" << mapCenter.x << ", y=" << mapCenter.y
                  << ", z=" << mapCenter.z << std::endl;
    }

    // set camera position from spawn data - ensure we're starting at a valid
    // position Y coordinate (height) remains at default value (player.input.cpp
    // initializes it to 2.0f)
    cameraPos.x = playerSpawn.x;
    cameraPos.z =
        playerSpawn.y; // note: spawn y is used for the z-coordinate in 3D space

    if (G_debug) {
        // not using debug() because it's not a string
        std::cout << "Initial camera position: " << cameraPos.x << ", "
                  << cameraPos.y << ", " << cameraPos.z << std::endl;
    }

    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, levelVertices.size() * sizeof(float),
                 levelVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)nullptr);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    float  consoleVertices[] = {0.0f,
                                0.0f,
                                static_cast<float>(screen_width),
                                0.0f,
                                static_cast<float>(screen_width),
                                static_cast<float>(consoleHeight),
                                0.0f,
                                0.0f,
                                static_cast<float>(screen_width),
                                static_cast<float>(consoleHeight),
                                0.0f,
                                static_cast<float>(consoleHeight)};
    GLuint consoleVAO, consoleVBO;
    glGenVertexArrays(1, &consoleVAO);
    glGenBuffers(1, &consoleVBO);
    glBindVertexArray(consoleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, consoleVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(consoleVertices), consoleVertices,
                 GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)nullptr);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    SDL_GL_SetSwapInterval(1);

    // load and configure the font for best quality
    TTF_Font *consoleFont = TTF_OpenFont(default_font, 18);
    if (!consoleFont) {
        warn("Failed to load font", TTF_GetError());
    } else {
        // enable light hinting for better quality
        TTF_SetFontHinting(consoleFont, TTF_HINTING_LIGHT);

        // set other font properties for optimal quality
        TTF_SetFontStyle(consoleFont, TTF_STYLE_NORMAL);
        TTF_SetFontKerning(consoleFont,
                           1);              // enable kerning for better letter spacing
        TTF_SetFontOutline(consoleFont, 0); // no outline
    }

    extern std::string commandInput;

    G_running = true;
    SDL_Event event;
    Uint32    lastTime    = SDL_GetTicks();
    float     fps_timer   = 0.0f;
    float     fps_count   = 0;
    float     current_FPS = 0.0f;

    SDL_StartTextInput();

    // main loop
    while (G_running) {
        // update relative mouse mode
        if (consoleActive && currentRelativeMode) {
            SDL_SetRelativeMouseMode(SDL_FALSE);
            currentRelativeMode = false;
        } else if (!consoleActive && !currentRelativeMode) {
            SDL_SetRelativeMouseMode(SDL_TRUE);
            currentRelativeMode = true;
        }

        Uint32 currentTime = SDL_GetTicks();
        if (currentTime < lastTime) {
            G_deltaTime = ((UINT32_MAX - lastTime) + currentTime + 1) / 1000.0f;
        } else {
            G_deltaTime = (currentTime - lastTime) / 1000.0f;
        }

        lastTime = currentTime;

        fps_timer += G_deltaTime;
        fps_count++;
        if (fps_timer >= 1.0f) {
            current_FPS = fps_count / fps_timer;
            fps_count   = 0;
            fps_timer   = 0.0f;
        }

        // process events
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                G_running = false;
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
                        processCommand(commandInput, G_running, sectors, walls,
                                       levelVertices, VBO, textureLevel, M_MouseSensitivity,
                                       S_CurrentVolume, window, C_CommandHistory,
                                       C_HistoryIndex);
                        mapCenter = computeLevelCenter(walls);
                    } else if (event.key.keysym.sym == SDLK_UP) {
                        if (!C_CommandHistory.empty() && C_HistoryIndex > 0) {
                            C_HistoryIndex--;
                            commandInput = C_CommandHistory[C_HistoryIndex];
                        }
                    } else if (event.key.keysym.sym == SDLK_DOWN) {
                        if (!C_CommandHistory.empty() &&
                            C_HistoryIndex <
                                static_cast<int>(C_CommandHistory.size()) - 1) {
                            C_HistoryIndex++;
                            commandInput = C_CommandHistory[C_HistoryIndex];
                        } else {
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
                    M_processMouseInput(event.motion.xrel, event.motion.yrel, M_pitch,
                                        M_yaw, M_MouseSensitivity, cameraFront);
                }
                break;
            default:
                break;
            }
        }

        if (!consoleActive) {
            const Uint8 *keyStates = SDL_GetKeyboardState(nullptr);
            glm::vec3    forwardDirection =
                glm::normalize(glm::vec3(cameraFront.x, 0.0f, cameraFront.z));
            glm::vec3 rightDirection =
                glm::normalize(glm::cross(forwardDirection, cameraUp));

            // track if any movement key is pressed
            bool movementKeyPressed = false;

            // use a combined movement direction to prevent diagonal speedup
            glm::vec3 movementDirection(0.0f, 0.0f, 0.0f);

            if (keyStates[SDL_SCANCODE_W]) {
                movementDirection += forwardDirection;
                movementKeyPressed = true;
            }
            if (keyStates[SDL_SCANCODE_S]) {
                movementDirection -= forwardDirection;
                movementKeyPressed = true;
            }
            if (keyStates[SDL_SCANCODE_A]) {
                movementDirection -= rightDirection;
                movementKeyPressed = true;
            }
            if (keyStates[SDL_SCANCODE_D]) {
                movementDirection += rightDirection;
                movementKeyPressed = true;
            }

            // normalize the combined movement direction to prevent diagonal speedup
            if (movementKeyPressed && glm::length(movementDirection) > 0.001f) {
                movementDirection = glm::normalize(movementDirection);

                // update velocity using the combined normalized direction
                updateVelocity(M_currentVelocity, movementDirection, true, G_deltaTime);

                // apply movement using the current velocity
                glm::vec3 movement = M_currentVelocity * G_deltaTime;
                moveWithCollision(cameraPos, movement, walls, mapCenter,
                                  M_collisionRadius);
            } else {
                // apply deceleration when no movement keys are pressed
                M_apply_deceleration(walls, mapCenter, G_deltaTime, cameraPos);
            }

            if (M_fly) { // enable flying movement {
                if (keyStates[SDL_SCANCODE_SPACE]) {
                    cameraPos.y += M_cameraSpeed * G_deltaTime;
                }
                if (keyStates[SDL_SCANCODE_LCTRL]) {
                    cameraPos.y -= M_cameraSpeed * G_deltaTime;
                }
            }
        }

        // --- head Bob Calculation ---
        // check if movement keys are pressed
        float M_headbob_offset = 0.0f;
        if (!consoleActive) {
            bool M_isMoving = false;
            {
                const Uint8 *keyStates = SDL_GetKeyboardState(nullptr);
                if (keyStates[SDL_SCANCODE_W] || keyStates[SDL_SCANCODE_S] ||
                    keyStates[SDL_SCANCODE_A] || keyStates[SDL_SCANCODE_D]) {
                    M_isMoving = true;
                }

                if (keyStates[SDL_SCANCODE_A] && keyStates[SDL_SCANCODE_D] &&
                    !(keyStates[SDL_SCANCODE_W] || keyStates[SDL_SCANCODE_S])) {
                    M_isMoving = false;
                }
                if (keyStates[SDL_SCANCODE_W] && keyStates[SDL_SCANCODE_S] &&
                    !(keyStates[SDL_SCANCODE_A] || keyStates[SDL_SCANCODE_D])) {
                    M_isMoving = false;
                }
            }

            // add variables for head bob deceleration (needs to be static, or will simply not work,
            // we don't know why)
            static float M_headbob_timer     = 0.0f;
            static float M_bobbing_speed     = 10.0f;
            static float M_bobbing_amplitude = 0.15f;
            static float M_headbob_amplitude = 0.0f;
            static bool  M_wasMoving         = false;

            if (M_isMoving) {
                // when moving, increase the timer and set amplitude to max
                M_headbob_timer += G_deltaTime * M_bobbing_speed;
                M_headbob_amplitude = M_bobbing_amplitude;
                M_wasMoving         = true;
            } else {
                if (M_wasMoving) {
                    // continue the head bob cycle but decrease amplitude
                    M_headbob_timer += G_deltaTime * M_bobbing_speed;

                    // gradually decrease amplitude (deceleration)
                    float decelerationRate = 2.0f; // adjust this value to control how
                    // quickly head bob fades out
                    M_headbob_amplitude -= decelerationRate * G_deltaTime;

                    // clamp to zero to avoid negative values
                    if (M_headbob_amplitude <= 0.0f) {
                        M_headbob_amplitude = 0.0f;
                        M_wasMoving         = false;
                        M_headbob_timer     = 0.0f; // reset timer only when fully stopped
                    }
                }
            }

            // apply head bob with current amplitude
            M_headbob_offset = sin(M_headbob_timer) * M_headbob_amplitude;
        }

        // --- end Head Bob Calculation ---

        // animate console overlay
        float targetAnim = consoleActive ? 1.0f : 0.0f;
        if (consoleAnim < targetAnim) {
            consoleAnim += consoleAnimSpeed * G_deltaTime;
            if (consoleAnim > targetAnim)
                consoleAnim = targetAnim;
        } else if (consoleAnim > targetAnim) {
            consoleAnim -= consoleAnimSpeed * G_deltaTime;
            if (consoleAnim < targetAnim)
                consoleAnim = targetAnim;
        }

        // first render the 3D scene
        RT_render(main_shader_program, ourTextureLoc, textureLevel, mapCenter, modelLoc,
                  viewLoc, projLoc, VAO, levelVertices, M_headbob_offset,
                  R_CurrentTextureKey);

        // clear depth buffer to ensure text isn't occluded by 3D geometry
        glClear(GL_DEPTH_BUFFER_BIT);

        // render console if active
        if (consoleAnim > 0.0f) {
            // Draw console background
            glUseProgram(console_shader_program);
            glm::mat4 ortho = glm::ortho(0.0f, static_cast<float>(screen_width),
                                         static_cast<float>(screen_height), 0.0f);
            glUniformMatrix4fv(consoleProjLoc, 1, GL_FALSE, glm::value_ptr(ortho));
            glm::mat4 consoleModel = glm::translate(
                glm::mat4(1.0f),
                glm::vec3(0.0f, -consoleHeight + consoleAnim * consoleHeight, 0.0f));
            glUniformMatrix4fv(consoleModelLoc, 1, GL_FALSE,
                               glm::value_ptr(consoleModel));
            glUniform4f(consoleColorLoc, 0.0f, 0.0f, 0.0f, 0.8f);

            glBindVertexArray(consoleVAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glBindVertexArray(0);

            // draw console text
            RT_renderText(consoleFont, "> " + commandInput, 10.0f, 10.0f,
                          text_shader_program);
        }

        // render HUD elements
        RT_renderText(consoleFont,
                      "FPS: " + std::to_string(static_cast<int>(current_FPS)),
                      screen_width - 100, 10.0f, text_shader_program);
        if (G_Paused)
            RT_renderText(consoleFont, "Paused", screen_width / 2 - 50, 50,
                          text_shader_program);

        // swap the frame buffer
        SDL_GL_SwapWindow(window);
    }

    // cleanup
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(main_shader_program);
    glDeleteVertexArrays(1, &consoleVAO);
    glDeleteBuffers(1, &consoleVBO);
    glDeleteProgram(console_shader_program);
    glDeleteProgram(text_shader_program);

    TTF_CloseFont(consoleFont);
    TTF_Quit();
    IMG_Quit();
    SDL_StopTextInput();
    // Mix_FreeMusic(music);
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    // use the summary function to print the total number of messages, e.g:
    // [SUMMARY] 10 messages
    if (G_debug) {
        summary();
    }
    return 0;
}