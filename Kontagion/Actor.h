#ifndef ACTOR_INCLUDED
#define ACTOR_INCLUDED

#include "GraphObject.h"

class StudentWorld;
class Socrates;

class Actor : public GraphObject
{
public:
	Actor(StudentWorld* w, int imageID, double x, double y, int dir, int depth);

	// Action to perform for each tick.
	virtual void doSomething() = 0;

	// Is this actor dead?
	bool isDead() const;

	// Mark this actor as dead.
	void setDead();

	// Get this actor's world
	StudentWorld* world() const;

	// If this actor can suffer damage, make it do so and return true;
	// otherwise, return false.
	virtual bool takeDamage(int damage);

	// Does this object block bacterium movement?
	virtual bool blocksBacteriumMovement() const;

	// Is this object edible?
	virtual bool isEdible() const;

	// Does the existence of this object prevent a level from being completed?
	virtual bool preventsLevelCompleting() const;

	// Does this actor overlap with a certain position?
	bool isOverlapping(int x, int y) const;

	// How far is this actor from another position?
	double getDistance(double x, double y) const;

private:
	StudentWorld* m_world;
	bool m_alive;
};

//////////////////////////////////////////////////////////////////////////////////////

class Dirt : public Actor
{
public:
	Dirt(StudentWorld* w, double x, double y);
	virtual void doSomething();
	virtual bool takeDamage(int damage);
	virtual bool blocksBacteriumMovement() const;
private:
	int m_hp = 1;
};

//////////////////////////////////////////////////////////////////////////////////////

class Food : public Actor
{
public:
	Food(StudentWorld* w, double x, double y);
	virtual void doSomething();
	virtual bool isEdible() const;
};

//////////////////////////////////////////////////////////////////////////////////////

class Pit : public Actor
{
public:
	Pit(StudentWorld* w, double x, double y);
	virtual void doSomething();
	virtual bool preventsLevelCompleting() const;
private:
	int m_nEColi;
	int m_nRegularSalmonella;
	int m_nAggressiveSalmonella;
};

//////////////////////////////////////////////////////////////////////////////////////

class Projectile : public Actor
{
public:
	Projectile(StudentWorld* w, int imageID, double x, double y, int dir, int dist, int power);
	void doSomething();
	virtual void beImplemented() const = 0;
private:
	int m_travelDistance;
	int m_damagePower;
};

//////////////////////////////////////////////////////////////////////////////////////

class Spray : public Projectile
{
public:
	Spray(StudentWorld* w, double x, double y, int dir);
	virtual void beImplemented() const;
};

//////////////////////////////////////////////////////////////////////////////////////

class Flame : public Projectile
{
public:
	Flame(StudentWorld* w, double x, double y, int dir);
	virtual void beImplemented() const;
};

//////////////////////////////////////////////////////////////////////////////////////

class Goodie : public Actor
{
public:
	Goodie(StudentWorld* w, int imageID, double x, double y);
	void doSomething();
	virtual void performSpecialAction(Socrates* socrates) = 0;
	virtual void playSound();
private:
	int m_lifetime;
};

//////////////////////////////////////////////////////////////////////////////////////

class RestoreHealthGoodie : public Goodie
{
public:
	RestoreHealthGoodie(StudentWorld* w, double x, double y);
	virtual void performSpecialAction(Socrates* socrates);
};

//////////////////////////////////////////////////////////////////////////////////////

class FlamethrowerGoodie : public Goodie
{
public:
	FlamethrowerGoodie(StudentWorld* w, double x, double y);
	virtual void performSpecialAction(Socrates* socrates);
};

//////////////////////////////////////////////////////////////////////////////////////

class ExtraLifeGoodie : public Goodie
{
public:
	ExtraLifeGoodie(StudentWorld* w, double x, double y);
	virtual void performSpecialAction(Socrates* socrates);
};

//////////////////////////////////////////////////////////////////////////////////////

class Fungus : public Goodie
{
public:
	Fungus(StudentWorld* w, double x, double y);
	virtual void performSpecialAction(Socrates* socrates);
	virtual void playSound();
};

//////////////////////////////////////////////////////////////////////////////////////

class Agent : public Actor
{
public:
	Agent(StudentWorld* w, int imageID, double x, double y, int dir, int hitPoints);
	virtual bool takeDamage(int damage);

	// How many hit points does this agent currently have?
	int numHitPoints() const;

	// Restore this agent's hit points to their original level
	void restoreHealth();

	// What sound should play when this agent is damaged but does not die?
	virtual void playHurt() const = 0;

	// What sound should play when this agent is damaged and dies?
	virtual void playDead() const = 0;
private:
	int m_hp;
	int m_maxHP;
};

//////////////////////////////////////////////////////////////////////////////////////

class Socrates : public Agent
{
public:
	Socrates(StudentWorld* w, double x, double y);
	virtual void doSomething();

	// Increase the number of flamethrower charges the object has.
	void addFlames();

	// How many flamethrower charges does the object have?
	int numFlames() const;

	// How many spray charges does the object have?
	int numSprays() const;

	virtual void playHurt() const;
	virtual void playDead() const;
private:
	int m_nFlames;
	int m_nSprays;
	void moveAroundCircle(int angle);
};

//////////////////////////////////////////////////////////////////////////////////////

class Bacterium : public Agent
{
public:
	Bacterium(StudentWorld* w, int imageID, double x, double y, int hitPoints);
	virtual bool preventsLevelCompleting() const;
	void doSomething();
	int foodEaten() const;
	void eatFood();
	void divide();
	virtual int getDamage() const = 0;
	virtual void addBacterium(double newX, double newY) = 0;
	virtual void doMore() = 0;
	virtual bool aggressiveSalmonellaOnly() = 0;
private:
	int m_foodEaten;
};

//////////////////////////////////////////////////////////////////////////////////////

class EColi : public Bacterium
{
public:
	EColi(StudentWorld* w, double x, double y);
	virtual int getDamage() const;
	virtual void addBacterium(double newX, double newY);
	virtual void doMore();
	virtual void playHurt() const;
	virtual void playDead() const;
	virtual bool aggressiveSalmonellaOnly();
};

//////////////////////////////////////////////////////////////////////////////////////

class Salmonella : public Bacterium
{
public:
	Salmonella(StudentWorld* w, double x, double y, int hitPoints);
	virtual int getDamage() const = 0;
	virtual void addBacterium(double newX, double newY) = 0;
	virtual void doMore();
	virtual void playHurt() const;
	virtual void playDead() const;
	virtual void attemptMove(int angle);
private:
	int m_movementPlan;
};

//////////////////////////////////////////////////////////////////////////////////////

class RegularSalmonella : public Salmonella
{
public:
	RegularSalmonella(StudentWorld* w, double x, double y);
	virtual int getDamage() const;
	virtual void addBacterium(double newX, double newY);
	virtual bool aggressiveSalmonellaOnly();
};

//////////////////////////////////////////////////////////////////////////////////////

class AggressiveSalmonella : public Salmonella
{
public:
	AggressiveSalmonella(StudentWorld* w, double x, double y);
	virtual int getDamage() const;
	virtual void addBacterium(double newX, double newY);
	virtual bool aggressiveSalmonellaOnly();
	virtual void attemptMove(int angle);
};

#endif // ACTOR_INCLUDED