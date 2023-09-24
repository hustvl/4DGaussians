/*
 * Copyright (C) 2020, Inria
 * GRAPHDECO research group, https://team.inria.fr/graphdeco
 * All rights reserved.
 *
 * This software is free for non-commercial, research and evaluation use 
 * under the terms of the LICENSE.md file.
 *
 * For inquiries contact sibr@inria.fr and/or George.Drettakis@inria.fr
 */


# include "core/view/SceneDebugView.hpp"
# include "core/graphics/RenderUtility.hpp"
# include "core/graphics/Input.hpp"
# include "core/graphics/GUI.hpp"
#include <core/raycaster/CameraRaycaster.hpp>

#include <sstream>

namespace sibr
{

	Mesh::Ptr generateCamFrustum(const InputCamera & cam, float near, float far)
	{
		static const Mesh::Triangles tris = {
			{0,0,1},{1,1,2},{2,2,3},{3,3,0},
			{4,4,5},{5,5,6},{6,6,7},{7,7,4},
			{0,0,4},{1,1,5},{2,2,6},{3,3,7},
		};

		std::vector<Vector3f> dirs;
		for (const auto & c : cam.getImageCorners()) {
			dirs.push_back(CameraRaycaster::computeRayDir(cam, c.cast<float>() + 0.5f*Vector2f(1, 1)));
		}
		float znear = (near >= 0 ? near : cam.znear());
		float zfar = (far >= 0 ? far : cam.zfar());
		Mesh::Vertices vertices;
		for (int k = 0; k < 2; k++) {
			float dist = (k == 0 ? znear : zfar);
			for (const auto & d : dirs) {
				vertices.push_back(cam.position() + dist * d);
			}
		}

		auto out = std::make_shared<Mesh>();
		out->vertices(vertices);
		out->triangles(tris);
		return out;
	}

	Mesh::Ptr generateCamFrustumColored(const InputCamera & cam, const Vector3f & col, float znear, float zfar)
	{
		auto out = generateCamFrustum(cam, znear, zfar);
		Mesh::Colors cols(out->vertices().size(), col);
		out->colors(cols);
		return out;
	}

	Mesh::Ptr generateCamQuadWithUvs(const InputCamera & cam, float dist)
	{
		static const Mesh::Triangles quadTriangles = {
			{ 0,1,2 },{ 0,2,3 }
		};
		static const Mesh::UVs quadUVs = {
			{ 0,1 } ,{ 1,1 } ,{ 1,0 } ,{ 0,0 }
		};

		std::vector<Vector3f> dirs;
		for (const auto & c : cam.getImageCorners()) {
			dirs.push_back(CameraRaycaster::computeRayDir(cam, c.cast<float>() + 0.5f*Vector2f(1, 1)));
		}
		std::vector<Vector3f> vertices;
		for (const auto & d : dirs) {
			vertices.push_back(cam.position() + dist * d);
		}

		auto out = std::make_shared<Mesh>();
		out->vertices(vertices);
		out->triangles(quadTriangles);
		out->texCoords(quadUVs);
		return out;
	}


	LabelsManager::CameraInfos::CameraInfos(const InputCamera& cam, uint id, bool highlight)
		: cam(cam), id(id), highlight(highlight) {
	}

	void LabelsManager::setupLabelsManagerShader()
	{
		_labelShader.init("text-imgui",
			loadFile(Resources::Instance()->getResourceFilePathName("text-imgui.vp")),
			loadFile(Resources::Instance()->getResourceFilePathName("text-imgui.fp")));
		_labelShaderPosition.init(_labelShader, "position");
		_labelShaderScale.init(_labelShader, "scale");
		_labelShaderViewport.init(_labelShader, "viewport");
	}

	void LabelsManager::setupLabelsManagerMeshes(const std::vector<InputCamera::Ptr> & cams)
	{
		_labelMeshes.clear();
		for (const auto & cam : cams) {
			unsigned int sepIndex = 0;
			_labelMeshes[cam->id()] = {};
			_labelMeshes[cam->id()].mesh = generateMeshForText(std::to_string(cam->id()), sepIndex);
			_labelMeshes[cam->id()].splitIndex = sepIndex;
		}
	}

