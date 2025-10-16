#pragma once
#include "Transitor.h"
class FadeTransitor : public Transitor
{
public:
	FadeTransitor(void);
	~FadeTransitor(void) = default;
	
	void Start(void);
	void Update(void) override;
	void Draw(void) override;
private:
};

