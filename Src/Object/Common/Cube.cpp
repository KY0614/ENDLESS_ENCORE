#include <DxLib.h>
#include <vector>
#include "Cube.h"

Cube::Cube(const Transform& parent) : transformParent_(parent)
{
}

Cube::Cube(const Cube& base, const Transform& parent) : transformParent_(parent)
{
}

Cube::~Cube(void)
{
}

void Cube::MakeCube(VECTOR center, float size, COLOR_U8  col)
{
    float h = size / 2.0f;

    // 8頂点を定義（立方体の各隅）
    VECTOR v[8] = {
        VGet(center.x - h, center.y - h, center.z - h), // 0: 左下前
        VGet(center.x + h, center.y - h, center.z - h), // 1: 右下前
        VGet(center.x + h, center.y + h, center.z - h), // 2: 右上前
        VGet(center.x - h, center.y + h, center.z - h), // 3: 左上前
        VGet(center.x - h, center.y - h, center.z + h), // 4: 左下後
        VGet(center.x + h, center.y - h, center.z + h), // 5: 右下後
        VGet(center.x + h, center.y + h, center.z + h), // 6: 右上後
        VGet(center.x - h, center.y + h, center.z + h)  // 7: 左上後
    };

    ////底面座標を中心座標に
    //VECTOR v[8] = {
    //    VGet(center.x - h, center.y,     center.z - h), // 下
    //    VGet(center.x + h, center.y,     center.z - h),
    //    VGet(center.x + h, center.y + size, center.z - h),
    //    VGet(center.x - h, center.y + size, center.z - h),
    //    VGet(center.x - h, center.y,     center.z + h),
    //    VGet(center.x + h, center.y,     center.z + h),
    //    VGet(center.x + h, center.y + size, center.z + h),
    //    VGet(center.x - h, center.y + size, center.z + h)
    //};

    // 各面ごとに三角形を2枚定義（全6面 → 12三角形）

    // 各面の三角形の頂点インデックス（12三角形 * 3）
    int faceIndices[6][2][3] = {
    { {2, 1, 0}, {3, 2, 0} }, // 前面
    { {4, 5, 6}, {4, 6, 7} }, // 背面
    { {3, 0, 4}, {7, 3, 4} }, // 左面
    { {6, 5, 1}, {2, 6, 1} }, // 右面
    { {6, 2, 3}, {7, 6, 3} }, // 上面
    { {0, 1, 5}, {0, 5, 4} }  // 底面
    };

    int vi = 0;
    for (int f = 0; f < 6; ++f) {
        VECTOR normal;

        // 各面の法線ベクトル（正しいライティング用）
        switch (f) {
        case 0: normal = VGet(0, 0, -1); break; // 前
        case 1: normal = VGet(0, 0, 1); break; // 後
        case 2: normal = VGet(-1, 0, 0); break; // 左
        case 3: normal = VGet(1, 0, 0); break; // 右
        case 4: normal = VGet(0, 1, 0); break; // 上
        case 5: normal = VGet(0, -1, 0); break; // 下
        }

        for (int t = 0; t < 2; ++t) {
            for (int i = 0; i < 3; ++i) {
                int idx = faceIndices[f][t][i];
                Vertex[vi].pos = v[idx];
                Vertex[vi].norm = normal;
                Vertex[vi].dif = col;
                Vertex[vi].spc = GetColorU8(0, 0, 0, 0);
                Vertex[vi].u = Vertex[vi].v = Vertex[vi].su = Vertex[vi].sv = 0.0f;
                vi++;
            }
        }

    }
}

void Cube::MakeBox(VECTOR center, float width, float height, float depth, COLOR_U8 col)
{
    VERTEX3D verts[36]; // 6面 × 2三角形 × 3頂点 = 36
    int vi = 0;

    float hw = width / 2.0f;
    float hh = height / 2.0f;
    float hd = depth / 2.0f;

    //VECTOR p[8] =
    //{
    //    VGet(center.x - hw, center.y - hh, center.z - hd), // 0: 左下手前
    //    VGet(center.x + hw, center.y - hh, center.z - hd), // 1: 右下手前
    //    VGet(center.x + hw, center.y + hh, center.z - hd), // 2: 右上手前
    //    VGet(center.x - hw, center.y + hh, center.z - hd), // 3: 左上手前
    //    VGet(center.x - hw, center.y - hh, center.z + hd), // 4: 左下奥
    //    VGet(center.x + hw, center.y - hh, center.z + hd), // 5: 右下奥
    //    VGet(center.x + hw, center.y + hh, center.z + hd), // 6: 右上奥
    //    VGet(center.x - hw, center.y + hh, center.z + hd), // 7: 左上奥
    //};

    //底面中心座標
    VECTOR p[8] =
    {
        VGet(center.x - hw, center.y,         center.z - hd), // 0: 左下手前
        VGet(center.x + hw, center.y,         center.z - hd), // 1: 右下手前
        VGet(center.x + hw, center.y + height, center.z - hd), // 2: 右上手前
        VGet(center.x - hw, center.y + height, center.z - hd), // 3: 左上手前
        VGet(center.x - hw, center.y,         center.z + hd), // 4: 左下奥
        VGet(center.x + hw, center.y,         center.z + hd), // 5: 右下奥
        VGet(center.x + hw, center.y + height, center.z + hd), // 6: 右上奥
        VGet(center.x - hw, center.y + height, center.z + hd), // 7: 左上奥
    };

    struct Face {
        int a, b, c, d;
        VECTOR normal;
    };

    Face faces[6] =
    {
        { 0, 1, 2, 3, VGet(0.0f,  0.0f, -1.0f) }, // 前
        { 5, 4, 7, 6, VGet(0.0f,  0.0f,  1.0f) }, // 後ろ
        { 4, 0, 3, 7, VGet(-1.0f,  0.0f,  0.0f) }, // 左
        { 1, 5, 6, 2, VGet(1.0f,  0.0f,  0.0f) }, // 右
        { 3, 2, 6, 7, VGet(0.0f,  1.0f,  0.0f) }, // 上
        { 4, 5, 1, 0, VGet(0.0f, -1.0f,  0.0f) }, // 下
    };

    for (int i = 0; i < 6; ++i)
    {
        Face f = faces[i];

        // 三角形1: a, b, c
        verts[vi++] = { p[f.c], f.normal, col, GetColorU8(0,0,0,0), 0,0,0,0 };
        verts[vi++] = { p[f.b], f.normal, col, GetColorU8(0,0,0,0), 0,0,0,0 };
        verts[vi++] = { p[f.a], f.normal, col, GetColorU8(0,0,0,0), 0,0,0,0 };

        // 三角形2: a, c, d
        verts[vi++] = { p[f.d], f.normal, col, GetColorU8(0,0,0,0), 0,0,0,0 };
        verts[vi++] = { p[f.c], f.normal, col, GetColorU8(0,0,0,0), 0,0,0,0 };
        verts[vi++] = { p[f.a], f.normal, col, GetColorU8(0,0,0,0), 0,0,0,0 };
    }
    
    // 描画（12ポリゴン）
    DrawPolygon3D(verts, 12, DX_NONE_GRAPH, true);
}

void Cube::Draw(void)
{
    // 立方体描画（12ポリゴン）
    DrawPolygon3D(Vertex, 12, DX_NONE_GRAPH, false);
}
