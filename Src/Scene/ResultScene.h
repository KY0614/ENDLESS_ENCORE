#pragma once
#include <memory>
#include "SceneBase.h"

class Score;

class ResultScene : public SceneBase
{
public:

	// コンストラクタ
	ResultScene(void);

	// デストラクタ
	~ResultScene(void);

	void Init(void) override;
	void Update(void) override;
	void Draw(void) override;

private:
};

