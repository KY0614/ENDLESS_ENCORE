#pragma once
#include <DxLib.h>

class ModelFrameUtility
{
public:

	static void SetFrameLocalMatrixPos(int modelId, int frameIdx, VECTOR& pos);

	static void GetFrameLocalMatrix(int modelId, int frameIdx,
		VECTOR& scl, MATRIX& matRot, VECTOR& pos);

	static void GetFrameAnimAttachLocalMatrix(int modelId,
		int attachNo, int frameIdx, VECTOR& scl, MATRIX& matRot, VECTOR& pos);

	static void SetFrameAnimAttachLocalMatrixPos(int modelId,
		int attachNo, int frameIdx, VECTOR& pos);
};

