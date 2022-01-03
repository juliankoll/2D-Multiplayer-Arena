#pragma once
#include <memory>
#include "iostream" 
using namespace std;
#include "SFML/Graphics.hpp"
class Player {
public:
	Player(int i_x, int i_y, int i_width, int i_height, float i_vel, float i_maxHp, int i_dmg);
    void givePath(vector<int> pathX, vector<int> pathY, int pathLenght);
    void deletePath();
    void move();
    void draw();
    void setTexture(int index);

    bool hasNewPath = false;
    vector<int> pathXpositions;
    vector<int> pathYpositions;
    int pathLenght;
    int cPathIndex;

    //status effects
    bool targetAble = true;
    bool inVladW = false;

    bool interruptedPath = false;
private:

	int y;
	int x;
	float velocity;
    
    int width;
    int height;
    int cTextureI;
    vector<sf::Texture> textures;

    void initTextures();
    //Getters and Setters (autogenerated)

    float hp;
    float maxHp;
    int dmg;


    bool findingPath;
    long long lastMoveTime;
   
    
public:
    //used in pathfinding threads => mutices are locked and unlocked----------
    void setFindingPath(bool i_findingPath);
    bool isFindingPath() const;
    void skipPathToIndex(int index);
    //------------------------------------------------------------------------


    int pathsFound = 0;
    inline int getY() const { return y; }
    inline void setY(int y) { this->y = y; }

    inline int getX() const { return x; }
    inline void setX(int x) { this->x = x; }

    inline float getVelocity() const { return velocity; }
    inline void setVelocity(float velocity) { this->velocity = velocity; }

    inline int getWidth() const { return width; }
    inline void setwidth(int width) { this->width = width; }

    inline int getHeight() const { return height; }
    inline void setheight(int height) { this->height = height; }

    inline float getHp() const { return hp; }
    inline void setHp(float hp) { this->hp = hp; }

    inline float getMaxHp() const { return maxHp; }
    inline void setMaxHp(float maxHp) { this->maxHp = maxHp; }

    inline int getDmg() const { return dmg; }
    inline void setDmg(int dmg) { this->dmg = dmg; }

    inline bool hasPath() const {
        if (pathLenght == -1) return false;
        return true;
    }


    inline int getPathgoalX() const {
        return pathXpositions[pathLenght - 1];
    }

    inline int getPathgoalY() const {
        return pathYpositions[pathLenght - 1];
    }

    inline int getTextureIndex() const {
        return cTextureI;
    }
};
