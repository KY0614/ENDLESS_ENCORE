#pragma once
#include "Transitor.h"
class FadeTransitor : public Transitor
{
public:
	// インスタンスの生成
	static void CreateInstance(void);

	// インスタンスの取得
	static FadeTransitor& GetInstance(void);

	FadeTransitor(int interval = 60);

	void Update(void) override;
	void Draw(void) override;


	// リソースの破棄
	void Destroy(void);

private:
	// 静的インスタンス
	static FadeTransitor* instance_;

};

