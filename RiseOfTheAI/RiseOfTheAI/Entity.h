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
enum moveState  { IDLE, SlideAttack, JUMPING, FLOATING};
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

    //This is a slightly implemetation set up in Entity.cpp for all these movements
    //By design I don't want Idle for example to mean different things to 
    //different objects so generally those implementation cases are the same.
    //Just for future proofing I like making new move names for even things like a 
    //sliding motion upwards and downwards and one for side to side
    //I remember Sterling spoke slightly on future proofing and thinking of behaviors of objects 
    //In case I need to something with that sliding motion that concept in code especially in the 
    //implementation .Cpp file, well I can now have assurances just incase there needs to be furter 
    //distinction between AI's but also makes me more aware of edits even if movements are similar between AI.
    void aiIdle(Entity player);
    void aiSlideAttack(Entity player);
    void aiFloating(Entity player);
    void aiLeaping(Entity player);
 
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



