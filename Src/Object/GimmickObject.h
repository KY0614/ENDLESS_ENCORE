#pragma once
#include "ActorBase.h"

class GimmickObject : public ActorBase
{
public:
	//コンストラクタ
	GimmickObject(void);
	//デストラクタ
	~GimmickObject(void);

	/// <summary>
	///	初期化
	/// </summary>
	void Init(void) override;

	/// <summary>
	///	更新処理
	/// </summary>
	void Update(void) override;

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw(void) override;

private:

};

