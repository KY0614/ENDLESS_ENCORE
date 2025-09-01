#include "../../Utility/AsoUtility.h"
#include "../../Manager/Generic/SceneManager.h"
#include "../../Manager/Generic/Camera.h"
#include "../../Manager/Generic/ResourceManager.h"
#include "GaugeUI.h"

GaugeUI::GaugeUI(bool isCircle, float activeTime):
	activeTime_(activeTime), isCircle_(isCircle), currentTime_(0.0f)
{
	shadowImg_ = -1;
	circleImg_ = -1;
	cShadowImg_ = -1;
	isActive_ = false;
    alpha_ = 1.0f;

    width_ = 0.0f;
    height_ = 0.0f;
} 

void GaugeUI::Init(void)
{
    uiImg_ = ResourceManager::GetInstance().Load(
        ResourceManager::SRC::UI_BAR).handleId_;

    shadowImg_ = ResourceManager::GetInstance().Load(
        ResourceManager::SRC::UI_BARSHADOW).handleId_;

    width_ = UI_DEFAULT_WIDTH;    // 長方形ゲージの幅
    height_ = UI_DEFAULT_HEIGHT;    // 長方形ゲージの高さ

    if (!isCircle_) return;
    circleImg_ = ResourceManager::GetInstance().Load(
        ResourceManager::SRC::UI_CIRCLE).handleId_;

    cShadowImg_ = ResourceManager::GetInstance().Load(
        ResourceManager::SRC::UI_CIRCLESHADOW).handleId_;

}

void GaugeUI::Update(void)
{
	if (!isActive_)return;

	currentTime_ += SceneManager::GetInstance().GetDeltaTime();
	if (currentTime_ >= activeTime_) 
    {
		currentTime_ = activeTime_;
	}
}

void GaugeUI::Draw(void)
{
	if (!isActive_) return;

    float progress = currentTime_ / activeTime_;

    //SetUseLighting(false);
    //SetUseZBuffer3D(false);

    if (isCircle_) 
    {
        DrawCircleGauge(progress);
    }
    else 
    {
        DrawRectGauge(progress);
    }

    //SetUseZBuffer3D(true);
    //SetUseLighting(true);
}

void GaugeUI::Reset(void)
{
    currentTime_ = 0.0f;         // ゲージの進捗をリセット（ゼロ）
    isActive_ = false;        // 非表示・非アクティブに
}

void GaugeUI::DrawCircleGauge(float progress)
{
    const float radius = 50.0f;
    const int segmentCount = 80; // 扇の分割数
    const float angleMax = -DX_TWO_PI * progress;
    float baseAngle = DX_PI_F / 2.0f; // +90度のオフセット（上からスタート）
    std::vector<VERTEX3D> verts;

    // 中心点
    VERTEX3D center;
    center.pos = pos_;
    center.dif = GetColorU8(255, 255, 255, 255);
    center.norm = VGet(0.0f, 0.0f, -1.0f);
    center.u = 0.5f;
    center.v = 0.5f;
    verts.push_back(center);

    for (int i = 0; i <= segmentCount * progress; ++i) {
        float angle = baseAngle + angleMax * (float)i / segmentCount;
        float x = cosf(angle) * radius;
        float y = sinf(angle) * radius;

        VERTEX3D v;
        v.pos = VGet(pos_.x + x, pos_.y + y, pos_.z);
        v.dif = GetColorU8(255, 255, 255, 255);
        v.norm = VGet(0.0f, 0.0f, 1.0f);
        v.spc = GetColorU8(0, 0, 0, 0);
        v.u = 0.5f + x / (2 * radius); // 中心から±0.5のUV
        v.v = 0.5f - y / (2 * radius);
        verts.push_back(v);
    }

    // 扇型を描画
    for (int i = 1; i < (int)verts.size() - 1; ++i) 
    {
        VERTEX3D tri[3] = { verts[i], verts[i + 1], verts[0] };
        DrawPolygon3D(tri, 2, cShadowImg_,true);
        DrawPolygon3D(tri, 2,circleImg_,true);
    }
}

void GaugeUI::DrawRectGauge(float progress)
{
    const float width = width_;
    const float height = height_;
    float drawWidth = width * progress;

    COLOR_U8 dif = GetColorU8(255, 255, 255, 255);
    COLOR_U8 spec = GetColorU8(0, 0, 0, 0);
    auto MakeQuad = [&](float drawWidth, int texHandle) {
        VERTEX3D verts[6];

        // ゲージの高さ方向の傾き量（Z方向に何ユニット近づけるか）
        const float zTilt = -30.0f;

        VERTEX3D v0 = {
            VGet(-width / 2, height / 2, 0),
            VGet(0.0f, 0.0f, 1.0f),
            dif,
            spec,
            0.0f, 0.0f };

        VERTEX3D v1 = {
            VGet(-width / 2 + drawWidth, height / 2, 0),
            VGet(0.0f, 0.0f, 1.0f),
            dif,
            spec,
            drawWidth / width, 0.0f };

        VERTEX3D v2 = {
            VGet(-width / 2, -height / 2, zTilt),
            VGet(0.0f, 0.0f, 1.0f),
            dif,
            spec,
            0.0f, 1.0f };

        VERTEX3D v3 = {
            VGet(-width / 2 + drawWidth, -height / 2, zTilt),
            VGet(0.0f, 0.0f, 1.0f),
            dif,
            spec,
            drawWidth / width, 1.0f };

        verts[0] = v0;
        verts[1] = v1;
        verts[2] = v2;
        verts[3] = v2;
        verts[4] = v1;
        verts[5] = v3;

        for (int i = 0; i < 6; ++i) 
        {
            verts[i].pos = VAdd(verts[i].pos, pos_);
        }
        SetLightDirection(VGet(0.0f, 0.0f, -1.0f));
        DrawPolygon3D(verts, 2, texHandle, true);
        SetLightDirection(VGet(0.0f, -0.5f, 0.3f));
        };

    MakeQuad(width, shadowImg_);
    MakeQuad(drawWidth, uiImg_);
}
