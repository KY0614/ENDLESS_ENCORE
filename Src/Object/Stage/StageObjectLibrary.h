#pragma once
#include <string>
#include <vector>
#include <unordered_map>


class StageObjectLibrary
{
public:

    struct ObjectParams {
        std::string id_ = "";               //識別子
        std::string name_ = "";             //名前
        bool placeable_ = false;            //上に物を置けるか（例：机）
        bool carryable_ = false;            //持ち運びできるか（例：カップ）
        bool interactable_ = false;         //プレイヤーがアクション可能か

        std::string interactType_;          //"place", "machine", "serve", "charge", など動作タイプ（任意）
        float interactTime_ = 0.0f;          //インタラクトにかかる時間（例：注ぐ時間）

        std::vector<std::string> acceptedItems_;   //インタラクト対象にできるアイテム（例：コップだけなど）
        std::vector<std::string> produces_;        //対象アイテムを受け取った後、渡すアイテム

        // ゲーム的なカテゴリ（分類）例："machine", "item", "counter"
        std::string category_;

    };

    static std::pair<std::string, ObjectParams> LoadData(const std::string& objName);

private:

};

