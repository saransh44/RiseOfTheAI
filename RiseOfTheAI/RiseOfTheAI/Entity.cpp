#include "Entity.h"

Entity::Entity()
{
    entityType = DEFAULT;
    
    position = glm::vec3(0);
    custSizing = glm::vec3(0);
    movement = glm::vec3(0);

    speed = 0;
    width = 0.5;
    height = 0.5;

    isStatic = false;
    isActive = true;
}

Entity::Entity(EntityType type) {
    entityType = type;
    isStatic = false;
    isActive = true;
    movement = glm::vec3(0);

    position = glm::vec3(0);
    custSizing = glm::vec3(0);

    speed = 0;
    width = 1;
    height = 1;
}

void Entity::Update(float deltaTime, Entity player, Entity* objects, int objectCount, Entity* enemies, int enemyCount)
{
    if (isActive == false) return;

    collidedTop = false;
    collidedBottom = false;
    collidedLeft = false;
    collidedRight = false;

    if (entityType == ENEMY) {
        AI(player);
    }
    if (animIndices != NULL) {
        if (glm::length(movement) != 0) {
            animTime += deltaTime;

            if (animTime >= 0.25f)
            {
                animTime = 0.0f;
                animIndex++;
                if (animIndex >= animFrames)
                {
                    animIndex = 0;
                }
            }
        }
        else {
            animIndex = 0;
        }
    }

    //simple physics
    velocity += acceleration * deltaTime;

    position.x += velocity.x * deltaTime;        // Move on X
    CheckCollisionsX(objects, objectCount);    // Fix if needed   
    
    position.y += velocity.y * deltaTime;        // Move on Y
    CheckCollisionsY(objects, objectCount);    // Fix if needed
    
    //I was having no interaction with enemies and so lost as to what was going on.
    //It turns out I wasn't actually checking eneimes, ideally we would make the objects array a bit bigger and move entities of AI in but I digress. 
    //That might also be a little slow because everytime you would go through checking a ton of tiles first before hitting AI 
    //so maybe having a seperate array is faster.
    if (entityType == PLAYER) {

        // Outside of screen
        /*if (position.x > 5 || position.x < -5 || position.y < -3.5) {
            winnerWinnerChickenDinner = false;
        }*/
        CheckCollisionsY(enemies, enemyCount);
        CheckCollisionsX(enemies, enemyCount);
    }
}

void Entity::DrawSpriteFromTextureAtlas(ShaderProgram* program, GLuint textureID, int index)
{
    float u = (float)(index % animCols) / (float)animCols;
    float v = (float)(index / animCols) / (float)animRows;

    float width = 1.0f / (float)animCols;
    float height = 1.0f / (float)animRows;

    float texCoords[] = { u, v + height, u + width, v + height, u + width, v,
        u, v + height, u + width, v, u, v };

    float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };

    glBindTexture(GL_TEXTURE_2D, textureID);

    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->positionAttribute);

    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
    glEnableVertexAttribArray(program->texCoordAttribute);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}

