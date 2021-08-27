#include "Player.hpp"
#include "Renderer.hpp"
#include "Projectile.hpp"
#include "Terrain.hpp"
#include "Utils.hpp"
#include "PathfindingHandler.hpp"
#include "Indicator.hpp"

using namespace std::chrono;

namespace abilityRecources {

    int worldRows, worldCols;
    Terrain* terrain;
    Player** players;
    int playerCount;
    Pathfinding* pFinding; 
    void init(Player** i_players, int i_playerCount, Terrain* i_terrain, int i_worldRows, int i_worldCols, Pathfinding* i_pathfinding) {
        players = i_players;
        playerCount = i_playerCount;
        terrain = i_terrain;
        worldRows = i_worldRows;
        worldCols = i_worldCols;
        pFinding = i_pathfinding;
    }
}
using namespace abilityRecources;

class Fireball {
public:
    Fireball(int i_myPlayerIndex) {
        this->myPlayerI = i_myPlayerIndex;

        //Turn player to mouse coords and set mouse coords as goal coords
        this->goalRow = 0;
        this->goalCol = 0;
        Renderer::getMousePos(&goalCol, &goalRow, true);
        Player* myPlayer = players [myPlayerI];
        //if projectile destination is above player
        if (this->goalRow < myPlayer->getRow()) {
            this->startCol = myPlayer->getCol() + (myPlayer->getWidth() / 2);
            this->startRow = myPlayer->getRow();
            myPlayer->setTexture(2);
        }
        //below
        if (this->goalRow > myPlayer->getRow()) {
            this->startCol = myPlayer->getCol() + (myPlayer->getWidth() / 2);
            this->startRow = myPlayer->getRow() + myPlayer->getHeight();
            myPlayer->setTexture(3);
        }

        limitGoalPosToRange();
        this->helpProjectile = new Projectile(startRow, startCol, velocity, goalRow, goalCol, false, radius, myPlayer);
    }

    void update() {
        if (this->exploding == false) {
            auto collidables = terrain->getCollidables();
            this->helpProjectile->move(worldRows, worldCols, collidables->data(), collidables->size());
            //if the projectile reaches its max range or collides with anything, it should explode
            if ((abs(this->startRow - this->helpProjectile->getRow()) * abs(this->startRow - this->helpProjectile->getRow())
                + abs(this->startCol - this->helpProjectile->getCol()) * abs(this->startCol - this->helpProjectile->getCol())
                > this->range * this->range) || this->helpProjectile->isDead() == true) {
                this->exploding = true;
                this->fireStartTime = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
                
                this->explosionRow = this->helpProjectile->getRow() 
                + this->helpProjectile->getRadius() - this->explosionRange;
                this->explosionCol = this->helpProjectile->getCol() 
                + this->helpProjectile->getRadius() - this->explosionRange;
            }
        } 
        else {
            long cTime = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
            this->timeDiff = cTime - this->fireStartTime.count();
            if (this->timeDiff < 2000) {
                for (int i = 0; i < playerCount; i++) {
                        Player* c = players [i];
                        bool collision = Utils::collisionRectCircle(c->getCol(), c->getRow(), c->getWidth(), c->getHeight(),
                            this->explosionCol, this->explosionRow, this->explosionRange, 10);
                        if (collision == true) {
                            if (this->dealtDamage == false) {
                                c->setHp(c->getHp() - this->explosionDmg);
                            }
                            else {
                                c->setHp(c->getHp() - this->burnDmg);
                            }
                        }
                    
                }
                this->dealtDamage = true;
            }
            else {
                this->finished = true;
            }
        }
    }

    void draw() {
        if (this->exploding == false) {
            this->helpProjectile->draw(sf::Color(255, 120, 0, 255));
        }
        else {
            Renderer::drawCircle(this->explosionRow, this->explosionCol, this->explosionRange,
                sf::Color(255, 120, 0, 255), true, 0, false);
        }
    }


    //create from network input(row is just current row so even with lag the start is always synced)
    Fireball(int i_startRow, int i_startCol, int i_goalRow, int i_goalCol, int i_myPlayerIndex) {
        this->startRow = i_startRow;
        this->startCol = i_startCol;
        this->goalRow = i_goalRow;
        this->goalCol = i_goalCol;
        this-> myPlayerI = i_myPlayerIndex;

        limitGoalPosToRange();
        this->helpProjectile = new Projectile(startRow, startCol, velocity, goalRow, goalCol, false, radius, players [myPlayerI]);
    }