	void LabelsManager::renderLabels(const Camera & eye, const Viewport & vp, const std::vector<CameraInfos>& cams_info)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		_labelShader.begin();
		// Bind the ImGui font texture.
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)(ImGui::GetFont()->ContainerAtlas->TexID));
		_labelShaderViewport.set(Vector2f(vp.finalWidth(), vp.finalHeight()));

		for (const auto & camInfos : cams_info) {
			const auto & inputCam = camInfos.cam;
			if (!inputCam.isActive()) { continue; }
			const uint uid = camInfos.id;
			if (_labelMeshes.count(uid) == 0) {
				continue;
			}
			// Draw the label.
			// TODO: we could try to use depth testing to have the labels overlap properly.
			// As the label is put at the position of the camera, the label will intersect with the frustum mesh, causing artifacts.
			// One way of solving this would be to just shift the label away a bit and enable depth testing (+ GL_LEQUAl for the text).
			const Vector3f camProjPos = eye.project(inputCam.position());
			if (!eye.frustumTest(inputCam.position(), camProjPos.xy())) {
				continue;
			}
			_labelShaderPosition.set(camProjPos);
			const auto & label = _labelMeshes[uid];
			// Render the background label.
			_labelShaderScale.set(0.8f*_labelScale);
			label.mesh->renderSubMesh(0, label.splitIndex, false, false);
			// Render the text label.
			_labelShaderScale.set(1.0f*_labelScale);
			label.mesh->renderSubMesh(label.splitIndex, int(label.mesh->triangles().size()) * 3, false, false);

		}
		_labelShader.end();
		glDisable(GL_BLEND);
	}

	void ImageCamViewer::initImageCamShaders()
	{
		const std::string vertex_str = loadFile(Resources::Instance()->getResourceFilePathName("uv_mesh.vert"));

		_shader2D.init("cameraImageShader", vertex_str, loadFile(Resources::Instance()->getResourceFilePathName("alpha_uv_tex.frag")));
		_mvp2D.init(_shader2D, "mvp");
		_alpha2D.init(_shader2D, "alpha");

		_shaderArray.init("cameraImageShaderArray", vertex_str, loadFile(Resources::Instance()->getResourceFilePathName("alpha_uv_tex_array.frag")));
		_mvpArray.init(_shaderArray, "mvp");
		_alphaArray.init(_shaderArray, "alpha");
		_sliceArray.init(_shaderArray, "slice");
	}

	void ImageCamViewer::renderImage(const Camera & eye, const InputCamera & cam,
		const std::vector<RenderTargetRGBA32F::Ptr> & rts, int cam_id)
	{
		const auto quad = generateCamQuadWithUvs(cam, _cameraScaling);
		if (cam_id < rts.size() && rts[cam_id]) {
			_shader2D.begin();
			_mvp2D.set(eye.viewproj());
			_alpha2D.set(_alphaImage);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, rts[cam_id]->handle());
			quad->render(true, false, Mesh::FillRenderMode, false, false);
			_shader2D.end();
		}
	}

	void ImageCamViewer::renderImage(const Camera & eye, const InputCamera & cam, uint tex2Darray_handle, int cam_id)
	{
		const auto quad = generateCamQuadWithUvs(cam, _cameraScaling);
		_shaderArray.begin();
		_mvpArray.set(eye.viewproj());
		_alphaArray.set(_alphaImage);
		_sliceArray.set(cam_id);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D_ARRAY, tex2Darray_handle);
		quad->render(true, false, Mesh::FillRenderMode, false, false);
		_shaderArray.end();
	}

	SceneDebugView::SceneDebugView(const IIBRScene::Ptr & scene, 
		const InteractiveCameraHandler::Ptr & camHandler, const BasicDatasetArgs & myArgs)
	{

		initImageCamShaders();
		setupLabelsManagerShader();

		_scene = scene;
		_userCurrentCam = camHandler;

		if (!_scene->cameras()->inputCameras().empty()) {
			camera_handler.fromTransform(_scene->cameras()->inputCameras()[0]->transform(), true, false);
			camera_handler.setupInterpolationPath(_scene->cameras()->inputCameras());
		}

		_showImages = true;

		const std::string camerasDir = myArgs.dataset_path.get() + "/cameras";
		if (directoryExists(camerasDir)) {
			_topViewPath = camerasDir + "/topview.txt";
				if (!directoryExists(camerasDir)) {
					makeDirectory(camerasDir);
				}
		}
		else {
			_topViewPath = parentDirectory(myArgs.dataset_path) + "/topview.txt";
		}

		setup();
	}

	SceneDebugView::SceneDebugView(const IIBRScene::Ptr & scene, const Viewport & viewport,
		const InteractiveCameraHandler::Ptr & camHandler, const BasicDatasetArgs & myArgs) : SceneDebugView(scene, camHandler, myArgs) {
		SIBR_WRG << "Deprecated SceneDebugView constructor, use the version without viewport passed as argument." << std::endl;
	}

	void SceneDebugView::onUpdate(Input & input, const float deltaTime, const Viewport & viewport)
	{
		MultiMeshManager::onUpdate(input, viewport);

		//Camera stub size
		if (input.key().isActivated(Key::LeftControl) && input.mouseScroll() != 0.0) {
			_cameraScaling = std::max(0.001f, _cameraScaling + (float)input.mouseScroll()*0.1f);
		}
		if (input.key().isActivated(Key::LeftControl) && input.key().isReleased(Key::P)) {
			MeshData & guizmo = getMeshData("guizmo");
			guizmo.active = !guizmo.active;
		}

		MeshData & proxy = getMeshData("proxy");
		if( proxy.meshPtr->triangles().size() == 0 )
			// SfM Points only
			proxy.renderMode = Mesh::RenderMode::PointRenderMode;

		if (input.key().isActivated(Key::LeftControl) && input.key().isReleased(Key::Z)) {
			//MeshData & proxy = getMeshData("proxy");
			if (proxy.renderMode == Mesh::RenderMode::FillRenderMode) {
				proxy.renderMode = Mesh::RenderMode::LineRenderMode;
			} else {
				proxy.renderMode = Mesh::RenderMode::FillRenderMode;
			}
		}

		if (input.key().isReleased(Key::T)) {
			save();
		}
	}

	void SceneDebugView::onUpdate(Input & input, const Viewport & viewport)
	{
		onUpdate(input, 1.0f / 60.0f, viewport);
	}

	void SceneDebugView::onUpdate(Input & input)
	{
		// Update camera with a fixed timestep.
		onUpdate(input, 1.0f / 60.0f);
	}

	void SceneDebugView::onRender(Window & win)
	{
		// We need no information about the window, we render wherever we are.
		onRender(win.viewport());
	}

	void SceneDebugView::onRender(const Viewport & viewport)
	{
		glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Scene debug view");

		viewport.clear(backgroundColor);
		viewport.bind();

		addMeshAsLines("scene cam", generateCamFrustum(_userCurrentCam->getCamera(), 0.0f, _cameraScaling)).setColor({ 1,0,0 });

		if (_scene) {
			for (int i = 0; i < (int)_scene->cameras()->inputCameras().size(); ++i) {
				_cameras[i].highlight =  _scene->cameras()->isCameraUsedForRendering(_scene->cameras()->inputCameras()[i]->id());
			}
		}	

		auto used_cams = std::make_shared<Mesh>(), non_used_cams = std::make_shared<Mesh>();
		for (const auto & camInfos : _cameras) {
			if (!camInfos.cam.isActive()) { continue; }
			(camInfos.highlight ? used_cams : non_used_cams)->merge(*generateCamFrustum(camInfos.cam, 0.0f, _cameraScaling));
		}

		addMeshAsLines("used cams", used_cams).setColor({ 0,1,0 });
		addMeshAsLines("non used cams", non_used_cams).setColor({ 0,0,1 });

		renderMeshes();

		if (_scene && _showImages) {
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			int cam_id = 0;
			for (const auto & camInfos : _cameras) {
				if (camInfos.cam.isActive()) {
					const auto & scene_rts = _scene->renderTargets();
					if (scene_rts->getInputRGBTextureArrayPtr()) {
						renderImage(camera_handler.getCamera(), camInfos.cam, scene_rts->getInputRGBTextureArrayPtr()->handle(), cam_id);
					} else {
						renderImage(camera_handler.getCamera(), camInfos.cam, scene_rts->inputImagesRT(), cam_id);
					}
				}
				++cam_id;
			}
			glDisable(GL_BLEND);
		}

		if (_showLabels) {
			renderLabels(camera_handler.getCamera(), viewport, _cameras);
		}

		camera_handler.onRender(viewport);
		glPopDebugGroup();
	}

	void SceneDebugView::onGUI()
	{
		if (ImGui::Begin("Top view settings")) {
			gui_options();
			list_mesh_onGUI();
			gui_cameras();
		}
		ImGui::End();
		
	}

	void SceneDebugView::save()
	{
		
		std::ofstream outfile(_topViewPath, std::ios::out | std::ios::trunc);
		std::cerr << "Saving topview camera to " << _topViewPath << std::endl;
		// save camera view proj matrix
		camera_handler.getCamera().writeToFile(outfile);
	}

	void SceneDebugView::setScene(const IIBRScene::Ptr & scene, bool preserveCamera)
	{
		_scene = scene;
		const InputCamera cameraBack = camera_handler.getCamera();
		setup();
		camera_handler.setup(_scene->cameras()->inputCameras(), camera_handler.getViewport(), camera_handler.getRaycaster());
		camera_handler.setupInterpolationPath(_scene->cameras()->inputCameras());
		// Optionally restore the camera pose.
		if (preserveCamera) {
			camera_handler.fromCamera(cameraBack, false);
		}
	}

	void SceneDebugView::updateActiveCams(const std::vector<uint>& cams_id)
	{
		for (auto & cam : _cameras) {
			cam.highlight = false;
		}
		for (const uint id : cams_id) {
			if (id < _cameras.size()) {
				_cameras[id].highlight = true;
			}
		}
	}

	void SceneDebugView::gui_options()
	{

		if (ImGui::CollapsingHeader("OptionsSceneDebugView##")) {
			if (ImGui::Button("Save topview")) {
				save();
			}

			ImGui::PushScaledItemWidth(120);
			ImGui::InputFloat("Camera scale", &_cameraScaling, 0.1f, 10.0f);
			_cameraScaling = std::max(0.001f, _cameraScaling);

			ImGui::Checkbox("Draw labels ", &_showLabels);
			if (_showLabels) {
				ImGui::SameLine();
				ImGui::InputFloat("Label scale", &_labelScale, 0.2f, 10.0f);
			}

			ImGui::Separator();
			ImGui::Checkbox("Draw Input Images ", &_showImages);
			if (_showImages) {
				ImGui::SameLine();
				ImGui::SliderFloat("Alpha", &_alphaImage, 0, 1.0);
			}
			
			camera_handler.onGUI("Top view settings");
			ImGui::PopItemWidth();
			ImGui::Separator();
		}
	}

	void SceneDebugView::gui_cameras()
	{
		if (ImGui::CollapsingHeader("Cameras##SceneDebugView")) {
			
			ImGui::SliderInt("Camera ID info", &_cameraIdInfoGUI, 0, static_cast<int>(_cameras.size()) - 1);


			ImGui::Columns(4); // 0 name | snapto | active| size 

			ImGui::Separator();
			ImGui::Text("Camera"); ImGui::NextColumn();
			ImGui::Text("SnapTo"); ImGui::NextColumn();
			ImGui::Text("Active"); ImGui::NextColumn();

			static std::vector<std::string> cam_info_option_str = { "size", "focal", "fov_y","aspect" };
			if (ImGui::BeginCombo("Info", cam_info_option_str[_camInfoOption].c_str())) {
				for (int i = 0; i < (int)cam_info_option_str.size(); ++i) {
					if (ImGui::Selectable(cam_info_option_str[i].c_str(), _camInfoOption == i)) {
						_camInfoOption = (CameraInfoDisplay)i;
					}					
				}
				ImGui::EndCombo();
			}
			ImGui::NextColumn();
			ImGui::Separator();
	
			//for (uint i = 0; i < _cameras.size(); ++i) 
			{
				std::string name = "cam_" + intToString<4>(_cameraIdInfoGUI);
				ImGui::Text(name.c_str());
				ImGui::NextColumn();

				if (ImGui::Button(("SnapTo##" + name).c_str())) {
					const auto & input_cam = _scene->cameras()->inputCameras()[0];

					auto size = camera_handler.getViewport().finalSize();
					float ratio_dst = size[0] / size[1];
					float ratio_src = input_cam->w() / (float)input_cam->h();
					InputCamera cam = InputCamera(_cameras[_cameraIdInfoGUI].cam, (int)size[0], (int)size[1]);

					if (ratio_src < ratio_dst) {
						float fov_h = 2 * atan(tan(input_cam->fovy() / 2) * ratio_src / ratio_dst);
						cam.fovy(fov_h);
					} else {
						cam.fovy(input_cam->fovy());
					}

					cam.znear(0.0001f);
					camera_handler.fromCamera(cam, true, false);
				}
				ImGui::NextColumn();

				ImGui::Checkbox(("##is_valid" + name).c_str(), &_cameras[_cameraIdInfoGUI].highlight);
				ImGui::NextColumn();

				const auto & cam = _cameras[_cameraIdInfoGUI].cam;
				std::stringstream tmp;
				switch (_camInfoOption)
				{
					case SIZE: tmp << cam.w() << " x " << cam.h(); break;
					case FOCAL: tmp << cam.focal(); break;
					case FOV_Y: tmp << cam.fovy(); break;
					case ASPECT: tmp << cam.aspect(); break;
					default: break;
				}
				ImGui::Text(tmp.str().c_str());

				ImGui::Columns(1);
			}
			
		}
	}

	void SceneDebugView::setup()
	{
		if (_scene) {
			setupLabelsManagerMeshes(_scene->cameras()->inputCameras());
			setupMeshes();

			_cameras.clear();
			for (const auto & inputCam : _scene->cameras()->inputCameras()) {
				_cameras.push_back(CameraInfos(*inputCam, inputCam->id(), _scene->cameras()->isCameraUsedForRendering(inputCam->id())));
			}
		}

		_snapToImage = 0;
		_showLabels = false;

		// check if topview.txt exists
		std::ifstream topViewFile(_topViewPath);
		if (topViewFile.good())
		{
			SIBR_LOG << "Loaded saved topview (" << _topViewPath << ")." << std::endl;
			// Intialize a temp camera (used to load the saved top view pose) with
			// the current top view camera to get the resolution/fov right.
			InputCamera cam(camera_handler.getCamera());
			cam.readFromFile(topViewFile);
			// Apply it to the top view FPS camera.
			//camera_handler.fromCamera(cam, false);
			camera_handler.fromTransform(cam.transform(), false, true);
		}

	}

	void SceneDebugView::setupMeshes()
	{
		// no colors and no texture ? try to find capreal
		bool success = false;
		Mesh sdv_mesh;
		Mesh::Ptr mp;
		if (!_scene->proxies()->proxyPtr()->hasColors() && !_scene->proxies()->proxyPtr()->hasTexCoords()) {
			std::string fn;
			if (fileExists(fn = _scene->data()->basePathName() + "/capreal/mesh.ply")) {
				if (sdv_mesh.load(fn, _scene->data()->basePathName()))
					success = true;
			}
			// in sibr subdir
			else if (fileExists(fn = _scene->data()->basePathName() + "/../capreal/mesh.ply")) {
				if (sdv_mesh.load(fn, _scene->data()->basePathName()))
					success = true;
			}
			if (success) {
				Mesh::Ptr mp;
				mp.reset(new Mesh);
				mp->merge(sdv_mesh);
				addMesh("proxy", mp);
			}
			else
				addMesh("proxy", _scene->proxies()->proxyPtr());
		}
		else
			addMesh("proxy", _scene->proxies()->proxyPtr());

		// Add a gizmo.
		addMeshAsLines("guizmo", RenderUtility::createAxisGizmo())
			.setDepthTest(false).setColorMode(MeshData::ColorMode::VERTEX);
	}

} // namespace
