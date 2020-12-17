#include "Entity.h"

Entity::Entity()
{
    entityType = DEFAULT;
    
    position = glm::vec3(0);
    custSizing = glm::vec3(0);

    speed = 0;
    width = 0.5;
    height = 0.5;

    isStatic = true;
    isActive = true;
}

Entity::Entity(EntityType type) {
    entityType = type;
    isStatic = true;
    isActive = true;

    position = glm::vec3(0);
    custSizing = glm::vec3(0);

    speed = 0;
    width = 1;
    height = 1;
}

/*void Entity::UpdatePos(const float incrementX, const float incrementY)
{
    position.x += incrementX;
    position.y += incrementY;
}*/

void Entity::Update(float deltaTime, Entity player, Entity* objects, int objectCount, Entity* enemies, int enemyCount)
{
    collidedTop = false;
    collidedBottom = false;
    collidedLeft = false;
    collidedRight = false;

    if (entityType == ENEMY) {
        AI(player);
    }


    position.x += velocity.x * deltaTime;        // Move on X
    CheckCollisionsX(objects, objectCount);    // Fix if needed

    position.y += velocity.y * deltaTime;        // Move on Y
    CheckCollisionsY(objects, objectCount);    // Fix if needed
    //gravity
    velocity += acceleration * deltaTime;
}

//Very useful method but had to modify the implementation 
//a bit that way its not just rendering textures of a standard size at the center of the screen
void Entity::Render(ShaderProgram* program, glm::vec3 sizing) {
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, position);
    modelMatrix = glm::scale(modelMatrix, sizing);
    program->SetModelMatrix(modelMatrix);

    float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
    float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

    glBindTexture(GL_TEXTURE_2D, textureID);

    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->positionAttribute);

    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
    glEnableVertexAttribArray(program->texCoordAttribute);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}

bool Entity::CheckCollision(Entity other)
{
    float xdist = fabs(position.x - other.position.x) - ((width + other.width) / 2.0f);
    float ydist = fabs(position.y - other.position.y) - ((height + other.height) / 2.0f);
    if (xdist < 0 && ydist < 0)
    {
        //Win/lose boolean tracking block of code
        collisionDetected = true;
        if (other.entityType == GOAL) {
            winnerWinnerChickenDinner = true;
        }
        else if (other.entityType == TILE) {
            winnerWinnerChickenDinner = false;
        }
        return true;
    }
    return false;
}

void Entity::CheckCollisionsY(Entity* objects, int objectCount)
{
    for (int i = 0; i < objectCount; i++)
    {
        Entity object = objects[i];
        if (CheckCollision(object))
        {
            float ydist = fabs(position.y - object.position.y);
            float penetrationY = fabs(ydist - (height / 2) - (object.height / 2));
            if (velocity.y > 0) {
                position.y -= penetrationY;
                velocity.y = 0;
                collidedTop = true;
            }
            else if (velocity.y < 0) {
                position.y += penetrationY;
                velocity.y = 0;
                collidedBottom = true;

            }
        }
    }
}

void Entity::CheckCollisionsX(Entity* objects, int objectCount)
{
    for (int i = 0; i < objectCount; i++)
    {
        Entity object = objects[i];
        if (CheckCollision(object))
        {
            float xdist = fabs(position.x - object.position.x);
            float penetrationX = fabs(xdist - (width / 2) - (object.width / 2));
            if (velocity.x > 0) {
                position.x -= penetrationX;
                velocity.x = 0;
                collidedRight = true;
            }
            else if (velocity.x < 0) {
                position.x += penetrationX;
                velocity.x = 0;
                collidedLeft = true;
            }
        }
    }
}

void Entity::Jump()
{
    if (collidedBottom)
    {
        velocity.y = 4.5f;
    }
}

void Entity::aiIdle(Entity player)
{
    
}

void Entity::aiSlideAttack(Entity player)
{
    switch (move) { //do something different depending on state!
    case IDLE:
        if (glm::distance(position, player.position) < 2.0f) {
            move = SlideAttack;
        }
        break;
    case SlideAttack:
        if (player.position.x > position.x) {
            velocity.x = 1.0f; 
        }
        if (player.position.x < position.x) {
            velocity.x = -1.0f; 
        }
        break;
    }
}
void Entity::aiFloating(Entity player)
{
    switch (move) { //do something different depending on state!
    case IDLE:
        if (glm::distance(position, player.position) < 1.0f) {
            move = FLOATING;
        }
        else
        {
            velocity.y = 0.0f;
            velocity.x = 0.0f;
        }
        break;

    case FLOATING:
        if (position.x > 3.0f) {
            velocity.y = -0.1f;
            velocity.x = -0.1f;
            acceleration = glm::vec3(-0.2f, -0.1f, 0);
            //std::cout << "1" << std::endl; //debugging
        }
        //check which direction the velocity is when reaching the below 0.5 threshold
        else if (position.y < 0.0f) {
            if (velocity.x > 0.1f) {
                velocity.y = 0.1f;
                velocity.x = 0.1f;            
                acceleration = glm::vec3(0.2f, 0.1f, 0);

            }
            else if (velocity.x < -0.1f) {
                velocity.y = 0.1f;
                velocity.x = -0.1f;
                acceleration = glm::vec3(-0.2f, 0.1f, 0);

            }
        }
        else if (position.x < -3.0f) {
            velocity.y = -0.1f;
            velocity.x = 0.1f;
            acceleration = glm::vec3(0.2f, -0.1f, 0);
        }

        else
        {
            //velocity.y = -0.75f;
            //velocity.x = -1.0f;
            //acceleration = glm::vec3(-0.3f, -0.1f, 0);
            //velocity.y = 0.0f;
            //velocity.x = 0.0f;
            //acceleration = glm::vec3(0.0f, 0.0f, 0);

        }
        break;

        /*else
        {
            velocity.y = 1.0f;
            velocity.x = -1.0f;
        }*/
        /*else if (position.x > -2.0f) {
            //velocity.y = -1.0f;
            //velocity.x = -0.5f;
        }*/
    }
}

void Entity::AI(Entity player) {
    switch (aiType) {
    case GOOMBA:
        aiSlideAttack(player);
        break;

    case GHOST:
        aiFloating(player);
        break;

    case FROG:
        break;
    }
}

