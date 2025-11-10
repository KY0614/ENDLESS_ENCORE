#pragma once
#include <memory>
#include "SceneBase.h"

class SelectScene : public SceneBase
{
public:

	// コンストラクタ
	SelectScene(void);

	// デストラクタ
	~SelectScene(void);

	/// <summary>
	/// データ読込処理
	/// </summary>
	void LoadData(void) override;

	/// <summary>
	/// 初期化処理
	/// </summary>
	void Init(void) override;

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update(void) override;

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw(void) override;

private:
	//選択肢文字列リスト
	std::vector<std::wstring> selectList_;
	//選択肢関数テーブル
	using SelectFunc_t = std::function<void()>;
	std::map<std::wstring, SelectFunc_t> selectFuncTable_;
	//現在選択しているもの
	int cursorIdx_;

	int currentIdx_;

	void DebugDraw(void);
};

