#include "../Application.h"
#include "../Object/Common/AnimationController.h"
#include "../Manager/Generic/ResourceManager.h"
#include "../Manager/Generic/SceneManager.h"
#include "Collider/ColliderBase.h"
#include "ActorBase.h"

ActorBase::ActorBase(void)
{
}

ActorBase::~ActorBase(void)
{
}

void ActorBase::Init(void)
{

}

void ActorBase::Update(void)
{
}

void ActorBase::Draw(void)
{
#ifdef _DEBUG
	//所有しているコライダの描画
	for (const auto& own : ownColliders_)
	{
		own.second->Draw();
	}
#endif // _DEBUG
}

const Transform& ActorBase::GetTransform(void) const
{
	return transform_;
}

const VECTOR ActorBase::GetFramePos(const std::wstring& frameName) const
{
	VECTOR ret = {};
	//フレームIDを取得して、フレームの座標を取得する
	const int frameId = MV1SearchFrame(transform_.modelId, frameName.c_str());
	ret = MV1GetFramePosition(transform_.modelId, frameId);
	return ret;
}

void ActorBase::AddCollider(std::weak_ptr<Collider> collider)
{
	colliders_.emplace_back(collider);
}

void ActorBase::AddHitCollider(const std::weak_ptr<ColliderBase> hitCollider)
{
	for (const auto& c : hitColliders_)
	{
		if (c.lock() == hitCollider.lock())
		{
			return;
		}
	}
	hitColliders_.emplace_back(hitCollider);
}

void ActorBase::ClearHitCollider(void)
{
	hitColliders_.clear();
}

const std::weak_ptr<ColliderBase> ActorBase::GetOwnCollider(int key) const
{
	//指定されたキーに対応する自身の衝突情報が存在しない場合は空を返す
	if (ownColliders_.count(key) == 0)
	{
		static std::weak_ptr<ColliderBase> nullPtr;
		return nullPtr;
	}
	return ownColliders_.find(key)->second;
}

//void ActorBase::DrawShadow(void)
//{
//	int i = 0;
//	MV1_COLL_RESULT_POLY_DIM HitResDim;
//	MV1_COLL_RESULT_POLY* HitRes;
//	VERTEX3D Vertex[3];
//	VECTOR SlideVec;
//	int ModelHandle;
//
//	//ライティングを無効にする
//	SetUseLighting(FALSE);
//
//	//Ｚバッファを有効にする
//	SetUseZBuffer3D(TRUE);
//
//	//テクスチャアドレスモードを CLAMP にする( テクスチャの端より先は端のドットが延々続く )
//	SetTextureAddressMode(DX_TEXADDRESS_CLAMP);
//
//	//影を落とすモデルの数だけ繰り返し
//	for (const auto& c : colliders_)
//	{
//		//チェックするモデルは、jが0の時はステージモデル、1以上の場合はコリジョンモデル
//
//		ModelHandle = c.lock()->modelId_;
//
//		//影の高さとサイズを設定
//		const float PLAYER_SHADOW_HEIGHT = 700.0f;
//		const float PLAYER_SHADOW_SIZE = 50.0f;
//
//		const int maxAlpha = 128;	//影の最大不透明度
//
//		//プレイヤーの直下に存在する地面のポリゴンを取得
//		HitResDim = MV1CollCheck_Capsule(ModelHandle, -1, transform_.pos, VAdd(transform_.pos, VGet(0.0f, -PLAYER_SHADOW_HEIGHT, 0.0f)), PLAYER_SHADOW_SIZE);
//
//		//頂点データで変化が無い部分をセット
//		Vertex[0].dif = GetColorU8(255, 255, 255, 255);
//		Vertex[0].spc = GetColorU8(0, 0, 0, 0);
//		Vertex[0].su = 0.0f;
//		Vertex[0].sv = 0.0f;
//		Vertex[1] = Vertex[0];
//		Vertex[2] = Vertex[0];
//
//		//球の直下に存在するポリゴンの数だけ繰り返し
//		HitRes = HitResDim.Dim;
//		for (i = 0; i < HitResDim.HitNum; i++, HitRes++)
//		{
//			//ポリゴンの座標は地面ポリゴンの座標
//			Vertex[0].pos = HitRes->Position[0];
//			Vertex[1].pos = HitRes->Position[1];
//			Vertex[2].pos = HitRes->Position[2];
//
//			//ちょっと持ち上げて重ならないようにする
//			SlideVec = VScale(HitRes->Normal, 0.5f);
//			Vertex[0].pos = VAdd(Vertex[0].pos, SlideVec);
//			Vertex[1].pos = VAdd(Vertex[1].pos, SlideVec);
//			Vertex[2].pos = VAdd(Vertex[2].pos, SlideVec);
//
//			//ポリゴンの不透明度を設定する
//			Vertex[0].dif.a = 0;
//			Vertex[1].dif.a = 0;
//			Vertex[2].dif.a = 0;
//			if (HitRes->Position[0].y > transform_.pos.y - PLAYER_SHADOW_HEIGHT)
//				Vertex[0].dif.a = static_cast<BYTE>(
//					maxAlpha * (1.0f - fabs(HitRes->Position[0].y - transform_.pos.y) / PLAYER_SHADOW_HEIGHT));
//
//			if (HitRes->Position[1].y > transform_.pos.y - PLAYER_SHADOW_HEIGHT)
//				Vertex[1].dif.a = static_cast<BYTE>(
//					maxAlpha * (1.0f - fabs(HitRes->Position[1].y - transform_.pos.y) / PLAYER_SHADOW_HEIGHT));
//
//			if (HitRes->Position[2].y > transform_.pos.y - PLAYER_SHADOW_HEIGHT)
//				Vertex[2].dif.a = static_cast<BYTE>(
//					maxAlpha * (1.0f - fabs(HitRes->Position[2].y - transform_.pos.y) / PLAYER_SHADOW_HEIGHT));
//
//			//ＵＶ値は地面ポリゴンとプレイヤーの相対座標から割り出す
//			const float uvOffset = 0.5f;	//UV値の中心をプレイヤーの位置にするためのオフセット
//			Vertex[0].u = (HitRes->Position[0].x - transform_.pos.x) / (PLAYER_SHADOW_SIZE * 2.0f) + uvOffset;
//			Vertex[0].v = (HitRes->Position[0].z - transform_.pos.z) / (PLAYER_SHADOW_SIZE * 2.0f) + uvOffset;
//			Vertex[1].u = (HitRes->Position[1].x - transform_.pos.x) / (PLAYER_SHADOW_SIZE * 2.0f) + uvOffset;
//			Vertex[1].v = (HitRes->Position[1].z - transform_.pos.z) / (PLAYER_SHADOW_SIZE * 2.0f) + uvOffset;
//			Vertex[2].u = (HitRes->Position[2].x - transform_.pos.x) / (PLAYER_SHADOW_SIZE * 2.0f) + uvOffset;
//			Vertex[2].v = (HitRes->Position[2].z - transform_.pos.z) / (PLAYER_SHADOW_SIZE * 2.0f) + uvOffset;
//
//			//影ポリゴンを描画
//			DrawPolygon3D(Vertex, 1, imgShadow_, TRUE);
//		}
//
//		//検出した地面ポリゴン情報の後始末
//		MV1CollResultPolyDimTerminate(HitResDim);
//	}
//
//	//ライティングを有効にする
//	SetUseLighting(TRUE);
//
//	//Ｚバッファを無効にする
//	SetUseZBuffer3D(FALSE);
//}