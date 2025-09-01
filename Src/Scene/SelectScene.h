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

	void Init(void) override;
	void Update(void) override;
	void Draw(void) override;

private:
	//メニューの文字列リスト
	std::vector<std::wstring> selectList_;

	int currentIdx_;

	void DebugDraw(void);
};

