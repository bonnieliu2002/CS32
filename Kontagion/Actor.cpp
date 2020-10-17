#include "Actor.h"
#include "StudentWorld.h"
#include <cmath>
using namespace std;

/**********************************************************************************/
/*                        ACTOR CLASS IMPLEMENTATION                              */
/**********************************************************************************/
Actor::Actor(StudentWorld* w, int imageID, double x, double y, int dir, int depth)
	: GraphObject(imageID, x, y, dir, depth)
{
	m_world = w;
	m_alive = true;
}

bool Actor::isDead() const
{
	return !m_alive;
}

void Actor::setDead()
{
	m_alive = false;
}

StudentWorld* Actor::world() const
{
	return m_world;
}

bool Actor::takeDamage(int damage)
{
	return false;
}

bool Actor::blocksBacteriumMovement() const
{
	return false;
}

bool Actor::isEdible() const
{
	return false;
}

bool Actor::preventsLevelCompleting() const
{
	return false;
}

// check if actor is overlapping with a certain position with coordinates (x, y)
bool Actor::isOverlapping(int x, int y) const
{
	if (getDistance(x, y) <= SPRITE_WIDTH)
		return true;
	return false;
}

// distance formula
double Actor::getDistance(double x, double y) const
{
	return sqrt(pow(getX() - x, 2) + pow(getY() - y, 2));
}

/**********************************************************************************/
/*                        DIRT CLASS IMPLEMENTATION                               */
/**********************************************************************************/
Dirt::Dirt(StudentWorld* w, double x, double y)
	: Actor(w, IID_DIRT, x, y, 0, 1)
{
}

void Dirt::doSomething()
{
}

bool Dirt::blocksBacteriumMovement() const
{
	return true;
}

// if dirt gets hit by a projectile once, it'll die
// we're not going to count that as being able to take damage
// doing so allows us to distinguish between dirt and agents, particularly bacterium, which is useful in StudentWorld.cpp
bool Dirt::takeDamage(int damage)
{
	m_hp -= damage;
	if (m_hp <= 0)
		setDead();
	return false;
}

/**********************************************************************************/
/*                        FOOD CLASS IMPLEMENTATION                               */
/**********************************************************************************/
Food::Food(StudentWorld* w, double x, double y)
	: Actor(w, IID_FOOD, x, y, 90, 1)
{
}

void Food::doSomething()
{
}

bool Food::isEdible() const
{
	return true;
}

/**********************************************************************************/
/*                        PIT CLASS IMPLEMENTATION                                */
/**********************************************************************************/
Pit::Pit(StudentWorld* w, double x, double y)
	: Actor(w, IID_PIT, x, y, 0, 1)
{
	m_nRegularSalmonella = 5;
	m_nAggressiveSalmonella = 3;
	m_nEColi = 2;
}

void Pit::doSomething()
{
	// if there are no more bacterium coming out of the pit, Pit should die
	if (m_nRegularSalmonella == 0 && m_nAggressiveSalmonella == 0 && m_nEColi == 0)
	{
		setDead();
		return;
	}
	// There's a 1 in 50 chance per tick that the Pit releases a bacterium
	// There's a 50% chance that that bacterium is a regular salmonella,
	// a 30% chance that that bacterium is an aggressive salmonella, and
	// a 20% chance that that bacterium is an E. Coli.
	int random = randInt(1, 500);
	if (random >= 1 && random <= 5 && m_nRegularSalmonella != 0)
	{
		world()->addActor(new RegularSalmonella(world(), getX(), getY()));
		m_nRegularSalmonella--;
		world()->playSound(SOUND_BACTERIUM_BORN);
	}
	else if (random >= 6 && random <= 8 && m_nAggressiveSalmonella != 0)
	{
		world()->addActor(new AggressiveSalmonella(world(), getX(), getY()));
		m_nAggressiveSalmonella--;
		world()->playSound(SOUND_BACTERIUM_BORN);
	}
	else if (random >= 9 && random <= 10 && m_nEColi != 0)
	{
		world()->addActor(new EColi(world(), getX(), getY()));
		m_nEColi--;
		world()->playSound(SOUND_BACTERIUM_BORN);
	}
}

