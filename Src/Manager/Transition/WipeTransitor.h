#pragma once
#include "./Transitor.h"
class WipeTransitor : public Transitor
{
public:

	WipeTransitor(TransitDirection direction);
	~WipeTransitor();

	void Update(void) override;
	void Draw(void) override;

private:
	int gradationH_;
};

