#pragma once

class ColliderBase
{
public:
	//å`èÛÇÃéÌóﬁ
	enum class SHAPE
	{
		SPHERE,
		CAPSULE,
		BOX,
		LINE,
		MODEL,
	};
	//É^ÉOÇÃéÌóﬁ
	enum class TAG
	{
		NONE,
		PLAYER,
		ENEMY,
		PLAYER_BULLET,
		ENEMY_BULLET,
		WALL,
	};

	//ColliderBase(SHAPE shape, TAG tag, Transform transform);
	~ColliderBase();
};

