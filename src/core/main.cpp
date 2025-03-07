// sdl2
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
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

// core
#include "../console/console.hpp"
#include "../errors/error.hpp"
#include "../fonts/fonts.hpp"
#include "../levels/load.hpp"
#include "../shaders/shaders.hpp"
#include "globals.hpp"
#include "main.hpp"

// player headers
#include "../player/input.hpp"
#include "../player/keyboard.hpp"
#include "../player/mouse.hpp"

// graphics
#include "../graphics/render.hpp"
#include "../graphics/texture.hpp"

int S_CurrentVolume = MIX_MAX_VOLUME / 2;
std::string R_CurrentTextureKey = "1";
std::vector<std::string> C_CommandHistory;
int C_HistoryIndex = -1;

// store the player spawn data
Spawn playerSpawn;

glm::vec3 computeLevelCenter(const std::vector<Wall> &walls) {
  if (walls.empty()) return glm::vec3(0.0f, 0.0f, 0.0f);

  // Initialize with valid data from the first wall.
  float minX = walls[0].x1, maxX = walls[0].x1;
  float minY = walls[0].y1, maxY = walls[0].y1;
  float minZ = walls[0].z1, maxZ = walls[0].z1;

  for (const Wall &w : walls) {
    // skip invalid walls (NaN or Inf)
    if (std::isnan(w.x1) || std::isnan(w.y1) || std::isnan(w.z1) ||
        std::isnan(w.x2) || std::isnan(w.y2) || std::isnan(w.z2) ||
        std::isinf(w.x1) || std::isinf(w.y1) || std::isinf(w.z1) ||
        std::isinf(w.x2) || std::isinf(w.y2) || std::isinf(w.z2)) {
      continue;  // skip this wall if invalid
    }

    // update bounds using valid walls only
    minX = std::min({minX, w.x1, w.x2});
    maxX = std::max({maxX, w.x1, w.x2});
    minY = std::min({minY, w.y1, w.y2});
    maxY = std::max({maxY, w.y1, w.y2});
    minZ = std::min({minZ, w.z1, w.z2});
    maxZ = std::max({maxZ, w.z1, w.z2});
  }

  return glm::vec3((minX + maxX) * 0.5f, (minY + maxY) * 0.5f,
                   (minZ + maxZ) * 0.5f);
}

std::vector<float> buildLevelVertices(const std::vector<Sector> &sectors,
                                      const std::vector<Wall> &walls) {
  std::vector<float> vertices;
  size_t totalTriangles = 0;
  for (const Sector &sector : sectors) totalTriangles += sector.wallCount * 2;
  vertices.reserve(totalTriangles * 5 * 3);

  const float textureScale = 2.5f;

  for (const Sector &sector : sectors) {
    for (int i = 0; i < sector.wallCount; ++i) {
      int wallIndex = sector.startWall + i;
      if (wallIndex < 0 || wallIndex >= static_cast<int>(walls.size()))
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

      vertices.insert(
          vertices.end(),
          {bl[0], bl[1], bl[2], texBL[0], texBL[1], br[0], br[1], br[2],
           texBR[0], texBR[1], tr[0], tr[1], tr[2], texTR[0], texTR[1]});
      vertices.insert(
          vertices.end(),
          {bl[0], bl[1], bl[2], texBL[0], texBL[1], tr[0], tr[1], tr[2],
           texTR[0], texTR[1], tl[0], tl[1], tl[2], texTL[0], texTL[1]});
    }
  }
  return vertices;
}

// -- main shaders --

std::string vertex_shader_source_STR =
    readShaderFile("shaders/main/vertex.vert");
const char *vertex_shader_source = vertex_shader_source_STR.c_str();

std::string fragment_shader_source_STR =
    readShaderFile("shaders/main/fragment.frag");
const char *fragment_shader_source = fragment_shader_source_STR.c_str();

// -- end main shaders --

// -- text shaders --

std::string text_vertex_shader_source_STR =
    readShaderFile("shaders/text/vertex.vert");
const char *text_vertex_shader_source = text_vertex_shader_source_STR.c_str();

