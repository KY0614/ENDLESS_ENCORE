#pragma once

class Audience
{
public:
	//コンストラクタ
	Audience(void);
	//デストラクタ
	~Audience(void);

	void Init(void);

	void Update(void);

	void Draw(void);

private:
};

#pragma region 内容

// パリィ成功　→　拍手
// パリィ失敗　→　残念そうな声？

//成功タイミングによって拍手の量が変動？

#pragma endregion

