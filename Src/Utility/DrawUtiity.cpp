#include <DxLib.h>
#include <array>
#include "DrawUtiity.h"
using namespace std;
void DrawUtiity::DrawGraph(int x, int y, int tex, bool alpha, int secondTex)
{
	int gw, gh;
	GetGraphSize(tex, &gw, &gh);
	array<VERTEX2DSHADER, 4> vertices = {};
	for (auto& v : vertices) {
		v.dif = { 0xff,0xff,0xff,0xff };
		v.spc = { 0xff,0xff,0xff,0xff };
		v.rhw = 1.0f;//必ず１
		v.su = 0.0f;
		v.sv = 0.0f;
		v.pos.z = 0.1f;
	}
	//左上
	vertices[0].pos.x = x;
	vertices[0].pos.y = y;
	vertices[0].u = 0.0f;
	vertices[0].v = 0.0f;

	//右上
	vertices[1].pos.x = x + gw;
	vertices[1].pos.y = y;
	vertices[1].u = 1.0f;
	vertices[1].v = 0.0f;

	//左下
	vertices[2].pos.x = x;
	vertices[2].pos.y = y + gh;
	vertices[2].u = 0.0f;
	vertices[2].v = 1.0f;

	//右下
	vertices[3].pos.x = x + gw;
	vertices[3].pos.y = y + gh;
	vertices[3].u = 1.0f;
	vertices[3].v = 1.0f;

	SetUseTextureToShader(0, tex);
	if (secondTex != -1) {
		SetUseTextureToShader(1, tex);
	}
	DrawPrimitive2DToShader(vertices.data(), vertices.size(),
		DX_PRIMTYPE_TRIANGLESTRIP);
}
void DrawUtiity::DrawGraph(int x, int y, int psH, int tex, bool alpha, int secondTex)
{
	int gw, gh;
	GetGraphSize(tex, &gw, &gh);
	array<VERTEX2DSHADER, 4> vertices = {};
	for (auto& v : vertices) {
		v.dif = { 0xff,0xff,0xff,0xff };
		v.spc = { 0xff,0xff,0xff,0xff };
		v.rhw = 1.0f;//必ず１
		v.su = 0.0f;
		v.sv = 0.0f;
		v.pos.z = 0.1f;
	}
	//左上
	vertices[0].pos.x = x;
	vertices[0].pos.y = y;
	vertices[0].u = 0.0f;
	vertices[0].v = 0.0f;

	//右上
	vertices[1].pos.x = x + gw;
	vertices[1].pos.y = y;
	vertices[1].u = 1.0f;
	vertices[1].v = 0.0f;

	//左下
	vertices[2].pos.x = x;
	vertices[2].pos.y = y + gh;
	vertices[2].u = 0.0f;
	vertices[2].v = 1.0f;

	//右下
	vertices[3].pos.x = x + gw;
	vertices[3].pos.y = y + gh;
	vertices[3].u = 1.0f;
	vertices[3].v = 1.0f;

	SetUseTextureToShader(0, tex);
	if (secondTex != -1) {
		SetUseTextureToShader(1, tex);
	}
	SetUsePixelShader(psH);
	DrawPrimitive2DToShader(vertices.data(), vertices.size(),
		DX_PRIMTYPE_TRIANGLESTRIP);
}
