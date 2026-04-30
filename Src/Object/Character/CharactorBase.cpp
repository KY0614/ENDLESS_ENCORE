#include "../Libs/ImGui/imgui.h"
#include "../Utility/CommonUtility.h"
#include "../Manager/Generic/SceneManager.h"
#include "../Manager/Generic/ResourceManager.h"
#include "../Common/AnimationController.h"
#include "../Common/Collider/ColliderLine.h"
#include "../Common/Collider/ColliderCapsule.h"
#include "../Common/Collider/ColliderModel.h"
#include "CharactorBase.h"

CharactorBase::CharactorBase(void):
	ActorBase()
{
	isJump_ = false;
	movedPos_ = CommonUtility::VECTOR_ZERO;
	moveDir_ = CommonUtility::VECTOR_ZERO;
	movePow_ = CommonUtility::VECTOR_ZERO;
	prevPos_  = CommonUtility::VECTOR_ZERO;
	jumpPow_ = CommonUtility::VECTOR_ZERO;
	//丸影画像
	imgShadow_ = ResourceManager::GetInstance().Load(
		ResourceManager::SRC::CHARACTOR_SHADOW).handleId_;
}

CharactorBase::~CharactorBase(void)
{
}

void CharactorBase::Init(void)
{
}

void CharactorBase::Update(void)
{
	//移動前座標を更新
	prevPos_ = transform_.pos;

	//各キャラクターごとの更新処理
	UpdateProcess();

	//移動方向に応じた遅延回転
	//DelayRotate();

	//重力による移動量
	CalcGravityPower();

	//衝突判定前準備
	CollisionReserve();

	//衝突判定
	Collision();

	//モデル制御更新
	transform_.Update();

	//アニメーション再生
	animationController_->Update();

	//各キャラクターごとの更新後処理
	UpdateProcessPost();

	UpdateImGui();
}

void CharactorBase::Draw(void)
{
}

void CharactorBase::CalcGravityPower(void)
{
	//重力方向
	VECTOR dirGravity = CommonUtility::DIR_D;
	//重力の強さ
	float gravityPow = GetGravityPower() * SceneManager::GetInstance().GetDeltaTime();
	//重力
	VECTOR gravity = VScale(dirGravity, gravityPow);
	jumpPow_ = VAdd(jumpPow_, gravity);
	if (jumpPow_.y < MAX_FALL_SPEED)
	{
		jumpPow_.y = MAX_FALL_SPEED;
	}
}

void CharactorBase::DelayRotate(void)
{
	// 移動方向から回転に変換する
	Quaternion goalRot = Quaternion::LookRotation(moveDir_);

	// 回転の補間
	transform_.quaRot =
		Quaternion::Slerp(transform_.quaRot, goalRot, 0.2f);
}

void CharactorBase::Collision(void)
{
	//現在座標を起点に移動後座標を決める
	movedPos_ = VAdd(transform_.pos, movePow_);
	//移動
	transform_.pos = movedPos_;
	// カプセルによる衝突判定
	CollisionCapsule();
	// ジャンプ量を加算
	transform_.pos = VAdd(transform_.pos, jumpPow_);
	// 衝突(重力)
	CollisionGravity();	
}

