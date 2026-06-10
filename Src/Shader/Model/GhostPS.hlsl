// VS/PS共通
#include "../Common/VertexToPixelHeader.hlsli"

// IN
#define PS_INPUT VertexToPixelLit

//PS
#include "../Common/Pixel/PixelShader3DHeader.hlsli"


// 定数バッファ：スロット4番目(b4と書く)
cbuffer cbParam : register(b4)
{
    float4 g_color;
    
    float3 g_light_dir;     //ライト方向
    float dummy;
    
    float4 g_ambient_color; //環境光の色
    
    float4 g_fog_color;     //霧の色
    
    float3 g_camera_pos;    //カメラの位置
    float g_time;           //ダミー   
}

float4 main(PS_INPUT PSInput) : SV_TARGET
{
    float4 color;
    color = diffuseMapTexture.Sample(diffuseMapSampler, PSInput.uv);
    if (color.a < 0.01f) discard; // アルファ値が0のピクセルは破棄)
    
    color *= g_color; //定数バッファの色を乗算
    
    float3 normal = PSInput.normal;
    //ライト
    float lihgt = dot(normal, -g_light_dir);
    
    //リムライト
    //視線方向（カメラ→ピクセル）
    float3 viewDir = normalize(g_camera_pos - PSInput.worldPos);
    //基本リム値
    float rim = 1.0 - saturate(dot(viewDir, normal));
    //鋭くする（数値を大きくすると細くなる）
    rim = pow(rim, 6.0);
    //時間で揺らぐ（sinで脈動）
    float pulse = 0.7 + 0.3 * sin(g_time * 5.0);
    //ノイズ風のゆらぎ（UVベース）
    float noise = frac(sin(dot(PSInput.uv * g_time, float2(12.9898, 78.233))) * 43758.5453);
    rim *= lerp(0.8, 1.5, noise);
    //赤い発光色
    float3 rimColor = float3(1.0, 0.0, 0.0);
    
    //色の合成
    float3 rgb = (color.rgb * g_color.rgb * lihgt) + g_ambient_color.rgb;
    ////リムライト成分も合成
    //rgb += rim * rimColor * pulse * 2.0;
    //rgb.r += 0.1;
    
    //霧
    float fogFactor = PSInput.fogFactor;
    float3 fogCol = g_fog_color.rgb;
    //霧の合成
    float3 finalColor = lerp(fogCol, rgb, fogFactor);
    

    //V座標は下に行くほど値が大きくなるので、
    //フェード開始位置からの距離を計算し、フェード率を求める
    float fadeStart = 0.2f; //フェード開始
    float fadeEnd = 1.0f;   //フェード終了
    float alphaFactor = 1.0f - saturate((PSInput.uv.y - fadeStart) / (fadeEnd - fadeStart));
    //alphaFactor = pow(alphaFactor, 2.0f);
    // 元々のテクスチャやマテリアルが持つアルファ値に、フェード率を乗算
    float finalAlpha = color.a * alphaFactor;
    
    return float4(finalColor, finalAlpha);
}