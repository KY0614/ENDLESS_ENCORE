#pragma once
class Transitor
{
public:
	enum class TransitDirection
	{
		TOP,
		DOWN,
		LEFT,
		RIGHT
	};

	Transitor(int interval = 60);
	~Transitor(void);

	virtual void Update(void) = 0;
	virtual void Draw(void) = 0;

	void Start(void);

	bool IsEnd(void) const { return frame_ >= interval_; }

protected:

	int oldRT_;
	int newRT_;

	int interval_;
	int frame_;
};