void CharactorBase::CollisionCapsule(void)
{
	//カプセルを移動させる
	Transform trans = Transform(transform_);
	trans.pos = movedPos_;
	trans.Update();
	// カプセルコライダ
	int capsuleType = static_cast<int>(COLLIDER_TYPE::CAPSULE);
	// カプセルコライダが無ければ処理を抜ける
	if (ownColliders_.count(capsuleType) == 0) return;
	// カプセルコライダ情報
	ColliderCapsule* colliderCapsule =
		dynamic_cast<ColliderCapsule*>(ownColliders_.at(capsuleType).get());
	if (colliderCapsule == nullptr) return;
	// 登録されている衝突物を全てチェック
	for (const std::weak_ptr<ColliderBase> hitCol : hitColliders_)
	{
		// モデル以外は処理を飛ばす
		if (hitCol.lock()->GetShape() != ColliderBase::SHAPE::MODEL) continue;
		// 派生クラスへキャスト
		const ColliderModel* colliderModel =
			dynamic_cast<const ColliderModel*>(hitCol.lock().get());
		if (colliderModel == nullptr) continue;
		MV1_COLL_RESULT_POLY_DIM hits = MV1CollCheck_Capsule(
			colliderModel->GetFollow()->modelId, -1,
			colliderCapsule->GetPosTop(), 
			colliderCapsule->GetPosDown(),
			colliderCapsule->GetRadius());
		// 衝突した複数のポリゴンと衝突回避するまで、
		// プレイヤーの位置を移動させる
		for (int i = 0; i < hits.HitNum; i++)
		{
			auto hit = hits.Dim[i];
			// 地面と異なり、衝突回避位置が不明なため、何度か移動させる
			// この時、移動させる方向は、移動前座標に向いた方向であったり、
			// 衝突したポリゴンの法線方向だったりする
			for (int tryCnt = 0; tryCnt < CNT_TRY_COLLISION; tryCnt++)
			{
				// 再度、モデル全体と衝突検出するには、効率が悪過ぎるので、
				// 最初の衝突判定で検出した衝突ポリゴン1枚と衝突判定を取る
				int pHit = HitCheck_Capsule_Triangle(
					colliderCapsule->GetPosTop(), colliderCapsule->GetPosDown(),
					colliderCapsule->GetRadius(),
					hit.Position[0], hit.Position[1], hit.Position[2]);
				if (pHit)
				{
					// 法線の方向にちょっとだけ移動させる
					transform_.pos =
						VAdd(transform_.pos,
							VScale(hit.Normal, COLLISION_BACK_DIS));

					////法線の方向にちょっとだけ移動させる
					//const float adjustDist = 2.0f;
					//movedPos_ = VAdd(movedPos_, VScale(hit.Normal, adjustDist));
					////カプセルも一緒に移動させる
					//trans.pos = movedPos_;
					//trans.Update();
					continue;
				}
				break;
			}
		}
		// 検出した地面ポリゴン情報の後始末
		MV1CollResultPolyDimTerminate(hits);
	}
}

void CharactorBase::CollisionGravity(void)
{
	// 線分コライダ
	int lineType = static_cast<int>(COLLIDER_TYPE::LINE);
	// 線分コライダが無ければ処理を抜ける
	if (ownColliders_.count(lineType) == 0) return;
	// 線分コライダ情報
	ColliderLine* colliderLine_ = dynamic_cast<ColliderLine*>(
		ownColliders_.at(lineType).get());
		if (colliderLine_ == nullptr) return;
	dynamic_cast<ColliderLine*>(ownColliders_.at(lineType).get());
	// 線分の始点と終点を取得
	VECTOR s = colliderLine_->GetPosStart();
	VECTOR e = colliderLine_->GetPosEnd();
	// 登録されている衝突物を全てチェック
	for (const auto& hitCol : hitColliders_)
	{
		//ステージ以外は処理を飛ばす
		if (hitCol.lock()->GetTag() != ColliderBase::TAG::STAGE) continue;
		//派生クラスへキャスト
		const ColliderModel* colliderModel =
			dynamic_cast<const ColliderModel*>(hitCol.lock().get());
		if (colliderModel == nullptr) continue;
		//ステージモデル(地面)との衝突
		MV1_COLL_RESULT_POLY hit = MV1CollCheck_Line(
			colliderModel->GetFollow()->modelId, -1, s, e);
		if (hit.HitFlag > 0)
		{
			//衝突地点から、少し上に移動
			transform_.pos =
				VAdd(hit.HitPosition, VScale(
					CommonUtility::DIR_U, 2.0f));
			const float GROUND_OFFSET = 2.0f;	//地面からのオフセット
			//ジャンプリセット
			jumpPow_ = CommonUtility::VECTOR_ZERO;
			if(isJump_)
			{
				// ジャンプ中であれば、ジャンプアニメーションを途中から再生する
				JumpAnimationPlay();
			}
			isJump_ = false;
		}
	}
}

