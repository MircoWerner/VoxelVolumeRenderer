#include "VolumeRenderLogic.h"

#include "../../../lib/imgui/imgui.h"
#include "../renderengine/utils/IOUtils.h"

void VolumeRenderLogic::init() {
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    for (auto &m_startupParameter: m_startupParameters) {
        Volume::VOLUME_DATA_SET volumeDataSet = Volume::parseVolumeDataSet(m_startupParameter);
        if (volumeDataSet != Volume::INVALID) {
            m_volumeRenderer.m_volume.m_volumeDataSet = volumeDataSet;
            break;
        }
    }
    m_volumeRenderer.init(windowSize.x, windowSize.y);
    m_coordinateSystemRenderer.init();
    switch (m_volumeRenderer.m_volume.m_volumeDataSet) {
        case Volume::V800:
            teleportPos(Position800Far);
            break;
        case Volume::V400:
            teleportPos(Position400Far);
            m_volumeRenderer.m_ambientOcclusion.m_vdcao.m_coneTraceSteps = 16;
            m_volumeRenderer.m_ambientOcclusion.m_vdcao.m_coneTraceStepSize = 0.5f;
            m_volumeRenderer.m_ambientOcclusion.m_vdcao.m_coneApertureAngle = 30.f;
            m_volumeRenderer.m_ambientOcclusion.m_vdcao.m_attenuation = 16.f;
            m_volumeRenderer.m_ambientOcclusion.m_vctao.m_coneTraceSteps = 32;
            m_volumeRenderer.m_ambientOcclusion.m_vctao.m_coneTraceStepSize = 0.25f;
            m_volumeRenderer.m_ambientOcclusion.m_vctao.m_coneApertureAngle = 30.f;
            m_volumeRenderer.m_ambientOcclusion.m_vctao.m_attenuation = 1024.f;
            break;
        default:
            break;
    }
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << "[STARTUP_Total] Time = "
              << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()
              << "[µs]" << std::endl;
}

void VolumeRenderLogic::update(float time, KeyboardInput *keyboardInput, MouseInput *mouseInput) {
    static const float CAMERA_POS_STEP = 3.f;
    static const float MOUSE_SENSITIVITY = 0.25f;

    float factor = 1.f;
    if (keyboardInput->isKeyPressed(GLFW_KEY_LEFT_CONTROL)) {
        factor = 25.f;
    }

    glm::vec3 cameraInc(0.f);
    if (keyboardInput->isKeyPressed(GLFW_KEY_W)) {
        cameraInc.z -= factor;
        m_executeComputeShader = true;
    }
    if (keyboardInput->isKeyPressed(GLFW_KEY_S)) {
        cameraInc.z += factor;
        m_executeComputeShader = true;
    }
    if (keyboardInput->isKeyPressed(GLFW_KEY_A)) {
        cameraInc.x -= factor;
        m_executeComputeShader = true;
    }
    if (keyboardInput->isKeyPressed(GLFW_KEY_D)) {
        cameraInc.x += factor;
        m_executeComputeShader = true;
    }
    if (keyboardInput->isKeyPressed(GLFW_KEY_LEFT_SHIFT)) {
        cameraInc.y -= factor;
        m_executeComputeShader = true;
    }
    if (keyboardInput->isKeyPressed(GLFW_KEY_SPACE)) {
        cameraInc.y += factor;
        m_executeComputeShader = true;
    }

    wireframe = keyboardInput->isKeyPressed(GLFW_KEY_T);

    m_camera.moveCenter(cameraInc.x * CAMERA_POS_STEP * time, cameraInc.y * CAMERA_POS_STEP * time,
                        cameraInc.z * CAMERA_POS_STEP * time);

    if (mouseInput->isRightButtonPressed()) {
        glm::vec2 motion = mouseInput->getMotion();
        m_camera.move(0.0f, motion.y * MOUSE_SENSITIVITY * time, -motion.x * MOUSE_SENSITIVITY * time);
        m_executeComputeShader = true;
    }
    if (mouseInput->getScroll(false).y != 0) {
        m_camera.move(-mouseInput->getScroll(true).y, 0.f, 0.f);
        m_executeComputeShader = true;
    }
}

