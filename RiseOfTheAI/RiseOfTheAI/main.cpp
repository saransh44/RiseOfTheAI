#define GL_SILENCE_DEPRECATION //For silencing pesky notifications on Mac
#define TILE_COUNT 69
#define AI_COUNT 3

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <vector>
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "Entity.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace std;

SDL_Window* displayWindow;
bool gameIsRunning = true;

ShaderProgram textured;
glm::mat4 viewMatrix, modelMatrix, projectionMatrix;

GLuint textID;
GLuint tileID;

ShaderProgram unTextured;

struct GameState {
    Entity player;
    Entity tiles[TILE_COUNT];
    Entity AI[AI_COUNT];
};

GameState state;

bool ended = false;


GLuint LoadTexture(const char* filePath) {
    int w, h, n;
    unsigned char* image = stbi_load(filePath, &w, &h, &n, STBI_rgb_alpha);

    if (image == NULL) {
        std::cout << "Unable to load image. Make sure the path is correct\n";
        assert(false);
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    stbi_image_free(image);
    return textureID;
}

void DrawText(ShaderProgram* program, GLuint fontTextureID, std::string text, float size, float spacing, glm::vec3 position)
{
    float width = 1.0f / 16.0f;
    float height = 1.0f / 16.0f;

    std::vector<float> vertices;
    std::vector<float> texCoords;

    for (int i = 0; i < text.size(); i++) {
        int index = (int)text[i];

        float u = (float)(index % 16) / 16.0f;
        float v = (float)(index / 16) / 16.0f;


        texCoords.insert(texCoords.end(), { u, v + height, u + width, v + height, u + width, v, u, v + height, u + width, v, u, v });

        float offset = (size + spacing) * i;
        vertices.insert(vertices.end(), { offset + (-0.5f * size), (-0.5f * size),
                                        offset + (0.5f * size), (-0.5f * size),
                                        offset + (0.5f * size), (0.5f * size),
                                        offset + (-0.5f * size), (-0.5f * size),
                                        offset + (0.5f * size), (0.5f * size),
                                        offset + (-0.5f * size), (0.5f * size) });
    }

    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, position);
    program->SetModelMatrix(modelMatrix);
    glBindTexture(GL_TEXTURE_2D, fontTextureID);


    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices.data());
    glEnableVertexAttribArray(program->positionAttribute);

    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords.data());
    glEnableVertexAttribArray(program->texCoordAttribute);

    glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 2.0f);

    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}

int borderTileCount = -1;

//borders
float bottomBorder;
float leftBorder;
float rightBorder;
float innerRightBorder;
float innerBottomBorder;


