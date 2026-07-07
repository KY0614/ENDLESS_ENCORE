#pragma once
#include <vector>
#include <memory>

class UIBase;

class UIManager
{
public:
	//インスタンスの生成
	static void CreateInstance(void);

	//インスタンスの取得
	static UIManager& GetInstance(void);

	//コンストラクタ
	UIManager(void);
	//デストラクタ
	~UIManager(void);

	/// <summary>
	/// 初期化
	/// </summary>
	void Init(void);

	/// <summary>
	/// 更新
	/// </summary>
	void Update(void);

	/// <summary>
	/// 描画
	/// </summary>	
	void Draw(void);

	/// <summary>
	/// 解放
	/// </summary>
	void Release(void);

	/// <summary>
	/// 破棄
	/// </summary>
	void Destroy(void);

	/// <summary>
	/// UIの追加
	/// </summary>
	/// <param name="ui">追加するUI</param>
	void AddUI(std::shared_ptr<UIBase> ui) { uis_.emplace_back(ui); }

private:
	//シングルトン用インスタンス
	static UIManager* instance_;

	//追加されたUIのリスト
	std::vector<std::weak_ptr<UIBase>> uis_;
};