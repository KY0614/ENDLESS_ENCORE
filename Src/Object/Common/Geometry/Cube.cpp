#include <algorithm>
#include <vector>
#include"../../../Common/Quaternion.h"
#include "../../../Utility/CommonUtility.h"
#include"Sphere.h"
#include"Capsule.h"
#include "Cube.h"

Cube::Cube(const VECTOR& pos,
    const Quaternion& rot,
    const VECTOR min,
    const VECTOR max) :
	GeometryBase(pos, rot)
{
    boudingBox_.vMin = min;
    boudingBox_.vMax = max;
}

Cube::Cube(const Cube& copyBase,
    const VECTOR& pos,
    const Quaternion& rot) : 
	GeometryBase(pos, rot)
{
}

Cube::~Cube(void)
{
}

const bool Cube::IsHit(GeometryBase& geometry)
{
    return false;
}

const bool Cube::IsHit(Model& model)
{
    return false;
}

const bool Cube::IsHit(Cube& cube)
{
    return false;
}

const bool Cube::IsHit(Sphere& sphere)
{
    return false;
}

const bool Cube::IsHit(Capsule& capsule)
{
    // OBB のローカル中心
    VECTOR localCenter = VScale(VAdd(boudingBox_.vMin, boudingBox_.vMax), 0.5f);

    // OBB のワールド中心
    VECTOR worldCenter = VAdd(
        VAdd(
            VAdd(
                VScale(boudingBox_.axis[0], localCenter.x),
                VScale(boudingBox_.axis[1], localCenter.y)
            ),
            VScale(boudingBox_.axis[2], localCenter.z)
        ),
        parentPos_
    );

    // カプセル線分をOBBのローカル空間に変換
    VECTOR rel1 = VSub(capsule.GetPosTop(), worldCenter);
    VECTOR rel2 = VSub(capsule.GetPosDown(), worldCenter);

    VECTOR local1 = {
        VDot(rel1, boudingBox_.axis[0]),
        VDot(rel1, boudingBox_.axis[1]),
        VDot(rel1, boudingBox_.axis[2])
    };

    VECTOR local2 = {
        VDot(rel2, boudingBox_.axis[0]),
        VDot(rel2, boudingBox_.axis[1]),
        VDot(rel2, boudingBox_.axis[2])
    };

    // スラブ法で最近接点を見つける
    // AABBとして処理する（OBBローカル空間内で）

    float distSq = ClosestSegmentAABB(local1, local2, boudingBox_.vMin, boudingBox_.vMax);

    return distSq <= (capsule.GetRadius() * capsule.GetRadius());

}

const bool Cube::IsHit(Line& _line)
{
    return false;
}

void Cube::Draw(void)
{
    VECTOR vertices[8];
    CalculateVertices(vertices);

    // 12本のエッジのインデックス
    static const int edges[12][2] = {
        {0,1},{0,2},{0,4}, {1,3},{1,5},
        {2,3},{2,6}, {3,7},
        {4,5},{4,6}, {5,7},{6,7}
    };

    for (int i = 0; i < 12; ++i)
    {
        DrawLine3D(vertices[edges[i][0]], vertices[edges[i][1]], COLOR);
    }
}

inline void Cube::SetHalfSize(const VECTOR& _halfSize)
{
}

void Cube::UpdateObbAxis(void)
{
}

void Cube::CalculateVertices(VECTOR outVertices[8]) const
{
}

float Cube::ClosestSegmentAABB(const VECTOR& segA, const VECTOR& segB, const VECTOR& aabbMin, const VECTOR& aabbMax)
{

    // 線分とAABBの最短距離?を求める
    // → 各軸でクランプを行う

    float t = 0.0f;
    float minDistSq = FLT_MAX;

    // 線分上の点 P(t) = A + t*(B - A), 0 <= t <= 1
    const int steps = 10;
    for (int i = 0; i <= steps; ++i)
    {
        float ft = static_cast<float>(i) / steps;
        VECTOR point = VAdd(segA, VScale(VSub(segB, segA), ft));

        // AABB内の最近接点
        VECTOR clamped = {
            std::max(aabbMin.x, std::min(point.x, aabbMax.x)),
            std::max(aabbMin.y, std::min(point.y, aabbMax.y)),
            std::max(aabbMin.z, std::min(point.z, aabbMax.z))
        };

        float distSq = CommonUtility::SqrMagnitudeF(VSub(point, clamped));
        if (distSq < minDistSq)
        {
            minDistSq = distSq;
            t = ft;
        }
    }

    return minDistSq;
}
