#include "../Common/Pixel/PixelShader2DHeader.hlsli"

// 定数バッファ：スロット4番目(b4と書く)
cbuffer cbParam : register(b4)
{
    float g_black_uv_x;
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

    float diffNoiseLine = abs(PSInput.uv.x - g_black_uv_x);
    if (diffNoiseLine < 0.001f)
    {
        dstCol = (0.0f, 0.0f, 0.0f);
    }
    
    return dstCol;
}