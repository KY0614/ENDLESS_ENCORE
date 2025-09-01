#pragma once
#include "../SceneBase.h"

class KeyConfigScene : public SceneBase
{
public:

	KeyConfigScene(void);
	~KeyConfigScene(void);

	void Init(void)override;
	void Update(void)override;
	void Draw(void)override;

private:
	int backImg_;
	int keyConfImg_;
	int menuBackImg_;
};