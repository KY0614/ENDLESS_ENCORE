#pragma once

class CollisionManager
{
public:

	//インスタンスの生成
	static void CreateInstance(void);

	//静的インスタンスの取得
	static CollisionManager& GetInstance(void);

	/// <summary>
	/// 初期化
	/// </summary>
	void Init(void);

	/// <summary>
	/// 解放（シーン切替時に一旦解放）
	/// </summary>
	void Release(void);

	/// <summary>
	/// リソースの完全破棄
	/// </summary>
	void Destroy(void);

private:
	//インスタンス用
	static CollisionManager* instance_;
};

