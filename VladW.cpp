#include "VladW.hpp"
#include "GlobalRecources.hpp"
#include "Utils.hpp"
#include "Graph.hpp"

static int i_onCDPhase = 0;
static int i_addToNetworkPhase = 0;
static int i_abilityIndex = 1;

VladW::VladW(int i_myPlayerIndex) : Ability(i_myPlayerIndex, false, i_onCDPhase, i_addToNetworkPhase, i_abilityIndex) {
	endPhaseAfterMS(2000);
	GlobalRecources::players[myPlayerIndex]->inVladW = true;
	GlobalRecources::players[myPlayerIndex]->targetAble = false;
}
//constructor through networking
VladW::VladW() : Ability(NetworkCommunication::receiveNextToken(), true, i_onCDPhase, i_addToNetworkPhase, i_abilityIndex) {
	endPhaseAfterMS(2000 - NetworkCommunication::receiveNextToken());
	GlobalRecources::players[myPlayerIndex]->inVladW = true;
	GlobalRecources::players[myPlayerIndex]->targetAble = false;
}

void VladW::execute0() {
	shared_ptr<Player> myPlayer = GlobalRecources::players[myPlayerIndex];
	for (int i = 0; i < GlobalRecources::playerCount; i++) {
		if (i != myPlayerIndex) {
			shared_ptr<Player> current = GlobalRecources::players[i];
			if (current->targetAble == true) {
				if (Utils::colisionRectCircle(current->getX(), current->getY(), current->getWidth(), current->getHeight(),
					myPlayer->getX() + (myPlayer->getWidth() / 2) - range, myPlayer->getY() + (myPlayer->getHeight() / 2) - range, range, 10) == true) {
					current->setHp(current->getHp() - damage);
				}
			}
		}
	}
}

void VladW::draw0() {
	shared_ptr<Player> current = GlobalRecources::players[myPlayerIndex];
	Renderer::drawCircle(current->getX() + (current->getWidth() / 2) - range,
		current->getY() + (current->getHeight() / 2) - range, range, sf::Color(150, 0, 0, 255), true, 0, false);
}

void VladW::init1() {
	GlobalRecources::players[myPlayerIndex]->inVladW = false;
	GlobalRecources::players[myPlayerIndex]->targetAble = true;

	shared_ptr<Player> current = GlobalRecources::players[myPlayerIndex];


	int y = current->getY();
	int x = current->getX();

	GlobalRecources::pFinding->findPath(x, y, myPlayerIndex);
}

void VladW::send() {
	NetworkCommunication::addToken(this->myPlayerIndex);
	auto cTime = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
	int timeSinceStart = cTime - this->getStartTime(this->getPhase());
	NetworkCommunication::addToken(timeSinceStart);
}