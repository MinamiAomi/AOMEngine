#include "Enemy.h"

#include "Graphics/ResourceManager.h"

struct PartInit {
    Vector3 translate;
    Quaternion rotate;
};

PartInit partInits[] = {
    {{0.0f, 1.0f, 0.0f},{}},
    {{0.0f, 1.0f, 0.0f},{}}
};

void Enemy::Initialize(const Vector3& basePosition) {
    SetName("Enemy");

    transform.translate = basePosition + Vector3{ 1.5f, 0.0f, 0.0f };
    transform.rotate = Quaternion::identity;
    transform.scale = Vector3::one * 0.5f;

    const char* partModelName[] = {
        "MimicBody",
        "MimicHead"
    };
    ResourceManager* resourceManager = ResourceManager::GetInstance();
    
    for (size_t i = 0; i < static_cast<size_t>(Part::NumParts); ++i) {
        parts_[i] = std::make_unique<PartData>();
        parts_[i]->model.SetModel(resourceManager->FindModel(partModelName[i]));
        parts_[i]->model.SetUseOutline(false);
        parts_[i]->transform.SetParent(&transform);
        parts_[i]->transform.translate = partInits[i].translate;
        parts_[i]->transform.rotate = partInits[i].rotate;
        parts_[i]->transform.UpdateMatrix();
    }

    collider_ = std::make_unique<BoxCollider>();
    collider_->SetGameObject(this);
    collider_->SetName("Enemy");
    collider_->SetSize(Vector3::one);
}

void Enemy::Update() {

    transform.rotate = Quaternion::MakeForYAxis(-4.0f * Math::ToRadian) * transform.rotate;

    const float moveSpeed = 0.1f;
    Vector3 move = { 0.0f,0.0f,1.0f };
    move = transform.rotate * move * moveSpeed;
    transform.translate += move;

    transform.UpdateMatrix();

    UpdateAnimation();

    for (size_t i = 0; i < static_cast<size_t>(Part::NumParts); ++i) {
        parts_[i]->transform.UpdateMatrix();
        parts_[i]->model.SetWorldMatrix(parts_[i]->transform.worldMatrix);
    }

    collider_->SetCenter(transform.translate + Vector3{ 0.0f,0.5f,0.0f });
    collider_->SetOrientation(transform.rotate);
}

void Enemy::UpdateAnimation() {
    // 開けて閉じる周期
    const uint16_t cycle = 240;
    // 1フレーム変化量
    const float delta = Math::TwoPi / cycle;

    animationParameter_ += delta;
    animationParameter_ = std::fmod(animationParameter_, Math::TwoPi);

    // 開閉最大角
    const Quaternion openLimitRotate = Quaternion::MakeForXAxis(-45.0f * Math::ToRadian);

    animationParameter_ += delta;
    float triangleWave = std::abs(std::sin(animationParameter_));
    parts_[static_cast<size_t>(Part::Head)]->transform.rotate = Quaternion::Slerp(triangleWave, Quaternion::identity, openLimitRotate);
}