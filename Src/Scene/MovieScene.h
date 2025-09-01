#pragma once
#include <memory>
#include "SceneBase.h"

class MovieScene : public SceneBase
{
public:

	// コンストラクタ
	MovieScene(void);

	// デストラクタ
	~MovieScene(void);

	void Init(void) override;
	void Update(void) override;
	void Draw(void) override;

private:
};

