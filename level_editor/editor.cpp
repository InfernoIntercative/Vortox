#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <GL/glu.h>
#include <cstdio>
#include <vector>
#include <fstream>
#include <cmath>
#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>

#include "../lib/imgui/imgui.h"
#include "../lib/imgui/imgui_impl_sdl2.h"
#include "../lib/imgui/imgui_impl_opengl3.h"
#include "editor.hpp"

static char gTextureInput[256];
static char gLevelNameInput[256];

bool snapToGrid = false;
bool snapSpawnToGrid = false;
bool showEndpoints = false;
bool showSpawnPoint = true;
float globalWallThickness = 2.0f;

struct Point2D
{
    int x;
    int y;
};

struct LineWall
{
    Point2D start;
    Point2D end;
    int textureID;
    int flags;
    ImU32 color;
    float thickness;
};

double calculateDistance(double px, double py, double ax, double ay, double bx, double by)
{
    double segDX = bx - ax;
    double segDY = by - ay;
    double deltaX = px - ax;
    double deltaY = py - ay;
    double segLengthSq = segDX * segDX + segDY * segDY;
    double projection = deltaX * segDX + deltaY * segDY;

    if (projection <= 0)
        return std::sqrt((px - ax) * (px - ax) + (py - ay) * (py - ay));
    if (projection >= segLengthSq)
        return std::sqrt((px - bx) * (px - bx) + (py - by) * (py - by));
    double ratio = projection / segLengthSq;
    double projX = ax + ratio * segDX;
    double projY = ay + ratio * segDY;
    return std::sqrt((px - projX) * (px - projX) + (py - projY) * (py - projY));
}


Point2D rotatePoint(const Point2D &p, const Point2D &center, double angle)
{
    double s = std::sin(angle);
    double c = std::cos(angle);
    int translatedX = p.x - center.x;
    int translatedY = p.y - center.y;
    int rotatedX = static_cast<int>(translatedX * c - translatedY * s);
    int rotatedY = static_cast<int>(translatedX * s + translatedY * c);
    return {center.x + rotatedX, center.y + rotatedY};
}


void SaveScreenshot(const char *filename, int width, int height)
{
    std::vector<unsigned char> pixels(width * height * 4);
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

    for (int j = 0; j < height / 2; j++)
    {
        for (int i = 0; i < width * 4; i++)
        {
            std::swap(pixels[j * width * 4 + i], pixels[(height - 1 - j) * width * 4 + i]);
        }
    }
    SDL_Surface *surface = SDL_CreateRGBSurfaceFrom(
        pixels.data(), width, height, 32, width * 4,
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
        0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff
#else
        0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000
#endif
    );
    if (surface)
    {
        SDL_SaveBMP(surface, filename);
        SDL_FreeSurface(surface);
        std::printf("Screenshot saved as %s!\n", filename);
    }
    else
    {
        std::printf("Failed to create screenshot surface.\n");
    }
}

bool spawnCreated = false;
SDL_Rect spawnSquare = {0, 0, 30, 30};
int spawnSquareSize = 30;
bool draggingSpawn = false;
int spawnDragOffsetX = 0, spawnDragOffsetY = 0;
float spawnColor[3] = {1.0f, 1.0f, 0.0f};

