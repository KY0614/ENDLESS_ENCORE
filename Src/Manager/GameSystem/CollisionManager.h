#pragma once
#include <vector>
#include <memory>

class ColliderBase;

class CollisionManager
{
public:
	//衝突するコライダのペア
	struct CollisionPair
	{
		std::weak_ptr<ColliderBase> colA_;	//衝突するコライダのペア
		std::weak_ptr<ColliderBase> colB_;	//衝突するコライダのペア
		bool isHit_; //衝突しているか
	};

	//インスタンスの生成
	static void CreateInstance(void);

	//静的インスタンスの取得
	static CollisionManager& GetInstance(void);

	/// <summary>
	/// 当たり判定を行うコライダーのペアを追加
	/// </summary>
	/// <param name="col1"></param>
	/// <param name="col2"></param>
	void AddColisonPair(const std::weak_ptr<ColliderBase>& colA, const std::weak_ptr<ColliderBase>& colB);

	/// <summary>
	/// 初期化
	/// </summary>
	void Init(void);

	/// <summary>
	/// 更新
	/// </summary>
	void Update(void);

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

	//コライダのリスト
	std::vector<CollisionPair> collisionPairs_;

	//コンストラクタ
	CollisionManager(void);

	//コピーの禁止
	CollisionManager(const CollisionManager& _copy) = delete;
	CollisionManager& operator= (const CollisionManager& _copy) = delete;

	//デストラクタ
	~CollisionManager(void);

	/// <summary>
	/// 衝突判定を行う
	/// </summary>
	bool CheckCollision(void);
};

