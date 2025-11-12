#pragma once
#include<DxLib.h>
#include"../../../Common/Quaternion.h"

class Model;
class Cube;
class Sphere;
class Capsule;
class Line;

class GeometryBase
{
public:

	//通常色
	static constexpr int NORMAL_COLOR = 0x000000;

	//デストラクタ
	virtual~GeometryBase(void) = 0;

	//描画
	virtual void Draw(void) = 0;

	//各種当たり判定
	virtual const bool IsHit(GeometryBase& _geometry) = 0;
	virtual const bool IsHit(Model& _model) = 0;
	virtual const bool IsHit(Cube& _cube) = 0;
	virtual const bool IsHit(Sphere& _sphere) = 0;
	virtual const bool IsHit(Capsule& _capsule) = 0;
	virtual const bool IsHit(Line& _line) = 0;

	//ヒット後の処理
	virtual void HitAfter(void);

	//親情報を返す
	inline const VECTOR& GetColParentPos(void)const { return parentPos_; }
	inline const Quaternion& GetColParentRot(void)const { return parentQuaRot_; }

protected:

	/// <summary>
	/// コンストラクタ(外部で作る必要のない基底なのでprotected)
	/// </summary>
	/// <param name="_pos">追従する親の座標</param>
	/// <param name="_rot">追従する親の回転</param>
	GeometryBase(const VECTOR& pos, const Quaternion& rot);

	// 相対座標を回転させてワールド座標で取得する
	const VECTOR GetRotPos(const VECTOR& localPos) const;

	const VECTOR& parentPos_;			//親の座標
	const Quaternion& parentQuaRot_;	//親の回転
};