//Very useful method but had to modify the implementation 
//a bit that way its not just rendering textures of a standard size at the center of the screen
void Entity::Render(ShaderProgram* program, glm::vec3 sizing) {
    if (isActive == false) {
        return;
    }
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

/*bool Entity::CheckCollision(Entity *other) {
    if (other->entityType == this->entityType) return false; //Tiles touching each other?
    if (isStatic == true) return false;
    if (isActive == false || other->isActive == false) return false;

    float xdist = fabs(position.x - other->position.x) - ((width + other->width) / 2.0f);
    float ydist = fabs(position.y - other->position.y) - ((height + other->height) / 2.0f);

    collisionDetected = false;
    if (xdist < 0 && ydist < 0)
    {
        //Win/lose boolean tracking block of code
        if (other->entityType == ENEMY) {
            other->collisionDetected = true;

        }
        collisionDetected = true;
        
        //otherEntityTypeInteraction = other.entityType;
    }
    return collisionDetected;
}*/
bool Entity::CheckCollision(Entity* other) {
    if (other->entityType == this->entityType) return false; //Tiles touching each other?
    if (isStatic == true) return false;
    if (isActive == false || other->isActive == false) return false;

    float xdist = fabs(position.x - other->position.x) - ((width + other->width) / 2.0f);
    float ydist = fabs(position.y - other->position.y) - ((height + other->height) / 2.0f);

    if (xdist < 0 && ydist < 0) return true;
    return false;
}

/*void Entity::CheckCollisionsY(Entity* objects, int objectCount)
{
    for (int i = 0; i < objectCount; i++)
    {
        Entity* object = &objects[i];
        if (object->isActive) {
            if (CheckCollision(object))
            {
                float xdist = fabs(position.x - object->position.x);
                float ydist = fabs(position.y - object->position.y);
                float penetrationY = fabs(ydist - (height / 2) - (object->height / 2));
                if (velocity.y > 0) {
                    /*if (!(entityType == PLAYER && object->entityType == ENEMY)) {
                        position.y -= penetrationY;
                    }*/
                    /*if (entityType == PLAYER && object->entityType == ENEMY) {
                        std::cout << "top" << std::endl;
                    }
                    position.y -= penetrationY;
                    velocity.y = 0;
                    collidedTop = true;
                    object->collidedBottom = true;

                }
                else if (velocity.y < 0) {
                    /*if (!(entityType == PLAYER && object->entityType == ENEMY)) {
                        position.y += penetrationY;
                    }*/
                    /*if (entityType == PLAYER && object->entityType == ENEMY) {
                        std::cout << "bottom" << std::endl;
                    }
                    position.y += penetrationY;
                    velocity.y = 0;
                    collidedBottom = true;
                    object->collidedTop = true;
                }
                /*if ((object->entityType == ENEMY) && (xdist < 0.6)) {
                    object->isActive = false;
                    std::cout << "here";
                }*/
                /*if (entityType == PLAYER && object->entityType == ENEMY) {
                    isActive = false;
                }*/
               /*if (entityType == PLAYER && object->entityType == ENEMY && object->collidedTop && collidedBottom && velocity.y <= 0) { //since velocity gets reset to 0 above need to have <=
                    object->isActive = false;
                    object->isStatic = true;
                    //std::cout << object->collisionDetected;

                    //velocity.y = 1.0f;
                    enemiesLeft--;
                }
               
            //std::cout << object->isActive << std::endl;
            }
            
        }
    }
}*/
void Entity::CheckCollisionsY(Entity* objects, int objectCount)
{
    for (int i = 0; i < objectCount; i++)
    {
        Entity* object = &objects[i];
        if (object->isActive == false) continue;

        if (CheckCollision(object)) {
            float xdist = fabs(position.x - object->position.x);
            float ydist = fabs(position.y - object->position.y);
            float penetrationY = fabs(ydist - (height / 2) - (object->height / 2));
            if (velocity.y > 0) {
                if (entityType == PLAYER && object->entityType == ENEMY) {
                    std::cout << "top" << std::endl;
                    //isActive = false;
                    isStatic = true;
                }

                position.y -= penetrationY;
                velocity.y = 0;
                collidedTop = true;
            }
            else if (velocity.y < 0) {
                position.y += penetrationY;
                velocity.y = 0;
                collidedBottom = true;
  
                if (entityType == PLAYER && object->entityType == ENEMY) {
                    std::cout << "bottom" << std::endl;
                    object->isActive = false;
                    object->isStatic = true;
                    enemiesLeft--;
                }
            }
        }
    }
}

void Entity::CheckCollisionsX(Entity* objects, int objectCount)
{
    for (int i = 0; i < objectCount; i++)
    {
        Entity* object = &objects[i];
        if (object->isActive) {
            if (CheckCollision(object))
            {
                float xdist = fabs(position.x - object->position.x);
                float penetrationX = fabs(xdist - (width / 2) - (object->width / 2));
                if (velocity.x > 0) {

                    /*if (!(entityType == PLAYER && object->entityType == ENEMY)) {
                        position.x -= penetrationX;
                    }*/
                    if (entityType == PLAYER && object->entityType == ENEMY) {
                        std::cout << "right" << std::endl;
                        //isActive = false;
                        isStatic = true;
                    }
                    position.x -= penetrationX;
                    velocity.x = 0;
                    collidedRight = true;
                    object->collidedLeft = true;
                }
                else if (velocity.x < 0) {
                    /*if (!(entityType == PLAYER && object->entityType == ENEMY)) {
                        position.x += penetrationX;
                    }*/
                    if (entityType == PLAYER && object->entityType == ENEMY) {
                        std::cout << "left" << std::endl;
                        isStatic = true;

                    }
                    position.x += penetrationX;
                    velocity.x = 0;
                    collidedLeft = true;
                    object->collidedRight = true;
                }
            }
        }
    }
}

void Entity::Jump()
{
    if (collidedBottom)
    {
        velocity.y = 5.0f;
    }
}

void Entity::aiIdle(Entity player)
{
    
}
void Entity::aiLeaping(Entity player)
{
    switch (move) { //do something different depending on state!
    case IDLE:
        /*if (glm::distance(position, player.position) < 2.0f) {
            move = JUMPING;
        }*/
        break;

    case JUMPING:
        if (position.y > -1.5) {
            velocity.y = -velocity.y;
            velocity.x = -0.3f;
        }
        else if (velocity.y == 0) {
            velocity.y = 1.0f;
            velocity.x = 0.3f;
        }
        /*else if (collidedLeft) {
            velocity.y = 1.0f;
            velocity.x = -0.3f;
        }*/
        /*else {
            
        }*/

        break;
    }
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
        else if (player.position.x < position.x) {
            velocity.x = -1.0f; 
        }
        else {
            isStatic = true;
        }
        break;
    }
}
void Entity::aiFloating(Entity player)
{
    switch (move) { //do something different depending on state!
    case IDLE:
        /*if (glm::distance(position, player.position) < 1.0f) {
            move = FLOATING;
        }
        else {
            velocity.y = 0.0f;
            velocity.x = 0.0f;
        }*/
        break;

    case FLOATING:
        const float yVel = 0.4f;
        const float xVel = 0.4f; 

        const float xAccel = 0.4f;
        const float yAccel = 0.2f;

        if (position.x > 4.0f) {
            velocity.y = -yVel;
            velocity.x = -xVel;
            acceleration = glm::vec3(-xAccel, -yAccel, 0);
            //std::cout << "1" << std::endl; //debugging
        }
        //check which direction the velocity is when reaching the below a certain horizontal boundary threshold
        else if (position.y < 0.6f) { 
            if (velocity.x > 0.1f) { 
                velocity.y = -velocity.y;
                //velocity.x = 0.4f;       
                acceleration = glm::vec3(xAccel, yAccel, 0);

            }
            else if (velocity.x < -0.1f) {
                velocity.y = -velocity.y;
                //velocity.x = -0.4f;
                acceleration = glm::vec3(-xAccel, yAccel, 0);

            }
        }
        else if (position.x < -3.4f) {
            velocity.y = -yVel;
            velocity.x = xVel;
            acceleration = glm::vec3(xAccel, -yAccel, 0);
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
        aiLeaping(player);
    }
}

