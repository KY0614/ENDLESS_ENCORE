#pragma once
class Transitor
{
public:
	Transitor(void);
	~Transitor(void);

	void Update(void);
	void Draw(void);

	void Start(void);

private:

	int oldRT_;
	int newRT_;
};

