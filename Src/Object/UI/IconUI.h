#pragma once
#include "../../Manager/Generic/ResourceManager.h"
#include "UIBase.h"

class IconUI : public UIBase
{
public:

    IconUI(VECTOR localPos,VECTOR parentPos,ResourceManager::SRC src);
    ~IconUI() = default;

    void Init(void) override;
    void Update(void) override;
    void Draw(void) override;
    void SetUISize(const float size) { size_ = size; }

protected:
    VECTOR localPos_;
    VECTOR parentPos_;
    float size_;

    ResourceManager::SRC imgSrc_;
};

