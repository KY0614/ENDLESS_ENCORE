#pragma once
class ResourceManager;

class SceneBase
{

public:

	// コンストラクタ
	SceneBase(void);

	// デストラクタ
	virtual ~SceneBase(void) = 0;

	// 初期化処理
	virtual void Init(void) = 0;

	// 更新処理
	virtual void Update(void) = 0;

	// 描画処理
	virtual void Draw(void) = 0;

	// ImGui更新処理
	virtual void UpdateImGui(void) = 0;
protected:

};
