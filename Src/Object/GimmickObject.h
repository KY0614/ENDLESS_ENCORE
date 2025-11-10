#pragma once
#include "Common/Transform.h"
#include "ActorBase.h"

class GimmickObject
{
public:
	//コンストラクタ
	GimmickObject(void);
	//デストラクタ
	~GimmickObject(void);

	/// <summary>
	///	初期化
	/// </summary>
	void Init(void);

	/// <summary>
	///	更新処理
	/// </summary>
	void Update(void);

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw(void);

private:
	//モデル情報
	Transform transform_;
};