bool Pit::preventsLevelCompleting() const
{
	return true;
}

/**********************************************************************************/
/*                    PROJECTILE CLASS IMPLEMENTATION                             */
/**********************************************************************************/
Projectile::Projectile(StudentWorld* w, int imageID, double x, double y, int dir, int dist, int power)
	: Actor(w, imageID, x, y, dir, 1)
{
	m_travelDistance = dist;
	m_damagePower = power;
}

void Projectile::doSomething()
{
	if (isDead())
		return;
	// try to damage an actor; if it works, set itself (projectile) to dead
	if (world()->damageOneActor(this, m_damagePower))
		setDead();
	// otherwise, keep moving forward
	else
	{
		moveAngle(getDirection(), SPRITE_WIDTH);
		m_travelDistance -= SPRITE_WIDTH;
	}
	// if projectile has travelled its limit, set itself to dead
	if (m_travelDistance <= 0)
		setDead();
}

/**********************************************************************************/
/*                         SPRAY CLASS IMPLEMENTATION                             */
/**********************************************************************************/
Spray::Spray(StudentWorld* w, double x, double y, int dir)
	: Projectile(w, IID_SPRAY, x, y, dir, 112, 2)
{
}

void Spray::beImplemented() const
{
	return;
}

/**********************************************************************************/
/*                         FLAME CLASS IMPLEMENTATION                             */
/**********************************************************************************/
Flame::Flame(StudentWorld* w, double x, double y, int dir)
	: Projectile(w, IID_FLAME, x, y, dir, 32, 5)
{
}

void Flame::beImplemented() const
{
	return;
}

/**********************************************************************************/
/*                         GOODIE CLASS IMPLEMENTATION                            */
/**********************************************************************************/
Goodie::Goodie(StudentWorld* w, int imageID, double x, double y)
	: Actor(w, imageID, x, y, 0, 1)
{
	m_lifetime = max(randInt(0, 300 - 10 * w->getLevel() - 1), 50);
}

void Goodie::doSomething()
{
	if (isDead())
		return;
	// check if Socrates picked up the Goodie
	Socrates* socrates = world()->getOverlappingSocrates(this);
	if (socrates != nullptr)
	{
		// all goodies play the same sound, but fungus plays no sound at all
		playSound();
		// each Goodie subclass performs a special action, usually affecting the player
		performSpecialAction(socrates);
		setDead();
	}
	// after every tick, subtract lifetime by one
	m_lifetime--;
	// if lifetime reaches 0, too much time has passed, and set Goodie to dead
	if (m_lifetime <= 0)
		setDead();
}

// this is for most Goodies (except fungus)
void Goodie::playSound()
{
	world()->playSound(SOUND_GOT_GOODIE);
}

/**********************************************************************************/
/*                   RESTOREHEALTHGOODIE CLASS IMPLEMENTATION                     */
/**********************************************************************************/
RestoreHealthGoodie::RestoreHealthGoodie(StudentWorld* w, double x, double y)
	: Goodie(w, IID_RESTORE_HEALTH_GOODIE, x, y)
{
}

void RestoreHealthGoodie::performSpecialAction(Socrates* s)
{
	world()->increaseScore(250);
	s->restoreHealth();
}

/**********************************************************************************/
/*                    FLAMETHROWERGOODIE CLASS IMPLEMENTATION                     */
/**********************************************************************************/
FlamethrowerGoodie::FlamethrowerGoodie(StudentWorld* w, double x, double y)
	: Goodie(w, IID_FLAME_THROWER_GOODIE, x, y)
{
}

void FlamethrowerGoodie::performSpecialAction(Socrates* s)
{
	world()->increaseScore(300);
	s->addFlames();
}

