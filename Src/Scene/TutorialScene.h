#pragma once
#include <memory>
#include "SceneBase.h"

class StageManager;
class Player;
class OrderCustomerManager;
class Timer;

class TutorialScene : public SceneBase
{
public:

	//コンストラクタ
	TutorialScene(void);

	//デストラクタ
	~TutorialScene(void);


	/// <summary>
	/// データ読込処理
	/// </summary>
	void LoadData(void) override;


	void Init(void) override;
	void Update(void) override;
	void Draw(void) override;

private:
	//関数ポインタ
	using UpdateFunc_t = void(TutorialScene::*)();
	using DrawFunc_t = void(TutorialScene::*)();

	UpdateFunc_t update_;
	DrawFunc_t draw_;

};

