#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include <stdio.h>
#include <vector>
#include <fstream>
#include <cmath>
#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>

// im gui
#include "../lib/imgui/imgui.h"
#include "../lib/imgui/imgui_impl_sdl2.h"
#include "../lib/imgui/imgui_impl_opengl3.h"

// header
#include "editor.hpp"

struct Point
{
    int x, y;
};

struct Wall
{
    Point start;
    Point end;
    int texture;
    int flag;
    ImU32 color;
};

double pointToSegmentDistance(double px, double py, double ax, double ay, double bx, double by)
{
    double vx = bx - ax, vy = by - ay;
    double wx = px - ax, wy = py - ay;
    double c1 = vx * wx + vy * wy;
    if (c1 <= 0)
        return sqrt((px - ax) * (px - ax) + (py - ay) * (py - ay));
    double c2 = vx * vx + vy * vy;
    if (c2 <= c1)
        return sqrt((px - bx) * (px - bx) + (py - by) * (py - by));
    double bVal = c1 / c2;
    double projx = ax + bVal * vx;
    double projy = ay + bVal * vy;
    return sqrt((px - projx) * (px - projx) + (py - projy) * (py - projy));
}

int main(int argc, char *argv[])
{
    // initialization of SDL with OpenGL
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

    SDL_Window *window = SDL_CreateWindow(WINDOW_TITLE,
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          WINDOW_WIDTH, WINDOW_HEIGHT,
                                          SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // VSync

    // initialization of Dear ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init("#version 150");

    // variables for the editor
    double zoom = 1.0;
    double offsetX = 0.0, offsetY = 0.0;
    int gridSize = 30;
    bool showGrid = true;
    bool panning = false;
    int panStartX = 0, panStartY = 0;
    double offsetStartX = 0.0, offsetStartY = 0.0;
    std::vector<Wall> walls;
    bool drawingWall = false;
    Point startPoint = {0, 0};
    std::vector<Wall> redoStack;
    Point worldMousePos = {0, 0};
    const double levelScale = 0.1;
    bool selectionMode = false;
    int selectedWallIndex = -1;
    bool draggingWall = false;
    bool draggingEndpoint = false;
    int draggedEndpoint = -1;
    Point dragStartMouse = {0, 0};
    Point dragStartWallStart = {0, 0};
    Point dragStartWallEnd = {0, 0};
    std::string levelVersion = "4.22";
    double ceilingHeight = 10.0;
    double floorHeight = 0.0;
    std::string textureInput = "resources/textures/brick.png";
    bool editingTexture = false;
    bool showHUD = true; // toggle hud

    std::vector<ImU32> wallColorPalette = {
        IM_COL32(255, 255, 255, 255),
        IM_COL32(255, 0, 0, 255),
        IM_COL32(0, 255, 0, 255),
        IM_COL32(0, 0, 255, 255)};
    std::vector<ImU32> bgColors = {
        IM_COL32(30, 30, 30, 255),
        IM_COL32(20, 20, 50, 255),
        IM_COL32(50, 50, 50, 255)};
    int bgColorIndex = 0;
    ImU32 bgColor = bgColors[bgColorIndex];
    std::string toolMode = "Drawing";

    bool running = true;
    SDL_Event event;
    while (running)
    {
        // process the events
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                running = false;
            if (event.type == SDL_MOUSEWHEEL)
            {
                int mouseX, mouseY;
                SDL_GetMouseState(&mouseX, &mouseY);
                double oldZoom = zoom;
                if (event.wheel.y > 0)
                    zoom *= 1.1;
                else if (event.wheel.y < 0)
                    zoom /= 1.1;
                // updates offset to keep the cursor position in the same location in the world
                offsetX = offsetX + (mouseX / zoom - mouseX / oldZoom);
                offsetY = offsetY + (mouseY / zoom - mouseY / oldZoom);
            }
            if (event.type == SDL_MOUSEBUTTONDOWN)
            {
                // if the mouse is over the Heads-Up Display, ignore left click events.
                if (event.button.button == SDL_BUTTON_LEFT && io.WantCaptureMouse)
                    continue;

                if (event.button.button == SDL_BUTTON_RIGHT)
                {
                    panning = true;
                    panStartX = event.button.x;
                    panStartY = event.button.y;
                    offsetStartX = offsetX;
                    offsetStartY = offsetY;
                    toolMode = "Panning";
                }
                else if (event.button.button == SDL_BUTTON_LEFT)
                {
                    int worldX = static_cast<int>(event.button.x / zoom - offsetX);
                    int worldY = static_cast<int>(event.button.y / zoom - offsetY);
                    // apply snapping if CTRL press
                    if (SDL_GetModState() & KMOD_CTRL)
                    {
                        worldX = static_cast<int>(round(worldX / static_cast<double>(gridSize)) * gridSize);
                        worldY = static_cast<int>(round(worldY / static_cast<double>(gridSize)) * gridSize);
                    }

                    if (!selectionMode)
                    {
                        toolMode = "Drawing";
                        if (!drawingWall)
                        {
                            startPoint = {worldX, worldY};
                            drawingWall = true;
                            redoStack.clear();
                        }
                        else
                        {
                            Wall newWall;
                            newWall.start = startPoint;
                            newWall.end = {worldX, worldY};
                            newWall.texture = 1;
                            newWall.flag = 0;
                            newWall.color = wallColorPalette[0];
                            walls.push_back(newWall);
                            drawingWall = false;
                        }
                    }
                    else // selection mode
                    {
                        toolMode = "Selection";
                        const double threshold = 5.0;
                        int foundIndex = -1;
                        for (size_t i = 0; i < walls.size(); i++)
                        {
                            double distStart = sqrt(pow(worldX - walls[i].start.x, 2) + pow(worldY - walls[i].start.y, 2));
                            double distEnd = sqrt(pow(worldX - walls[i].end.x, 2) + pow(worldY - walls[i].end.y, 2));
                            if (distStart < threshold)
                            {
                                foundIndex = static_cast<int>(i);
                                draggingEndpoint = true;
                                draggedEndpoint = 0;
                                dragStartMouse = {worldX, worldY};
                                dragStartWallStart = walls[i].start;
                                break;
                            }
                            else if (distEnd < threshold)
                            {
                                foundIndex = static_cast<int>(i);
                                draggingEndpoint = true;
                                draggedEndpoint = 1;
                                dragStartMouse = {worldX, worldY};
                                dragStartWallEnd = walls[i].end;
                                break;
                            }
                        }
                        if (foundIndex == -1)
                        {
                            for (size_t i = 0; i < walls.size(); i++)
                            {
                                double dist = pointToSegmentDistance(worldX, worldY,
                                                                     walls[i].start.x, walls[i].start.y,
                                                                     walls[i].end.x, walls[i].end.y);
                                if (dist < threshold)
                                {
                                    foundIndex = static_cast<int>(i);
                                    draggingWall = true;
                                    dragStartMouse = {worldX, worldY};
                                    dragStartWallStart = walls[i].start;
                                    dragStartWallEnd = walls[i].end;
                                    break;
                                }
                            }
                        }
                        selectedWallIndex = foundIndex;
                    }
                }
            }
            if (event.type == SDL_MOUSEBUTTONUP)
            {
                if (event.button.button == SDL_BUTTON_RIGHT)
                    panning = false;
                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    if (selectionMode)
                    {
                        draggingWall = false;
                        draggingEndpoint = false;
                        draggedEndpoint = -1;
                    }
                }
            }
            if (event.type == SDL_MOUSEMOTION)
            {
                // calculate mouse position in world with snapping if CTRL is pressed
                int tmpX = static_cast<int>(event.motion.x / zoom - offsetX);
                int tmpY = static_cast<int>(event.motion.y / zoom - offsetY);
                if (SDL_GetModState() & KMOD_CTRL)
                {
                    tmpX = static_cast<int>(round(tmpX / static_cast<double>(gridSize)) * gridSize);
                    tmpY = static_cast<int>(round(tmpY / static_cast<double>(gridSize)) * gridSize);
                }
                worldMousePos = {tmpX, tmpY};

                if (panning)
                {
                    offsetX = offsetStartX + (event.motion.x - panStartX) / zoom;
                    offsetY = offsetStartY + (event.motion.y - panStartY) / zoom;
                }
                if (selectionMode)
                {
                    if (draggingWall && selectedWallIndex != -1)
                    {
                        int dx = worldMousePos.x - dragStartMouse.x;
                        int dy = worldMousePos.y - dragStartMouse.y;
                        walls[selectedWallIndex].start.x = dragStartWallStart.x + dx;
                        walls[selectedWallIndex].start.y = dragStartWallStart.y + dy;
                        walls[selectedWallIndex].end.x = dragStartWallEnd.x + dx;
                        walls[selectedWallIndex].end.y = dragStartWallEnd.y + dy;
                    }
                    if (draggingEndpoint && selectedWallIndex != -1)
                    {
                        int dx = worldMousePos.x - dragStartMouse.x;
                        int dy = worldMousePos.y - dragStartMouse.y;
                        if (draggedEndpoint == 0)
                        {
                            walls[selectedWallIndex].start.x = dragStartWallStart.x + dx;
                            walls[selectedWallIndex].start.y = dragStartWallStart.y + dy;
                        }
                        else if (draggedEndpoint == 1)
                        {
                            walls[selectedWallIndex].end.x = dragStartWallEnd.x + dx;
                            walls[selectedWallIndex].end.y = dragStartWallEnd.y + dy;
                        }
                    }
                }
            }
            if (event.type == SDL_KEYDOWN)
            {
                // toggle to show/not show the HUD with the keybind H
                if (event.key.keysym.sym == SDLK_h)
                {
                    showHUD = !showHUD;
                }
                else if (event.key.keysym.sym == SDLK_s)
                {
                    selectionMode = !selectionMode;
                    if (selectionMode)
                        toolMode = "Selection";
                    else
                    {
                        toolMode = "Drawing";
                        selectedWallIndex = -1;
                    }
                }
                else if (event.key.keysym.sym == SDLK_DELETE)
                {
                    if (selectionMode && selectedWallIndex != -1)
                    {
                        walls.erase(walls.begin() + selectedWallIndex);
                        selectedWallIndex = -1;
                    }
                }
                else if (event.key.keysym.sym == SDLK_z && (SDL_GetModState() & KMOD_CTRL))
                {
                    if (drawingWall)
                        drawingWall = false;
                    else if (!walls.empty())
                    {
                        redoStack.push_back(walls.back());
                        walls.pop_back();
                    }
                }
                else if (event.key.keysym.sym == SDLK_y && (SDL_GetModState() & KMOD_CTRL))
                {
                    if (!redoStack.empty())
                    {
                        walls.push_back(redoStack.back());
                        redoStack.pop_back();
                    }
                }
                else if (event.key.keysym.sym == SDLK_F5)
                {
                    std::string fileName;
                    std::cout << "enter file name (or leave empty for level.txt): ";
                    std::getline(std::cin, fileName);
                    if (fileName.empty())
                        fileName = "level.xym";
                    std::ofstream ofs(fileName);
                    if (ofs.is_open())
                    {
                        ofs << "level_version= " << levelVersion << "\n";
                        ofs << "ceiling_height= " << ceilingHeight << "\n";
                        ofs << "floor_height= " << floorHeight << "\n";
                        ofs << "{\n";
                        ofs << "    level_song= \"resources/songs/demo.ogg\"\n";
                        ofs << "    creator= russian95\n";
                        ofs << "    description= \"test level for the Xylon\"\n";
                        ofs << "    ambient_light= 0.0\n";
                        ofs << "}\n\n";
                        ofs << "[SECTOR]\n";
                        ofs << "1 0 " << walls.size() << " 0.0 5.0\n\n";
                        ofs << "[WALL]\n";
                        for (const auto &wall : walls)
                        {
                            ofs << (wall.start.x * levelScale) << " " << (wall.start.y * levelScale) << " "
                                << (wall.end.x * levelScale) << " " << (wall.end.y * levelScale) << " "
                                << wall.flag << "\n";
                        }
                        ofs << "\n[TEXTURES]\n";
                        ofs << "1=" << textureInput << "\n";
                        ofs.close();
                        printf("Level saved as %s!\n", fileName.c_str());
                    }
                    else
                    {
                        printf("Error saving level file.\n");
                    }
                }
                else if (event.key.keysym.sym == SDLK_F9)
                {
                    std::string fileName;
                    std::cout << "Enter file name to load: ";
                    std::getline(std::cin, fileName);
                    if (fileName.empty())
                        std::cout << "File name empty. Aborting load.\n";
                    else
                    {
                        std::ifstream ifs(fileName);
                        if (!ifs.is_open())
                        {
                            printf("Error opening file %s\n", fileName.c_str());
                        }
                        else
                        {
                            walls.clear();
                            std::string line;
                            bool wallSectionFound = false;
                            while (std::getline(ifs, line))
                            {
                                if (line.find("[WALL]") != std::string::npos)
                                {
                                    wallSectionFound = true;
                                    break;
                                }
                            }
                            if (!wallSectionFound)
                                std::cout << "No [WALL] section found in file.\n";
                            else
                            {
                                while (std::getline(ifs, line))
                                {
                                    if (line.empty())
                                        continue;
                                    if (line[0] == '[')
                                        break;
                                    std::istringstream iss(line);
                                    double sx, sy, ex, ey;
                                    int flag;
                                    if (!(iss >> sx >> sy >> ex >> ey >> flag))
                                        continue;
                                    Wall wall;
                                    wall.start.x = static_cast<int>(sx / levelScale);
                                    wall.start.y = static_cast<int>(sy / levelScale);
                                    wall.end.x = static_cast<int>(ex / levelScale);
                                    wall.end.y = static_cast<int>(ey / levelScale);
                                    wall.flag = flag;
                                    wall.texture = 1;
                                    wall.color = wallColorPalette[0];
                                    walls.push_back(wall);
                                }
                                std::cout << "Loaded " << walls.size() << " walls from " << fileName << "\n";
                            }
                        }
                    }
                }
                else if (event.key.keysym.sym == SDLK_g)
                {
                    showGrid = !showGrid;
                }
                else if (event.key.keysym.sym == SDLK_EQUALS)
                {
                    gridSize += 5;
                }
                else if (event.key.keysym.sym == SDLK_MINUS)
                {
                    gridSize = std::max(5, gridSize - 5);
                }
                else if (event.key.keysym.sym == SDLK_b)
                {
                    bgColorIndex = (bgColorIndex + 1) % bgColors.size();
                    bgColor = bgColors[bgColorIndex];
                }
                else if (event.key.keysym.sym == SDLK_v)
                {
                    if (selectionMode && selectedWallIndex != -1)
                    {
                        ImU32 currentColor = walls[selectedWallIndex].color;
                        int index = 0;
                        for (size_t i = 0; i < wallColorPalette.size(); i++)
                        {
                            if (wallColorPalette[i] == currentColor)
                            {
                                index = i;
                                break;
                            }
                        }
                        index = (index + 1) % wallColorPalette.size();
                        walls[selectedWallIndex].color = wallColorPalette[index];
                    }
                }
                else if (event.key.keysym.sym == SDLK_d)
                {
                    if (selectionMode && selectedWallIndex != -1)
                    {
                        Wall dup = walls[selectedWallIndex];
                        dup.start.x += 5;
                        dup.start.y += 5;
                        dup.end.x += 5;
                        dup.end.y += 5;
                        walls.push_back(dup);
                    }
                }
                else if (event.key.keysym.sym == SDLK_x)
                {
                    walls.clear();
                }
            }
        } // end of the event loop

        // frame imgui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        // window of the Editor HUD (only if showHUD == true)
        if (showHUD)
        {
            ImGui::Begin("HUD");
            ImGui::Text("level_version: %s", levelVersion.c_str());
            ImGui::Text("ceiling_height: %.2f", ceilingHeight);
            ImGui::Text("floor_height: %.2f", floorHeight);
            ImGui::Text("texture: %s", textureInput.c_str());
            ImGui::Text("zoom: %.2f", zoom);
            ImGui::Text("mouse: (%d, %d)", worldMousePos.x, worldMousePos.y);
            ImGui::Text("tool: %s", toolMode.c_str());
            ImGui::Text("grid size: %d", gridSize);
            ImGui::Text("grid: %s", showGrid ? "ON" : "OFF");
            if (drawingWall)
            {
                double length = sqrt(pow(worldMousePos.x - startPoint.x, 2) + pow(worldMousePos.y - startPoint.y, 2));
                ImGui::Text("wall length: %.2f", length);
            }
            ImGui::End();
        }

        // renders the scene: clears the screen and draws the grid and walls using ImGui DrawList
        int windowWidth = static_cast<int>(io.DisplaySize.x);
        int windowHeight = static_cast<int>(io.DisplaySize.y);
        glViewport(0, 0, windowWidth, windowHeight);
        glClearColor(((bgColor >> 16) & 0xFF) / 255.0f,
                     ((bgColor >> 8) & 0xFF) / 255.0f,
                     (bgColor & 0xFF) / 255.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImDrawList *draw_list = ImGui::GetBackgroundDrawList();
        if (showGrid)
        {
            double worldMinX = -offsetX;
            double worldMaxX = windowWidth / zoom - offsetX;
            double worldMinY = -offsetY;
            double worldMaxY = windowHeight / zoom - offsetY;
            for (int x = static_cast<int>(floor(worldMinX / gridSize) * gridSize); x <= worldMaxX; x += gridSize)
            {
                int screenX = static_cast<int>((x + offsetX) * zoom);
                draw_list->AddLine(ImVec2(screenX, 0), ImVec2(screenX, windowHeight), IM_COL32(50, 50, 50, 255));
            }
            for (int y = static_cast<int>(floor(worldMinY / gridSize) * gridSize); y <= worldMaxY; y += gridSize)
            {
                int screenY = static_cast<int>((y + offsetY) * zoom);
                draw_list->AddLine(ImVec2(0, screenY), ImVec2(windowWidth, screenY), IM_COL32(50, 50, 50, 255));
            }
        }
        // draw already created walls
        for (size_t i = 0; i < walls.size(); i++)
        {
            int startX = static_cast<int>((walls[i].start.x + offsetX) * zoom);
            int startY = static_cast<int>((walls[i].start.y + offsetY) * zoom);
            int endX = static_cast<int>((walls[i].end.x + offsetX) * zoom);
            int endY = static_cast<int>((walls[i].end.y + offsetY) * zoom);
            ImU32 color = walls[i].color;
            if (selectionMode && (int)i == selectedWallIndex)
                color = IM_COL32(0, 255, 0, 255);
            draw_list->AddLine(ImVec2(startX, startY), ImVec2(endX, endY), color, 2.0f);
        }
        // if drawing a wall, draw the preview line
        if (!selectionMode && drawingWall)
        {
            int previewStartX = static_cast<int>((startPoint.x + offsetX) * zoom);
            int previewStartY = static_cast<int>((startPoint.y + offsetY) * zoom);
            int previewEndX = static_cast<int>((worldMousePos.x + offsetX) * zoom);
            int previewEndY = static_cast<int>((worldMousePos.y + offsetY) * zoom);
            draw_list->AddLine(ImVec2(previewStartX, previewStartY), ImVec2(previewEndX, previewEndY), IM_COL32(255, 0, 0, 255), 2.0f);
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    // clean
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