void VolumeRenderLogic::render() {
    if (wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // enable wireframe rendering
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    m_volumeRenderer.render(&m_camera, windowSize.x, windowSize.y, m_executeComputeShader);
    m_executeComputeShader = false;

    m_coordinateSystemRenderer.render(&m_camera, windowSize.x, windowSize.y);
}

void VolumeRenderLogic::renderGui() {
    ImGuiWindowFlags windowFlags = 0;
    windowFlags |= ImGuiWindowFlags_MenuBar;

    static bool guiOpen = true;
    ImGui::Begin("Voxel Volume Renderer", &guiOpen, windowFlags);

    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("Menu")) {
            ImGui::MenuItem("Help", nullptr, nullptr);
            ImGui::MenuItem("About", nullptr, nullptr);
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    ImGui::PushItemWidth(-ImGui::GetWindowWidth() * 0.5f);

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                ImGui::GetIO().Framerate);

    ImGui::Spacing();

    ImGui::Text("Cell (X,Y,Z) = (%i,%i,%i)", static_cast<int>(glm::floor(m_camera.getPosition().x)),
                static_cast<int>(glm::floor(m_camera.getPosition().y)),
                static_cast<int>(glm::floor(m_camera.getPosition().z)));
    ImGui::Text("Position (X,Y,Z) = (%.3f,%.3f,%.3f)", m_camera.getPosition().x, m_camera.getPosition().y,
                m_camera.getPosition().z);
    ImGui::Text("Rotation (R,Phi,Theta) = (%.3f,%.3f,%.3f)", m_camera.getR(), m_camera.getPhi(),
                m_camera.getTheta());

    ImGui::NewLine();

    if (ImGui::CollapsingHeader("Teleport")) {
        if (ImGui::Button("Center")) {
            m_camera.setCenter(glm::vec3(m_volumeRenderer.m_volume.m_volumeDimension) * glm::vec3(0.5f));
            m_executeComputeShader = true;
        }

        if (ImGui::Button("800^3, far")) {
            teleportPos(Position800Far);
        }
        if (ImGui::Button("800^3, near")) {
            teleportPos(Position800Near);
        }
        if (ImGui::Button("800^3, debug 1")) {
            teleportPos(Position800Debug1);
        }
        if (ImGui::Button("800^3, debug 2")) {
            teleportPos(Position800Debug2);
        }
        if (ImGui::Button("400^3, far")) {
            teleportPos(Position400Far);
        }
//        if (ImGui::Button("800^3, filter")) {
//            teleportPos(5);
//        }

        ImGui::Spacing();
    }

    if (ImGui::CollapsingHeader("Ambient Occlusion")) {
        const char *items[] = {"Disabled", "Ray Traced AO (RTAO)", "Distance Field AO (DFAO)", "Local Voxel AO (LVAO)",
                               "Voxel Distance Field Cone Traced Ambient Occlusion (VDCAO)",
                               "Horizon-Based Ambient Occlusion (HBAO)", "Voxel Cone Traced Ambient Occlusion (VCTAO)"};
        if (ImGui::Combo("AO technique", &m_volumeRenderer.m_ambientOcclusion.m_method, items, IM_ARRAYSIZE(items))) {
            m_executeComputeShader = true;
        }
        ImGui::Spacing();

        if (m_volumeRenderer.m_ambientOcclusion.m_method == 1) {
            ImGui::Text("Accumulated samples: %i/%i", m_volumeRenderer.m_ambientOcclusion.m_rtao.m_accumulatedSamples,
                        m_volumeRenderer.m_ambientOcclusion.m_rtao.m_totalSamples);
            if (ImGui::InputInt("Samples per frame", &m_volumeRenderer.m_ambientOcclusion.m_rtao.m_samples)) {
                m_executeComputeShader = true;
            }
            if (ImGui::InputInt("Samples total", &m_volumeRenderer.m_ambientOcclusion.m_rtao.m_totalSamples)) {
                m_executeComputeShader = true;
            }
            if (ImGui::InputFloat("Distance to half occlusion",
                                  &m_volumeRenderer.m_ambientOcclusion.m_rtao.m_distanceToHalfOcclusion)) {
                m_executeComputeShader = true;
            }
        } else if (m_volumeRenderer.m_ambientOcclusion.m_method == 2) {
            if (ImGui::InputInt("Number of steps", &m_volumeRenderer.m_ambientOcclusion.m_dfao.m_numberOfSteps)) {
                m_executeComputeShader = true;
            }
            if (ImGui::InputFloat("Contrast", &m_volumeRenderer.m_ambientOcclusion.m_dfao.m_contrast)) {
                m_executeComputeShader = true;
            }
        } else if (m_volumeRenderer.m_ambientOcclusion.m_method == 3) {
            // nothing
        } else if (m_volumeRenderer.m_ambientOcclusion.m_method == 4) {
            if (ImGui::InputInt("Cone steps", &m_volumeRenderer.m_ambientOcclusion.m_vdcao.m_coneTraceSteps)) {
                m_executeComputeShader = true;
            }
            if (ImGui::InputFloat("Cone step size", &m_volumeRenderer.m_ambientOcclusion.m_vdcao.m_coneTraceStepSize)) {
                m_executeComputeShader = true;
            }
            if (ImGui::InputFloat("Cone aperture angle",
                                  &m_volumeRenderer.m_ambientOcclusion.m_vdcao.m_coneApertureAngle)) {
                m_executeComputeShader = true;
            }
            if (ImGui::InputFloat("Attenuation", &m_volumeRenderer.m_ambientOcclusion.m_vdcao.m_attenuation)) {
                m_executeComputeShader = true;
            }
        } else if (m_volumeRenderer.m_ambientOcclusion.m_method == 5) {
            if (ImGui::InputInt("Number of directions (Nd)", &m_volumeRenderer.m_ambientOcclusion.m_hbao.m_Nd)) {
                m_executeComputeShader = true;
            }
            if (ImGui::InputInt("Number of steps (Ns)", &m_volumeRenderer.m_ambientOcclusion.m_hbao.m_Ns)) {
                m_executeComputeShader = true;
            }
            if (ImGui::InputFloat("Step size", &m_volumeRenderer.m_ambientOcclusion.m_hbao.m_stepSize)) {
                m_executeComputeShader = true;
            }
            if (ImGui::InputFloat("Radius of influence (R)", &m_volumeRenderer.m_ambientOcclusion.m_hbao.m_R)) {
                m_executeComputeShader = true;
            }
            if (ImGui::InputFloat("Tangent bias", &m_volumeRenderer.m_ambientOcclusion.m_hbao.m_tangentBias)) {
                m_executeComputeShader = true;
            }
            if (ImGui::Checkbox("Execute bilateral filter",
                                &m_volumeRenderer.m_ambientOcclusion.m_hbao.m_executeBilateralFilter)) {
                m_executeComputeShader = true;
            }
        } else if (m_volumeRenderer.m_ambientOcclusion.m_method == 6) {
            if (ImGui::InputInt("Cone steps",
                                &m_volumeRenderer.m_ambientOcclusion.m_vctao.m_coneTraceSteps)) {
                m_executeComputeShader = true;
            }
            if (ImGui::InputFloat("Cone step size",
                                  &m_volumeRenderer.m_ambientOcclusion.m_vctao.m_coneTraceStepSize)) {
                m_executeComputeShader = true;
            }
            if (ImGui::InputFloat("Cone aperture angle",
                                  &m_volumeRenderer.m_ambientOcclusion.m_vctao.m_coneApertureAngle)) {
                m_executeComputeShader = true;
            }
            if (ImGui::InputFloat("Attenuation",
                                  &m_volumeRenderer.m_ambientOcclusion.m_vctao.m_attenuation)) {
                m_executeComputeShader = true;
            }
        }

        ImGui::Spacing();
    }

    if (ImGui::CollapsingHeader("Emission")) {
        const char *items[] = {"Disabled", "Voxel Cone Traced Emission (VCTE)",
                               "Directional RGB Distance Field Emission (DDFE)"};
        if (ImGui::Combo("Emission technique", &m_volumeRenderer.m_emission.m_method, items, IM_ARRAYSIZE(items))) {
            m_executeComputeShader = true;
        }
        ImGui::InputFloat("Emission factor", &m_volumeRenderer.m_emissionFactor);
        ImGui::Spacing();
        if (m_volumeRenderer.m_emission.m_method == 1) {
            if (ImGui::Checkbox("Diffuse cones",
                                &m_volumeRenderer.m_emission.m_vcte.m_diffuse)) {
                m_executeComputeShader = true;
            }
            if (ImGui::InputInt("Diffuse cones steps",
                                &m_volumeRenderer.m_emission.m_vcte.m_diffuseConeTraceSteps)) {
                m_executeComputeShader = true;
            }
            if (ImGui::InputFloat("Diffuse cones step size",
                                  &m_volumeRenderer.m_emission.m_vcte.m_diffuseConeTraceStepSize)) {
                m_executeComputeShader = true;
            }
            if (ImGui::InputFloat("Diffuse cones aperture angle",
                                  &m_volumeRenderer.m_emission.m_vcte.m_diffuseConeApertureAngle)) {
                m_executeComputeShader = true;
            }
            ImGui::Spacing();
            if (ImGui::Checkbox("Specular cones",
                                &m_volumeRenderer.m_emission.m_vcte.m_specular)) {
                m_executeComputeShader = true;
            }
            if (ImGui::InputInt("Specular cones steps",
                                &m_volumeRenderer.m_emission.m_vcte.m_specularConeTraceSteps)) {
                m_executeComputeShader = true;
            }
            if (ImGui::InputFloat("Specular cones step size",
                                  &m_volumeRenderer.m_emission.m_vcte.m_specularConeTraceStepSize)) {
                m_executeComputeShader = true;
            }
            if (ImGui::InputFloat("Specular Cone Aperture Angle",
                                  &m_volumeRenderer.m_emission.m_vcte.m_specularConeApertureAngle)) {
                m_executeComputeShader = true;
            }
        }

        ImGui::Spacing();
    }

    if (ImGui::CollapsingHeader("Bilateral Filter")) {
        if (ImGui::Button("Filter AO")) {
            m_volumeRenderer.executeBilateralFilter(windowSize.x, windowSize.y);
        }

        ImGui::InputInt("Kernel radius", &m_volumeRenderer.m_bilateralFilter.m_kernelRadius);
        ImGui::InputFloat("Sigma spatial", &m_volumeRenderer.m_bilateralFilter.m_sigmaSpatial);

        ImGui::Spacing();
    }

    if (ImGui::CollapsingHeader("Cell Properties")) {
        static int m_cellPropertyId = 0;
        ImGui::InputInt("Cell-ID", &m_cellPropertyId);
        if (m_cellPropertyId >= 0 &&
            m_cellPropertyId < m_volumeRenderer.m_volume.m_cellProperties->m_properties.size() &&
            m_volumeRenderer.m_volume.m_cellProperties->m_properties[m_cellPropertyId].valid) {
            if (ImGui::ColorEdit4("RGBA",
                                  m_volumeRenderer.m_volume.m_cellProperties->m_properties[m_cellPropertyId].rgba)) {
                m_volumeRenderer.m_volume.reloadCellPropertiesBuffer(m_cellPropertyId);
                m_executeComputeShader = true;
            }
            if (ImGui::InputFloat("Emittance",
                                  &m_volumeRenderer.m_volume.m_cellProperties->m_properties[m_cellPropertyId].emittance)) {
                m_volumeRenderer.m_volume.reloadCellPropertiesBuffer(m_cellPropertyId);
                m_executeComputeShader = true;
            }
            if (ImGui::InputFloat("Roughness",
                                  &m_volumeRenderer.m_volume.m_cellProperties->m_properties[m_cellPropertyId].roughness)) {
                m_volumeRenderer.m_volume.reloadCellPropertiesBuffer(m_cellPropertyId);
                m_executeComputeShader = true;
            }
        }
        if (ImGui::Button("Save properties to file")) {
            m_volumeRenderer.m_volume.saveCellProperties();
        }
        if (ImGui::Button("Recalculate VCTGI information")) {
            m_volumeRenderer.m_volume.recalculateEmissionInformation();
            m_executeComputeShader = true;
        }

        ImGui::Spacing();
    }

    if (ImGui::CollapsingHeader("Debug")) {
        const char *items[] = {"Disabled", "Normal", "Depth", "Albedo", "Ambient Occlusion", "Emission"};
        ImGui::Combo("Debug framebuffer", &m_volumeRenderer.m_fragmentDebugMode, items, IM_ARRAYSIZE(items));

        ImGui::Checkbox("Render Coordinate System", &m_coordinateSystemRenderer.m_render);
        if (ImGui::Button("Reload Shaders")) {
            m_volumeRenderer.reloadVertexFragmentShader();
            m_volumeRenderer.reloadVolumeComputeShader();
            m_volumeRenderer.m_ambientOcclusion.reload();
            m_volumeRenderer.m_emission.reload();
            m_volumeRenderer.m_bilateralFilter.reload();
            m_executeComputeShader = true;
        }
        if (ImGui::Button("Screenshot")) {
            IOUtils::writeFramebufferToFile("screenshot.png", windowSize.x, windowSize.y);
        }

        if (ImGui::Checkbox("Render SDF", &m_volumeRenderer.m_debugSdf)) {
            m_executeComputeShader = true;
        }
        if (m_volumeRenderer.m_debugSdf) {
            if (ImGui::InputInt("SDF value", &m_volumeRenderer.m_debugSdfValue)) {
                m_executeComputeShader = true;
            }
        }

        ImGui::Spacing();
    }

//    ImGui::ShowDemoWindow();

    ImGui::End();
}

