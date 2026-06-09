// VS/PS共通
#include "../Common/VertexToPixelHeader.hlsli"

// IN
#include "../Common/Vertex/VertexInputType.hlsli"
#define VERTEX_INPUT DX_MV1_VERTEX_TYPE_8FRAME

// OUT
#define VS_OUTPUT VertexToPixelLit
#include "../Common/Vertex/VertexShader3DHeader.hlsli"

// 定数バッファ 
cbuffer cbParam : register(b7)
{
    float3 g_camera_pos; //カメラ座標
    float dummy; //ダミー
    
    float g_fog_start; //フォグの開始座標
    float g_fog_end; //フォグの終了座標
    float2 dummy2; //ダミー
}

#define L_W_MAT g_localWorldMatrix.lwMatrix

VS_OUTPUT main(VS_INPUT VSInput)
{
// スキンメッシュ用のローカル⇒ワールド変換行列を作成+++( 開始 )
    int4 lBoneIdx;
    float4 lL_W_Mat[3];
    float4 lWeight;
// BONE4
    lBoneIdx = VSInput.blendIndices0;
    lWeight = VSInput.blendWeight0;
    lL_W_Mat[0] = L_W_MAT[lBoneIdx.x + 0] * lWeight.xxxx;
    lL_W_Mat[1] = L_W_MAT[lBoneIdx.x + 1] * lWeight.xxxx;
    lL_W_Mat[2] = L_W_MAT[lBoneIdx.x + 2] * lWeight.xxxx;
    lL_W_Mat[0] += L_W_MAT[lBoneIdx.y + 0] * lWeight.yyyy;
    lL_W_Mat[1] += L_W_MAT[lBoneIdx.y + 1] * lWeight.yyyy;
    lL_W_Mat[2] += L_W_MAT[lBoneIdx.y + 2] * lWeight.yyyy;
    lL_W_Mat[0] += L_W_MAT[lBoneIdx.z + 0] * lWeight.zzzz;
    lL_W_Mat[1] += L_W_MAT[lBoneIdx.z + 1] * lWeight.zzzz;
    lL_W_Mat[2] += L_W_MAT[lBoneIdx.z + 2] * lWeight.zzzz;
    lL_W_Mat[0] += L_W_MAT[lBoneIdx.w + 0] * lWeight.wwww;
    lL_W_Mat[1] += L_W_MAT[lBoneIdx.w + 1] * lWeight.wwww;
    lL_W_Mat[2] += L_W_MAT[lBoneIdx.w + 2] * lWeight.wwww;
// BONE8
    lBoneIdx = VSInput.blendIndices1;
    lWeight = VSInput.blendWeight1;
    lL_W_Mat[0] += L_W_MAT[lBoneIdx.x + 0] * lWeight.xxxx;
    lL_W_Mat[1] += L_W_MAT[lBoneIdx.x + 1] * lWeight.xxxx;
    lL_W_Mat[2] += L_W_MAT[lBoneIdx.x + 2] * lWeight.xxxx;
    lL_W_Mat[0] += L_W_MAT[lBoneIdx.y + 0] * lWeight.yyyy;
    lL_W_Mat[1] += L_W_MAT[lBoneIdx.y + 1] * lWeight.yyyy;
    lL_W_Mat[2] += L_W_MAT[lBoneIdx.y + 2] * lWeight.yyyy;
    lL_W_Mat[0] += L_W_MAT[lBoneIdx.z + 0] * lWeight.zzzz;
    lL_W_Mat[1] += L_W_MAT[lBoneIdx.z + 1] * lWeight.zzzz;
    lL_W_Mat[2] += L_W_MAT[lBoneIdx.z + 2] * lWeight.zzzz;
    lL_W_Mat[0] += L_W_MAT[lBoneIdx.w + 0] * lWeight.wwww;
    lL_W_Mat[1] += L_W_MAT[lBoneIdx.w + 1] * lWeight.wwww;
    lL_W_Mat[2] += L_W_MAT[lBoneIdx.w + 2] * lWeight.wwww;

// 頂点座標変換 +++++++++++++++++++++++++++++++++++++( 開始 )
    VS_OUTPUT ret;
    float4 lLocalPosition;
    float4 lWorldPosition;
    float4 lViewPosition;
   
    // float3 → float4
    lLocalPosition.xyz = VSInput.pos;
    lLocalPosition.w = 1.0f;
    
    // ローカル座標をワールド座標に変換
    // ローカル座標をワールド座標に変換(剛体)
    lWorldPosition.w = 1.0f;
    lWorldPosition.x = dot(lLocalPosition, lL_W_Mat[0]);
    lWorldPosition.y = dot(lLocalPosition, lL_W_Mat[1]);
    lWorldPosition.z = dot(lLocalPosition, lL_W_Mat[2]);
    ret.worldPos = lWorldPosition.xyz; // ワールド座標をピクセルシェーダへ引き継ぐ
    
    // ワールド座標をビュー座標に変換
    lViewPosition.w = 1.0f;
    lViewPosition.xyz = mul(lWorldPosition, g_base.viewMatrix);
    ret.vwPos.xyz = lViewPosition.xyz;
   
    // ビュー座標を射影座標に変換
    ret.svPos = mul(lViewPosition, g_base.projectionMatrix);
    
    //カメラから頂点までのワールド空間での距離を計算
    float distance = length(lWorldPosition.xyz - g_camera_pos);
    float fog = (g_fog_end - distance) / (g_fog_end - g_fog_start);
    fog = saturate(fog);
    ret.fogFactor = fog;
    // 頂点座標変換 +++++++++++++++++++++++++++++++++++++( 終了 )
    
    // その他、ピクセルシェーダへ引継&初期化 ++++++++++++( 開始 )
    // UV座標
    ret.uv.x = VSInput.uv0.x;
    ret.uv.y = VSInput.uv0.y;
    // 法線
    // 法線をローカル空間からワールド空間へ変換
    ret.normal.x = dot(VSInput.norm, lL_W_Mat[0]);
    ret.normal.y = dot(VSInput.norm, lL_W_Mat[1]);
    ret.normal.z = dot(VSInput.norm, lL_W_Mat[2]);
    ret.normal = normalize(ret.normal);

    // ディフューズカラー
    ret.diffuse = VSInput.diffuse;
    
    // ライト方向(ローカル)
    ret.lightDir = float3(0.0f, 0.0f, 0.0f);
    
    //ライトから見た座標
    ret.lightPow = float(0.0f);
    
    // その他、ピクセルシェーダへ引継&初期化 ++++++++++++( 終了 )
    // 出力パラメータを返す
    return ret;
}