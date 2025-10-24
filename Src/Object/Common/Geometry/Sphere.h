#pragma once
#include <DxLib.h>

class Transform;

class Sphere
{
public:
	//デバッグ時の簡易球体表示の色
	static constexpr int COLOR = 0x000000;

	//コンストラクタ
	//球体をつけるオブジェクトを指定する
	Sphere(const Transform& parent);
	Sphere(const VECTOR& parentPos, const Transform& parent);
	Sphere(const Sphere& base, const Transform& parent);

	//デストラクタ
	~Sphere(void);

	//描画
	void Draw(void);
	void Draw(int col);

	//親Transformからの相対位置を取得
	VECTOR GetLocalPos(void) const { return localPos_; }

	//親Transformからの相対位置をセット
	void SetLocalPos(const VECTOR& pos) { localPos_ = pos; }

	void SetPos(const VECTOR& pos) { localPos_ = pos; }

	//ワールド座標を取得
	VECTOR GetPos(void) const { return GetRotPos(localPos_); }

	//相対座標を回転させてワールド座標で取得する
	VECTOR GetRotPos(const VECTOR& localPos) const;

	//半径
	float GetRadius(void) const { return radius_; }
	void SetRadius(float radius) { radius_ = radius; }

	//高さ
	float GetHeight(void) const { return localPos_.y; }

	//球体の中心座標
	//VECTOR GetCenter(void) const;

private:

	//球体をくっつける相手
	const Transform& transformParent_;

	//親Transformからの相対位置
	VECTOR localPos_;

	VECTOR parentPos_;

	//半径
	float radius_;

};

