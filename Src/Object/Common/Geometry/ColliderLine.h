#pragma once
#include "ColliderBase.h"
#include <DxLib.h>

class Transform;

class ColliderLine : public ColliderBase
{
public:
	//コンストラクタ
	ColliderLine(TAG tag,
		const Transform* follow,
		const VECTOR& localStartPos,
		const VECTOR& localEndPos);

	//デストラクタ
	~ColliderLine(void);

	// ローカル座標での設定
	void SetLocalPosStart(const VECTOR& pos);

	void SetLocalPosEnd(const VECTOR& pos);

	// ローカル座標の取得
	const VECTOR& GetLocalPosStart(void) const;

	const VECTOR& GetLocalPosEnd(void) const;

	// ワールド座標の取得
	VECTOR GetPosStart(void) const;

	VECTOR GetPosEnd(void) const;

protected:

	// デバッグ用描画
	void DrawDebug(int color) override;

private:
	// デバッグ表示の球体半径
	static constexpr float RADIUS = 5.0f;
	// デバッグ表示の球体ポリゴン分割数
	static constexpr int DIV_NUM = 6;
	// 線分の開始座標(ローカル)
	VECTOR localPosStart_;
	// 線分の終了座標(ローカル)
	VECTOR localPosEnd_;
};

