#pragma once
#include <memory>
#include <vector>
#include <map>
#include <functional>
#include "../Order/Order.h"
#include "../ActorBase.h"

class StageObject;
class Furnitures;
class Player;
class AnimationController;
class ModelMaterial;
class ModelRenderer;
class IconUI;

class StageManager : public ActorBase
{

public:
	//オブジェクト関連

	//テーブル関連
	static constexpr int TABLE_COLUMN_NUM = 4;		//横側のテーブルの数（１列）
	static constexpr int TABLE_ROW_BACK_NUM = 4;	//手前側のテーブルの数
	static constexpr int TABLE_ROW_FRONT_NUM = 2;	//奥側のテーブルの数
	static constexpr int TABLE_CENTER_NUM = 4;	//テーブルの数
	static constexpr int MAX_TABLE_NUM = 16;	//テーブルの数

	static constexpr float TABLE_WIDTH = 92.5f;	//テーブルの横幅

	//座標

	static constexpr VECTOR TABLE_POS_BACK = { -150.0f, 0.0f, -270.0f };	//テーブルの座標(手前側）
	static constexpr VECTOR TABLE_POS_FRONT = { -115.0f, 0.0f, 180.0f };	//テーブルの座標（奥側）
	static constexpr VECTOR COLUMN_TABLE_LEFT_POS = { -250.0f, 0.0f, -192.0f };		//列（左）テーブルの座標
	static constexpr VECTOR COLUMN_TABLE_RIGHT_POS = { 320.0f, 0.0f, -192.0f };	//列(右）テーブルの座標
	static constexpr VECTOR CENTER_TABLE_POS = { -20.0f,0.0f,-100.0f };	//列(右）テーブルの座標
	static constexpr VECTOR COUNTER_POS = { 225.0f, 0.0f, 190.0f };			//カウンターの座標
	static constexpr VECTOR CASE_POS = { -57.0f, 0.0f, 190.0f };			//ショーケースの座標

	static constexpr VECTOR MACHINE_POS = { -128.0f, 76.0f, -175.0f };	//コーヒーマシンの座標
	static constexpr VECTOR CUPHOT_POS = { -45.0f, 76.0f, -175.0f };	//ホット用カップの座標
	static constexpr VECTOR CUPICE_POS = { 45.0f, 76.0f, -175.0f };		//アイス用カップの座標
	static constexpr VECTOR ICEDIS_POS = { 133.0f, 76.0f, -175.0f };	//アイスディスペンサーの座標
	static constexpr VECTOR DUSTBOX_POS = { 240.0f, 0.0f, -270.0f };	//ゴミ箱の座標

	//アニメーション種別
	enum class ANIM_TYPE
	{
		CREATE,
		IDLE,
		PAYING,
		
	};
	
	//コンストラクタ
	StageManager(Player& player);

	//デストラクタ
	~StageManager(void);

	void Init(void) override;
	void Update(void) override;
	void Draw(void) override;

	/// <summary>
	/// 現在の先頭のお客の注文を設定する
	/// </summary>
	/// <param name="order">先頭のお客の注文</param>
	void SetCurrentOrder(const Order::OrderData& order);

	/// <summary>
	/// 提供するアイテムを取得する
	/// </summary>
	/// <param name=""></param>
	/// <returns></returns>
	Order::OrderData GetServeItems(void)const { return servedItems_; }

	/// <summary>
	/// 提供したかどうかを取得する
	/// </summary>
	/// <param name=""></param>
	/// <returns>true:提供した　flase:未提供</returns>
	bool IsServed(void) const { return isServed_; }

	/// <summary>
	/// 提供データをリセットする
	/// </summary>
	/// <param name=""></param>
	void ResetServeData(void);

	int GetTableNum(void) const { return static_cast<int>(tables_.size()); }

	Transform GetCounterTran(void) const;
	Transform GetTableTran(int index) const;
	Transform GetShowCase(void) const;
	Transform GetDustBox(void) const;
	Transform GetFloorTran(void) const;

private:

	//アニメーション
	std::unique_ptr<AnimationController> animationController_;

	//スイーツケース用トランスフォーム
	Transform caseTran_;
	//ゴミ箱用トランスフォーム
	Transform dustBoxTran_;

	//プレイヤーの参照
	Player& player_;

	//ステージに配置するオブジェクト達
	std::vector<std::unique_ptr<StageObject>> objects_;

	//ステージに配置する机
	std::vector<std::unique_ptr<StageObject>> tables_;

	//カウンター用
	std::unique_ptr<StageObject> counter_;
	std::unique_ptr<StageObject> case_;

	//家具用
	std::unique_ptr<Furnitures> furnitures_;

	//注文された分を用意したかどうか
	bool isServed_;

	//提供済み商品
	Order::OrderData servedItems_;

	//現在のお客の注文内容
	Order::OrderData currentOrder_;

	//各注文が提供されたかどうかのフラグ
	std::vector<bool> isServedItems_; 

	void Init3DModel(void);

	void InitAnimation(void);

	/// <summary>
	/// 提供するアイテムを設定する
	/// </summary>
	/// <param name="obj"></param>
	void SurveItem(StageObject& obj);

	/// <summary>
	/// 提供したアイテムを削除する
	/// </summary>
	/// <param name=""></param>
	void DeleteSurvedItem(void);

	/// <summary>
	/// 持ち運び可能なオブジェクトのインタラクト処理
	/// </summary>
	/// <param name=""></param>
	void CarryableObjInteract(void);

	/// <summary>
	/// マシンのインタラクト処理
	/// </summary>
	/// <param name="">コーヒーマシンとアイスディスペンサーのインタラクト処理</param>
	void MachineInteract(void);

	/// <summary>
	/// 蓋のラックのインタラクト処理
	/// </summary>
	/// <param name="">インタラクト中はプレイヤーは操作できない</param>
	void LidRackInteract(void);

	/// <summary>
	/// コーヒーを生成する処理
	/// </summary>
	/// <param name="">マシンに設置されているカップに対して行う処理/param>
	void ProduceCoffee(int index);

	/// <summary>
	/// コーヒーを作る処理
	/// </summary>
	/// <param name="index">配列番号（カップをコーヒーに上書きするので）</param>
	/// <param name="pos">生成する座標</param>
	/// <param name="objName">作るコーヒーのパラメータ名（ホットかアイス)</param>
	void MakeCoffee(int index,StageObject& obj,std::string objName);

	/// <summary>
	/// カップにアイスディスペンサーで氷を入れる処理
	/// </summary>
	/// <param name="index">指定するアイスディスペンサーの要素番号</param>
	void DispenseIce2Cup(int index);

	/// <summary>
	/// コーヒーに蓋をつける
	/// </summary>
	/// <param name=""></param>
	void LidFollowCup(void);

	/// <summary>
	/// ゴミ箱のインタラクト処理
	/// </summary>
	/// <param name=""></param>
	void DustBoxInteract(void);

	/// <summary>
	/// オーダー数に応じた数のアイテムを提供しているかどうか
	/// </summary>
	/// <param name=""></param>
	/// <returns></returns>
	bool IsOrderCompleted(void);
};