    void limitGoalPosToRange() {
        float* vecToGoal = new float [2];
        vecToGoal [0] = goalCol - startCol;
        vecToGoal [1] = goalRow - startRow;
        //calculate vector lenght
        float lenght = sqrt((vecToGoal [0] * vecToGoal [0]) + (vecToGoal [1] * vecToGoal [1]));
        if (lenght > range) {
            //normalize vector lenght
            vecToGoal [0] /= lenght;
            vecToGoal [1] /= lenght;
            //stretch vector to range
            vecToGoal [0] *= range;
            vecToGoal [1] *= range;
            //place at starting point
            goalCol = startCol + vecToGoal [0];
            goalRow = startRow + vecToGoal [1];
        }
    }

public:
    int startRow;
    int startCol;
    int goalRow;
    int goalCol;
    bool finished = false;
    int myPlayerI;
private:
    bool dealtDamage = false;
    bool exploding = false;
    int explosionRange = 80;

    milliseconds fireStartTime;
    long timeDiff;
    int explosionRow, explosionCol;

    int explosionDmg = 30;
    float burnDmg = 0.25f;

    int radius = 50;
    int range = 700;
    float velocity = 15.0f;

    Projectile* helpProjectile;

};




class Transfusion {
public:
    Transfusion(int i_myPlayerIndex) {
        this->myPlayerIndex = i_myPlayerIndex;
        this->indicator = new PointAndClickIndicator(this->myPlayerIndex, this->range, playerCount, players, i_myPlayerIndex);

        lastRows = new int [positionsSavedCount];
        lastCols = new int [positionsSavedCount];
        for (int i = 0; i < positionsSavedCount; i++) {
            lastRows [i] = -1;
            lastCols [i] = -1;
        }
    }
    //constructor through networking
    Transfusion(int i_myPlayerIndex, int i_targetPlayerIndex) {
        this->myPlayerIndex = i_myPlayerIndex;
        this->targetPlayerIndex = i_targetPlayerIndex;

        me = players[myPlayerIndex];
        target = players[targetPlayerIndex];
        casting = true;

        lastRows = new int [positionsSavedCount];
        lastCols = new int [positionsSavedCount];
        for (int i = 0; i < positionsSavedCount; i++) {
            lastRows [i] = -1;
            lastCols [i] = -1;
        }
    }

    void update() {
        if(casting == false) {
            if(indicator->getTargetIndex() == -1) {
                indicator->update();
                if(indicator->endWithoutAction() == true) {
                    finishedWithoutCasting = true;
                }
            } 
            else {
                if(initializedEvents == false){
                    initializedEvents = true;
                    initEvents();
                }
                if(casting == false) {//if you still have to walk
                    int halfW = me->getWidth() / 2;
                    if(tempGoalRow != (target->getRow() + halfW) || (tempGoalCol != target->getCol() + halfW)) {
                        tempGoalRow = target->getRow() + halfW;
                        tempGoalCol = target->getCol() + halfW;
                        pFinding->findPath(tempGoalCol, tempGoalRow, myPlayerIndex);
                        abilityPathIndex = players[myPlayerIndex]->pathsFound;
                    }
                    bool stop = false;
                    //multithreading problem: we want, if another path is found for the player (e.g. through clicking)
                    //to stop going on the path the ability wants to find. But because of multithreading we cant say
                    //when the pathfinding is finished with this particular path, so we just count the paths up in the
                    //player obj and if the index is equal to abiltyPathIndex its still finding the path (same index
                    //as when it was saved) and if its one higher the path was found. if its 2 higher a new path
                    //was found and we interrupt.
                    if(players[myPlayerIndex]->pathsFound > abilityPathIndex + 1) {
                        stop = true;
                    }
                    if(stop == false) {
                        int halfW = me->getWidth() / 2;
                        int halfH = me->getHeight() / 2;
                        if(Utils::calcDist2D(me->getCol() + halfW, target->getCol() + halfW, 
                                    me->getRow() + halfH, target->getRow() + halfH) < range) {
                            //got into range, stop going on path an cast ability
                            players[myPlayerIndex]->deletePath();
                            casting = true;
                        }
                    } 
                    else {
                        //clicked somewhere else while finding path to target player to get in range => abort cast
                        finishedWithoutCasting = true;
                    }
                }
            }
        }
        else {
            if(castingInitialized == false) {
                castingInitialized = true;
                castStart = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
                
                tempGoalRow = target->getRow();
                tempGoalCol = target->getCol();
                int halfW = me->getWidth() / 2;
                bloodBall = new Projectile(me->getRow() + halfW, me->getCol() + halfW, velocity, 
                        tempGoalRow + halfW, tempGoalCol + halfW, false, radius, me);

                for (int i = 0; i < positionsSavedCount; i++) {
                    lastRows [i] = -1;
                    lastCols [i] = -1;
                }
            }

            long cTime = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
            timeDiff = cTime - castStart;
            
            if(flyBack == false){
                //blood ball got to enemy and should fly back
                if (Utils::collisionRectCircle(target->getRow(), target->getCol(), target->getWidth(), target->getHeight(),
                            bloodBall->getRow(), bloodBall->getCol(), bloodBall->getRadius(), 10) == true) {
                    flyBack = true;
                    target->setHp(target->getHp() - dmg);
                }

                if(tempGoalRow != target->getRow() || tempGoalCol != target->getCol()){
                    tempGoalRow = target->getRow();
                    tempGoalCol = target->getCol();
                    int tempBBrow = bloodBall->getRow();
                    int tempBBcol = bloodBall->getCol();
                    delete bloodBall;//definitly exists at this point so we can delete it

                    int halfW = me->getWidth() / 2;
                    bloodBall = new Projectile(tempBBrow + radius, tempBBcol + radius, velocity,
                            tempGoalRow + halfW, tempGoalCol + halfW, false, radius, me);
                }
            } 
            else {
                //blood ball got back to player with hp
                if (Utils::collisionRectCircle(me->getRow(), me->getCol(), me->getWidth(), me->getHeight(),
                            bloodBall->getRow(), bloodBall->getCol(), bloodBall->getRadius(), radius) == true) {
                    if(me->getHp() + heal <= me->getMaxHp()){
                        me->setHp(me->getHp() + heal);
                    }
                    else {
                        me->setHp(me->getMaxHp());
                    }

                    finishedCompletely = true;
                }

                if(tempGoalRow != me->getRow() || tempGoalCol != me->getCol()){
                    tempGoalRow = me->getRow();
                    tempGoalCol = me->getCol();
                    int tempBBrow = bloodBall->getRow();
                    int tempBBcol = bloodBall->getCol();
                    delete bloodBall;//definitly exists at this point so we can delete it

                    int halfW = me->getWidth() / 2;
                    bloodBall = new Projectile(tempBBrow + radius, tempBBcol + radius, velocity,
                            tempGoalRow + halfW, tempGoalCol + halfW, false, radius, me);
                }
            }
            bloodBall->move(worldRows, worldCols, nullptr, 0);//should go through walls so we just dont pass them
        }
    }

