#include "../Common/Pixel/PixelShader2DHeader.hlsli"

//定数バッファ：スロット番号4番目（b4）
cbuffer cbParam : register(b4)
{
    float4 g_color;
}

float4 main(PS_INPUT PSInput) : SV_TARGET
{
 	//UV座標調整
    float2 uv = PSInput.uv;
    float4 color = tex.Sample(texSampler, uv);

	//周りの画素を取得する（指定方向）
    float pi = 3.14159265f;
    float2 direction = float2(cos(pi), sin(pi));
    float uv_dis = 0.1f;

    color += tex.Sample(texSampler, uv + direction * uv_dis * 0.1f);
    color += tex.Sample(texSampler, uv + direction * uv_dis * 0.2f);
    color += tex.Sample(texSampler, uv + direction * uv_dis * 0.3f);
    color += tex.Sample(texSampler, uv + direction * uv_dis * 0.4f);

	//単純平均
    color /= 5.0f;
    
    float2 center = float2(0.5, 0.5);
    
     // UVから中心までの距離を計算
    float dist = distance(uv, center);
    
    // 色の強さを距離に応じて調整（端に行くほど強い）
    float factor = saturate(dist * 2.0) * 0.3f;
    
    //色を追加
        float3 damageTint = lerp(color.rgb, g_color.rgb, factor);
    
    //アルファは元のまま
    return float4(damageTint, color.a);
   // return color;
}