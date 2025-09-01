#pragma once
#include <DxLib.h>

class Transform;

class Cube
{
public:

	//デバッグ時の簡易カプセル表示の色
	static constexpr int COLOR = 0xffffff;

	//コンストラクタ
	//カプセルをつけるオブジェクトを指定する
	Cube(const Transform& parent);
	Cube(const Cube& base, const Transform& parent);

	//デストラクタ
	~Cube(void);

	void MakeCube(VECTOR center, float size, COLOR_U8  col);
	void MakeBox(VECTOR center, float width, float height, float depth, COLOR_U8  col);

	//描画
	void Draw(void);

private:
	// 頂点配列（最大12枚の三角形 → 36頂点）
	VERTEX3D Vertex[36];

	//カプセルをくっつける相手
	const Transform& transformParent_;

	//親Transformからの相対位置(上側)
	VECTOR localPosTop_;

	//親Transformからの相対位置(下側)
	VECTOR localPosDown_;

	//半径
	float radius_;

};

