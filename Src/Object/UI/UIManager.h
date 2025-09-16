#pragma once
#include <vector>
#include <memory>
#include <map>

class GaugeUI;
class IconUI;
class PopUpUI;

class UIManager
{
public:
	//インスタンスの生成
	static void CreateInstance(void);

	//インスタンスの取得
	static UIManager& GetInstance(void);

	UIManager(void);
	~UIManager(void);

	void Init(void);
	void Update(void);
	void PopUpUIUpdate(void);
	void Draw(void);
	void Release(void);

	//リソースの破棄
	void Destroy(void);

	void AddGaugeUI(GaugeUI* ui);
	void AddIconUI(IconUI* ui);
	void AddPopUpUI(int score, const VECTOR& pos);

private:
	//シングルトン用インスタンス
	static UIManager* instance_;

	std::vector<GaugeUI*> gaugeUIs_;
	std::vector<IconUI*> iconUIs_;
	std::vector<std::unique_ptr<PopUpUI>> popUpUIs_;
};