std::string text_fragment_shader_source_STR =
    readShaderFile("shaders/text/fragment.frag");
const char *text_fragment_shader_source =
    text_fragment_shader_source_STR.c_str();

// -- end text shaders --

// -- console shaders --

std::string console_vertex_shader_source_STR =
    readShaderFile("shaders/console/vertex.vert");
const char *console_vertex_shader_source =
    console_vertex_shader_source_STR.c_str();

std::string console_fragment_shader_source_STR =
    readShaderFile("shaders/console/fragment.frag");
const char *console_fragment_shader_source =
    console_fragment_shader_source_STR.c_str();

// -- end console shaders --

int init() {
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    critical("Failed to initialize SDL", SDL_GetError());
    return -1;
  }
  if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
    critical("Failed to initialize audio", Mix_GetError());
    return -1;
  }
  if (TTF_Init() < 0) {
    critical("Failed to initialize TTF", TTF_GetError());
    return -1;
  }
  if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
    critical("Failed to initialize SDL_image", IMG_GetError());
    return -1;
  }
  return 0;
}

int main() {
  // initialization
  if (init() < 0) return -1;

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  SDL_Window *window = SDL_CreateWindow(
      window_title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
      window_width, window_height, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
  if (!window) {
    critical("Failed to create window", SDL_GetError());
    SDL_Quit();
    return -1;
  }
  SDL_SetWindowGrab(window, SDL_TRUE);
  bool currentRelativeMode = true;
  SDL_SetRelativeMouseMode(SDL_TRUE);

  SDL_Surface *iconSurface = IMG_Load(window_icon);
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

  // TEMP: musiclevel is only for tests, musicLevel will be on the map
  const char *musicLevel = "resources/songs/demo.ogg";
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
  GLuint shaderProgram =
      createShaderProgram(vertex_shader_source, fragment_shader_source);
  GLuint consoleShaderProgram = createShaderProgram(
      console_vertex_shader_source, console_fragment_shader_source);
  GLuint textShaderProgram = createShaderProgram(text_vertex_shader_source,
                                                 text_fragment_shader_source);

  // checks if shader compilation failed
  if (shaderProgram == 0 || consoleShaderProgram == 0 ||
      textShaderProgram == 0) {
    critical("Failed to create shader programs.");
  }

  GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
  GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
  GLint projLoc = glGetUniformLocation(shaderProgram, "projection");
  GLint ourTextureLoc = glGetUniformLocation(shaderProgram, "ourTexture");

  // console
  GLint consoleProjLoc =
      glGetUniformLocation(consoleShaderProgram, "projection");
  GLint consoleModelLoc = glGetUniformLocation(consoleShaderProgram, "model");
  GLint consoleColorLoc = glGetUniformLocation(consoleShaderProgram, "color");

  std::vector<Sector> sectors;
  std::vector<Wall> walls;
  std::unordered_map<std::string, GLuint> textureLevel;

  if (!L_LoadLevel(default_level_path, sectors, walls, textureLevel,
                   playerSpawn)) {
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return -1;
  }

  std::vector<float> levelVertices = buildLevelVertices(sectors, walls);
  glm::vec3 mapCenter = computeLevelCenter(walls);

  // Initialize player position using spawn point
  std::cout << "Player spawn: x=" << playerSpawn.x << ", y=" << playerSpawn.y
            << ", size=" << playerSpawn.size << std::endl;
  std::cout << "Map center: x=" << mapCenter.x << ", y=" << mapCenter.y
            << ", z=" << mapCenter.z << std::endl;

  // Set camera position from spawn data - ensure we're starting at a valid
  // position Y coordinate (height) remains at default value (player.input.cpp
  // initializes it to 2.0f)
  cameraPos.x = playerSpawn.x;
  cameraPos.z =
      playerSpawn.y;  // Note: spawn y is used for the z-coordinate in 3D space

  std::cout << "Initial camera position: " << cameraPos.x << ", " << cameraPos.y
            << ", " << cameraPos.z << std::endl;

  GLuint VAO, VBO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, levelVertices.size() * sizeof(float),
               levelVertices.data(), GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                        (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  float consoleVertices[] = {0.0f,
                             0.0f,
                             static_cast<float>(window_width),
                             0.0f,
                             static_cast<float>(window_width),
                             static_cast<float>(consoleHeight),
                             0.0f,
                             0.0f,
                             static_cast<float>(window_width),
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

  G_running = true;
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
    float G_deltaTime = (currentTime - lastTime) / 1000.0f;
    lastTime = currentTime;

    fpsTimer += G_deltaTime;
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
          G_running = false;
          break;
        case SDL_KEYDOWN:
          if (event.key.keysym.sym == SDLK_QUOTE) consoleActive = true;
          if (event.key.keysym.sym == SDLK_ESCAPE)
            consoleActive = false;
          else if (consoleActive) {
            if (event.key.keysym.sym == SDLK_BACKSPACE && !commandInput.empty())
              commandInput.pop_back();
            else if (event.key.keysym.sym == SDLK_RETURN) {
              processCommand(commandInput, G_running, sectors, walls,
                             levelVertices, VBO, textureLevel,
                             M_MouseSensitivity, S_CurrentVolume, window,
                             C_CommandHistory, C_HistoryIndex);
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
          if (consoleActive) commandInput += event.text.text;
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
      const Uint8 *keyStates = SDL_GetKeyboardState(NULL);
      glm::vec3 forwardDirection =
          glm::normalize(glm::vec3(cameraFront.x, 0.0f, cameraFront.z));
      glm::vec3 rightDirection =
          glm::normalize(glm::cross(forwardDirection, cameraUp));

      if (keyStates[SDL_SCANCODE_W]) {
        M_move_forward(forwardDirection, walls, mapCenter, G_deltaTime,
                       cameraPos);
      }
      if (keyStates[SDL_SCANCODE_S]) {
        M_move_backward(forwardDirection, walls, mapCenter, G_deltaTime,
                        cameraPos);
      }
      if (keyStates[SDL_SCANCODE_A]) {
        M_move_left(rightDirection, walls, mapCenter, G_deltaTime, cameraPos);
      }
      if (keyStates[SDL_SCANCODE_D]) {
        M_move_right(rightDirection, walls, mapCenter, G_deltaTime, cameraPos);
      }

      if (M_fly) {  // enable flying movement {
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
        const Uint8 *keyStates = SDL_GetKeyboardState(NULL);
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

      if (M_isMoving) {
        headBobTimer += G_deltaTime * bobbingSpeed;
        M_headbob_offset = sin(headBobTimer) * bobbingAmplitude;
      } else {
        headBobTimer = 0.0f;
      }
    }

    // --- end Head Bob Calculation ---

    // animate console overlay
    float targetAnim = consoleActive ? 1.0f : 0.0f;
    if (consoleAnim < targetAnim) {
      consoleAnim += consoleAnimSpeed * G_deltaTime;
      if (consoleAnim > targetAnim) consoleAnim = targetAnim;
    } else if (consoleAnim > targetAnim) {
      consoleAnim -= consoleAnimSpeed * G_deltaTime;
      if (consoleAnim < targetAnim) consoleAnim = targetAnim;
    }

    // Pass M_headbob_offset to the render function
    RS_render(shaderProgram, ourTextureLoc, textureLevel, mapCenter, modelLoc,
              viewLoc, projLoc, VAO, levelVertices, M_headbob_offset,
              R_CurrentTextureKey);

    // render console overlay if active
    if (consoleAnim > 0.0f) {
      glUseProgram(consoleShaderProgram);
      glm::mat4 ortho = glm::ortho(0.0f, static_cast<float>(window_width),
                                   static_cast<float>(window_height), 0.0f);
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

      RS_renderText(consoleFont, "> " + commandInput, 10.0f, 10.0f,
                    textShaderProgram);
    }

    // render FPS and pause indicator
    RS_renderText(consoleFont,
                  "FPS: " + std::to_string(static_cast<int>(currentFPS)),
                  window_width - 100, 10.0f, textShaderProgram);
    if (G_Paused)
      RS_renderText(consoleFont, "Paused", window_width / 2 - 50, 50,
                    textShaderProgram);

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