/**********************************************************************************/
/*                      EXTRALIFEGOODIE CLASS IMPLEMENTATION                      */
/**********************************************************************************/
ExtraLifeGoodie::ExtraLifeGoodie(StudentWorld* w, double x, double y)
	: Goodie(w, IID_EXTRA_LIFE_GOODIE, x, y)
{
}

void ExtraLifeGoodie::performSpecialAction(Socrates* s)
{
	world()->increaseScore(500);
	world()->incLives();
}

/**********************************************************************************/
/*                         FUNGUS CLASS IMPLEMENTATION                            */
/**********************************************************************************/
Fungus::Fungus(StudentWorld* w, double x, double y)
	: Goodie(w, IID_FUNGUS, x, y)
{
}

void Fungus::performSpecialAction(Socrates* s)
{
	world()->increaseScore(-50);
	s->takeDamage(20);
}

// overrides Goodie::playSound(), which is virtual
void Fungus::playSound()
{
	// no sound
}

/**********************************************************************************/
/*                          AGENT CLASS IMPLEMENTATION                            */
/**********************************************************************************/
Agent::Agent(StudentWorld* w, int imageID, double x, double y, int dir, int hitPoints)
	: Actor(w, imageID, x, y, dir, 0)
{
	m_hp = m_maxHP = hitPoints;
}

bool Agent::takeDamage(int damage)
{
	m_hp -= damage;
	// Socrates, Salmonella, and E. Coli each has a different sound for getting hurt
	playHurt();
	if (m_hp <= 0)
	{
		setDead();
		// Socrates, Salmonella, and E. Coli each has a different sound for dying
		playDead();
	}
	// agents can take damage
	return true;
}

int Agent::numHitPoints() const
{
	return m_hp;
}

void Agent::restoreHealth()
{
	m_hp = m_maxHP;
}

/**********************************************************************************/
/*                         SOCRATES CLASS IMPLEMENTATION                          */
/**********************************************************************************/
Socrates::Socrates(StudentWorld* w, double x, double y)
	: Agent(w, IID_PLAYER, x, y, 0, 100)
{
	m_nFlames = 5;
	m_nSprays = 20;
}

void Socrates::doSomething()
{
	if (isDead())
		return;
	int ch;
	// if player pressed a key, then perform specified action for each key
	if (world()->getKey(ch))
	{
		switch (ch)
		{
			case KEY_PRESS_LEFT: // move Socrates counterclockwise
			case 'a':
			{
				setDirection(getDirection() + 5);
				moveAroundCircle(getDirection());
				break;
			}
			case KEY_PRESS_RIGHT: // move Socrates clockwise
			case 'd':
			{
				setDirection(getDirection() - 5);
				moveAroundCircle(getDirection());
				break;
			}
			case KEY_PRESS_SPACE: // shoot sprays
			{
				if (m_nSprays > 0)
				{
					double dx, dy;
					getPositionInThisDirection(getDirection(), SPRITE_WIDTH, dx, dy);
					world()->addActor(new Spray(world(), dx, dy, getDirection()));
					world()->playSound(SOUND_PLAYER_SPRAY);
					m_nSprays--;
				}
				break;
			}
			case KEY_PRESS_ENTER: // shoot flames
			{
				if (m_nFlames > 0)
				{
					m_nFlames--;
					world()->playSound(SOUND_PLAYER_FIRE);
					for (int i = 0, angle = 0; i < 16; i++, angle += 22)
					{
						double dx, dy;
						getPositionInThisDirection(angle, SPRITE_WIDTH, dx, dy);
						world()->addActor(new Flame(world(), dx, dy, angle));
					}
				}
				break;
			}
			default:
			{
				break;
			}
		}
	}
	// otherwise, no key was pressed, and if spray is not at maximum, replenish supply by adding one to spray count
	else
	{
		if (m_nSprays < 20)
		{
			m_nSprays++;
		}
	}
	return;
}