void Initialize() {
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("Rise Of the AI", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(0, 0, 640, 480);


    viewMatrix = glm::mat4(1.0f);
    //modelMatrix = glm::mat4(1.0f); //modelMatrix is used elegantly in Entity.cpp
    projectionMatrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    unTextured.Load("shaders/vertex.glsl", "shaders/fragment.glsl");

    unTextured.SetProjectionMatrix(projectionMatrix);
    unTextured.SetViewMatrix(viewMatrix);
    unTextured.SetColor(1.0f, 1.0f, 1.0f, 1.0f);
    glUseProgram(unTextured.programID);

    //textured.SetModelMatrix(modelMatrix);

    textured.Load("shaders/vertex_textured.glsl", "shaders/fragment_textured.glsl");
    textID = LoadTexture("font1.png");
    tileID = LoadTexture("tile.png");
    
    state.player.entityType = PLAYER;
    state.player.isStatic = false;
    state.player.width = .5f;
    state.player.height = .5f;
    state.player.position = glm::vec3(-1.0, -1.0f, 0);
    state.player.acceleration = glm::vec3(0, -9.81f, 0);
    state.player.textureID = LoadTexture("me.png");

    GLuint AIID1 = LoadTexture("ghost.png");
    state.AI[0].entityType = ENEMY;
    state.AI[0].width = 0.5;
    state.AI[0].height = 0.5;
    state.AI[0].textureID = AIID1;
    state.AI[0].isStatic = false; //it moves!!
    state.AI[0].acceleration = glm::vec3(0, -9.81f, 0);
    state.AI[0].position = glm::vec3(1.0f, 1.0f, 0.0f);
    state.AI[0].move = FLOATING;
    state.AI[0].aiType = GHOST;

    GLuint AIID2 = LoadTexture("goomba.png");
    state.AI[1].entityType = ENEMY;
    state.AI[1].width = 0.5;
    state.AI[1].height = 0.5;
    state.AI[1].textureID = AIID2;
    state.AI[1].isStatic = false; //it moves!!
    state.AI[1].acceleration = glm::vec3(0, -9.81f, 0);
    state.AI[1].position = glm::vec3(2.0f, -3.0f, 0.0f);
    state.AI[1].move = IDLE;
    state.AI[1].aiType = GOOMBA;

    GLuint AIID3 = LoadTexture("frog.png");
    state.AI[2].entityType = ENEMY;
    state.AI[2].width = 0.5;
    state.AI[2].height = 0.5;
    state.AI[2].textureID = AIID3;
    state.AI[2].isStatic = false; //it moves!!
    state.AI[2].acceleration = glm::vec3(0, -9.81f, 0);
    state.AI[2].position = glm::vec3(4.35f, -3.0f, 0);
    state.AI[2].move = JUMPING;
    state.AI[2].aiType = FROG;

    for (bottomBorder = -5; bottomBorder < 5.5; bottomBorder+=0.5) {
        borderTileCount++;

        state.tiles[borderTileCount].textureID = tileID;
        state.tiles[borderTileCount].position = glm::vec3(bottomBorder, -3.5f, 0);
        state.tiles[borderTileCount].entityType = TILE;
    }
    cout << borderTileCount << endl; //debugging

    for (leftBorder = -3; leftBorder < 4.6; leftBorder+= 2) {
        borderTileCount++;

        state.tiles[borderTileCount].textureID = tileID;
        state.tiles[borderTileCount].position = glm::vec3(-4, leftBorder, 0);
        state.tiles[borderTileCount].entityType = TILE;
    }
    cout << borderTileCount << endl; //debugging

    for (leftBorder = -2; leftBorder <  3.9; leftBorder += 2) {
        borderTileCount++;

        state.tiles[borderTileCount].textureID = tileID;
        state.tiles[borderTileCount].position = glm::vec3(-4.5f, leftBorder, 0);
        state.tiles[borderTileCount].entityType = TILE;
    }
    cout << borderTileCount << endl; //debugging
    
    for (rightBorder = -1.5f; rightBorder < 3.2; rightBorder += .5) {
        borderTileCount++;

        state.tiles[borderTileCount].textureID = tileID;
        state.tiles[borderTileCount].position = glm::vec3(4.75f, rightBorder, 0);
        state.tiles[borderTileCount].entityType = TILE;
    }

    for (innerRightBorder = -1.0f; innerRightBorder < 2.0; innerRightBorder += .5) {
        borderTileCount++;

        state.tiles[borderTileCount].textureID = tileID;
        state.tiles[borderTileCount].position = glm::vec3(3.75, innerRightBorder, 0);
        state.tiles[borderTileCount].entityType = TILE;
    }

    for (innerBottomBorder = -2.25; innerBottomBorder < 3.30; innerBottomBorder += .5) {
        borderTileCount++;

        state.tiles[borderTileCount].textureID = tileID;
        state.tiles[borderTileCount].position = glm::vec3(innerBottomBorder, -1.5f, 0);
        state.tiles[borderTileCount].entityType = TILE;
    }
    for (innerRightBorder = -3.0f; innerRightBorder < -0.5; innerRightBorder += .5) {
        borderTileCount++;

        state.tiles[borderTileCount].textureID = tileID;
        state.tiles[borderTileCount].position = glm::vec3(3.25, innerRightBorder, 0);
        state.tiles[borderTileCount].entityType = TILE;
    }
    cout << borderTileCount << endl; //debugging

    state.tiles[61].textureID = tileID;
    state.tiles[61].position = glm::vec3(1.0f, 0.0f, 0);
    state.tiles[61].entityType = TILE;

    state.tiles[62].textureID = tileID;
    state.tiles[62].position = glm::vec3(1.5f, 0.0f, 0);
    state.tiles[62].entityType = TILE;

    state.tiles[63].textureID = tileID;
    state.tiles[63].position = glm::vec3(2.75f, 1.0f, 0);
    state.tiles[63].entityType = TILE;

    //begin mini border slightly left of center
    state.tiles[64].textureID = tileID;
    state.tiles[64].position = glm::vec3(-0.5f, -0.5f, 0);
    state.tiles[64].entityType = TILE;

    state.tiles[65].textureID = tileID;
    state.tiles[65].position = glm::vec3(-0.5f, -1.0f, 0);
    state.tiles[65].entityType = TILE;
    //end mini border slightly left of center

    //begin mini border slightly left of center
    state.tiles[66].textureID = tileID;
    state.tiles[66].position = glm::vec3(-2.0f, -0.5f, 0);
    state.tiles[66].entityType = TILE;

    state.tiles[67].textureID = tileID;
    state.tiles[67].position = glm::vec3(-2.0f, -1.0f, 0);
    state.tiles[67].entityType = TILE;

    //end mini border slightly left of center

    state.tiles[68].textureID = tileID;
    state.tiles[68].position = glm::vec3(3.85f, -3.0f, 0);
    state.tiles[68].entityType = TILE;

    /*state.tiles[65].textureID = tileID;
    state.tiles[65].position = glm::vec3(2.4, 1.8, 0);
    state.tiles[65].entityType = TILE;*/

    /*int moreTiles = 26;
    for (int firstBarrierX = 4; firstBarrierX >= -1; firstBarrierX--) {
        moreTiles++;

        state.tiles[moreTiles].textureID = tileID;
        state.tiles[moreTiles].position = glm::vec3(firstBarrierX, -0.2, 0);
        state.tiles[moreTiles].entityType = TILE;
    }

    cout << moreTiles << endl;*/ //debugging

  
    textured.SetProjectionMatrix(projectionMatrix);
    textured.SetViewMatrix(viewMatrix);
    textured.SetColor(1.0f, 1.0f, 1.0f, 1.0f);

    glUseProgram(textured.programID);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
}


#define FIXED_TIMESTEP 0.0166666f
float lastTicks = 0;
float accumulator = 0.0f;

void Update() {9

    float ticks = (float)SDL_GetTicks() / 1000.0f;
    float deltaTime = ticks - lastTicks;
    lastTicks = ticks;

    deltaTime += accumulator;
    if (deltaTime < FIXED_TIMESTEP) {
        accumulator = deltaTime;
        return;
    }

    while (deltaTime >= FIXED_TIMESTEP) {
        state.player.Update(FIXED_TIMESTEP, state.player, state.tiles, TILE_COUNT, state.AI, AI_COUNT);
        for (int i = 0; i < AI_COUNT; i++) {
            state.AI[i].Update(FIXED_TIMESTEP, state.player, state.tiles, TILE_COUNT, state.AI, AI_COUNT);
            
        }

        deltaTime -= FIXED_TIMESTEP;
    }

    accumulator = deltaTime;
}


glm::vec3 tileSizing = glm::vec3(0.5f, 0.5f, 0.0f);
glm::vec3 pokeballSizing = glm::vec3(1.0f, 1.0f, 0.0f);
glm::vec3 pikSizing = glm::vec3(0.5f, 0.5f, 0.0f);

void Render() {
    glClear(GL_COLOR_BUFFER_BIT);

    state.player.Render(&textured, pikSizing);

    for (int i = 0; i < TILE_COUNT; i++) {
        state.tiles[i].Render(&textured, tileSizing);
    }

    for (int i = 0; i < AI_COUNT; i++)
    {
        state.AI[i].Render(&textured, tileSizing);
    }
    
    SDL_GL_SwapWindow(displayWindow);
}

void ProcessInput() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
        case SDL_WINDOWEVENT_CLOSE:
            gameIsRunning = false;
            break;

        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
            /*case SDLK_RIGHT:
                state.player.velocity.x = 3.0f;

                break;
            case SDLK_LEFT:
                state.player.velocity.x = -3.0f;

                break;*/
            
            case SDLK_SPACE:
                state.player.Jump();
                break;

            }
            break;
        }
    }

    state.player.velocity.x = 0;


    // Check for pressed/held keys below; emphasis on held and why my left and right arrow key code is here and commented up above
    const Uint8* keys = SDL_GetKeyboardState(NULL);

    if (keys[SDL_SCANCODE_LEFT])
    {
        state.player.velocity.x = -3.0f;
    }
    else if (keys[SDL_SCANCODE_RIGHT])
    {
        state.player.velocity.x = 3.0f;
    }
}

void Shutdown() {
    SDL_Quit();
}

int main(int argc, char* argv[]) {
    Initialize();

    while (gameIsRunning) {
        ProcessInput();
        Update();
        Render();
    }

    Shutdown();
    return 0;
}