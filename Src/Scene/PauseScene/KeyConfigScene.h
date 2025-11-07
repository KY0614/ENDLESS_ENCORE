#pragma once
#include "../SceneBase.h"

class KeyConfigScene : public SceneBase
{
public:

	KeyConfigScene(void);
	~KeyConfigScene(void);

	/// <summary>
	/// ƒf[ƒ^“Çˆ—
	/// </summary>
	void LoadData(void) override;

	void Init(void)override;
	void Update(void)override;
	void Draw(void)override;

private:
	int backImg_;
	int keyConfImg_;
	int menuBackImg_;
};