void Socrates::moveAroundCircle(int angle)
{
	int posAngle = 180 + angle;
	// make sure angle is between 0 and 360
	while (posAngle >= 360)
		posAngle -= 360;
	const double pi = 4 * atan(1);
	double x = VIEW_RADIUS * cos(posAngle * 1.0 / 360 * 2 * pi);
	double y = VIEW_RADIUS * sin(posAngle * 1.0 / 360 * 2 * pi);
	moveTo(VIEW_RADIUS + x, VIEW_RADIUS + y);
}

void Socrates::addFlames()
{
	m_nFlames += 5;
}

int Socrates::numFlames() const
{
	return m_nFlames;
}

int Socrates::numSprays() const
{
	return m_nSprays;
}

void Socrates::playHurt() const
{
	world()->playSound(SOUND_PLAYER_HURT);
}

void Socrates::playDead() const
{
	world()->playSound(SOUND_PLAYER_DIE);
}

/**********************************************************************************/
/*                         BACTERIUM CLASS IMPLEMENTATION                         */
/**********************************************************************************/
Bacterium::Bacterium(StudentWorld* w, int imageID, double x, double y, int hitPoints)
	: Agent(w, imageID, x, y, 90, hitPoints)
{
	m_foodEaten = 0;
}

bool Bacterium::preventsLevelCompleting() const
{
	return true;
}

void Bacterium::doSomething()
{
	if (isDead())
		return;
	// determine whether or not to chase Socrates
	bool chaseSocrates = aggressiveSalmonellaOnly();
	// check if Socrates overlaps with bacterium
	Socrates* socrates = world()->getOverlappingSocrates(this);
	// if so, make Socrates take the damage (severity of damage depends on the type of bacterium)
	if (socrates != nullptr)
		socrates->takeDamage(getDamage());
	// otherwise, check if food count is high enough to regenerate
	else if (foodEaten() == 3)
	{
		double newX = getX();
		if (newX < VIEW_WIDTH / 2)
			newX += SPRITE_WIDTH / 2;
		else if (newX > VIEW_WIDTH / 2)
			newX -= SPRITE_WIDTH / 2;
		double newY = getY();
		if (newY < VIEW_HEIGHT / 2)
			newY += SPRITE_WIDTH / 2;
		else if (newX > VIEW_HEIGHT / 2)
			newY -= SPRITE_WIDTH / 2;
		addBacterium(newX, newY);
		divide();
	}
	// otherwise, check if bacterium overlaps with food
	else
	{
		Actor* edible = world()->getOverlappingEdible(this);
		// if so, eat the food and set the food to dead
		if (edible != nullptr)
		{
			eatFood();
			edible->setDead();
		}
	}
	// if we chased Socrates already, return and don't do the next step
	if (chaseSocrates)
		return;
	// this details the more specific abilities of each type of bacterium
	doMore();
}

int Bacterium::foodEaten() const
{
	return m_foodEaten;
}

void Bacterium::eatFood()
{
	m_foodEaten++;
}

void Bacterium::divide()
{
	m_foodEaten = 0;
}

/**********************************************************************************/
/*                           ECOLI CLASS IMPLEMENTATION                           */
/**********************************************************************************/
EColi::EColi(StudentWorld* w, double x, double y)
	: Bacterium(w, IID_ECOLI, x, y, 5)
{
}

int EColi::getDamage() const
{
	return 4;
}

void EColi::addBacterium(double newX, double newY)
{
	world()->addActor(new EColi(world(), newX, newY));
}

void EColi::doMore()
{
	// aggressively hunt down Socrates
	int angle;
	bool socratesNearby = world()->getAngleToNearbySocrates(this, 256, angle);
	if (socratesNearby)
	{
		for (int i = 0; i < 10; i++)
		{
			double dx, dy;
			getPositionInThisDirection(angle, 2, dx, dy);
			// see if desired move is blocked by Dirt
			// if not, follow through on desired move
			if (!world()->isBacteriumMovementBlockedAt(this, dx, dy))
			{
				moveTo(dx, dy);
				return;
			}
			// otherwise, adjust angle by 10 degrees
			angle += 10;
		}
	}
}

