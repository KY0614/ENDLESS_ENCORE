// VS/PS共通
#include "../Common/VertexToPixelHeader.hlsli"

// IN
#include "../Common/Vertex/VertexInputType.hlsli"
#define VERTEX_INPUT DX_MV1_VERTEX_TYPE_NMAP_1FRAME

// OUT
#define VS_OUTPUT VertexToPixelLit
#include "../Common/Vertex/VertexShader3DHeader.hlsli"

// 定数バッファ：スロット4番目(b4と書く)
cbuffer cbParam : register(b7)
{
    float4 g_uv_scale;
    float g_scroll_time;
}

VS_OUTPUT main(VS_INPUT VSInput)
{
    
    VS_OUTPUT ret;
    
    // 頂点座標変換 +++++++++++++++++++++++++++++++++++++( 開始 )
    float4 lLocalPosition;
    float4 lWorldPosition;
    float4 lViewPosition;
    
    // float3 → float4
    lLocalPosition.xyz = VSInput.pos;
    lLocalPosition.w = 1.0f;

    float g_speed = 2.0f; //波の速さ
    float g_wave_height = 10.0f; //波の高さ
    float g_wave_frequency = 0.01f; //波の周波数
    lLocalPosition.y += sin(lLocalPosition.x * g_wave_height + g_scroll_time * g_speed) * g_wave_frequency;
    lLocalPosition.y += cos(lLocalPosition.z * 15.0f + g_scroll_time * 0.5f) * g_wave_frequency;
    
    // ローカル座標をワールド座標に変換(剛体)
    lWorldPosition.w = 1.0f;
    lWorldPosition.xyz = mul(lLocalPosition, g_base.localWorldMatrix);

    ret.worldPos = lWorldPosition.xyz;
    
    // ワールド座標をビュー座標に変換
    lViewPosition.w = 1.0f;
    lViewPosition.xyz = mul(lWorldPosition, g_base.viewMatrix);
    
    ret.vwPos.xyz = lViewPosition.xyz;
    
    // ビュー座標を射影座標に変換
    ret.svPos = mul(lViewPosition, g_base.projectionMatrix);
   
    // 頂点座標変換 +++++++++++++++++++++++++++++++++++++( 終了 )
   
    // その他、ピクセルシェーダへ引継&初期化 ++++++++++++( 開始 )
    // UV座標
    //ret.uv.x = VSInput.uv0.x;
    //ret.uv.y = VSInput.uv0.y;
    
    // UV座標
    ret.uv.x = VSInput.uv0.x * g_uv_scale;
    ret.uv.y = VSInput.uv0.y * g_uv_scale;
    
    // 法線
    ret.normal = normalize(
            mul(VSInput.norm, (float3x3) g_base.localWorldMatrix));
    
    // ディフューズカラー
    ret.diffuse = VSInput.diffuse;
    
    // ライト方向(ローカル)
    ret.lightDir = float3(0.0f, 0.0f, 0.0f);
    
    // その他、ピクセルシェーダへ引継&初期化 ++++++++++++( 終了 )
    // 出力パラメータを返す
    return ret;
}