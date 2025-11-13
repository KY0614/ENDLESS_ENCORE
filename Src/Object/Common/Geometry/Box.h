#pragma once
#include <DxLib.h>

class Transform;

class Box
{
public:
	//デバッグ時の簡易立方体表示の色
	static constexpr int COLOR = 0x00ff00; // 例として緑を設定

	// コンストラクタ
	// 立方体をつけるオブジェクトを指定する
	Box(const Transform& parent);
	Box(const Box& base, const Transform& parent);

	// デストラクタ
	~Box(void);

	// 描画 (ワイヤーフレーム)
	void Draw(void);
	// 描画 (色と塗りつぶしを指定)
	void Draw(int col, bool fill = false);

	// 親Transformからの相対中心位置を取得
	VECTOR GetLocalCenter(void) const { return localCenter_; }

	// 親Transformからの相対中心位置をセット
	void SetLocalCenter(const VECTOR& pos) { localCenter_ = pos; }

	// ワールド座標での中心位置を取得
	VECTOR GetCenter(void) const;

	// 相対座標を回転させてワールド座標で取得する
	VECTOR GetRotPos(const VECTOR& localPos) const;

	// サイズ（各軸方向の半分の長さ）を取得・設定
	VECTOR GetSize(void) const { return size_; }
	void SetSize(const VECTOR& size) { size_ = size; }

	// Bounding Box (AABB)の最小・最大座標をワールド座標で取得
	// (回転を考慮しない簡易的なもの。より正確なOBBのMin/Maxは複雑になるためここでは割愛)
	// 回転を考慮した場合は、8頂点の計算が必要になります。

private:
	// 立方体をくっつける相手
	const Transform& transformParent_;

	// 親Transformからの相対中心位置
	VECTOR localCenter_;

	VECTOR parentPos_;

	// サイズ（各軸方向の半分の長さ、x, y, z）
	VECTOR size_;
};