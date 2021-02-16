//Just a simple handler for simple initialization stuffs
#include "Utilities/BackendHandler.h"

#include <filesystem>
#include <json.hpp>
#include <fstream>

#include <Texture2D.h>
#include <Texture2DData.h>
#include <MeshBuilder.h>
#include <MeshFactory.h>
#include <NotObjLoader.h>
#include <ObjLoader.h>
#include <VertexTypes.h>
#include <ShaderMaterial.h>
#include <RendererComponent.h>
#include <TextureCubeMap.h>
#include <TextureCubeMapData.h>

#include <Timing.h>
#include <GameObjectTag.h>
#include <InputHelpers.h>

#include <IBehaviour.h>
#include <CameraControlBehaviour.h>
#include <FollowPathBehaviour.h>
#include <SimpleMoveBehaviour.h>

int lightingToggle = 1;
bool isTextureColor = true;
bool isToonShading = false;

KeyPressWatcher isTextureColorWatch = KeyPressWatcher(GLFW_KEY_6, [&]() {
	isTextureColor = !isTextureColor;
	});
	
KeyPressWatcher isToonShadingWatch = KeyPressWatcher(GLFW_KEY_7, [&]() {
	isToonShading = !isToonShading;
	});


int main() {
	int frameIx = 0;
	float fpsBuffer[128];
	float minFps, maxFps, avgFps;
	int selectedVao = 0; // select cube by default
	std::vector<GameObject> controllables;

	BackendHandler::InitAll();

	// Let OpenGL know that we want debug output, and route it to our handler function
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(BackendHandler::GlDebugMessage, nullptr);

	// Enable texturing
	glEnable(GL_TEXTURE_2D);

	// Push another scope so most memory should be freed *before* we exit the app
	{
		#pragma region Shader and ImGui
		// Load our shaders
		Shader::sptr shader = Shader::Create();
		shader->LoadShaderPartFromFile("shaders/vertex_shader.glsl", GL_VERTEX_SHADER);
		shader->LoadShaderPartFromFile("shaders/frag_Midterm.glsl", GL_FRAGMENT_SHADER);
		shader->Link();

		glm::vec3 lightPos = glm::vec3(0.0f, 0.0f, 5.0f);
		glm::vec3 lightCol = glm::vec3(0.9f, 0.85f, 0.5f);
		float     lightAmbientPow = 0.05f;
		float     lightSpecularPow = 1.0f;
		glm::vec3 ambientCol = glm::vec3(1.0f);
		float     ambientPow = 0.1f;
		float     lightLinearFalloff = 0.09f;
		float     lightQuadraticFalloff = 0.032f;

		// These are our application / scene level uniforms that don't necessarily update
		// every frame
		shader->SetUniform("u_LightPos", lightPos);
		shader->SetUniform("u_LightCol", lightCol);
		shader->SetUniform("u_AmbientLightStrength", lightAmbientPow);
		shader->SetUniform("u_SpecularLightStrength", lightSpecularPow);
		shader->SetUniform("u_AmbientCol", ambientCol);
		shader->SetUniform("u_AmbientStrength", ambientPow);
		shader->SetUniform("u_LightAttenuationConstant", 1.0f);
		shader->SetUniform("u_LightAttenuationLinear", lightLinearFalloff);
		shader->SetUniform("u_LightAttenuationQuadratic", lightQuadraticFalloff);

		//Bloom variables
		float BrightnessThreshold = 0;
		int BlurPasses = 10;

		//Toon Variables
		int Bands=6;
		float Intensity=40;
		float ScaleValue=2;

		PostEffect* basicEffect;
		PostEffect* bloomEffect;

		// We'll add some ImGui controls to control our shader
		BackendHandler::imGuiCallbacks.push_back([&]() {

			if (ImGui::CollapsingHeader("Midterm Bloom Lighting Settings"))
			{
				if (ImGui::SliderFloat("Brightness Threshold", &BrightnessThreshold, 0.0f, 0.5f)) {}
				if (ImGui::SliderInt("Blur Passes", &BlurPasses, 0.0f, 15.0f)) {}
			}
			if (ImGui::CollapsingHeader("Midterm Toon Shading Lighting Settings"))
			{
				if (ImGui::SliderInt("Bands", &Bands, 0.0f, 16.0f)) {
					shader->SetUniform("bands", Bands);
				}
				if (ImGui::SliderFloat("Light Intensity", &Intensity, 0.0f, 60.0f)) {
					shader->SetUniform("lightIntensity", Intensity);
				}
				if (ImGui::SliderFloat("Scale Factor", &ScaleValue, 0.0f, 20.0f)) {
					shader->SetUniform("scaleValue", ScaleValue);
				}
			}
			if (ImGui::CollapsingHeader("Scene Level Lighting Settings"))
			{
				if (ImGui::ColorPicker3("Ambient Color", glm::value_ptr(ambientCol))) {
					shader->SetUniform("u_AmbientCol", ambientCol);
				}
				if (ImGui::SliderFloat("Fixed Ambient Power", &ambientPow, 0.01f, 1.0f)) {
					shader->SetUniform("u_AmbientStrength", ambientPow);
				}
			}
			if (ImGui::CollapsingHeader("Light Level Lighting Settings"))
			{
				if (ImGui::DragFloat3("Light Pos", glm::value_ptr(lightPos), 0.01f, -10.0f, 10.0f)) {
					shader->SetUniform("u_LightPos", lightPos);
				}
				if (ImGui::ColorPicker3("Light Col", glm::value_ptr(lightCol))) {
					shader->SetUniform("u_LightCol", lightCol);
				}
				if (ImGui::SliderFloat("Light Ambient Power", &lightAmbientPow, 0.0f, 1.0f)) {
					shader->SetUniform("u_AmbientLightStrength", lightAmbientPow);
				}
				if (ImGui::SliderFloat("Light Specular Power", &lightSpecularPow, 0.0f, 1.0f)) {
					shader->SetUniform("u_SpecularLightStrength", lightSpecularPow);
				}
				if (ImGui::DragFloat("Light Linear Falloff", &lightLinearFalloff, 0.01f, 0.0f, 1.0f)) {
					shader->SetUniform("u_LightAttenuationLinear", lightLinearFalloff);
				}
				if (ImGui::DragFloat("Light Quadratic Falloff", &lightQuadraticFalloff, 0.01f, 0.0f, 1.0f)) {
					shader->SetUniform("u_LightAttenuationQuadratic", lightQuadraticFalloff);
				}
			}

			auto name = controllables[selectedVao].get<GameObjectTag>().Name;
			ImGui::Text(name.c_str());
			auto behaviour = BehaviourBinding::Get<SimpleMoveBehaviour>(controllables[selectedVao]);
			ImGui::Checkbox("Relative Rotation", &behaviour->Relative);

			ImGui::Text("Q/E -> Yaw\nLeft/Right -> Roll\nUp/Down -> Pitch\nY -> Toggle Mode");
		
			minFps = FLT_MAX;
			maxFps = 0;
			avgFps = 0;
			for (int ix = 0; ix < 128; ix++) {
				if (fpsBuffer[ix] < minFps) { minFps = fpsBuffer[ix]; }
				if (fpsBuffer[ix] > maxFps) { maxFps = fpsBuffer[ix]; }
				avgFps += fpsBuffer[ix];
			}
			ImGui::PlotLines("FPS", fpsBuffer, 128);
			ImGui::Text("MIN: %f MAX: %f AVG: %f", minFps, maxFps, avgFps / 128.0f);
			});

		#pragma endregion 

		// GL states
		glEnable(GL_DEPTH_TEST);
		//glEnable(GL_CULL_FACE);
		glDepthFunc(GL_LEQUAL); // New 

		#pragma region TEXTURE LOADING

		// Load some textures from files
		Texture2D::sptr grassStrands = Texture2D::LoadFromFile("images/GrassStrands.png");
		Texture2D::sptr grassStrandsSpec = Texture2D::LoadFromFile("images/GrassStrands_Spec.png");
		Texture2D::sptr Ground = Texture2D::LoadFromFile("images/Ground.png");
		Texture2D::sptr GroundSpec = Texture2D::LoadFromFile("images/Ground_Spec.png");
		Texture2D::sptr Ladybug = Texture2D::LoadFromFile("images/Ladybug.png");
		Texture2D::sptr LadybugSpec = Texture2D::LoadFromFile("images/Ladybug_Spec.png");
		Texture2D::sptr Trees = Texture2D::LoadFromFile("images/Trees.png");
		Texture2D::sptr Trees_Spec = Texture2D::LoadFromFile("images/Ladybug_Spec.png");
		Texture2D::sptr Well = Texture2D::LoadFromFile("images/Well.png");
		Texture2D::sptr WellSpec = Texture2D::LoadFromFile("images/Well_Spec.png");

		// Load the cube map 
		TextureCubeMap::sptr environmentMap = TextureCubeMap::LoadFromImages("images/cubemaps/skybox/ToonSky.jpg");

		// Creating an empty texture
		Texture2DDescription desc = Texture2DDescription();  
		desc.Width = 1;
		desc.Height = 1;
		desc.Format = InternalFormat::RGB8;
		Texture2D::sptr texture2 = Texture2D::Create(desc);
		// Clear it with a white colour
		texture2->Clear();

		#pragma endregion

		///////////////////////////////////// Scene Generation //////////////////////////////////////////////////
		#pragma region Scene Generation
		
		// We need to tell our scene system what extra component types we want to support
		GameScene::RegisterComponentType<RendererComponent>();
		GameScene::RegisterComponentType<BehaviourBinding>();
		GameScene::RegisterComponentType<Camera>();

		// Create a scene, and set it to be the active scene in the application
		GameScene::sptr scene = GameScene::Create("ILE 2");
		Application::Instance().ActiveScene = scene;

		// We can create a group ahead of time to make iterating on the group faster
		entt::basic_group<entt::entity, entt::exclude_t<>, entt::get_t<Transform>, RendererComponent> renderGroup =
			scene->Registry().group<RendererComponent>(entt::get_t<Transform>());

		// Create a material and set some properties for it
		ShaderMaterial::sptr GrassMat = ShaderMaterial::Create();
		GrassMat->Shader = shader;
		GrassMat->Set("s_Diffuse", grassStrands);
		GrassMat->Set("s_Specular", grassStrandsSpec);
		GrassMat->Set("u_Shininess", 2.0f);
		GrassMat->Set("u_TextureMix", 0.0f);

		ShaderMaterial::sptr GroundMat = ShaderMaterial::Create();
		GroundMat->Shader = shader;
		GroundMat->Set("s_Diffuse", Ground);
		GroundMat->Set("s_Specular", GroundSpec);
		GroundMat->Set("u_Shininess", 2.0f);
		GroundMat->Set("u_TextureMix", 0.0f);

		ShaderMaterial::sptr LadybugMat = ShaderMaterial::Create();
		LadybugMat->Shader = shader;
		LadybugMat->Set("s_Diffuse", Ladybug);
		LadybugMat->Set("s_Specular", LadybugSpec);
		LadybugMat->Set("u_Shininess", 2.0f);
		LadybugMat->Set("u_TextureMix", 0.0f);

		ShaderMaterial::sptr TreesMat = ShaderMaterial::Create();
		TreesMat->Shader = shader;
		TreesMat->Set("s_Diffuse", Trees);
		TreesMat->Set("s_Specular", Trees_Spec);
		TreesMat->Set("u_Shininess", 2.0f);
		TreesMat->Set("u_TextureMix", 0.0f);

		ShaderMaterial::sptr WellMat = ShaderMaterial::Create();
		WellMat->Shader = shader;
		WellMat->Set("s_Diffuse", Well);
		WellMat->Set("s_Specular", WellSpec);
		WellMat->Set("u_Shininess", 2.0f);
		WellMat->Set("u_TextureMix", 0.0f);

		GameObject obj1 = scene->CreateEntity("Grass");
		{
			VertexArrayObject::sptr vao = ObjLoader::LoadFromFile("models/Grass.obj");
			obj1.emplace<RendererComponent>().SetMesh(vao).SetMaterial(GrassMat);
			obj1.get<Transform>().SetLocalPosition(0.0f, 0.0f, 0.0f);
			obj1.get<Transform>().SetLocalRotation(90.0f, 0.0f, 0.0f);
			BehaviourBinding::BindDisabled<SimpleMoveBehaviour>(obj1);
		}

		GameObject obj2 = scene->CreateEntity("Ground");
		{
			VertexArrayObject::sptr vao = ObjLoader::LoadFromFile("models/Ground.obj");
			obj2.emplace<RendererComponent>().SetMesh(vao).SetMaterial(GroundMat);
			obj2.get<Transform>().SetLocalPosition(0.0f, 0.0f, 0.0f);
			obj2.get<Transform>().SetLocalRotation(90.0f, 0.0f, 0.0f);
			BehaviourBinding::BindDisabled<SimpleMoveBehaviour>(obj2);
		}

		GameObject obj3 = scene->CreateEntity("Ladybug");
		{
			VertexArrayObject::sptr vao = ObjLoader::LoadFromFile("models/Ladybug.obj");
			obj3.emplace<RendererComponent>().SetMesh(vao).SetMaterial(LadybugMat);
			obj3.get<Transform>().SetLocalPosition(0.0f, 0.0f, 0.5f);
			obj3.get<Transform>().SetLocalRotation(90.0f, 0.0f, 0.0f);

			// Bind returns a smart pointer to the behaviour that was added
			auto pathing = BehaviourBinding::Bind<FollowPathBehaviour>(obj3);
			// Set up a path for the object to follow
			pathing->Points.push_back({ 0.0f, -4.0f, 10.0f });
			pathing->Points.push_back({ 0.0f, 4.0f, 10.0f });
			pathing->Points.push_back({ 8.0f, 4.0f, 10.0f });
			pathing->Points.push_back({ 8.0f, -4.0f, 10.0f });
			pathing->Speed = 4.0f;
		}

		GameObject obj4 = scene->CreateEntity("Trees");
		{
			VertexArrayObject::sptr vao = ObjLoader::LoadFromFile("models/Trees.obj");
			obj4.emplace<RendererComponent>().SetMesh(vao).SetMaterial(TreesMat);
			obj4.get<Transform>().SetLocalPosition(0.0f, 0.0f, 0.0f);
			obj4.get<Transform>().SetLocalRotation(90.0f, 0.0f, 0.0f);
			BehaviourBinding::BindDisabled<SimpleMoveBehaviour>(obj4);
		}

		GameObject obj5 = scene->CreateEntity("Well");
		{
			VertexArrayObject::sptr vao = ObjLoader::LoadFromFile("models/Well.obj");
			obj5.emplace<RendererComponent>().SetMesh(vao).SetMaterial(WellMat);
			obj5.get<Transform>().SetLocalPosition(0.0f, 0.0f, 0.0f);
			obj5.get<Transform>().SetLocalRotation(90.0f, 0.0f, 0.0f);
			BehaviourBinding::BindDisabled<SimpleMoveBehaviour>(obj5);
		}

		// Create an object to be our camera
		GameObject cameraObject = scene->CreateEntity("Camera");
		{
			cameraObject.get<Transform>().SetLocalPosition(0, 3, 3).LookAt(glm::vec3(0, 0, 0));

			// We'll make our camera a component of the camera object
			Camera& camera = cameraObject.emplace<Camera>();// Camera::Create();
			camera.SetPosition(glm::vec3(0, 3, 3));
			camera.SetUp(glm::vec3(0, 1, 0));
			camera.LookAt(glm::vec3(0));
			camera.SetFovDegrees(90.0f); // Set an initial FOV
			camera.SetOrthoHeight(3.0f);
			BehaviourBinding::Bind<CameraControlBehaviour>(cameraObject);
		}

		int width, height;
		glfwGetWindowSize(BackendHandler::window, &width, &height);

		GameObject framebufferObject = scene->CreateEntity("Basic Effect");
		{
			basicEffect = &framebufferObject.emplace<PostEffect>();
			basicEffect->Init(width, height);
		}

		GameObject bloomObject = scene->CreateEntity("Bloom Effect");
		{
			bloomEffect = &bloomObject.emplace<BloomEffect>();
			bloomEffect->Init(width, height);
		}

		#pragma endregion 
		//////////////////////////////////////////////////////////////////////////////////////////

		/////////////////////////////////// SKYBOX ///////////////////////////////////////////////
		{
			// Load our shaders
			Shader::sptr skybox = std::make_shared<Shader>();
			skybox->LoadShaderPartFromFile("shaders/skybox-shader.vert.glsl", GL_VERTEX_SHADER);
			skybox->LoadShaderPartFromFile("shaders/skybox-shader.frag.glsl", GL_FRAGMENT_SHADER);
			skybox->Link();

			ShaderMaterial::sptr skyboxMat = ShaderMaterial::Create();
			skyboxMat->Shader = skybox;  
			skyboxMat->Set("s_Environment", environmentMap);
			skyboxMat->Set("u_EnvironmentRotation", glm::mat3(glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1, 0, 0))));
			skyboxMat->RenderLayer = 100;

			MeshBuilder<VertexPosNormTexCol> mesh;
			MeshFactory::AddIcoSphere(mesh, glm::vec3(0.0f), 1.0f);
			MeshFactory::InvertFaces(mesh);
			VertexArrayObject::sptr meshVao = mesh.Bake();
			
			GameObject skyboxObj = scene->CreateEntity("skybox");  
			skyboxObj.get<Transform>().SetLocalPosition(0.0f, 0.0f, 0.0f);
			skyboxObj.get_or_emplace<RendererComponent>().SetMesh(meshVao).SetMaterial(skyboxMat);
		}
		////////////////////////////////////////////////////////////////////////////////////////


		// We'll use a vector to store all our key press events for now (this should probably be a behaviour eventually)
		std::vector<KeyPressWatcher> keyToggles;
		{
			// This is an example of a key press handling helper. Look at InputHelpers.h an .cpp to see
			// how this is implemented. Note that the ampersand here is capturing the variables within
			// the scope. If you wanted to do some method on the class, your best bet would be to give it a method and
			// use std::bind
			keyToggles.emplace_back(GLFW_KEY_T, [&]() { cameraObject.get<Camera>().ToggleOrtho(); });

			controllables.push_back(obj2);

			keyToggles.emplace_back(GLFW_KEY_KP_ADD, [&]() {
				BehaviourBinding::Get<SimpleMoveBehaviour>(controllables[selectedVao])->Enabled = false;
				selectedVao++;
				if (selectedVao >= controllables.size())
					selectedVao = 0;
				BehaviourBinding::Get<SimpleMoveBehaviour>(controllables[selectedVao])->Enabled = true;
				});
			keyToggles.emplace_back(GLFW_KEY_KP_SUBTRACT, [&]() {
				BehaviourBinding::Get<SimpleMoveBehaviour>(controllables[selectedVao])->Enabled = false;
				selectedVao--;
				if (selectedVao < 0)
					selectedVao = controllables.size() - 1;
				BehaviourBinding::Get<SimpleMoveBehaviour>(controllables[selectedVao])->Enabled = true;
				});

			keyToggles.emplace_back(GLFW_KEY_Y, [&]() {
				auto behaviour = BehaviourBinding::Get<SimpleMoveBehaviour>(controllables[selectedVao]);
				behaviour->Relative = !behaviour->Relative;
				});
		}

		// Initialize our timing instance and grab a reference for our use
		Timing& time = Timing::Instance();
		time.LastFrame = glfwGetTime();

		///// Game loop /////
		while (!glfwWindowShouldClose(BackendHandler::window)) {
			glfwPollEvents();

			// Update the timing
			time.CurrentFrame = glfwGetTime();
			time.DeltaTime = static_cast<float>(time.CurrentFrame - time.LastFrame);

			time.DeltaTime = time.DeltaTime > 1.0f ? 1.0f : time.DeltaTime;

			// Update our FPS tracker data
			fpsBuffer[frameIx] = 1.0f / time.DeltaTime;
			frameIx++;
			if (frameIx >= 128)
				frameIx = 0;

			// We'll make sure our UI isn't focused before we start handling input for our game
			if (!ImGui::IsAnyWindowFocused()) {
				// We need to poll our key watchers so they can do their logic with the GLFW state
				// Note that since we want to make sure we don't copy our key handlers, we need a const
				// reference!
				for (const KeyPressWatcher& watcher : keyToggles) {
					watcher.Poll(BackendHandler::window);
				}
			}

			if (glfwGetKey(BackendHandler::window, GLFW_KEY_1) == GLFW_PRESS)
			{
				lightingToggle = 1;
			}
			if (glfwGetKey(BackendHandler::window, GLFW_KEY_2) == GLFW_PRESS)
			{
				lightingToggle = 2;
			}
			if (glfwGetKey(BackendHandler::window, GLFW_KEY_3) == GLFW_PRESS)
			{
				lightingToggle = 3;
			}
			if (glfwGetKey(BackendHandler::window, GLFW_KEY_4) == GLFW_PRESS)
			{
				lightingToggle = 4;
			}
			if (glfwGetKey(BackendHandler::window, GLFW_KEY_5) == GLFW_PRESS)
			{
				lightingToggle = 5;
			}
			
			isTextureColorWatch.Poll(BackendHandler::window);
			isToonShadingWatch.Poll(BackendHandler::window);
			
			if (lightingToggle < 1 || lightingToggle > 5) {
				lightingToggle = 1;
			}

			shader->SetUniform("u_TextureColor", (int)isTextureColor);
			shader->SetUniform("u_ToonShading", (int)isToonShading);
			shader->SetUniform("u_LightingToggle", lightingToggle);

			// Iterate over all the behaviour binding components
			scene->Registry().view<BehaviourBinding>().each([&](entt::entity entity, BehaviourBinding& binding) {
				// Iterate over all the behaviour scripts attached to the entity, and update them in sequence (if enabled)
				for (const auto& behaviour : binding.Behaviours) {
					if (behaviour->Enabled) {
						behaviour->Update(entt::handle(scene->Registry(), entity));
					}
				}
			});

			// Clear the screen
			basicEffect->Clear();
			bloomEffect->Clear();

			glClearColor(0.08f, 0.17f, 0.31f, 1.0f);
			glEnable(GL_DEPTH_TEST);
			glClearDepth(1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// Update all world matrices for this frame
			scene->Registry().view<Transform>().each([](entt::entity entity, Transform& t) {
				t.UpdateWorldMatrix();
			});
			
			// Grab out camera info from the camera object
			Transform& camTransform = cameraObject.get<Transform>();
			glm::mat4 view = glm::inverse(camTransform.LocalTransform());
			glm::mat4 projection = cameraObject.get<Camera>().GetProjection();
			glm::mat4 viewProjection = projection * view;
						
			// Sort the renderers by shader and material, we will go for a minimizing context switches approach here,
			// but you could for instance sort front to back to optimize for fill rate if you have intensive fragment shaders
			renderGroup.sort<RendererComponent>([](const RendererComponent& l, const RendererComponent& r) {
				// Sort by render layer first, higher numbers get drawn last
				if (l.Material->RenderLayer < r.Material->RenderLayer) return true;
				if (l.Material->RenderLayer > r.Material->RenderLayer) return false;

				// Sort by shader pointer next (so materials using the same shader run sequentially where possible)
				if (l.Material->Shader < r.Material->Shader) return true;
				if (l.Material->Shader > r.Material->Shader) return false;

				// Sort by material pointer last (so we can minimize switching between materials)
				if (l.Material < r.Material) return true;
				if (l.Material > r.Material) return false;
				
				return false;
			});


			// Start by assuming no shader or material is applied
			Shader::sptr current = nullptr;
			ShaderMaterial::sptr currentMat = nullptr;

			basicEffect->BindBuffer(0);

			// Iterate over the render group components and draw them
			renderGroup.each( [&](entt::entity e, RendererComponent& renderer, Transform& transform) {
				// If the shader has changed, set up it's uniforms
				if (current != renderer.Material->Shader) {
					current = renderer.Material->Shader;
					current->Bind();
					BackendHandler::SetupShaderForFrame(current, view, projection);
				}
				// If the material has changed, apply it
				if (currentMat != renderer.Material) {
					currentMat = renderer.Material;
					currentMat->Apply();
				}
				// Render the mesh
				BackendHandler::RenderVAO(renderer.Material->Shader, renderer.Mesh, viewProjection, transform);
			});

			basicEffect->UnbindBuffer();

			if (lightingToggle == 5) {
				auto temp = &bloomObject.get<BloomEffect>();
				temp->SetThreshold(BrightnessThreshold);
				temp->SetPasses(BlurPasses);
				bloomEffect->ApplyEffect(basicEffect);
				bloomEffect->DrawToScreen();
			}else{		
				basicEffect->DrawToScreen();
			}

			// Draw our ImGui content
			BackendHandler::RenderImGui();

			scene->Poll();
			glfwSwapBuffers(BackendHandler::window);
			time.LastFrame = time.CurrentFrame;
		}

		// Nullify scene so that we can release references
		Application::Instance().ActiveScene = nullptr;
		//Clean up the environment generator so we can release references
		EnvironmentGenerator::CleanUpPointers();
		BackendHandler::ShutdownImGui();
	}	

	// Clean up the toolkit logger so we don't leak memory
	Logger::Uninitialize();
	return 0;
}
			