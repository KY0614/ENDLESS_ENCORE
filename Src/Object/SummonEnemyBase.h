#pragma once
#include "ActorBase.h"

class SummonEnemyBase : public ActorBase
{
public:
	//召喚する種類
	enum class TYPE
	{
		SHORT,
		MIDDLE,
		LONG,
		MAX,
	};

	SummonEnemyBase(void);
	virtual ~SummonEnemyBase(void);

	/// <summary>
	/// 初期化処理
	/// </summary>
	virtual void Init(void)override;

	/// <summary>
	/// 更新処理
	/// </summary>
	virtual void Update(void)override;

	/// <summary>
	/// 描画処理
	/// </summary>
	virtual void Draw(void)override;

	void SetType(const TYPE type) { type_ = type; }

	void SetPos(const VECTOR& pos) { transform_.pos = pos; }

protected:
	//種類
	TYPE type_;



	/// <summary>
	/// パラメータを設定する
	/// </summary>
	virtual void SetParam(void) = 0;

private:

};

