#pragma once
#include "ShotBase.h"

class Sphere;

class EnemyShot :  public ShotBase
{
public:
	EnemyShot(void);
	~EnemyShot(void);

	void Init(void)override;
	void Update(void)override;
	void Draw(void)override;

private:
	//ƒJƒvƒZƒ‹
	std::unique_ptr<Sphere> sphere_;
};

