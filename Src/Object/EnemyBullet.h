#pragma once
#include "ShotBase.h"

class Sphere;

class EnemyBullet :  public ShotBase
{
public:
	EnemyBullet(int num);
	~EnemyBullet(void);

	void Init(void)override;
	void Update(void)override;
	void Draw(void)override;

private:
	int bulletNum_;

	//‹…‘Ì
	std::unique_ptr<Sphere> sphere_;
};