void EColi::playHurt() const
{
	world()->playSound(SOUND_ECOLI_HURT);
}

void EColi::playDead() const
{
	world()->playSound(SOUND_ECOLI_DIE);
}

bool EColi::aggressiveSalmonellaOnly()
{
	return false;
}

/**********************************************************************************/
/*                        SALMONELLA CLASS IMPLEMENTATION                         */
/**********************************************************************************/
Salmonella::Salmonella(StudentWorld* w, double x, double y, int hitPoints)
	: Bacterium(w, IID_SALMONELLA, x, y, hitPoints)
{
	m_movementPlan = 10;
}

void Salmonella::doMore()
{
	// if m_movementPlan is still greater than 0, try to continue moving in that direction
	if (m_movementPlan > 0)
	{
		m_movementPlan--;
		Salmonella::attemptMove(getDirection());
		return;
	}
	// otherwise, it's time to pick a new direction!
	else
	{
		int angle;
		// if there is food within 128 pixels of the salmonella, try to move toward the food
		if (world()->getAngleToNearestNearbyEdible(this, 128, angle))
		{
			setDirection(angle);
			Salmonella::attemptMove(angle);
		}
		else
		{
			Salmonella::attemptMove(getDirection());
		}
		return;
	}
}

void Salmonella::attemptMove(int angle)
{
	// test if position 3 pixels away at that angle is valid (not blocked by bacteria and not blocked by walls of petri dish)
	double dx, dy;
	getPositionInThisDirection(angle, 3, dx, dy);
	// if movement is blocked, generate random new direction and set salmonella's direction to that and reset movement plan to 10
	if (world()->isBacteriumMovementBlockedAt(this, dx, dy))
	{
		int newAngle = randInt(0, 359);
		setDirection(newAngle);
		m_movementPlan = 10;
		return;
	}
	// otherwise, position is valid, so move there
	moveTo(dx,dy);
}

void Salmonella::playHurt() const
{
	world()->playSound(SOUND_SALMONELLA_HURT);
}

void Salmonella::playDead() const
{
	world()->playSound(SOUND_SALMONELLA_DIE);
}

/**********************************************************************************/
/*                 REGULARSALMONELLA CLASS IMPLEMENTATION                         */
/**********************************************************************************/
RegularSalmonella::RegularSalmonella(StudentWorld* w, double x, double y)
	: Salmonella(w, x, y, 4)
{
}

int RegularSalmonella::getDamage() const
{
	return 2;
}

void RegularSalmonella::addBacterium(double newX, double newY)
{
	world()->addActor(new RegularSalmonella(world(), newX, newY));
}

bool RegularSalmonella::aggressiveSalmonellaOnly()
{
	return false;
}

/**********************************************************************************/
/*                 AGGRESSIVESALMONELLA CLASS IMPLEMENTATION                      */
/**********************************************************************************/
AggressiveSalmonella::AggressiveSalmonella(StudentWorld* w, double x, double y)
	: Salmonella(w, x, y, 10)
{
}

int AggressiveSalmonella::getDamage() const
{
	return 2;
}

void AggressiveSalmonella::addBacterium(double newX, double newY)
{
	world()->addActor(new AggressiveSalmonella(world(), newX, newY));
}

bool AggressiveSalmonella::aggressiveSalmonellaOnly()
{
	int angle;
	// if Socrates is nearby, try to move toward player
	if (world()->getAngleToNearbySocrates(this, 72, angle))
	{
		attemptMove(angle);
		return true;
	}
	return false;
}

void AggressiveSalmonella::attemptMove(int angle)
{
	double dx, dy;
	getPositionInThisDirection(angle, 3, dx, dy);
	if (!(world()->isBacteriumMovementBlockedAt(this, dx, dy)))
	{
		setDirection(angle);
		moveTo(dx, dy);
	}
}