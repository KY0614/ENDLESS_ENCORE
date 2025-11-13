#define NOMINMAX
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
    UpdateBoudingBoxAxis();
}

Cube::Cube(const VECTOR& pos,
    const Quaternion& rot,
    const VECTOR halfSize) :
    GeometryBase(pos, rot)
{
    boudingBox_.vMin = VScale(halfSize, -1.0f);
    boudingBox_.vMax = halfSize;

    UpdateBoudingBoxAxis();
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

bool Cube::IsHit(GeometryBase& geometry)
{
    return false;
}

bool Cube::IsHit(Model& model)
{
    return false;
}

bool Cube::IsHit(Cube& cube)
{
    return false;
}

bool Cube::IsHit(Sphere& sphere)
{
    return false;
}

bool Cube::IsHit(Capsule& capsule)
{
    // BoudingBoxのローカル中心
    VECTOR localCenter = VScale(VAdd(boudingBox_.vMin, boudingBox_.vMax), 0.5f);

    // BoudingBoxのワールド中心
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

    // カプセル線分をBoudingBoxのローカル空間に変換
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
    // AABBとして処理する（BoudingBoxローカル空間内で）

    float distSq = ClosestSegmentAABB(local1, local2, boudingBox_.vMin, boudingBox_.vMax);

    return distSq <= (capsule.GetRadius() * capsule.GetRadius());

}

bool Cube::IsHit(Line& line)
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

void Cube::UpdateBoudingBoxAxis(void)
{
    MATRIX rotMat;
    rotMat = parentQuaRot_.ToMatrix();

    boudingBox_.axis[0] = VTransform(VGet(1, 0, 0), rotMat); // Right
    boudingBox_.axis[1] = VTransform(VGet(0, 1, 0), rotMat); // Up
    boudingBox_.axis[2] = VTransform(VGet(0, 0, 1), rotMat); // Forward

}

void Cube::CalculateVertices(VECTOR outVertices[8]) const
{
    MATRIX rotMat;
    rotMat = parentQuaRot_.ToMatrix();

    int idx = 0;
    for (int x = 0; x <= 1; ++x)
    {
        for (int y = 0; y <= 1; ++y)
        {
            for (int z = 0; z <= 1; ++z)
            {
                VECTOR local;
                local.x = (x == 0) ? boudingBox_.vMin.x : boudingBox_.vMax.x;
                local.y = (y == 0) ? boudingBox_.vMin.y : boudingBox_.vMax.y;
                local.z = (z == 0) ? boudingBox_.vMin.z : boudingBox_.vMax.z;

                VECTOR world = VTransform(local, rotMat);
                world = VAdd(world, parentPos_);

                outVertices[idx++] = world;
            }
        }
    }
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
