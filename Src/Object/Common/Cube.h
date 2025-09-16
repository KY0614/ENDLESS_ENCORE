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
	Cube(void);
	Cube(const Cube& base);

	//デストラクタ
	~Cube(void);

	void MakeCube(VECTOR center, float size, COLOR_U8  col);
	void MakeBox(VECTOR center, float width, float height, float depth, COLOR_U8  col);

	//描画
	void Draw(void);

private:
	// 頂点配列（最大12枚の三角形 → 36頂点）
	VERTEX3D Vertex[36];

};

