#include "../Transform.h"
#include "Sphere.h"

Sphere::Sphere(const Transform& parent) : 
	transformParent_(parent)
{
	parentPos_ = {};
	radius_ = 0.0f;
	localPos_ = {};
}

Sphere::Sphere(const VECTOR& parentPos,const Transform& parent) : 
	parentPos_(parentPos),
	transformParent_(parent)
{
	radius_ = 0.0f;
	localPos_ = { 0.0f, 0.0f, 0.0f };
}

Sphere::Sphere(const Sphere& base, const Transform& parent) : transformParent_(parent)
{
	radius_ = base.GetRadius();
	localPos_ = base.GetLocalPos();
}

Sphere::~Sphere(void)
{
}

void Sphere::Draw(void)
{
	//Zバッファを有効にして描画(球体同士の前後関係を正しく描画するため)
	SetUseZBufferFlag(true);
	//上の球体
	VECTOR pos = GetPos();
	DrawSphere3D(pos, radius_, 5, COLOR, COLOR, false);
	SetUseZBufferFlag(false);//戻す
}

void Sphere::Draw(int col,bool fill)
{
	//Zバッファを有効にして描画(球体同士の前後関係を正しく描画するため)
	SetUseZBufferFlag(true);
	VECTOR pos = GetPos();
	DrawSphere3D(pos, radius_, 5, col, col, fill);
	SetUseZBufferFlag(false);//戻す
}

VECTOR Sphere::GetRotPos(const VECTOR& localPos) const
{
	VECTOR localRotPos = transformParent_.quaRot.PosAxis(localPos);
	return VAdd(transformParent_.pos, localRotPos);
}