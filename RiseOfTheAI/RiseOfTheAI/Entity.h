#pragma once
#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"

enum EntityType { PLAYER, GOAL, TILE, ENEMY, MESSAGE, DEFAULT};
enum moveState  { IDLE, WALKING, ATTACKING, JUMPING, FLOATING};
enum AIType {GOOMBA, GHOST, FROG};

class Entity {
public:
    EntityType entityType;
    moveState move;
    AIType aiType;

    GLuint textureID;

    bool isStatic;
    bool isActive;

    //these are the only real additions by me not provided in the slides nor in my past edits to Entity from the pong project
    bool winnerWinnerChickenDinner = false;
    bool collisionDetected = false;

    float width;
    float height;
    float speed;

    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 acceleration;
    glm::vec3 custSizing;

    Entity();
    Entity(EntityType type);

    bool CheckCollision(Entity other);

    void CheckCollisionsY(Entity* objects, int objectCount);
    void CheckCollisionsX(Entity* objects, int objectCount);

    //void UpdatePos(const float incrementX, const float incrementY);
    
    void Update(float deltaTime, Entity player, Entity* objects, int objectCount, Entity* enemies, int enemyCount);
    void Render(ShaderProgram* program, glm::vec3 sizing);

    void Jump();

    void aiIdle(Entity player);
    void aiWalking(Entity player);
    void aiAttacking(Entity player);

    void AI(Entity player);

    bool collidedTop = false;
    bool collidedBottom = false;
    bool collidedLeft = false;
    bool collidedRight = false;

    glm::vec3 sensorRight;
    glm::vec3 sensorLeft;

    bool playerNearbyRight;
    bool playerNearbyLeft;
};



