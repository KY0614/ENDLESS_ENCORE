#pragma once
#include<set>

class Collider
{

public :

	//衝突種別
	enum class TYPE
	{
		STAGE,
		PLAYER,
		ENEMY,
	};

	//コンストラクタ
	Collider(TYPE type, int modelId);

	//デストラクタ
	~Collider(void);

	//衝突種別
	TYPE type_;

	//モデルのハンドルID
	int modelId_;

};
