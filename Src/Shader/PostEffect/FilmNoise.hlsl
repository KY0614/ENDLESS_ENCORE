#include "../Common/Pixel/PixelShader2DHeader.hlsli"

// 定数バッファ：スロット4番目(b4と書く)
cbuffer cbParam : register(b4)
{
    float2 g_black_uv;
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

    float diffNoiseLine = abs(PSInput.uv.x - g_black_uv.x);
    if (diffNoiseLine < 0.001f)
    {
        dstCol = (0.0f, 0.0f, 0.0f);
    }

    float noiseScale = 0.1f;
    float2 localUV = (PSInput.uv - g_black_uv) / noiseScale + 0.5f;

    // 画像の範囲内だけ処理
    if (localUV.x >= 0.0f && localUV.x <= 1.0f && localUV.y >= 0.0f && localUV.y <= 1.0f)
    {
        //ノイズ用の画像
        float4 noiseCol = noiseTexture.Sample(texSampler, localUV);

        // もし画像にアルファがある場合はこちらを試してください
        dstCol.rgb = lerp(dstCol.rgb, noiseCol.rgb, noiseCol.a);
    }

    return dstCol;
}