//Just a simple handler for simple initialization stuffs
#include "Utilities/BackendHandler.h"
#include "Utilities/Util.h"

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

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/string_cast.hpp"

#include <IBehaviour.h>
#include <CameraControlBehaviour.h>
#include <FollowPathBehaviour.h>
#include <SimpleMoveBehaviour.h>

#define NUM_METEORS 300.0F
#define NUM_METEORS2 100.0F
#define PLANE_X 20.0f
#define PLANE_Y 20.0f
#define DNS_X 11.0f
#define DNS_Y 11.0f

#define P_UPPER 11.0f
#define P_LOWER 0.0f

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
		shader->LoadShaderPartFromFile("shaders/frag_A1_shader.glsl", GL_FRAGMENT_SHADER);
		shader->Link();

		glm::vec3 lightPos = glm::vec3(0.0f, 0.0f, 5.2f);
		glm::vec3 lightCol = glm::vec3(1.0f, 1.0f, 1.0f);
		float     lightAmbientPow = 0.6f;
		float     lightSpecularPow = 0.5f;
		glm::vec3 ambientCol = glm::vec3(1.0f);
		float     ambientPow = 0.42f;
		float     lightLinearFalloff = 0.2f;
		float     lightQuadraticFalloff = 0.29f;

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

		bool Option = true, Option2 = false, Option3 = false, Option4 = false, Option5 = false;

		// We'll add some ImGui controls to control our shader
		BackendHandler::imGuiCallbacks.push_back([&]() {

			if (ImGui::Checkbox("No Lighting", &Option)){
				Option2=false;
				Option3=false;
				Option4=false;
				Option5=false;
			}
			if (ImGui::Checkbox("Ambient Lighting", &Option2)){
				Option = false;
				Option3 = false;
				Option4 = false;
				Option5 = false;
			}
			if (ImGui::Checkbox("Specular Only", &Option3)){
				Option = false;
				Option2 = false;
				Option4 = false;
				Option5 = false;
			}
			if (ImGui::Checkbox("Specular, Ambient and Diffuse", &Option4)){
				Option = false;
				Option2 = false;
				Option3 = false;
				Option5 = false;
			}
			if (ImGui::Checkbox("Specular, Ambient and Toon Shading", &Option5)){
				Option = false;
				Option2 = false;
				Option3 = false;
				Option4 = false;
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
		glEnable(GL_CULL_FACE);
		glDepthFunc(GL_LEQUAL); // New 

		#pragma region TEXTURE LOADING

		// Load some textures from files

		Texture2D::sptr astronaut = Texture2D::LoadFromFile("images/astronaut.png");
		Texture2D::sptr spacerock = Texture2D::LoadFromFile("images/Space_Rock.png");
		Texture2D::sptr spacerock2 = Texture2D::LoadFromFile("images/Space_Rock_2.png");
		Texture2D::sptr spaceship = Texture2D::LoadFromFile("images/spaceship.png");
		Texture2D::sptr spaceshipSpec = Texture2D::LoadFromFile("images/Spaceship_specular.png");
		Texture2D::sptr starSparrow = Texture2D::LoadFromFile("images/StarSparrow_Black.png");
		Texture2D::sptr starSparrowSpec = Texture2D::LoadFromFile("images/StarSparrow_specular.png");
		Texture2D::sptr planet = Texture2D::LoadFromFile("images/planet.png");
		Texture2D::sptr planetSpec = Texture2D::LoadFromFile("images/planet_specular.png");

		// Load the cube map 
		TextureCubeMap::sptr environmentMap = TextureCubeMap::LoadFromImages("images/cubemaps/spacebox/space.png"); 

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
		GameScene::sptr scene = GameScene::Create("A1 Jonathan Narine 100741302");
		Application::Instance().ActiveScene = scene;

		// We can create a group ahead of time to make iterating on the group faster
		entt::basic_group<entt::entity, entt::exclude_t<>, entt::get_t<Transform>, RendererComponent> renderGroup =
			scene->Registry().group<RendererComponent>(entt::get_t<Transform>());

		// Create a material and set some properties for it
		ShaderMaterial::sptr astronautMat = ShaderMaterial::Create();  
		astronautMat->Shader = shader;
		astronautMat->Set("s_Diffuse", astronaut);
		astronautMat->Set("u_Shininess", 2.0f);
		astronautMat->Set("u_TextureMix", 0.0f); 
		
		ShaderMaterial::sptr spacerockMat = ShaderMaterial::Create();
		spacerockMat->Shader = shader;
		spacerockMat->Set("s_Diffuse", spacerock);
		spacerockMat->Set("u_Shininess", 2.0f);
		spacerockMat->Set("u_TextureMix", 0.0f);
		
		ShaderMaterial::sptr spacerockMat2 = ShaderMaterial::Create();
		spacerockMat2->Shader = shader;
		spacerockMat2->Set("s_Diffuse", spacerock2);
		spacerockMat2->Set("u_Shininess", 2.0f);
		spacerockMat2->Set("u_TextureMix", 0.0f);
		
		ShaderMaterial::sptr spaceshipMat= ShaderMaterial::Create();  
		spaceshipMat->Shader = shader;
		spaceshipMat->Set("s_Diffuse", spaceship);
		spaceshipMat->Set("s_Specular", spaceshipSpec);
		spaceshipMat->Set("u_Shininess", 2.0f);
		spaceshipMat->Set("u_TextureMix", 0.0f); 

		ShaderMaterial::sptr starSparrowMat = ShaderMaterial::Create();
		starSparrowMat->Shader = shader;
		starSparrowMat->Set("s_Diffuse", starSparrow);
		starSparrowMat->Set("s_Specular", starSparrowSpec);
		starSparrowMat->Set("u_Shininess", 2.0f);
		starSparrowMat->Set("u_TextureMix", 0.0f);
		
		ShaderMaterial::sptr planetMat = ShaderMaterial::Create();
		planetMat->Shader = shader;
		planetMat->Set("s_Diffuse", planet);
		planetMat->Set("s_Specular", planetSpec);
		planetMat->Set("u_Shininess", 2.0f);
		planetMat->Set("u_TextureMix", 0.0f);
		
		
		GameObject obj1 = scene->CreateEntity("Astronaut"); 
		{
			VertexArrayObject::sptr vao = ObjLoader::LoadFromFile("models/astronaut.obj");
			obj1.emplace<RendererComponent>().SetMesh(vao).SetMaterial(astronautMat);
			obj1.get<Transform>().SetLocalPosition(0.0f, 0.0f, 5.0f);
			obj1.get<Transform>().SetLocalRotation(90.0f, 0.0f, 0.0f);
			BehaviourBinding::BindDisabled<SimpleMoveBehaviour>(obj1);
		}

		GameObject obj2 = scene->CreateEntity("Planet");
		{
			VertexArrayObject::sptr vao = ObjLoader::LoadFromFile("models/planet.obj");
			obj2.emplace<RendererComponent>().SetMesh(vao).SetMaterial(planetMat);
			obj2.get<Transform>().SetLocalPosition(0.0f, -12.0f, 0.0f);
			obj2.get<Transform>().SetLocalScale(2.0f, 2.0f, 2.0f);
			obj2.get<Transform>().SetLocalRotation(90.0f, 0.0f, 0.0f);
			BehaviourBinding::BindDisabled<SimpleMoveBehaviour>(obj2);
		}
		
		GameObject obj3 = scene->CreateEntity("Spaceship");
		{
			VertexArrayObject::sptr vao = ObjLoader::LoadFromFile("models/spaceship.obj");
			obj3.emplace<RendererComponent>().SetMesh(vao).SetMaterial(spaceshipMat);
			obj3.get<Transform>().SetLocalPosition(0.0f, 0.0f, 4.9f);
			obj3.get<Transform>().SetLocalRotation(90.0f, 0.0f, 0.0f);
			BehaviourBinding::BindDisabled<SimpleMoveBehaviour>(obj3);
		}

		
		GameObject obj4 = scene->CreateEntity("starSparrow"); 
		{
			VertexArrayObject::sptr vao = ObjLoader::LoadFromFile("models/starSparrow.obj");
			obj4.emplace<RendererComponent>().SetMesh(vao).SetMaterial(starSparrowMat);
			obj4.get<Transform>().SetLocalPosition(-6.0f, -4.0f, 5.0f);
			obj4.get<Transform>().SetLocalRotation(90.0f, 0.0f, 0.0f);

			// Bind returns a smart pointer to the behaviour that was added
			auto pathing = BehaviourBinding::Bind<FollowPathBehaviour>(obj4);
			// Set up a path for the object to follow
			pathing->Points.push_back({ -3.0f, -6.0f, 7.0f });
			pathing->Points.push_back({ -3.0f, 8.0f, 7.0f });
			pathing->Points.push_back({ 10.0f, 8.0f, 7.0f });
			pathing->Points.push_back({ 10.0f, -6.0f, 7.0f });
			pathing->Speed = 4.0f;
		}

		std::vector<GameObject> randomMetoroids;
		{
			VertexArrayObject::sptr vao = ObjLoader::LoadFromFile("models/space_rock.obj");
			for (int i = 0; i < NUM_METEORS/2; i++)
			{
				randomMetoroids.push_back(scene->CreateEntity("simpleRock" + (std::to_string(i + 1))));
				randomMetoroids[i].emplace<RendererComponent>().SetMesh(vao).SetMaterial(spacerockMat);
				//Randomly places
				randomMetoroids[i].get<Transform>().SetLocalPosition(glm::vec3(Util::GetRandomNumberBetween(glm::vec2(-PLANE_X, -PLANE_Y), glm::vec2(PLANE_X, PLANE_Y), glm::vec2(-DNS_X, -DNS_Y), glm::vec2(DNS_X, DNS_Y)), rand() % 11));
				randomMetoroids[i].get<Transform>().SetLocalRotation(rand()%360, rand() % 360,0);
			}
		}
		
		std::vector<GameObject> randomMetoroids2;
		{
			VertexArrayObject::sptr vao = ObjLoader::LoadFromFile("models/space_rock2.obj");
			for (int i = 0; i < NUM_METEORS2/2; i++)
			{
				randomMetoroids2.push_back(scene->CreateEntity("complexRock" + (std::to_string(i + 1))));
				randomMetoroids2[i].emplace<RendererComponent>().SetMesh(vao).SetMaterial(spacerockMat);
				//Randomly places
				randomMetoroids2[i].get<Transform>().SetLocalPosition(glm::vec3(Util::GetRandomNumberBetween(glm::vec2(-PLANE_X, -PLANE_Y), glm::vec2(PLANE_X, PLANE_Y), glm::vec2(-DNS_X, -DNS_Y), glm::vec2(DNS_X, DNS_Y)), rand() % 11));
				randomMetoroids2[i].get<Transform>().SetLocalRotation(rand()%360, rand() % 360,0);
			}
		}

		// Create an object to be our camera
		GameObject cameraObject = scene->CreateEntity("Camera");
		{
			cameraObject.get<Transform>().SetLocalPosition(2.52, -12.39, 12.97).LookAt(glm::vec3(0, 0, 0));

			// We'll make our camera a component of the camera object
			Camera& camera = cameraObject.emplace<Camera>();// Camera::Create();
			camera.SetPosition(glm::vec3(0, -10, 0));
			camera.SetUp(glm::vec3(0, 1, 0));
			camera.LookAt(glm::vec3(0));
			camera.SetFovDegrees(90.0f); // Set an initial FOV
			camera.SetOrthoHeight(3.0f);
			BehaviourBinding::Bind<CameraControlBehaviour>(cameraObject);
		}

		Framebuffer* testBuffer;
		GameObject framebufferObject = scene->CreateEntity("Basic Buffer");
		{
			int width, height;
			glfwGetWindowSize(BackendHandler::window, &width, &height);

			testBuffer = &framebufferObject.emplace<Framebuffer>();
			testBuffer->AddDepthTarget();
			testBuffer->AddColorTarget(GL_RGBA8);
			testBuffer->Init(width, height);
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
				// We need to poll our key watchers so tdhey can do their logic with the GLFW state
				// Note that since we want to make sure we don't copy our key handlers, we need a const
				// refereasnce!s
				for (const KeyPressWatcher& watcher : keyToggles) {
					watcher.Poll(BackendHandler::window);
				}
			}

			shader->SetUniform("u_Option1", (int)Option);
			shader->SetUniform("u_Option2", (int)Option2);
			shader->SetUniform("u_Option3", (int)Option3);
			shader->SetUniform("u_Option4", (int)Option4);
			shader->SetUniform("u_Option5", (int)Option5);


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
			testBuffer->Clear();

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

			//std::cout << glm::to_string(camTransform.GetLocalPosition()) << std::endl;

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

			testBuffer->Bind();

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

			testBuffer->Unbind();

			testBuffer->DrawToBackbuffer();


			// Draw our ImGui content
			BackendHandler::RenderImGui();

			scene->Poll();
			glfwSwapBuffers(BackendHandler::window);
			time.LastFrame = time.CurrentFrame;
		}

		// Nullify scene so that we can release references
		Application::Instance().ActiveScene = nullptr;
		BackendHandler::ShutdownImGui();
	}	

	// Clean up the toolkit logger so we don't leak memory
	Logger::Uninitialize();
	return 0;
}