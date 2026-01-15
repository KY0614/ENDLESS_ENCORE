#include "../Common/Pixel/PixelShader2DHeader.hlsli"

// 定数バッファ：スロット4番目(b4と書く)
cbuffer cbParam : register(b4)
{
    float4 g_color; //モデルカラー
    
    float2 g_noise_uv;  //ノイズのUV座標
    float2 dummy;       //16バイトアライメント用ダミー
    
    float g_vine_pow;   //ビネットの強度
    float3 dummy3;      //16バイトアライメント用ダミー    
}

float4 main(PS_INPUT PSInput) : SV_TARGET
{
	// UV座標とテクスチャを参照して、最適な色を取得する
    float4 srcCol = tex.Sample(texSampler, PSInput.uv);
    if (srcCol.a < 0.01f)
    {
		// 描画しない(アルファテスト)
        discard;
    }

    float4 dstCol = srcCol;
    
    //黒線ノイズ----------------------------------------------------------
    float diffNoiseLine = abs(PSInput.uv.x - g_noise_uv.x);
    if (diffNoiseLine < 0.001f)
    {
        dstCol = (0.0f, 0.0f, 0.0f);
    }
    
    dstCol.rgb *= g_color.rgb;
    //セピア----------------------------------------------------------
    float3 sepia = dstCol.rgb;
    sepia.r = dot(dstCol.rgb, float3(0.393f, 0.769f, 0.189f));
    sepia.g = dot(dstCol.rgb, float3(0.349f, 0.686f, 0.168f));
    sepia.b = dot(dstCol.rgb, float3(0.272f, 0.534f, 0.131f));
    dstCol.rgb = lerp(dstCol.rbg, sepia, 1.0f);

    //ビネット----------------------------------------------------------
    float2 uv = PSInput.uv;
    float x = abs(uv.x - 0.5f); // 0.0～1.0f → -0.5～0.5 → 0.0～0.5
    float y = abs(uv.y - 0.5f); // 0.0～1.0f → -0.5～0.5 → 0.0～0.5
    float dis = (x * x + y * y);

	//ビネットの強度を計算
    float vignette = 1.0f - saturate(dis * g_vine_pow);

    //色にビネットを乗算
    dstCol.rgb *= vignette;
    
    return dstCol;
}