void CharactorBase::DrawShadow(void)
{
	int i = 0;
	MV1_COLL_RESULT_POLY_DIM HitResDim;
	MV1_COLL_RESULT_POLY* HitRes;
	VERTEX3D Vertex[3];
	VECTOR SlideVec;
	int ModelHandle;

	//ライティングを無効にする
	SetUseLighting(FALSE);

	//Ｚバッファを有効にする
	SetUseZBuffer3D(TRUE);

	//テクスチャアドレスモードを CLAMP にする( テクスチャの端より先は端のドットが延々続く )
	SetTextureAddressMode(DX_TEXADDRESS_CLAMP);

	//影を落とすモデルの数だけ繰り返し
	for (const auto& c : colliders_)
	{
		//チェックするモデルは、jが0の時はステージモデル、1以上の場合はコリジョンモデル

		ModelHandle = c.lock()->modelId_;

		//影の高さとサイズを設定
		const float PLAYER_SHADOW_HEIGHT = 700.0f;
		const float PLAYER_SHADOW_SIZE = 50.0f;

		const int maxAlpha = 128;	//影の最大不透明度

		//プレイヤーの直下に存在する地面のポリゴンを取得
		HitResDim = MV1CollCheck_Capsule(ModelHandle, -1, transform_.pos, VAdd(transform_.pos, VGet(0.0f, -PLAYER_SHADOW_HEIGHT, 0.0f)), PLAYER_SHADOW_SIZE);

		//頂点データで変化が無い部分をセット
		Vertex[0].dif = GetColorU8(255, 255, 255, 255);
		Vertex[0].spc = GetColorU8(0, 0, 0, 0);
		Vertex[0].su = 0.0f;
		Vertex[0].sv = 0.0f;
		Vertex[1] = Vertex[0];
		Vertex[2] = Vertex[0];

		//球の直下に存在するポリゴンの数だけ繰り返し
		HitRes = HitResDim.Dim;
		for (i = 0; i < HitResDim.HitNum; i++, HitRes++)
		{
			//ポリゴンの座標は地面ポリゴンの座標
			Vertex[0].pos = HitRes->Position[0];
			Vertex[1].pos = HitRes->Position[1];
			Vertex[2].pos = HitRes->Position[2];

			//ちょっと持ち上げて重ならないようにする
			SlideVec = VScale(HitRes->Normal, 0.5f);
			Vertex[0].pos = VAdd(Vertex[0].pos, SlideVec);
			Vertex[1].pos = VAdd(Vertex[1].pos, SlideVec);
			Vertex[2].pos = VAdd(Vertex[2].pos, SlideVec);

			//ポリゴンの不透明度を設定する
			Vertex[0].dif.a = 0;
			Vertex[1].dif.a = 0;
			Vertex[2].dif.a = 0;
			if (HitRes->Position[0].y > transform_.pos.y - PLAYER_SHADOW_HEIGHT)
				Vertex[0].dif.a = static_cast<BYTE>(
					maxAlpha * (1.0f - fabs(HitRes->Position[0].y - transform_.pos.y) / PLAYER_SHADOW_HEIGHT));

			if (HitRes->Position[1].y > transform_.pos.y - PLAYER_SHADOW_HEIGHT)
				Vertex[1].dif.a = static_cast<BYTE>(
					maxAlpha * (1.0f - fabs(HitRes->Position[1].y - transform_.pos.y) / PLAYER_SHADOW_HEIGHT));

			if (HitRes->Position[2].y > transform_.pos.y - PLAYER_SHADOW_HEIGHT)
				Vertex[2].dif.a = static_cast<BYTE>(
					maxAlpha * (1.0f - fabs(HitRes->Position[2].y - transform_.pos.y) / PLAYER_SHADOW_HEIGHT));

			//ＵＶ値は地面ポリゴンとプレイヤーの相対座標から割り出す
			const float uvOffset = 0.5f;	//UV値の中心をプレイヤーの位置にするためのオフセット
			Vertex[0].u = (HitRes->Position[0].x - transform_.pos.x) / (PLAYER_SHADOW_SIZE * 2.0f) + uvOffset;
			Vertex[0].v = (HitRes->Position[0].z - transform_.pos.z) / (PLAYER_SHADOW_SIZE * 2.0f) + uvOffset;
			Vertex[1].u = (HitRes->Position[1].x - transform_.pos.x) / (PLAYER_SHADOW_SIZE * 2.0f) + uvOffset;
			Vertex[1].v = (HitRes->Position[1].z - transform_.pos.z) / (PLAYER_SHADOW_SIZE * 2.0f) + uvOffset;
			Vertex[2].u = (HitRes->Position[2].x - transform_.pos.x) / (PLAYER_SHADOW_SIZE * 2.0f) + uvOffset;
			Vertex[2].v = (HitRes->Position[2].z - transform_.pos.z) / (PLAYER_SHADOW_SIZE * 2.0f) + uvOffset;

			//影ポリゴンを描画
			DrawPolygon3D(Vertex, 1, imgShadow_, TRUE);
		}

		//検出した地面ポリゴン情報の後始末
		MV1CollResultPolyDimTerminate(HitResDim);
	}

	//ライティングを有効にする
	SetUseLighting(TRUE);

	//Ｚバッファを無効にする
	SetUseZBuffer3D(FALSE);
}