    void initEvents() {
        targetPlayerIndex = indicator->getTargetIndex();
        me = players[myPlayerIndex];
        target = players[targetPlayerIndex];

        finishedSelectingTarget = true;

        //if player out of range, run into range
        int halfW = me->getWidth() / 2;
        int halfH = me->getHeight() / 2;

        float dist = Utils::calcDist2D(me->getCol() + halfW, target->getCol() + halfW, 
                me->getRow() + halfH, target->getRow() + halfH);
        if(dist > range) {
            tempGoalRow = target->getRow() + halfW;
            tempGoalCol = target->getCol() + halfH;
            pFinding->findPath(tempGoalCol, tempGoalRow, myPlayerIndex);
            abilityPathIndex = players[myPlayerIndex]->pathsFound;
        } 
        else {
            casting = true;
        }

        sf::Cursor cursor;
        if (cursor.loadFromSystem(sf::Cursor::Arrow)) {
            Renderer::currentWindow->setMouseCursor(cursor);
        }
    }

    void draw() {
        
        if(indicator != nullptr) {
            if(indicator->getTargetIndex() == -1) {
                indicator->draw();
            }
        } 
        if(bloodBall != nullptr) {
            lastRows [cPositionSaveIndex] = bloodBall->getRow();
            lastCols [cPositionSaveIndex] = bloodBall->getCol();
            cPositionSaveIndex ++;
            if (cPositionSaveIndex >= positionsSavedCount) {
                cPositionSaveIndex = 0;
            }
            bloodBall->draw(sf::Color(255, 0, 0, 255));
            for (int i = 0; i < positionsSavedCount; i++) {
                if (lastRows [i] != -1) {
                    Renderer::drawCircle(lastRows [i], lastCols [i], bloodBall->getRadius(), sf::Color(255, 0, 0, 255), true, 0, false);
                }
            }
        }
    }

    bool finishedWithoutCasting = false;
    bool finishedCompletely = false;
    bool finishedSelectingTarget = false;
    int myPlayerIndex;
    int targetPlayerIndex;
    bool casting = false;
    bool addedToNetwork = false;
private:
    bool castingInitialized = false;
    long castStart;
    long timeDiff;
    float pathPercent;
    Projectile* bloodBall = nullptr;
    int tempGoalRow, tempGoalCol;
    bool flyBack = false;

    PointAndClickIndicator* indicator = nullptr;
    int range = 300;

    bool initializedEvents = false;

    int abilityPathIndex;

    Player* me;
    Player* target;

    int dmg = 30;
    int heal = 15;
    int radius = 10;
    int velocity = 5.0f;


    int positionsSavedCount = 10;
    int* lastRows;
    int* lastCols;
    int cPositionSaveIndex = 0;
};