#pragma once
#include "../../Common/Vector2.h"
#include "UIBase.h"

class BarUI : public UIBase
{
public:
	struct BarUIInfo
	{
		Vector2 pos_;	//位置
		Vector2 size_;	//サイズ	
	};

	/// <summary>
	/// コンストラクタ
	/// </summary>
	/// <param name="uiSrc">ui画像のリソースID</param>
	/// <param name="uiBackSrc">バック画像のリソースID</param>
	BarUI(void);
	//デストラクタ
	~BarUI(void);

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

	void DrawParry(void);
	void DrawParryCD(void);

	/// <summary>
	/// UI画像のリソースID設定
	/// </summary>
	/// <param name="uiSrc">UI画像のリソースID</param>
	/// <param name="uiBackSrc">UI背景画像のリソースID</param>
	void SetBarUISrc(const ResourceManager::SRC uiSrc, const ResourceManager::SRC uiBackSrc);

	/// <summary>
	/// UI表示位置設定
	/// </summary>
	/// <param name="pos">座標(int)</param>
	void SetBarPos(const Vector2 pos);

	/// <summary>
	/// サイズ設定
	/// </summary>
	/// <param name="size">サイズ</param>
	void SetBarSize(const Vector2 size);

	void SetBarMaxWidth(const int width) { barMaxWidth_ = width; }

private:
	//バーUI情報
	BarUIInfo barUiInfo_;

	int barMaxWidth_;		//バーUIの最大幅

	//バーUI背景画像のリソースID
	int barUIFrameImg_;

	int parryBarImg_;
	int parryCDBarImg_;
};