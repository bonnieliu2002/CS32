#include "StudentWorld.h"
#include "Actor.h"
#include <sstream>
#include <iomanip>
#include <list>
using namespace std;

GameWorld* createStudentWorld(string assetPath)
{
	return new StudentWorld(assetPath);
}

/**********************************************************************************/
/*					   STUDENTWORLD CLASS IMPLEMENTATION                          */
/**********************************************************************************/
StudentWorld::StudentWorld(string assetDir)
	: GameWorld(assetDir)
{
}

StudentWorld::~StudentWorld()
{
	for (auto it = m_actors.begin(); it != m_actors.end();)
	{
		delete (*it);
		it = m_actors.erase(it);
	}
}

int StudentWorld::init()
{
	// place Socrates at (0, 128)
	m_player = new Socrates(this, 0, VIEW_HEIGHT/2);
	// place Pit(s) randomly without overlap; number of pits in game = level
	for (int i = 0; i < getLevel(); i++)
	{
		double x, y;
		generateRandomPos(x, y);
		Actor* temp = new Pit(this, x, y);
		addActor(temp);
	}
	// place Food items randomly without overlap
	for (int i = 0; i < min(5 * getLevel(), 25); i++)
	{
		double x, y;
		generateRandomPos(x, y);
		Actor* temp = new Food(this, x, y);
		addActor(temp);
	}
	// place Dirt items randomly, allowing overlap with other Dirt piles
	for (int i = 0; i < max(180 - 20 * getLevel(), 20); i++)
	{
		double x = -1480234;
		double y = 1480234;
		generateRandomPos(x, y);
		Actor* temp = new Dirt(this, x, y);
		addActor(temp);
	}
	return 1;
}

int StudentWorld::move()
{
	m_player->doSomething();
	// if player made a move that caused it to die, return immediately
	if (m_player->isDead())
	{
		decLives();
		return GWSTATUS_PLAYER_DIED;
	}
	// let all the other actors make a move
	for (auto it = m_actors.begin(); it != m_actors.end();)
	{
		(*it)->doSomething();
		if ((*it)->isDead())
		{
			delete* it;
			it = m_actors.erase(it);
		}
		else
		{
			it++;
		}
	}
	// check if all bacterias and pits have disappeared
	bool timeToAdvance = true;
	for (auto it = m_actors.begin(); it != m_actors.end(); it++)
	{
		if ((*it)->preventsLevelCompleting())
		{
			timeToAdvance = false;
			break;
		}
	}
	if (timeToAdvance)
		return GWSTATUS_FINISHED_LEVEL;
	// add new objects (e.g. goodie or fungus)
	int chanceNewFungus = max(510 - getLevel() * 10, 200);
	int tempRand = randInt(0, chanceNewFungus);
	if (tempRand == 0)
	{
		double dx, dy;
		int angle = randInt(0, 359);
		getPositionOnCircumference(angle, dx, dy);
		addActor(new Fungus(this, dx, dy));
	}
	// if a new goodie is created, the probability of each type is listed below
	// Extra Life Goodie: 10%
	// Flamethrower Goodie: 30%
	// Restore Health Goodie: 60%
	int chanceGoodie = max(510 - getLevel() * 10, 250);
	tempRand = randInt(0, chanceGoodie);
	if (tempRand == 0)
	{
		double dx, dy;
		int angle = randInt(0, 359);
		getPositionOnCircumference(angle, dx, dy);
		tempRand = randInt(1, 10);
		if (tempRand == 1)
			addActor(new ExtraLifeGoodie(this, dx, dy));
		else if (tempRand > 1 && tempRand < 5)
			addActor(new FlamethrowerGoodie(this, dx, dy));
		else
			addActor(new RestoreHealthGoodie(this, dx, dy));
	}
	// print/update status bar
	ostringstream oss;
	int k;
	// if score is negative, don't put 0s in front of the negative sign
	if (getScore() < 0)
		oss.fill(' ');
	else
		oss.fill('0');
	oss << "Score: " << setw(6) << getScore();
	oss << "  Level: " << getLevel() << "  Lives: " << getLives() << "  health: " << m_player->numHitPoints() << "  Sprays: " << m_player->numSprays() << "  Flames: " << m_player->numFlames();
	setGameStatText(oss.str());
	return GWSTATUS_CONTINUE_GAME;
}

void StudentWorld::cleanUp()
{
	delete m_player;
	for (auto it = m_actors.begin(); it != m_actors.end();)
	{
		delete (*it);
		it = m_actors.erase(it);
	}
}

void StudentWorld::addActor(Actor* a)
{
	m_actors.push_back(a);
}

