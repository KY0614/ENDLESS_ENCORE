#pragma once
#include <DxLib.h>

class ModelFrameUtility
{
public:

	static void SetFrameLocalMatrixPos(int modelId, int frameIdx, VECTOR& pos);

	static void GetFrameLocalMatrix(int modelId, int frameIdx,
		VECTOR& scl, MATRIX& matRot, VECTOR& pos);
};