void VolumeRenderLogic::cleanUp() {
    m_volumeRenderer.cleanUp();
    m_coordinateSystemRenderer.cleanUp();
}

void VolumeRenderLogic::onWindowResized(int width, int height) {
    windowSize = glm::vec2(width, height);
    m_volumeRenderer.resizeFramebuffers(width, height);
    m_executeComputeShader = true;
}

bool VolumeRenderLogic::generateSceneScreenshot(int number, IRenderLogic::SceneScreenshotInfo *info) {
    info->width = 1920;
    info->height = 1080;
    info->iterations = 1;

    m_coordinateSystemRenderer.m_render = false;

// === EVALUATION 800 FAR ===
    switch (number) {
        case 0:
            teleportPos(Position800Far);

            m_volumeRenderer.m_ambientOcclusion.m_method = AmbientOcclusion::AO_RTAO;
            m_volumeRenderer.m_fragmentDebugMode = VolumeRenderer::FragmentDebugMode::DebugDisabled;
            m_volumeRenderer.m_emission.m_method = Emission::EM::EM_DISABLED;

            m_volumeRenderer.m_ambientOcclusion.m_rtao.m_samples = 8;
            m_volumeRenderer.m_ambientOcclusion.m_rtao.m_totalSamples = 1024;
            m_volumeRenderer.m_ambientOcclusion.m_rtao.m_distanceToHalfOcclusion = 256.f;

            info->fileName = "s0_rtao.png";
            info->iterations = 128;

            return true;
        case 1:
            teleportPos(Position800Far);

            m_volumeRenderer.m_ambientOcclusion.m_method = AmbientOcclusion::AO_RTAO;
            m_volumeRenderer.m_fragmentDebugMode = VolumeRenderer::FragmentDebugMode::DebugAmbientOcclusion;
            m_volumeRenderer.m_emission.m_method = Emission::EM::EM_DISABLED;

            m_volumeRenderer.m_ambientOcclusion.m_rtao.m_samples = 8;
            m_volumeRenderer.m_ambientOcclusion.m_rtao.m_totalSamples = 1024;
            m_volumeRenderer.m_ambientOcclusion.m_rtao.m_distanceToHalfOcclusion = 256.f;

            info->fileName = "s0_rtao_factor.png";
            info->iterations = 128;

            return true;
        case 2:
            teleportPos(Position800Far);

            m_volumeRenderer.m_ambientOcclusion.m_method = AmbientOcclusion::AO_VDCAO;
            m_volumeRenderer.m_fragmentDebugMode = VolumeRenderer::FragmentDebugMode::DebugDisabled;
            m_volumeRenderer.m_emission.m_method = Emission::EM::EM_DISABLED;

            info->fileName = "s0_vdcao.png";

            return true;
        case 3:
            teleportPos(Position800Far);

            m_volumeRenderer.m_ambientOcclusion.m_method = AmbientOcclusion::AO_VDCAO;
            m_volumeRenderer.m_fragmentDebugMode = VolumeRenderer::FragmentDebugMode::DebugAmbientOcclusion;
            m_volumeRenderer.m_emission.m_method = Emission::EM::EM_DISABLED;

            info->fileName = "s0_vdcao_factor.png";

            return true;
        case 4:
            teleportPos(Position800Far);

            m_volumeRenderer.m_ambientOcclusion.m_method = AmbientOcclusion::AO_VCTAO;
            m_volumeRenderer.m_fragmentDebugMode = VolumeRenderer::FragmentDebugMode::DebugDisabled;
            m_volumeRenderer.m_emission.m_method = Emission::EM::EM_DISABLED;

            info->fileName = "s0_vctao.png";

            return true;
        case 5:
            teleportPos(Position800Far);

            m_volumeRenderer.m_ambientOcclusion.m_method = AmbientOcclusion::AO_VCTAO;
            m_volumeRenderer.m_fragmentDebugMode = VolumeRenderer::FragmentDebugMode::DebugAmbientOcclusion;
            m_volumeRenderer.m_emission.m_method = Emission::EM::EM_DISABLED;

            info->fileName = "s0_vctao_factor.png";

            return true;
        case 6:
            teleportPos(Position800Far);

            m_volumeRenderer.m_ambientOcclusion.m_method = AmbientOcclusion::AO_LVAO;
            m_volumeRenderer.m_fragmentDebugMode = VolumeRenderer::FragmentDebugMode::DebugDisabled;
            m_volumeRenderer.m_emission.m_method = Emission::EM::EM_DISABLED;

            info->fileName = "s0_lvao.png";

            return true;
        case 7:
            teleportPos(Position800Far);

            m_volumeRenderer.m_ambientOcclusion.m_method = AmbientOcclusion::AO_LVAO;
            m_volumeRenderer.m_fragmentDebugMode = VolumeRenderer::FragmentDebugMode::DebugAmbientOcclusion;
            m_volumeRenderer.m_emission.m_method = Emission::EM::EM_DISABLED;

            info->fileName = "s0_lvao_factor.png";

            return true;
        case 8:
            teleportPos(Position800Far);

            m_volumeRenderer.m_ambientOcclusion.m_method = AmbientOcclusion::AO_DFAO;
            m_volumeRenderer.m_fragmentDebugMode = VolumeRenderer::FragmentDebugMode::DebugDisabled;
            m_volumeRenderer.m_emission.m_method = Emission::EM::EM_DISABLED;

            info->fileName = "s0_dfao.png";

            return true;
        case 9:
            teleportPos(Position800Far);

            m_volumeRenderer.m_ambientOcclusion.m_method = AmbientOcclusion::AO_DFAO;
            m_volumeRenderer.m_fragmentDebugMode = VolumeRenderer::FragmentDebugMode::DebugAmbientOcclusion;
            m_volumeRenderer.m_emission.m_method = Emission::EM::EM_DISABLED;

            info->fileName = "s0_dfao_factor.png";

            return true;
        case 10:
            teleportPos(Position800Far);

            m_volumeRenderer.m_ambientOcclusion.m_method = AmbientOcclusion::AO_HBAO;
            m_volumeRenderer.m_fragmentDebugMode = VolumeRenderer::FragmentDebugMode::DebugDisabled;
            m_volumeRenderer.m_emission.m_method = Emission::EM::EM_DISABLED;
            m_volumeRenderer.m_ambientOcclusion.m_hbao.m_executeBilateralFilter = true;

            info->fileName = "s0_hbao.png";

            return true;
        case 11:
            teleportPos(Position800Far);

            m_volumeRenderer.m_ambientOcclusion.m_method = AmbientOcclusion::AO_HBAO;
            m_volumeRenderer.m_fragmentDebugMode = VolumeRenderer::FragmentDebugMode::DebugAmbientOcclusion;
            m_volumeRenderer.m_emission.m_method = Emission::EM::EM_DISABLED;
            m_volumeRenderer.m_ambientOcclusion.m_hbao.m_executeBilateralFilter = true;

            info->fileName = "s0_hbao_factor.png";

            return true;
        case 12:
            teleportPos(Position800Far);

            m_volumeRenderer.m_ambientOcclusion.m_method = AmbientOcclusion::AO_HBAO;
            m_volumeRenderer.m_fragmentDebugMode = VolumeRenderer::FragmentDebugMode::DebugDisabled;
            m_volumeRenderer.m_emission.m_method = Emission::EM::EM_DISABLED;
            m_volumeRenderer.m_ambientOcclusion.m_hbao.m_executeBilateralFilter = false;

            info->fileName = "s0_hbao_wofilter.png";

            return true;
        case 13:
            teleportPos(Position800Far);

            m_volumeRenderer.m_ambientOcclusion.m_method = AmbientOcclusion::AO_HBAO;
            m_volumeRenderer.m_fragmentDebugMode = VolumeRenderer::FragmentDebugMode::DebugAmbientOcclusion;
            m_volumeRenderer.m_emission.m_method = Emission::EM::EM_DISABLED;
            m_volumeRenderer.m_ambientOcclusion.m_hbao.m_executeBilateralFilter = false;

            info->fileName = "s0_hbao_wofilter_factor.png";

            return true;
    }

// === EVALUATION 800 NEAR ===
//    switch (number) {
//        case 0:
//            teleportPos(Position800Near);
//
//            m_volumeRenderer.m_ambientOcclusion.m_method = AmbientOcclusion::AO_RTAO;
//            m_volumeRenderer.m_fragmentDebugMode = VolumeRenderer::FragmentDebugMode::DebugDisabled;
//            m_volumeRenderer.m_emission.m_method = Emission::EM::EM_DISABLED;
//
//            m_volumeRenderer.m_ambientOcclusion.m_rtao.m_samples = 8;
//            m_volumeRenderer.m_ambientOcclusion.m_rtao.m_totalSamples = 1024;
//            m_volumeRenderer.m_ambientOcclusion.m_rtao.m_distanceToHalfOcclusion = 256.f;
//
//            info->fileName = "s1_rtao.png";
//            info->iterations = 128;
//
//            return true;
//        case 1:
//            teleportPos(Position800Near);
//
//            m_volumeRenderer.m_ambientOcclusion.m_method = AmbientOcclusion::AO_RTAO;
//            m_volumeRenderer.m_fragmentDebugMode = VolumeRenderer::FragmentDebugMode::DebugAmbientOcclusion;
//            m_volumeRenderer.m_emission.m_method = Emission::EM::EM_DISABLED;
//
//            m_volumeRenderer.m_ambientOcclusion.m_rtao.m_samples = 8;
//            m_volumeRenderer.m_ambientOcclusion.m_rtao.m_totalSamples = 1024;
//            m_volumeRenderer.m_ambientOcclusion.m_rtao.m_distanceToHalfOcclusion = 256.f;
//
//            info->fileName = "s1_rtao_factor.png";
//            info->iterations = 128;
//
//            return true;
//        case 2:
//            teleportPos(Position800Near);
//
//            m_volumeRenderer.m_ambientOcclusion.m_method = AmbientOcclusion::AO_VDCAO;
//            m_volumeRenderer.m_fragmentDebugMode = VolumeRenderer::FragmentDebugMode::DebugDisabled;
//            m_volumeRenderer.m_emission.m_method = Emission::EM::EM_DISABLED;
//
//            info->fileName = "s1_vdcao.png";
//
//            return true;
//        case 3:
//            teleportPos(Position800Near);
//
//            m_volumeRenderer.m_ambientOcclusion.m_method = AmbientOcclusion::AO_VDCAO;
//            m_volumeRenderer.m_fragmentDebugMode = VolumeRenderer::FragmentDebugMode::DebugAmbientOcclusion;
//            m_volumeRenderer.m_emission.m_method = Emission::EM::EM_DISABLED;
//
//            info->fileName = "s1_vdcao_factor.png";
//
//            return true;
//        case 4:
//            teleportPos(Position800Near);
//
//            m_volumeRenderer.m_ambientOcclusion.m_method = AmbientOcclusion::AO_VCTAO;
//            m_volumeRenderer.m_fragmentDebugMode = VolumeRenderer::FragmentDebugMode::DebugDisabled;
//            m_volumeRenderer.m_emission.m_method = Emission::EM::EM_DISABLED;
//
//            info->fileName = "s1_vctao.png";
//
//            return true;
//        case 5:
//            teleportPos(Position800Near);
//
//            m_volumeRenderer.m_ambientOcclusion.m_method = AmbientOcclusion::AO_VCTAO;
//            m_volumeRenderer.m_fragmentDebugMode = VolumeRenderer::FragmentDebugMode::DebugAmbientOcclusion;
//            m_volumeRenderer.m_emission.m_method = Emission::EM::EM_DISABLED;
//
//            info->fileName = "s1_vctao_factor.png";
//
//            return true;
//        case 6:
//            teleportPos(Position800Near);
//
//            m_volumeRenderer.m_ambientOcclusion.m_method = AmbientOcclusion::AO_LVAO;
//            m_volumeRenderer.m_fragmentDebugMode = VolumeRenderer::FragmentDebugMode::DebugDisabled;
//            m_volumeRenderer.m_emission.m_method = Emission::EM::EM_DISABLED;
//
//            info->fileName = "s1_lvao.png";
//
//            return true;
//        case 7:
//            teleportPos(Position800Near);
//
//            m_volumeRenderer.m_ambientOcclusion.m_method = AmbientOcclusion::AO_LVAO;
//            m_volumeRenderer.m_fragmentDebugMode = VolumeRenderer::FragmentDebugMode::DebugAmbientOcclusion;
//            m_volumeRenderer.m_emission.m_method = Emission::EM::EM_DISABLED;
//
//            info->fileName = "s1_lvao_factor.png";
//
//            return true;
//        case 8:
//            teleportPos(Position800Near);
//
//            m_volumeRenderer.m_ambientOcclusion.m_method = AmbientOcclusion::AO_DFAO;
//            m_volumeRenderer.m_fragmentDebugMode = VolumeRenderer::FragmentDebugMode::DebugDisabled;
//            m_volumeRenderer.m_emission.m_method = Emission::EM::EM_DISABLED;
//
//            info->fileName = "s1_dfao.png";
//
//            return true;
//        case 9:
//            teleportPos(Position800Near);
//
//            m_volumeRenderer.m_ambientOcclusion.m_method = AmbientOcclusion::AO_DFAO;
//            m_volumeRenderer.m_fragmentDebugMode = VolumeRenderer::FragmentDebugMode::DebugAmbientOcclusion;
//            m_volumeRenderer.m_emission.m_method = Emission::EM::EM_DISABLED;
//
//            info->fileName = "s1_dfao_factor.png";
//
//            return true;
//        case 10:
//            teleportPos(Position800Near);
//
//            m_volumeRenderer.m_ambientOcclusion.m_method = AmbientOcclusion::AO_HBAO;
//            m_volumeRenderer.m_fragmentDebugMode = VolumeRenderer::FragmentDebugMode::DebugDisabled;
//            m_volumeRenderer.m_emission.m_method = Emission::EM::EM_DISABLED;
//            m_volumeRenderer.m_ambientOcclusion.m_hbao.m_executeBilateralFilter = true;
//
//            info->fileName = "s1_hbao.png";
//
//            return true;
//        case 11:
//            teleportPos(Position800Near);
//
//            m_volumeRenderer.m_ambientOcclusion.m_method = AmbientOcclusion::AO_HBAO;
//            m_volumeRenderer.m_fragmentDebugMode = VolumeRenderer::FragmentDebugMode::DebugAmbientOcclusion;
//            m_volumeRenderer.m_emission.m_method = Emission::EM::EM_DISABLED;
//            m_volumeRenderer.m_ambientOcclusion.m_hbao.m_executeBilateralFilter = true;
//
//            info->fileName = "s1_hbao_factor.png";
//
//            return true;
//        case 12:
//            teleportPos(Position800Near);
//
//            m_volumeRenderer.m_ambientOcclusion.m_method = AmbientOcclusion::AO_HBAO;
//            m_volumeRenderer.m_fragmentDebugMode = VolumeRenderer::FragmentDebugMode::DebugDisabled;
//            m_volumeRenderer.m_emission.m_method = Emission::EM::EM_DISABLED;
//            m_volumeRenderer.m_ambientOcclusion.m_hbao.m_executeBilateralFilter = false;
//
//            info->fileName = "s1_hbao_wofilter.png";
//
//            return true;
//        case 13:
//            teleportPos(Position800Near);
//
//            m_volumeRenderer.m_ambientOcclusion.m_method = AmbientOcclusion::AO_HBAO;
//            m_volumeRenderer.m_fragmentDebugMode = VolumeRenderer::FragmentDebugMode::DebugAmbientOcclusion;
//            m_volumeRenderer.m_emission.m_method = Emission::EM::EM_DISABLED;
//            m_volumeRenderer.m_ambientOcclusion.m_hbao.m_executeBilateralFilter = false;
//
//            info->fileName = "s1_hbao_wofilter_factor.png";
//
//            return true;
//    }

// === EVALUATION 400 FAR ===
//    switch (number) {
//        case 0:
//            teleportPos(Position400Far);
//
//            m_volumeRenderer.m_ambientOcclusion.m_method = AmbientOcclusion::AO_RTAO;
//            m_volumeRenderer.m_fragmentDebugMode = VolumeRenderer::FragmentDebugMode::DebugDisabled;
//            m_volumeRenderer.m_emission.m_method = Emission::EM::EM_DISABLED;
//
//            m_volumeRenderer.m_ambientOcclusion.m_rtao.m_samples = 8;
//            m_volumeRenderer.m_ambientOcclusion.m_rtao.m_totalSamples = 1024;
//            m_volumeRenderer.m_ambientOcclusion.m_rtao.m_distanceToHalfOcclusion = 256.f;
//
//            info->fileName = "s2_rtao.png";
//            info->iterations = 128;
//
//            return true;
//        case 1:
//            teleportPos(Position400Far);
//
//            m_volumeRenderer.m_ambientOcclusion.m_method = AmbientOcclusion::AO_RTAO;
//            m_volumeRenderer.m_fragmentDebugMode = VolumeRenderer::FragmentDebugMode::DebugAmbientOcclusion;
//            m_volumeRenderer.m_emission.m_method = Emission::EM::EM_DISABLED;
//
//            m_volumeRenderer.m_ambientOcclusion.m_rtao.m_samples = 8;
//            m_volumeRenderer.m_ambientOcclusion.m_rtao.m_totalSamples = 1024;
//            m_volumeRenderer.m_ambientOcclusion.m_rtao.m_distanceToHalfOcclusion = 256.f;
//
//            info->fileName = "s2_rtao_factor.png";
//            info->iterations = 128;
//
//            return true;
//        case 2:
//            teleportPos(Position400Far);
//
//            m_volumeRenderer.m_ambientOcclusion.m_method = AmbientOcclusion::AO_VDCAO;
//            m_volumeRenderer.m_fragmentDebugMode = VolumeRenderer::FragmentDebugMode::DebugDisabled;
//            m_volumeRenderer.m_emission.m_method = Emission::EM::EM_DISABLED;
//            m_volumeRenderer.m_ambientOcclusion.m_vdcao.m_coneTraceSteps = 16;
//            m_volumeRenderer.m_ambientOcclusion.m_vdcao.m_coneTraceStepSize = 0.5f;
//            m_volumeRenderer.m_ambientOcclusion.m_vdcao.m_coneApertureAngle = 30.f;
//            m_volumeRenderer.m_ambientOcclusion.m_vdcao.m_attenuation = 16.f;
//
//            info->fileName = "s2_vdcao.png";
//
//            return true;
//        case 3:
//            teleportPos(Position400Far);
//
//            m_volumeRenderer.m_ambientOcclusion.m_method = AmbientOcclusion::AO_VDCAO;
//            m_volumeRenderer.m_fragmentDebugMode = VolumeRenderer::FragmentDebugMode::DebugAmbientOcclusion;
//            m_volumeRenderer.m_emission.m_method = Emission::EM::EM_DISABLED;
//            m_volumeRenderer.m_ambientOcclusion.m_vdcao.m_coneTraceSteps = 16;
//            m_volumeRenderer.m_ambientOcclusion.m_vdcao.m_coneTraceStepSize = 0.5f;
//            m_volumeRenderer.m_ambientOcclusion.m_vdcao.m_coneApertureAngle = 30.f;
//            m_volumeRenderer.m_ambientOcclusion.m_vdcao.m_attenuation = 16.f;
//
//            info->fileName = "s2_vdcao_factor.png";
//
//            return true;
//        case 4:
//            teleportPos(Position400Far);
//
//            m_volumeRenderer.m_ambientOcclusion.m_method = AmbientOcclusion::AO_VCTAO;
//            m_volumeRenderer.m_fragmentDebugMode = VolumeRenderer::FragmentDebugMode::DebugDisabled;
//            m_volumeRenderer.m_emission.m_method = Emission::EM::EM_DISABLED;
//            m_volumeRenderer.m_ambientOcclusion.m_vctao.m_coneTraceSteps = 32;
//            m_volumeRenderer.m_ambientOcclusion.m_vctao.m_coneTraceStepSize = 0.25f;
//            m_volumeRenderer.m_ambientOcclusion.m_vctao.m_coneApertureAngle = 30.f;
//            m_volumeRenderer.m_ambientOcclusion.m_vctao.m_attenuation = 1024.f;
//
//            info->fileName = "s2_vctao.png";
//
//            return true;
//        case 5:
//            teleportPos(Position400Far);
//
//            m_volumeRenderer.m_ambientOcclusion.m_method = AmbientOcclusion::AO_VCTAO;
//            m_volumeRenderer.m_fragmentDebugMode = VolumeRenderer::FragmentDebugMode::DebugAmbientOcclusion;
//            m_volumeRenderer.m_emission.m_method = Emission::EM::EM_DISABLED;
//            m_volumeRenderer.m_ambientOcclusion.m_vctao.m_coneTraceSteps = 32;
//            m_volumeRenderer.m_ambientOcclusion.m_vctao.m_coneTraceStepSize = 0.25f;
//            m_volumeRenderer.m_ambientOcclusion.m_vctao.m_coneApertureAngle = 30.f;
//            m_volumeRenderer.m_ambientOcclusion.m_vctao.m_attenuation = 1024.f;
//
//            info->fileName = "s2_vctao_factor.png";
//
//            return true;
//    }

//    switch (number) {
//        case 0:
//            teleportPos(Position800Far);
//
//            m_volumeRenderer.m_ambientOcclusion.m_method = AmbientOcclusion::AO_VDCAO;
//            m_volumeRenderer.m_fragmentDebugMode = VolumeRenderer::FragmentDebugMode::DebugDisabled;
//            m_volumeRenderer.m_emission.m_method = Emission::EM::EM_DDFE;
//            m_volumeRenderer.m_emissionFactor = 0.5;
//
//            info->fileName = "gi_s0_vdcao.png";
//
//            return true;
//        case 1:
//            teleportPos(Position800Far);
//
//            m_volumeRenderer.m_ambientOcclusion.m_method = AmbientOcclusion::AO_VCTAO;
//            m_volumeRenderer.m_fragmentDebugMode = VolumeRenderer::FragmentDebugMode::DebugDisabled;
//            m_volumeRenderer.m_emission.m_method = Emission::EM::EM_VCTE;
//            m_volumeRenderer.m_emission.m_vcte.m_specular = false;
//            m_volumeRenderer.m_emissionFactor = 1.0;
//
//            info->fileName = "gi_s0_vctao_diffuseonly.png";
//
//            return true;
//        case 2:
//            teleportPos(Position800Far);
//
//            m_volumeRenderer.m_ambientOcclusion.m_method = AmbientOcclusion::AO_VCTAO;
//            m_volumeRenderer.m_fragmentDebugMode = VolumeRenderer::FragmentDebugMode::DebugDisabled;
//            m_volumeRenderer.m_emission.m_method = Emission::EM::EM_VCTE;
//            m_volumeRenderer.m_emission.m_vcte.m_specular = true;
//            m_volumeRenderer.m_emissionFactor = 1.0;
//
//            info->fileName = "gi_s0_vctao.png";
//
//            return true;
//        case 3:
//            teleportPos(Position800Near);
//
//            m_volumeRenderer.m_ambientOcclusion.m_method = AmbientOcclusion::AO_VDCAO;
//            m_volumeRenderer.m_fragmentDebugMode = VolumeRenderer::FragmentDebugMode::DebugDisabled;
//            m_volumeRenderer.m_emission.m_method = Emission::EM::EM_DDFE;
//            m_volumeRenderer.m_emissionFactor = 0.5;
//
//            info->fileName = "gi_s1_vdcao.png";
//
//            return true;
//        case 4:
//            teleportPos(Position800Near);
//
//            m_volumeRenderer.m_ambientOcclusion.m_method = AmbientOcclusion::AO_VCTAO;
//            m_volumeRenderer.m_fragmentDebugMode = VolumeRenderer::FragmentDebugMode::DebugDisabled;
//            m_volumeRenderer.m_emission.m_method = Emission::EM::EM_VCTE;
//            m_volumeRenderer.m_emission.m_vcte.m_specular = false;
//            m_volumeRenderer.m_emissionFactor = 1.0;
//
//            info->fileName = "gi_s1_vctao_diffuseonly.png";
//
//            return true;
//        case 5:
//            teleportPos(Position800Near);
//
//            m_volumeRenderer.m_ambientOcclusion.m_method = AmbientOcclusion::AO_VCTAO;
//            m_volumeRenderer.m_fragmentDebugMode = VolumeRenderer::FragmentDebugMode::DebugDisabled;
//            m_volumeRenderer.m_emission.m_vcte.m_specular = true;
//            m_volumeRenderer.m_emission.m_method = Emission::EM::EM_VCTE;
//            m_volumeRenderer.m_emissionFactor = 1.0;
//
//            info->fileName = "gi_s1_vctao.png";
//
//            return true;
//    }

//    switch (number) {
//        case 0:
//            teleportPos(Position800Debug1);
//
//            m_volumeRenderer.m_ambientOcclusion.m_method = AmbientOcclusion::AO_VDCAO;
//            m_volumeRenderer.m_fragmentDebugMode = VolumeRenderer::FragmentDebugMode::DebugDisabled;
//            m_volumeRenderer.m_emission.m_method = Emission::EM::EM_DDFE;
//            m_volumeRenderer.m_emissionFactor = 0.5;
//
//            info->fileName = "s9_gi_vdcao_rgbdf.png";
//
//            return true;
//        case 1:
//            teleportPos(Position800Debug2);
//
//            m_volumeRenderer.m_ambientOcclusion.m_method = AmbientOcclusion::AO_VDCAO;
//            m_volumeRenderer.m_fragmentDebugMode = VolumeRenderer::FragmentDebugMode::DebugDisabled;
//            m_volumeRenderer.m_emission.m_method = Emission::EM::EM_DDFE;
//            m_volumeRenderer.m_emissionFactor = 0.5;
//
//            info->fileName = "s10_gi_vdcao_rgbdf.png";
//
//            return true;
//        case 2:
//            teleportPos(Position800Near);
//
//            m_volumeRenderer.m_ambientOcclusion.m_method = AmbientOcclusion::AO_VDCAO;
//            m_volumeRenderer.m_fragmentDebugMode = VolumeRenderer::FragmentDebugMode::DebugDisabled;
//            m_volumeRenderer.m_emission.m_method = Emission::EM::EM_DDFE;
//            m_volumeRenderer.m_emissionFactor = 0.5;
//
//            info->fileName = "s11_gi_vdcao_rgbdf.png";
//
//            return true;
//    }
//    switch (number) {
//        case 0:
//            teleportPos(Position800Debug1);
//
//            m_volumeRenderer.m_ambientOcclusion.m_method = AmbientOcclusion::AO_VDCAO;
//            m_volumeRenderer.m_fragmentDebugMode = VolumeRenderer::FragmentDebugMode::DebugDisabled;
//            m_volumeRenderer.m_emission.m_method = Emission::EM::EM_DDFE;
//            m_volumeRenderer.m_emissionFactor = 0.5;
//
//            info->fileName = "s9_gi_vdcao_dirrgbdf.png";
//
//            return true;
//        case 1:
//            teleportPos(Position800Debug2);
//
//            m_volumeRenderer.m_ambientOcclusion.m_method = AmbientOcclusion::AO_VDCAO;
//            m_volumeRenderer.m_fragmentDebugMode = VolumeRenderer::FragmentDebugMode::DebugDisabled;
//            m_volumeRenderer.m_emission.m_method = Emission::EM::EM_DDFE;
//            m_volumeRenderer.m_emissionFactor = 0.5;
//
//            info->fileName = "s10_gi_vdcao_dirrgbdf.png";
//
//            return true;
//        case 2:
//            teleportPos(Position800Near);
//
//            m_volumeRenderer.m_ambientOcclusion.m_method = AmbientOcclusion::AO_VDCAO;
//            m_volumeRenderer.m_fragmentDebugMode = VolumeRenderer::FragmentDebugMode::DebugDisabled;
//            m_volumeRenderer.m_emission.m_method = Emission::EM::EM_DDFE;
//            m_volumeRenderer.m_emissionFactor = 0.5;
//
//            info->fileName = "s11_gi_vdcao_dirrgbdf.png";
//
//            return true;
//    }

    return false;
}

