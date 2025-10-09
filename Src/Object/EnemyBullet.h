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

	void Destroy(void);

	const bool& CheckBulletStateShot(void)const { return state_ == STATE::SHOT; }

	const STATE& GetState(void)const { return state_; }

	void ChangeState(const STATE state) { state_ = state; }

	void SetIsAlive(const bool isAlive) { isAlive_ = isAlive; }

	void SetLocalPos(const VECTOR pos);

	void ShotBullet(void);

	void ResetBullet(void);

private:
	//èÛë‘
	STATE state_;

	Transform& parentTran_;

	bool isAlive_;

	//ãÖëÃ
	std::unique_ptr<Sphere> sphere_;
	
	void Move(void);
};

