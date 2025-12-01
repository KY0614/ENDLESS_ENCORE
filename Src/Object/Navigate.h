#pragma once
#include "ActorBase.h"
class Navigate : public ActorBase
{
public:
	Navigate(void);
	~Navigate(void);

	/// <summary>
	///	‰Šú‰»
	/// </summary>
	void Init(void) override;

	/// <summary>
	///	XVˆ—
	/// </summary>
	void Update(void) override;

	/// <summary>
	/// •`‰æˆ—
	/// </summary>
	void Draw(void) override;

private:
};