void VolumeRenderLogic::teleportPos(int pos) {
    switch (pos) {
        case Position800Far:
            m_camera.setCenter(glm::vec3(243, 711, 6) - glm::vec3(m_volumeRenderer.m_volume.m_volumeMargin));
            m_camera.setR(0);
            m_camera.setPhi(0.414f);
            m_camera.setTheta(3.754f);
            m_executeComputeShader = true;
            break;
        case Position800Near:
            m_camera.setCenter(glm::vec3(398, 728, 319) - glm::vec3(m_volumeRenderer.m_volume.m_volumeMargin));
            m_camera.setR(0);
            m_camera.setPhi(0.198f);
            m_camera.setTheta(3.962f);
            m_executeComputeShader = true;
            break;
        case Position800Debug1:
            m_camera.setCenter(glm::vec3(382, 545, 358) - glm::vec3(m_volumeRenderer.m_volume.m_volumeMargin));
            m_camera.setR(0);
            m_camera.setPhi(0.293f);
            m_camera.setTheta(4.661f);
            m_executeComputeShader = true;
            break;
        case Position800Debug2:
            m_camera.setCenter(glm::vec3(379, 480, 552) - glm::vec3(m_volumeRenderer.m_volume.m_volumeMargin));
            m_camera.setR(0);
            m_camera.setPhi(-0.041f);
            m_camera.setTheta(3.405f);
            m_executeComputeShader = true;
            break;
        case Position400Far:
            m_camera.setCenter(glm::vec3(316, 251, 138) - glm::vec3(m_volumeRenderer.m_volume.m_volumeMargin));
            m_camera.setR(0);
            m_camera.setPhi(0.355f);
            m_camera.setTheta(2.074f);
            m_executeComputeShader = true;
            break;
        case 5:
            m_camera.setCenter(glm::vec3(427.27, 618.45, 388.16) - glm::vec3(m_volumeRenderer.m_volume.m_volumeMargin));
            m_camera.setR(0);
            m_camera.setPhi(0.159f);
            m_camera.setTheta(2.918f);
            m_executeComputeShader = true;
            break;
        default:
            break;
    }
}
