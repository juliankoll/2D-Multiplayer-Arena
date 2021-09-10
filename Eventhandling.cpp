#include "Eventhandling.hpp"
#include "Renderer.hpp"
#include "Player.hpp"
#include "Terrain.hpp"
#include "Projectile.hpp"
#include "PathfindingHandler.hpp"
#include "Utils.hpp"
#include "PortableClient.hpp"
#include "PortableServer.hpp"
#include "Menu.hpp"
#include "WorldHandling.hpp"
#include "UiHandling.hpp"
#include "ProjectileHandling.hpp"
#include "NetworkCommunication.hpp"
#include "Playerhandling.hpp"
#include "AbilityHandling.hpp"
#include "GlobalRecources.hpp"

static Pathfinding* pathfinding;

static Menu* menu;
static bool menuActive = true;

static PortableServer* server;
static PortableClient* client;

static UiHandling* uiHandling;
static WorldHandling* worldHandling;
static ProjectileHandling* projectileHandling;
static PlayerHandling* playerHandling;
static AbilityHandling* abilityHandling;
static bool received = true;

static void initServer();
static void initClient();
static void sendData();
static void recvAndImplementData();
static std::thread* networkThread;


void eventhandling::init() {
	//be sure to not change the order, they depend on each other heavily
	playerHandling = new PlayerHandling();
	worldHandling = new WorldHandling();

	int worldRows = worldHandling->getWorldRows();
	int worldCols = worldHandling->getWorldCols();
	uiHandling = new UiHandling(worldHandling->getFrameRows(), worldHandling->getFrameCols());
	pathfinding = new Pathfinding(worldRows, worldCols, worldHandling->getTerrain(), playerHandling->getPlayers(),
		playerHandling->getPlayerCount());
	projectileHandling = new ProjectileHandling(worldRows, worldCols, playerHandling->getPlayers(), playerHandling->getPlayerCount());
	NetworkCommunication::init();
	menu = new Menu();

	//so we dont have to worry about this messy recource-hell anymore
	GlobalRecources::init(playerHandling->getPlayers(), playerHandling->getPlayerCount(), worldHandling->getTerrain(), 
												worldRows, worldCols, pathfinding, pathfinding->getPathfingingMutex());
}

void eventhandling::eventloop() {
	//if host or client has not been selected, wait for it to happen
	if (menuActive == true) {
		menu->update();
		if (menu->hostServer() == true) {
			playerHandling->setPlayerIndex(0);//server has player index 0
			pathfinding->setPlayerIndex(0);
			projectileHandling->setPlayerIndex(0);
			abilityHandling = new AbilityHandling(0);

			networkThread = new std::thread(&initServer);
			menuActive = false;//go to game
		}
		if (menu->connectAsClient() == true) {
			playerHandling->setPlayerIndex(1);//right now there are only two players so the client just has index 1
			pathfinding->setPlayerIndex(1);
			projectileHandling->setPlayerIndex(1);
			abilityHandling = new AbilityHandling(1);
			networkThread = new std::thread(&initClient);
			menuActive = false;//go to game
		}
	}

	//if host or client has been selected
	if (menuActive == false) {
		worldHandling->update();
		//pass current hp informations to uiHandling so that it can draw a proper life bar
		uiHandling->updateLifeBar(playerHandling->getMyPlayer()->getHp(), playerHandling->getMyPlayer()->getMaxHp());
		//does pathfinding on click and player collision
		pathfinding->update();
		//moves all abilities and ticks through their states (example projectile->explosion->buring etc.)
		abilityHandling->update();

		//pass collidbales to projectile management every update so that projectiles can even be stopped by moving terrain
		auto collidables = worldHandling->getTerrain()->getCollidables();
		projectileHandling->update(collidables->data(), collidables->size());

		//pass game information back and forth through tcp sockets
		if ((server != nullptr && server->isConnected() == true) || (client != nullptr && client->isConnected() == true)) {
			if (received == true) {//handshaking: only if something was received send again. Prevents lag and unwanted behavior
				sendData();
				received = false;
			}
			recvAndImplementData();
		}
	}
}


void eventhandling::drawingloop() {
	int mouseX = -1, mouseY = -1;
	Renderer::getMousePos(&mouseX, &mouseY, true, true);//writes mouse coords into mouseX, mouseY
	Renderer::drawRect(mouseY, mouseX, 50, 50, sf::Color(255, 0, 0, 255), false);
	if (menuActive == true) {
		menu->drawMenu();
	}
	else {
		worldHandling->draw();//draw first, lifebars and stuff should be drawn over it
		abilityHandling->drawAbilities();
		projectileHandling->draw();
		playerHandling->draw();
		uiHandling->draw();//draw last, should be over every item ingame
		abilityHandling->drawCDs();
	}
}


//Neworking part-----------------------------------------------------------------------------------------------------------------


static void initServer() {
	server = new PortableServer();
	server->waitForClient();
	server->receiveMultithreaded();
}

static void initClient() {
	std::string s = "192.168.178.28";//TODO: typeable ip
	client = new PortableClient(s.c_str());
	client->waitForServer();
	client->receiveMultithreaded();
}


static void sendData() {
	NetworkCommunication::initNewCommunication();

	abilityHandling->sendData();
	playerHandling->sendPlayerData();
	projectileHandling->sendProjectiles();


	if (playerHandling->getPlayerIndex() == 0) {
		NetworkCommunication::sendTokensToServer(server);
	}
	if (playerHandling->getPlayerIndex() == 1) {
		NetworkCommunication::sendTokensToClient(client);
	}
}

static void recvAndImplementData() {

	bool receivedSth = false;
	if (playerHandling->getPlayerIndex() == 0) {
		if (server->newMessage() == true) {
			receivedSth = true;
			NetworkCommunication::receiveTonkensFromServer(server);
		}
	}
	else {
		if (client->newMessage() == true) {
			receivedSth = true;
			NetworkCommunication::receiveTonkensFromClient(client);
		}
	}
	if (receivedSth == true) {
		abilityHandling->receiveData();
		playerHandling->receivePlayerData(pathfinding);
		projectileHandling->receiveProjectiles();

		received = true;
	}
}