// generates a valid random position for Actors to be placed in the arena
void StudentWorld::generateRandomPos(double& x, double& y)
{
	while (!isValid(x, y))
	{
		// generate random x value within width of petri dish
		x = randInt(VIEW_WIDTH / 2 - 120, VIEW_WIDTH / 2 + 120);
		// based on x value above, generate a range of possible y values using formula for circle
		int maxY = (int)(VIEW_RADIUS + sqrt(-1984 - pow(x, 2) + 256 * x));
		int minY = (int)(VIEW_RADIUS - sqrt(-1984 - pow(x, 2) + 256 * x));
		// generate random y value within range
		y = randInt(minY, maxY);
	}
}

// checks if random position generated is a valid one
bool StudentWorld::isValid(double& x, double& y) const
{
	// is this position within 120 pixels of the center of the arena?
	if (pow(x - 128, 2) + pow(y - 128, 2) > pow(120, 2))
		return false;
	for (auto it = m_actors.begin(); it != m_actors.end(); it++)
	{
		// if this position overlaps with other Dirt piles, it's fine. Continue the loop.
		if ((*it)->blocksBacteriumMovement())
			continue;
		// if this position overlaps with non-Dirt items, return false
		if ((*it)->isOverlapping(x, y))
			return false;
	}
	return true;
}

Socrates* StudentWorld::getOverlappingSocrates(Actor* a) const
{
	// check if overlapping with Socrates
	if (a->isOverlapping(m_player->getX(), m_player->getY()))
		return m_player;
	return nullptr;
}

Actor* StudentWorld::getOverlappingEdible(Actor* a) const
{
	// check each actor to see if actor is food and overlaps with our passed-in actor a 
	for (auto it = m_actors.begin(); it != m_actors.end(); it++)
	{
		if (a->isOverlapping((*it)->getX(), (*it)->getY()) && (*it)->isEdible())
			return (*it);
	}
	return nullptr;
}

bool StudentWorld::isBacteriumMovementBlockedAt(Actor* a, double x, double y) const
{
	// check if (x, y) coordinates are out of range of petri dish
	if (x > (VIEW_WIDTH / 2 + 128) || x < (VIEW_WIDTH / 2 - 128))
		return true;
	if (y < VIEW_RADIUS - sqrt(pow(VIEW_RADIUS, 2) - pow(x - VIEW_RADIUS, 2)) || y > VIEW_RADIUS + sqrt(pow(VIEW_RADIUS, 2) - pow(x - VIEW_RADIUS, 2)))
		return true;
	// for each actor, check if it's a Dirt pile, and if it is, is the passed-in actor a close enough to be considered "blocked" by the Dirt pile?
	for (auto it = m_actors.begin(); it != m_actors.end(); it++)
	{
		if ((*it)->blocksBacteriumMovement() && sqrt(pow(x - (*it)->getX(), 2) + pow(y - (*it)->getY(), 2)) <= SPRITE_RADIUS)
			return true;
	}
	return false;
}

bool StudentWorld::getAngleToNearbySocrates(Actor* a, int dist, int& angle) const
{
	// if actor a is not within dist units away from Socrates, return false
	if (a->getDistance(m_player->getX(), m_player->getY()) > dist)
		return false;
	// otherwise, set angle to the angle between actor a and Socrates in degrees, and return true
	angle = atan2((m_player->getY() - a->getY()), (m_player->getX() - a->getX())) * 180 / PI;
	return true;
}

bool StudentWorld::getAngleToNearestNearbyEdible(Actor* a, int dist, int& angle) const
{
	// check each actor. if it is a food item and it's within dist units of actor a, set angle to angle between them and return true
	for (auto it = m_actors.begin(); it != m_actors.end(); it++)
	{
		if ((*it)->isEdible() && (*it)->getDistance(a->getX(), a->getY()) <= dist)
		{
			angle = atan2((*it)->getY() - a->getY(), (*it)->getX() - a->getX()) * 180 / PI;
			return true;
		}
	}
	// otherwise, no food item close enough to Actor a was found, so return false
	return false;
}

void StudentWorld::getPositionOnCircumference(int angle, double& x, double& y) const
{
	// x equals radius times cosine theta
	x = VIEW_WIDTH / 2 + VIEW_RADIUS * cos(angle * PI / 180);
	// y equals radius times sin theta
	y = VIEW_HEIGHT / 2 + VIEW_RADIUS * sin(angle * PI / 180);
}

bool StudentWorld::damageOneActor(Actor* a, int damage)
{
	for (auto it = m_actors.begin(); it != m_actors.end(); it++)
	{
		// if actor is close enough and actor is bacterium, set bacterium to dead and increase game score by 100
		if ((*it)->getDistance(a->getX(), a->getY()) <= SPRITE_WIDTH && (*it)->takeDamage(damage))
		{
			increaseScore(100);
			if ((*it)->isDead())
			{
				// there's a 50% chance that the bacterium killed becomes food
				int rand = randInt(0, 1);
				if (rand == 0)
					addActor(new Food(this, (*it)->getX(), (*it)->getY()));
			}
			return true;
		}
	}
	return false;
}