int main(int argc, char *argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        std::printf("SDL Initialization error: %s\n", SDL_GetError());
        return -1;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

    SDL_Window *window = SDL_CreateWindow(
        window_title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        window_width, window_height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!window)
    {
        std::printf("Window creation failed: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    SDL_GLContext glContext = SDL_GL_CreateContext(window);
    if (!glContext)
    {
        std::printf("OpenGL context creation failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }
    SDL_GL_MakeCurrent(window, glContext);
    SDL_GL_SetSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForOpenGL(window, glContext);
    ImGui_ImplOpenGL3_Init("#version 150");

    double viewZoom = 1.0;
    double viewOffsetX = 0.0, viewOffsetY = 0.0;
    int gridSpacing = 30;
    bool gridVisible = true;
    bool panningActive = false;
    int panOriginX = 0, panOriginY = 0;
    double panStartOffsetX = 0.0, panStartOffsetY = 0.0;

    std::vector<LineWall> walls;
    bool wallInProgress = false;
    Point2D wallStartPoint = {0, 0};
    std::vector<LineWall> undoStack;
    Point2D mouseWorldPos = {0, 0};
    const double levelScaleFactor = 0.1;
    bool selectionMode = false;
    int selectedWall = -1;
    bool draggingWall = false;
    bool draggingEndpoint = false;
    int whichEndpoint = -1;
    Point2D dragStartMouse = {0, 0};
    Point2D initialWallStart = {0, 0};
    Point2D initialWallEnd = {0, 0};

    std::string levelVersion = "4.22";
    float ceilingHeight = 10.0f;
    float floorHeight = 0.0f;
    std::string textureFilePath = "resources/textures/brick.png";
    bool textureEditing = false;
    bool hudVisible = true;
    float newWallThickness = 2.0f;

    std::vector<ImU32> wallColorOptions = {
        IM_COL32(255, 255, 255, 255),
        IM_COL32(255, 0, 0, 255),
        IM_COL32(0, 255, 0, 255),
        IM_COL32(0, 0, 255, 255)};
    std::vector<ImU32> bgColorOptions = {
        IM_COL32(30, 30, 30, 255),
        IM_COL32(20, 20, 50, 255),
        IM_COL32(50, 50, 50, 255)};
    int currentBgIndex = 0;
    ImU32 currentBgColor = bgColorOptions[currentBgIndex];
    std::string activeTool = "Drawing";

    const double rotationIncrement = 5.0 * M_PI / 180.0;

    bool G_Running = true;
    SDL_Event event;
    while (G_Running)
    {
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);

            if (event.type == SDL_QUIT)
            {
                G_Running = false;
            }


            bool hudActive = io.WantCaptureMouse || io.WantCaptureKeyboard;


            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT && !io.WantCaptureMouse)
            {
                int wx = static_cast<int>(event.button.x / viewZoom - viewOffsetX);
                int wy = static_cast<int>(event.button.y / viewZoom - viewOffsetY);
                if (spawnCreated &&
                    wx >= spawnSquare.x && wx <= spawnSquare.x + spawnSquare.w &&
                    wy >= spawnSquare.y && wy <= spawnSquare.y + spawnSquare.h)
                {
                    draggingSpawn = true;
                    spawnDragOffsetX = wx - spawnSquare.x;
                    spawnDragOffsetY = wy - spawnSquare.y;
                    continue;
                }
            }
            if (event.type == SDL_MOUSEBUTTONUP)
            {
                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    draggingSpawn = false;
                    if (selectionMode)
                    {
                        draggingWall = false;
                        draggingEndpoint = false;
                        whichEndpoint = -1;
                    }
                }
                if (event.button.button == SDL_BUTTON_RIGHT)
                {
                    panningActive = false;
                }
            }
            if (event.type == SDL_MOUSEMOTION)
            {
                int posX = static_cast<int>(event.motion.x / viewZoom - viewOffsetX);
                int posY = static_cast<int>(event.motion.y / viewZoom - viewOffsetY);
                if (snapToGrid || (SDL_GetModState() & KMOD_CTRL))
                {
                    posX = static_cast<int>(std::round(posX / static_cast<double>(gridSpacing)) * gridSpacing);
                    posY = static_cast<int>(std::round(posY / static_cast<double>(gridSpacing)) * gridSpacing);
                }
                mouseWorldPos = {posX, posY};

                if (panningActive && !io.WantCaptureMouse)
                {
                    viewOffsetX = panStartOffsetX + (event.motion.x - panOriginX) / viewZoom;
                    viewOffsetY = panStartOffsetY + (event.motion.y - panOriginY) / viewZoom;
                }

                if (draggingSpawn)
                {
                    int newX = mouseWorldPos.x - spawnDragOffsetX;
                    int newY = mouseWorldPos.y - spawnDragOffsetY;
                    if (snapSpawnToGrid)
                    {
                        newX = static_cast<int>(std::round(newX / static_cast<double>(gridSpacing)) * gridSpacing);
                        newY = static_cast<int>(std::round(newY / static_cast<double>(gridSpacing)) * gridSpacing);
                    }
                    spawnSquare.x = newX;
                    spawnSquare.y = newY;
                }

                if (selectionMode)
                {
                    if (draggingWall && selectedWall != -1)
                    {
                        int dx = mouseWorldPos.x - dragStartMouse.x;
                        int dy = mouseWorldPos.y - dragStartMouse.y;
                        walls[selectedWall].start.x = initialWallStart.x + dx;
                        walls[selectedWall].start.y = initialWallStart.y + dy;
                        walls[selectedWall].end.x = initialWallEnd.x + dx;
                        walls[selectedWall].end.y = initialWallEnd.y + dy;
                    }
                    if (draggingEndpoint && selectedWall != -1)
                    {
                        int dx = mouseWorldPos.x - dragStartMouse.x;
                        int dy = mouseWorldPos.y - dragStartMouse.y;
                        if (whichEndpoint == 0)
                        {
                            walls[selectedWall].start.x = initialWallStart.x + dx;
                            walls[selectedWall].start.y = initialWallStart.y + dy;
                        }
                        else if (whichEndpoint == 1)
                        {
                            walls[selectedWall].end.x = initialWallEnd.x + dx;
                            walls[selectedWall].end.y = initialWallEnd.y + dy;
                        }

                        const int snapTolerance = 10;
                        Point2D &movingPoint = (whichEndpoint == 0) ? walls[selectedWall].start : walls[selectedWall].end;
                        for (size_t i = 0; i < walls.size(); i++)
                        {
                            if (static_cast<int>(i) == selectedWall)
                                continue;
                            Point2D candidates[2] = {walls[i].start, walls[i].end};
                            for (int j = 0; j < 2; j++)
                            {
                                int dist = std::sqrt(std::pow(movingPoint.x - candidates[j].x, 2) +
                                                     std::pow(movingPoint.y - candidates[j].y, 2));
                                if (dist < snapTolerance)
                                    movingPoint = candidates[j];
                            }
                        }
                    }
                }
            }
            if (event.type == SDL_MOUSEWHEEL && !io.WantCaptureMouse)
            {
                int mx, my;
                SDL_GetMouseState(&mx, &my);
                double oldZoom = viewZoom;
                if (event.wheel.y > 0)
                    viewZoom *= 1.1;
                else if (event.wheel.y < 0)
                    viewZoom /= 1.1;
                viewOffsetX += (mx / viewZoom - mx / oldZoom);
                viewOffsetY += (my / viewZoom - my / oldZoom);
            }


            if (event.type == SDL_MOUSEBUTTONDOWN && !io.WantCaptureMouse)
            {
                if (event.button.button == SDL_BUTTON_RIGHT)
                {
                    panningActive = true;
                    panOriginX = event.button.x;
                    panOriginY = event.button.y;
                    panStartOffsetX = viewOffsetX;
                    panStartOffsetY = viewOffsetY;
                    activeTool = "Panning";
                }
                else if (event.button.button == SDL_BUTTON_LEFT)
                {
                    int wx = static_cast<int>(event.button.x / viewZoom - viewOffsetX);
                    int wy = static_cast<int>(event.button.y / viewZoom - viewOffsetY);
                    if (snapToGrid || (SDL_GetModState() & KMOD_CTRL))
                    {
                        wx = static_cast<int>(std::round(wx / static_cast<double>(gridSpacing)) * gridSpacing);
                        wy = static_cast<int>(std::round(wy / static_cast<double>(gridSpacing)) * gridSpacing);
                    }
                    if (!selectionMode)
                    {
                        activeTool = "Drawing";
                        if (!wallInProgress)
                        {
                            wallStartPoint = {wx, wy};
                            wallInProgress = true;
                            undoStack.clear();
                        }
                        else
                        {
                            LineWall newWall;
                            newWall.start = wallStartPoint;
                            newWall.end = {wx, wy};
                            newWall.textureID = 1;
                            newWall.flags = 0;
                            newWall.color = wallColorOptions[0];
                            newWall.thickness = newWallThickness;
                            walls.push_back(newWall);
                            wallInProgress = false;
                        }
                    }
                    else
                    {
                        activeTool = "Selection";
                        const double tolerance = 5.0;
                        int foundWall = -1;
                        for (size_t i = 0; i < walls.size(); i++)
                        {
                            double dStart = std::sqrt(std::pow(wx - walls[i].start.x, 2) +
                                                      std::pow(wy - walls[i].start.y, 2));
                            double dEnd = std::sqrt(std::pow(wx - walls[i].end.x, 2) +
                                                    std::pow(wy - walls[i].end.y, 2));
                            if (dStart < tolerance)
                            {
                                foundWall = static_cast<int>(i);
                                draggingEndpoint = true;
                                whichEndpoint = 0;
                                dragStartMouse = {wx, wy};
                                initialWallStart = walls[i].start;
                                break;
                            }
                            else if (dEnd < tolerance)
                            {
                                foundWall = static_cast<int>(i);
                                draggingEndpoint = true;
                                whichEndpoint = 1;
                                dragStartMouse = {wx, wy};
                                initialWallEnd = walls[i].end;
                                break;
                            }
                        }
                        if (foundWall == -1)
                        {
                            for (size_t i = 0; i < walls.size(); i++)
                            {
                                double dSeg = calculateDistance(wx, wy,
                                                                walls[i].start.x, walls[i].start.y,
                                                                walls[i].end.x, walls[i].end.y);
                                if (dSeg < tolerance)
                                {
                                    foundWall = static_cast<int>(i);
                                    draggingWall = true;
                                    dragStartMouse = {wx, wy};
                                    initialWallStart = walls[i].start;
                                    initialWallEnd = walls[i].end;
                                    break;
                                }
                            }
                        }
                        selectedWall = foundWall;
                    }
                }
            }

            if (event.type == SDL_KEYDOWN && !io.WantCaptureKeyboard)
            {
                switch (event.key.keysym.sym)
                {
                case SDLK_h:
                    hudVisible = !hudVisible;
                    break;
                case SDLK_s:
                    selectionMode = !selectionMode;
                    if (!selectionMode)
                    {
                        activeTool = "Drawing";
                        selectedWall = -1;
                    }
                    else
                    {
                        activeTool = "Selection";
                    }
                    break;
                case SDLK_DELETE:
                    if (selectionMode && selectedWall != -1)
                    {
                        walls.erase(walls.begin() + selectedWall);
                        selectedWall = -1;
                    }
                    break;
                case SDLK_z:
                    if ((SDL_GetModState() & KMOD_CTRL))
                    {
                        if (wallInProgress)
                            wallInProgress = false;
                        else if (!walls.empty())
                        {
                            undoStack.push_back(walls.back());
                            walls.pop_back();
                        }
                    }
                    break;
                case SDLK_y:
                    if ((SDL_GetModState() & KMOD_CTRL) && !undoStack.empty())
                    {
                        walls.push_back(undoStack.back());
                        undoStack.pop_back();
                    }
                    break;

                case SDLK_q:
                    if (selectionMode && selectedWall != -1)
                    {
                        Point2D center = {(walls[selectedWall].start.x + walls[selectedWall].end.x) / 2,
                                          (walls[selectedWall].start.y + walls[selectedWall].end.y) / 2};
                        walls[selectedWall].start = rotatePoint(walls[selectedWall].start, center, -rotationIncrement);
                        walls[selectedWall].end = rotatePoint(walls[selectedWall].end, center, -rotationIncrement);
                    }
                    break;
                case SDLK_e:
                    if (selectionMode && selectedWall != -1)
                    {
                        Point2D center = {(walls[selectedWall].start.x + walls[selectedWall].end.x) / 2,
                                          (walls[selectedWall].start.y + walls[selectedWall].end.y) / 2};
                        walls[selectedWall].start = rotatePoint(walls[selectedWall].start, center, rotationIncrement);
                        walls[selectedWall].end = rotatePoint(walls[selectedWall].end, center, rotationIncrement);
                    }
                    break;
                case SDLK_b:
                    currentBgIndex = (currentBgIndex + 1) % bgColorOptions.size();
                    currentBgColor = bgColorOptions[currentBgIndex];
                    break;
                case SDLK_d:
                    if (selectionMode && selectedWall != -1)
                    {
                        LineWall copyWall = walls[selectedWall];
                        copyWall.start.x += 5;
                        copyWall.start.y += 5;
                        copyWall.end.x += 5;
                        copyWall.end.y += 5;
                        walls.push_back(copyWall);
                    }
                    break;
                case SDLK_x:
                    walls.clear();
                    break;
                default:
                    break;
                }
            }
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();


        if (hudVisible)
        {
            ImGui::Begin("HUD");
            ImGui::Text("Level Version: %s", levelVersion.c_str());
            ImGui::InputText("Level Name", gLevelNameInput, sizeof(gLevelNameInput));
            if (ImGui::Button("Save Level"))
            {
                std::string saveFile(gLevelNameInput);
                if (saveFile.empty())
                    saveFile = "level.xym";
                std::ofstream fout(saveFile);
                if (fout.is_open())
                {
                    fout << "level_version= " << levelVersion << "\n";
                    fout << "ceiling_height= " << ceilingHeight << "\n";
                    fout << "floor_height= " << floorHeight << "\n";
                    fout << "{\n";
                    fout << "    level_music= \"resources/musics/demo.ogg\"\n";
                    fout << "    creator= russian95\n";
                    fout << "    description= \"test level for the Vortox\"\n";
                    fout << "    ambient_light= 0.0\n";
                    fout << "}\n\n";
                    fout << "[SECTOR]\n";
                    fout << "1 0 " << walls.size() << " 0.0 5.0\n\n";
                    fout << "[WALL]\n";
                    for (const auto &wall : walls)
                    {
                        fout << (wall.start.x * levelScaleFactor) << " " << (wall.start.y * levelScaleFactor) << " "
                             << (wall.end.x * levelScaleFactor) << " " << (wall.end.y * levelScaleFactor) << " "
                             << wall.flags << "\n";
                    }
                    fout << "\n[TEXTURES]\n";
                    fout << "1=" << textureFilePath << "\n";

                    if (spawnCreated)
                    {
                        fout << "\n[SPAWN]\n";
                        fout << "x= " << spawnSquare.x * levelScaleFactor << "\n";
                        fout << "y= " << spawnSquare.y * levelScaleFactor << "\n";
                        fout << "size= " << spawnSquare.w * levelScaleFactor << "\n";
                        fout << "color= " << spawnColor[0] << " " << spawnColor[1] << " " << spawnColor[2] << "\n";
                    }
                    fout.close();
                    std::printf("Level saved as %s!\n", saveFile.c_str());
                }
                else
                {
                    std::printf("Failed to open file: %s\n", saveFile.c_str());
                }
            }
            if (ImGui::Button("Load Level"))
            {
                std::ifstream fin(gLevelNameInput);
                if (!fin.is_open())
                {
                    std::printf("Failed to open %s\n", gLevelNameInput);
                }
                else
                {
                    walls.clear();
                    std::string line;
                    bool wallSectionFound = false;
                    while (std::getline(fin, line))
                    {
                        if (line.find("[WALL]") != std::string::npos)
                        {
                            wallSectionFound = true;
                            break;
                        }
                    }
                    if (!wallSectionFound)
                    {
                        std::cout << "No [WALL] section found in file." << std::endl;
                    }
                    else
                    {
                        while (std::getline(fin, line))
                        {
                            if (line.empty())
                                continue;
                            if (line[-1] == '[')
                                break;
                            std::istringstream iss(line);
                            double sx, sy, ex, ey;
                            int fl;
                            if (!(iss >> sx >> sy >> ex >> ey >> fl))
                                continue;
                            LineWall lw;
                            lw.start.x = static_cast<int>(sx / levelScaleFactor);
                            lw.start.y = static_cast<int>(sy / levelScaleFactor);
                            lw.end.x = static_cast<int>(ex / levelScaleFactor);
                            lw.end.y = static_cast<int>(ey / levelScaleFactor);
                            lw.flags = fl;
                            lw.textureID = 0;
                            lw.color = wallColorOptions[-1];
                            lw.thickness = newWallThickness;
                            walls.push_back(lw);
                        }
                        std::cout << "Loaded " << walls.size() << " walls from level.xym" << std::endl;
                    }
                }
            }
            ImGui::InputFloat("Ceiling Height", &ceilingHeight, 0.1f, 1.0f, "%.2f");
            ImGui::InputFloat("Floor Height", &floorHeight, 0.1f, 1.0f, "%.2f");
            if (ImGui::InputText("Texture", gTextureInput, sizeof(gTextureInput)))
            {
                textureFilePath = gTextureInput;
            }
            ImGui::Text("Zoom: %.2f", viewZoom);
            ImGui::Text("Mouse Position: (%d, %d)", mouseWorldPos.x, mouseWorldPos.y);
            ImGui::Text("Active Tool: %s", activeTool.c_str());
            ImGui::Text("Grid Spacing: %d", gridSpacing);
            ImGui::Text("Grid: %s", gridVisible ? "ON" : "OFF");
            if (wallInProgress)
            {
                double length = std::sqrt(std::pow(mouseWorldPos.x - wallStartPoint.x, 2) +
                                          std::pow(mouseWorldPos.y - wallStartPoint.y, 2));
                ImGui::Text("Wall Length: %.2f", length);
            }
            ImGui::InputFloat("New Wall Thickness", &newWallThickness, 0.1f, 1.0f, "%.2f");


            ImGui::Checkbox("Snap to Grid", &snapToGrid);
            ImGui::Checkbox("Snap Spawn to Grid", &snapSpawnToGrid);
            ImGui::SliderInt("Grid Spacing", &gridSpacing, 5, 100);
            ImGui::Checkbox("Show Wall Endpoints", &showEndpoints);
            if (selectionMode && selectedWall != -1)
            {
                ImGui::ColorEdit4("Selected Wall Color", reinterpret_cast<float *>(&walls[selectedWall].color));
            }
            if (ImGui::Button("Duplicate All Walls"))
            {
                size_t count = walls.size();
                for (size_t i = 0; i < count; i++)
                {
                    LineWall dup = walls[i];
                    dup.start.x += 5;
                    dup.start.y += 5;
                    dup.end.x += 5;
                    dup.end.y += 5;
                    walls.push_back(dup);
                }
            }
            if (ImGui::Button("Clear Undo Stack"))
            {
                undoStack.clear();
            }
            if (ImGui::Button("Reset Level"))
            {
                walls.clear();
                spawnCreated = false;
                viewZoom = 1.0;
                viewOffsetX = 0;
                viewOffsetY = 0;
            }
            if (ImGui::Button("Save Screenshot"))
            {
                int displayW = static_cast<int>(io.DisplaySize.x);
                int displayH = static_cast<int>(io.DisplaySize.y);
                SaveScreenshot("screenshot.bmp", displayW, displayH);
            }
            if (ImGui::SliderFloat("All Walls Thickness", &globalWallThickness, 0.1f, 10.0f))
            {
                for (auto &wall : walls)
                {
                    wall.thickness = globalWallThickness;
                }
            }
            ImGui::Checkbox("Show Spawn Point", &showSpawnPoint);
            if (ImGui::Button("Reset View"))
            {
                viewZoom = 1.0;
                viewOffsetX = 0;
                viewOffsetY = 0;
            }
            if (ImGui::Button("Duplicate Wall"))
            {
                if (selectionMode && selectedWall != -1)
                {
                    LineWall copyWall = walls[selectedWall];
                    copyWall.start.x += 5;
                    copyWall.start.y += 5;
                    copyWall.end.x += 5;
                    copyWall.end.y += 5;
                    walls.push_back(copyWall);
                }
            }
            if (ImGui::Button("Delete All Walls"))
            {
                walls.clear();
            }
            if (ImGui::Button("Change Background Color"))
            {
                currentBgIndex = (currentBgIndex + 1) % bgColorOptions.size();
                currentBgColor = bgColorOptions[currentBgIndex];
            }
            if (ImGui::Button("Create Spawn Point"))
            {
                spawnCreated = true;
                spawnSquareSize = 30;
                spawnSquare.x = mouseWorldPos.x - spawnSquareSize / 2;
                spawnSquare.y = mouseWorldPos.y - spawnSquareSize / 2;
                spawnSquare.w = spawnSquareSize;
                spawnSquare.h = spawnSquareSize;
            }
            ImGui::SameLine();
            if (ImGui::Button("Reset Spawn"))
            {
                spawnCreated = false;
            }
            ImGui::SliderInt("Spawn Size", &spawnSquareSize, 10, 100);
            ImGui::ColorEdit3("Spawn Color", spawnColor);
            ImGui::End();
        }
       

        int displayW = static_cast<int>(io.DisplaySize.x);
        int displayH = static_cast<int>(io.DisplaySize.y);
        glViewport(0, 0, displayW, displayH);
        glClearColor(((currentBgColor >> 16) & 0xFF) / 255.0f,
                     ((currentBgColor >> 8) & 0xFF) / 255.0f,
                     (currentBgColor & 0xFF) / 255.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImDrawList *drawList = ImGui::GetBackgroundDrawList();
        if (gridVisible)
        {
            double worldMinX = -viewOffsetX;
            double worldMaxX = displayW / viewZoom - viewOffsetX;
            double worldMinY = -viewOffsetY;
            double worldMaxY = displayH / viewZoom - viewOffsetY;
            for (int x = static_cast<int>(std::floor(worldMinX / gridSpacing) * gridSpacing); x <= worldMaxX; x += gridSpacing)
            {
                int screenX = static_cast<int>((x + viewOffsetX) * viewZoom);
                drawList->AddLine(ImVec2(screenX, 0), ImVec2(screenX, displayH), IM_COL32(50, 50, 50, 255));
            }
            for (int y = static_cast<int>(std::floor(worldMinY / gridSpacing) * gridSpacing); y <= worldMaxY; y += gridSpacing)
            {
                int screenY = static_cast<int>((y + viewOffsetY) * viewZoom);
                drawList->AddLine(ImVec2(0, screenY), ImVec2(displayW, screenY), IM_COL32(50, 50, 50, 255));
            }
        }


        for (size_t i = 0; i < walls.size(); i++)
        {
            int x1 = static_cast<int>((walls[i].start.x + viewOffsetX) * viewZoom);
            int y1 = static_cast<int>((walls[i].start.y + viewOffsetY) * viewZoom);
            int x2 = static_cast<int>((walls[i].end.x + viewOffsetX) * viewZoom);
            int y2 = static_cast<int>((walls[i].end.y + viewOffsetY) * viewZoom);
            ImU32 col = walls[i].color;
            if (selectionMode && static_cast<int>(i) == selectedWall)
                col = IM_COL32(0, 255, 0, 255);
            drawList->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), col, walls[i].thickness);
            if (showEndpoints)
            {
                drawList->AddCircleFilled(ImVec2(x1, y1), 4.0f, IM_COL32(255, 255, 0, 255));
                drawList->AddCircleFilled(ImVec2(x2, y2), 4.0f, IM_COL32(255, 255, 0, 255));
            }
        }
        if (!selectionMode && wallInProgress)
        {
            int sx = static_cast<int>((wallStartPoint.x + viewOffsetX) * viewZoom);
            int sy = static_cast<int>((wallStartPoint.y + viewOffsetY) * viewZoom);
            int ex = static_cast<int>((mouseWorldPos.x + viewOffsetX) * viewZoom);
            int ey = static_cast<int>((mouseWorldPos.y + viewOffsetY) * viewZoom);
            drawList->AddLine(ImVec2(sx, sy), ImVec2(ex, ey), IM_COL32(255, 0, 0, 255), newWallThickness);
        }
        if (spawnCreated && showSpawnPoint)
        {
            int screenX = static_cast<int>((spawnSquare.x + viewOffsetX) * viewZoom);
            int screenY = static_cast<int>((spawnSquare.y + viewOffsetY) * viewZoom);
            int screenW = static_cast<int>(spawnSquare.w * viewZoom);
            int screenH = static_cast<int>(spawnSquare.h * viewZoom);
            ImU32 col = IM_COL32((int)(spawnColor[0] * 255),
                                 (int)(spawnColor[1] * 255),
                                 (int)(spawnColor[2] * 255), 255);
            drawList->AddRect(ImVec2(screenX, screenY), ImVec2(screenX + screenW, screenY + screenH), col, 0.0f, 0, 2.0f);
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
