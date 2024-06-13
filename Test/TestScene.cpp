#include "TestScene.h"

#include "Framework/Engine.h"
#include "Input/Input.h"
#include "Graphics/RenderManager.h"
#include "Framework/ResourceManager.h"
#include "Graphics/Core/TextureLoader.h"
#include "LevelLoader.h"

void TestScene::OnInitialize() {

    camera_ = std::make_shared<Camera>();
    camera_->SetPosition({ 0.0f, 1.0f, -5.0f });
    camera_->SetRotate(Quaternion::MakeLookRotation({ 0.0f, -1.0f, 5.0f }));
    camera_->UpdateMatrices();
    RenderManager::GetInstance()->SetCamera(camera_);

    sunLight_ = std::make_shared<DirectionalLight>();
    sunLight_->direction = Vector3(1.0f, -1.0f, 1.0f).Normalized();
    RenderManager::GetInstance()->SetSunLight(sunLight_);

    //door_.Initialize();

    auto& skybox = RenderManager::GetInstance()->GetSkybox();
    skybox.SetTexture(TextureLoader::Load("Resources/skybox/skybox_8k.dds"));
    auto& lightingRenderlingPass = RenderManager::GetInstance()->GetLightingRenderingPass();
    lightingRenderlingPass.SetIrradianceTexture(TextureLoader::Load("Resources/skybox/skybox_8k_irradiance.dds"));
    lightingRenderlingPass.SetRadianceTexture(TextureLoader::Load("Resources/skybox/skybox_8k_radiance.dds"));

    Vector3 offset = { 40.0f, 0.0f, 0.0f };
    auto sphereModel = ResourceManager::GetInstance()->FindModel("sphere");
    for (uint32_t row = 0; row < kRowCount; ++row) {
        for (uint32_t column = 0; column < kColumnCount; ++column) {
            auto& sphere = spheres_[row][column];
            sphere.material = std::make_shared<PBRMaterial>();
            sphere.material->albedo = { 1.0f, 1.0f, 1.0f };
            sphere.material->metallic = (float)row * 0.1f;
            sphere.material->roughness = (float)column * 0.1f;
            sphere.model.SetModel(sphereModel);
            sphere.model.SetMaterial(sphere.material);
            sphere.model.SetWorldMatrix(Matrix4x4::MakeTranslation(Vector3{ -15.0f + column * 3.0f, -15.0f + row * 3.0f, 0.0f } + offset));
        }
    }

    euler_.x = Math::ToRadian;

    model_.SetModel(ResourceManager::GetInstance()->FindModel("human"));
    walk_ = ResourceManager::GetInstance()->FindAnimation("human_walk");
    skeleton_ = std::make_shared<Skeleton>();
    skeleton_->Create(model_.GetModel());
    model_.SetSkeleton(skeleton_);
    model_.SetWorldMatrix(Matrix4x4::MakeRotationY(Math::ToRadian * 180.0f));

    //room_.SetModel(ResourceManager::GetInstance()->FindModel("room"));

    LevelLoader::Load("Resources/scene.json", *Engine::GetGameObjectManager());
}

void TestScene::OnUpdate() {

    Engine::GetGameObjectManager()->Update();

    //door_.Update();

    time_ += 1.0f / 60.0f;
    time_ = std::fmod(time_, 1.0f);
    skeleton_->ApplyAnimation(walk_->GetAnimation("situp"), time_);
    skeleton_->Update();
    skeleton_->DebugDraw(model_.GetWorldMatrix());

    Input* input = Input::GetInstance();

    auto mouseMoveX = input->GetMouseMoveX();
    auto mouseMoveY = input->GetMouseMoveY();
    auto wheel = input->GetMouseWheel();

    Quaternion rotate = camera_->GetRotate();
    Vector3 position = camera_->GetPosition();

    Vector3 diffPosition;

    if (input->IsMousePressed(1)) {
        constexpr float rotSpeed = Math::ToRadian * 0.1f;
        euler_.x += rotSpeed * static_cast<float>(mouseMoveY);
        euler_.y += rotSpeed * static_cast<float>(mouseMoveX);
    }
    else if (input->IsMousePressed(2)) {
        Vector3 cameraX = rotate.GetRight() * (-static_cast<float>(mouseMoveX) * 0.01f);
        Vector3 cameraY = rotate.GetUp() * (static_cast<float>(mouseMoveY) * 0.01f);
        diffPosition += cameraX + cameraY;
    }
    else if (wheel != 0) {
        Vector3 cameraZ = rotate.GetForward() * (static_cast<float>(wheel / 120) * 0.5f);
        diffPosition += cameraZ;
    }


    camera_->SetPosition(position + diffPosition);
    camera_->SetRotate(Quaternion::MakeFromEulerAngle(euler_));
    camera_->UpdateMatrices();

    //sunLight_->DrawImGui("SunLight");
    //testObject_.DrawImGui("Sphere");
}

void TestScene::OnFinalize() {

}
