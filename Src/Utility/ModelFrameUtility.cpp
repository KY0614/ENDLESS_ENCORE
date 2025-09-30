#include "ModelFrameUtility.h"

void ModelFrameUtility::SetFrameLocalMatrixPos(int modelId, int frameIdx, VECTOR& pos)
{
	VECTOR tmpScl;
	MATRIX tmpMatRot;
	VECTOR tmpPos;

	// 対象フレームのワールド行列を大きさ・回転・位置に分解してを取得する
	GetFrameLocalMatrix(modelId, frameIdx, tmpScl, tmpMatRot, tmpPos);

	// 合成
	MATRIX ret = MGetIdent();
	ret = MMult(ret, MGetScale(tmpScl));
	ret = MMult(ret, tmpMatRot);
	ret = MMult(ret, MGetTranslate(pos));

	//対象フレームにワールド行列をセット
	MV1SetFrameUserLocalMatrix(modelId, frameIdx, ret);
}

void ModelFrameUtility::GetFrameLocalMatrix(int modelId, int frameIdx, VECTOR& scl, MATRIX& matRot, VECTOR& pos)
{
	// 対象フレームのローカル行列を取得する
	auto mat = MV1GetFrameLocalMatrix(modelId, frameIdx);

	// 拡大縮小成分
	scl = MGetSize(mat);

	// 回転成分＋拡大縮小成分
	matRot = MGetRotElem(mat);
	//回転成分のみにする
	auto revScl = VGet(1.0f / scl.x, 1.0f / scl.y, 1.0f / scl.z);
	matRot = MMult(matRot, MGetScale(revScl));

	// 移動成分
	pos = MGetTranslateElem(mat);
}
