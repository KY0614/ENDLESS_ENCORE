#pragma once
#include "ShotBase.h"

class Sphere;

class EnemyBullet :  public ShotBase
{
public:

	enum class STATE
	{
		NONE,
		SHOT,
		DETSTROY,
	};

	EnemyBullet(Transform& parent);
	~EnemyBullet(void);

	void Init(void)override;
	void Update(void)override;
	void Draw(void)override;

private:
	//èÛë‘
	STATE state_;

	Transform parentTran_;

	//ãÖëÃ
	std::unique_ptr<Sphere> sphere_;
	
	void Move(void);
};

