#include "Application.h"
#include "Assets/AssetManager.h"
#include "EventSystem/LayerStack.h"
#include "EventSystem/EventBus.h"
#include "Graphics/D3D12Core.h"
#include "Graphics/Renderer/MasterRenderer.h"
#include "Graphics/MemoryManager.h"
#include "Input/Mouse.h"
#include "Timer.h"
#include "UI/UI.h"
#include "Window.h"

#include "Math/Vector3.h"

#include "Utility/SystemPaths.h"

namespace Relentless
{
	Application* Application::s_Instance = nullptr;

	Application& Application::Get() noexcept
	{
		return *s_Instance;
	}

	Application::Application(const ApplicationSpecification& applicationSpecification) noexcept
		: m_ApplicationSpecification{ applicationSpecification }
	{
		RLS_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;

		OnStartUp();
	}

	void Application::Run() noexcept
	{
		MasterRenderer::ExecuteCommands();
		MasterRenderer::WaitForGPU();
		while (m_IsRunning)
		{
			PROFILE_FUNC;

			ExecuteMainThreadQueue();

			Timer::Update();
			
			
			MemoryManager::Get().PerformDeferredDeletion();

			{
				PROFILE_SCOPE("Application::Run::OnUpdateAndRender");

				for (auto& pLayer : LayerStack::Get())
				{
					pLayer->OnUpdate(Timer::GetDeltaTime());
					pLayer->OnRender();
				}
			}

			{
				PROFILE_SCOPE("Application::Run::OnImGuiRender");

				ImguiLayer::BeginFrame();
				for (auto& pLayer : LayerStack::Get())
					pLayer->OnImGuiRender();
				ImguiLayer::EndFrame();
			}

			MasterRenderer::PrepareBackBuffer();
			MemoryManager::Get().GetUploadBuffer()->Upload();
			MasterRenderer::ExecuteCommands();


			Window::Present();
			MasterRenderer::WaitAndSync();

			{
				PROFILE_SCOPE("Application::Run::OnPostRender");

				for (auto& pLayer : LayerStack::Get())
				{
					pLayer->OnPostRender();
				}
			}

			Mouse::Reset();

			Window::OnUpdate();
			D3D12Core::AdvanceToNextFrame();
		}
		ShutDown();
	}

	void Application::PushLayer(Layer* pLayer) const noexcept
	{
		LayerStack::Get().PushLayer(pLayer);
	}

	void Application::PushOverlay(Layer* pLayer) const noexcept
	{
		LayerStack::Get().PushOverlay(pLayer);
	}

	void Application::OnEvent(IEvent& event) noexcept
	{
		switch (event.GetEventType())
		{
		case EventType::WindowClosedEvent:
			m_IsRunning = false;
			break;
		case EventType::WindowResizedEvent:
		{
			if (!IsInitialized())
				return;

			MasterRenderer::WaitForGPU();
			Window::Resize();
			break;
		}
		}
	}

	void Application::SubmitToMainThread(const std::function<void()>& func)
	{
		std::scoped_lock(m_MainThreadFunctionQueueMutex);
		m_MainThreadFunctionQueue.push(func);
	}


	ThreadPool& Application::GetThreadPool() noexcept
	{
		return m_ThreadPool;
	}

	void run_vector3_tests() {

		auto assert_equal = [](auto value, auto expected, const char* message) {
			if (std::fabs(value - expected) > 0.0001f) {  // Floating point precision check
				std::cerr << "Test failed: " << message << " | Expected: " << expected << ", Got: " << value << std::endl;
			}
			else {
				std::cout << "Test passed: " << message << std::endl;
			}
		};

		// Test Vector3 addition
		auto test_add = [&]() {
			Vector3<float> v1(1.0f, 2.0f, 3.0f);
			Vector3<float> v2(4.0f, 5.0f, 6.0f);
			Vector3<float> result = v1 + v2;
			assert_equal(result.x, 5.0f, "Addition X");
			assert_equal(result.y, 7.0f, "Addition Y");
			assert_equal(result.z, 9.0f, "Addition Z");
		};

		// Test Vector3 subtraction
		auto test_subtract = [&]() {
			Vector3<float> v1(5.0f, 7.0f, 9.0f);
			Vector3<float> v2(1.0f, 2.0f, 3.0f);
			Vector3<float> result = v1 - v2;
			assert_equal(result.x, 4.0f, "Subtraction X");
			assert_equal(result.y, 5.0f, "Subtraction Y");
			assert_equal(result.z, 6.0f, "Subtraction Z");
		};

		// Test Vector3 multiplication by scalar
		auto test_multiply_scalar = [&]() {
			Vector3<float> v(1.0f, 2.0f, 3.0f);
			float scalar = 2.0f;
			Vector3<float> result = v * scalar;
			assert_equal(result.x, 2.0f, "Multiplication by scalar X");
			assert_equal(result.y, 4.0f, "Multiplication by scalar Y");
			assert_equal(result.z, 6.0f, "Multiplication by scalar Z");
		};

		// Test Vector3 dot product
		auto test_dot = [&]() {
			Vector3<float> v1(1.0f, 2.0f, 3.0f);
			Vector3<float> v2(4.0f, 5.0f, 6.0f);
			float result = v1.Dot(v2);
			assert_equal(result, 32.0f, "Dot Product");
		};

		// Test Vector3 cross product
		auto test_cross = [&]() {
			Vector3<float> v1(1.0f, 2.0f, 3.0f);
			Vector3<float> v2(4.0f, 5.0f, 6.0f);
			Vector3<float> result = v1.Cross(v2);
			assert_equal(result.x, -3.0f, "Cross Product X");
			assert_equal(result.y, 6.0f, "Cross Product Y");
			assert_equal(result.z, -3.0f, "Cross Product Z");
		};

		// Test Vector3 length
		auto test_length = [&]() {
			Vector3<float> v(3.0f, 4.0f, 0.0f);
			float length = v.Length();
			assert_equal(length, 5.0f, "Length");
		};

		// Test Vector3 normalization
		auto test_normalize = [&]() {
			Vector3<float> v(3.0f, 4.0f, 0.0f);
			v.Normalize();
			assert_equal(v.Length(), 1.0f, "Normalization Length");
		};

		// Test Vector3 are parallel
		auto test_are_parallel = [&]() {
			Vector3<float> v1(1.0f, 1.0f, 1.0f);
			Vector3<float> v2(2.0f, 2.0f, 2.0f); // Parallel to v1
			bool result = v1.IsParallel(v2);
			assert_equal(result, true, "Are Parallel");
		};

		auto test_distance = [&]() {
			Vector3<float> v1(1.0f, 2.0f, 3.0f);
			Vector3<float> v2(4.0f, 6.0f, 8.0f);
			float result = Vector3f::Distance(v1, v2);
			assert_equal(result, std::sqrt(50.0f), "Distance");
		};

		// Test Vector3 angle
		auto test_angle = [&]() {
			Vector3<float> v1(1.0f, 0.0f, 0.0f);
			Vector3<float> v2(0.0f, 1.0f, 0.0f);
			float result = v1.AngleToRadian(v2);
			assert_equal(result, Math::PI / 2, "Angle between vectors");  // 90 degrees
		};

		// Test Vector3 reflect
		auto test_reflect = [&]() {
			Vector3<float> v(1.0f, -1.0f, 0.0f);
			Vector3<float> normal(0.0f, 1.0f, 0.0f);
			Vector3<float> result = v.Reflect(normal);
			assert_equal(result.x, 1.0f, "Reflect X");
			assert_equal(result.y, 1.0f, "Reflect Y");
			assert_equal(result.z, 0.0f, "Reflect Z");
		};

		// Test Vector3 projection
		auto test_projection = [&]() {
			Vector3<float> v1(1.0f, 2.0f, 3.0f);
			Vector3<float> v2(4.0f, 0.0f, 0.0f);
			Vector3<float> result = v1.ProjectOnto(v2);
			assert_equal(result.x, 1.0f, "Projection X");
			assert_equal(result.y, 0.0f, "Projection Y");
			assert_equal(result.z, 0.0f, "Projection Z");
		};

		// Test Vector3 are orthogonal
		auto test_are_orthogonal = [&]() {
			Vector3<float> v1(1.0f, 0.0f, 0.0f);
			Vector3<float> v2(0.0f, 1.0f, 0.0f); // Orthogonal to v1
			bool result = v1.IsOrthogonal(v2);
			RLS_ASSERT(result, "Are NOT Orthogonal");
		};

		auto test_refract = [&]() {
			Vector3<float> v(0.0f, -1.0f, 0.0f);
			Vector3<float> normal(0.0f, 1.0f, 0.0f);
			float eta = 0.5f;  // Refraction index
			Vector3<float> result = v.Refract(normal, eta);
			assert_equal(result.x, 0.0f, "Refract X");
			assert_equal(result.y, -1.0f, "Refract Y");  // Example assumes total internal reflection or partial refraction
			assert_equal(result.z, 0.0f, "Refract Z");
		};

		// Test Vector3 rotation around an axis
		auto test_rotate_around_axis = [&]() {
			Vector3<float> v(1.0f, 0.0f, 0.0f);
			Vector3<float> axis(0.0f, 0.0f, 1.0f);
			float angle = Math::PI / 2;  // 90 degrees
			v.RotateAroundAxis(axis, angle);
			assert_equal(v.x, 0.0f, "Rotate Around Axis X");
			assert_equal(v.y, 1.0f, "Rotate Around Axis Y");
			assert_equal(v.z, 0.0f, "Rotate Around Axis Z");
		};

		// Test Vector3 linear interpolation
		auto test_lerp = [&]() {
			Vector3<float> v1(1.0f, 2.0f, 3.0f);
			Vector3<float> v2(4.0f, 5.0f, 6.0f);
			float t = 0.5f;  // Interpolation parameter
			Vector3<float> result = Vector3f::Lerp(v1, v2, t);
			assert_equal(result.x, 2.5f, "Lerp X");
			assert_equal(result.y, 3.5f, "Lerp Y");
			assert_equal(result.z, 4.5f, "Lerp Z");
		};

		// Test Vector3 component-wise multiplication
		//auto test_component_multiply = [&]() {
		//	Vector3<float> v1(1.0f, 2.0f, 3.0f);
		//	Vector3<float> v2(4.0f, 5.0f, 6.0f);
		//	Vector3<float> result = ComponentMultiply(v1, v2);
		//	assert_equal(result.x, 4.0f, "Component Multiply X");
		//	assert_equal(result.y, 10.0f, "Component Multiply Y");
		//	assert_equal(result.z, 18.0f, "Component Multiply Z");
		//};

		// Running all tests
		test_add();
		test_subtract();
		test_multiply_scalar();
		test_dot();
		test_cross();
		test_length();
		test_normalize();
		test_are_parallel();
		// Call more test lambdas here
		test_distance();
		test_angle();
		test_reflect();
		test_projection();
		test_are_orthogonal();
		test_refract();
		test_rotate_around_axis();
		test_lerp();
	}

	void run_quaternion_tests() {
		auto assert_approx_equal = [](float actual, float expected, const char* message, float epsilon = 0.0001f) {
			if (std::abs(actual - expected) > epsilon) {
				std::cerr << "Test failed: " << message << " | Expected: " << expected << ", got: " << actual << std::endl;
			}
			else {
				std::cout << "Test passed: " << message << std::endl;
			}
		};

		auto test_identity = [&]() {
			Quaternion q = Quaternion::Identity();
			assert_approx_equal(q.Q.x, 0.0f, "Identity X");
			assert_approx_equal(q.Q.y, 0.0f, "Identity Y");
			assert_approx_equal(q.Q.z, 0.0f, "Identity Z");
			assert_approx_equal(q.Q.w, 1.0f, "Identity W");
		};

		auto test_multiplication = [&]() {
			Quaternion q1(1, 0, 0, 0);
			Quaternion q2(0, 1, 0, 0);
			Quaternion result = q1 * q2;
			assert_approx_equal(result.Q.x, 0.0f, "Multiplication X");
			assert_approx_equal(result.Q.y, 0.0f, "Multiplication Y");
			assert_approx_equal(result.Q.z, -1.0f, "Multiplication Z"); // This needs correction based on quaternion rules
			assert_approx_equal(result.Q.w, 0.0f, "Multiplication W");
		};

		auto test_normalize = [&]() {
			Quaternion q(1, 2, 3, 4);
			q.Normalize();
			float mag = std::sqrt(q.Q.x * q.Q.x + q.Q.y * q.Q.y + q.Q.z * q.Q.z + q.Q.w * q.Q.w);
			assert_approx_equal(mag, 1.0f, "Normalize Magnitude");
		};

		auto test_from_euler = [&]() {
			Quaternion q = Quaternion::FromEuler(0, Math::PI / 2, 0); // Assuming this creates a 90-degree rotation about the x-axis
			float expectedX = std::sin(Math::PI / 4);
			float actualX = q.Q.x;
			std::cout << "Actual x: " << actualX << " vs. Expected x: " << expectedX << std::endl;
			assert_approx_equal(actualX, expectedX, "Euler X");
			assert_approx_equal(q.Q.w, std::cos(Math::PI / 4), "Euler W"); // cos(?/4)
		};

		auto test_slerp = [&]() {
			Quaternion start = Quaternion::Identity();
			Quaternion end(0, 0, 1, 0);
			Quaternion result = Quaternion::Slerp(start, end, 0.5f);
			// Expected result is midpoint in SLERP interpolation
			assert_approx_equal(result.Q.w, std::cos(Math::PI / 4), "Slerp W");
		};

		auto test_to_rotation_matrix = [&]() {
			Quaternion q = Quaternion::FromEuler(0, 0, Math::PI / 2); // 90 degrees yaw
			DirectX::XMMATRIX matrix = q.ToRotationXMMatrix();
			// Reading the correct components based on matrix output
			DirectX::XMVECTOR new_X = matrix.r[0]; // New X-axis (first row)
			DirectX::XMVECTOR new_Z = matrix.r[2]; // New Z-axis (third row)

			// The first row's third component shows new X position as +Z
			assert_approx_equal(DirectX::XMVectorGetZ(new_X), -1.0f, "New X axis should point in positive Z");
			// The third row's first component shows new Z position as -X
			assert_approx_equal(DirectX::XMVectorGetX(new_Z), 1.0f, "New Z axis should point in negative X");

			std::cout << "Rotation matrix:" << std::endl;
			for (int i = 0; i < 4; i++) {
				for (int j = 0; j < 4; j++) {
					std::cout << DirectX::XMVectorGetByIndex(matrix.r[j], i) << " ";
				}
				std::cout << std::endl;
			}
			std::cout << std::endl;
		};

		auto test_conversion = [&]() {
			// Example: Quaternion representing 90 degrees rotation around X-axis
			Quaternion quaternion = Quaternion::FromEuler(0.0f, Math::PI / 2.0f, 0.0f);
			Vector3f euler = quaternion.ToEulerAngles();
			
			std::cout << "Testing Quaternion to Euler Conversion..." << std::endl;
			std::cout << "Expected Euler Angles for 90 degrees Y rotation: Roll = 0, Pitch = 0, Yaw = 90" << std::endl;
			std::cout << "Calculated Euler Angles: Roll = " << euler.x << ", Pitch = " << euler.y << ", Yaw = " << euler.z << std::endl;

			// Add more tests for other axes and combined rotations
		};

		// Running all tests
		test_identity();
		test_multiplication();
		test_normalize();
		test_from_euler();
		test_slerp();
		test_to_rotation_matrix();
		test_conversion();
		// Add more test lambdas here if needed
	}

	void run_matrix_tests()
	{
		auto testSetRotationX = [&]() {
			Matrix4x4 matrix;
			// Initially set some scale and translation
			matrix.SetScale(2.0f, 3.0f, 4.0f);

			// Set an initial rotation around Y to test if it gets correctly replaced
			matrix.RotateY(DirectX::XM_PI / 6);  // 30 degrees around Y-axis
			matrix.SetTranslation(5.0f, 6.0f, 7.0f);

			float targetXRotationDegrees = 45.0f;  // Target rotation in degrees
			float targetXRotationRadians = DirectX::XMConvertToRadians(targetXRotationDegrees);  // Co
			matrix.SetRotationX(targetXRotationRadians);
			
			// Decompose to verify results
			Vector3f scale, translation;
			Quaternion rotation;
			bool decomposed = matrix.Decompose(scale, rotation, translation);
			assert(decomposed);

			// Check that the scale and translation remain unchanged
			assert(scale.x == 2.0f && scale.y == 3.0f && scale.z == 4.0f);
			assert(translation.x == 5.0f && translation.y == 6.0f && translation.z == 7.0f);

			// Check the new X rotation
			Vector3f eulerAngles = rotation.ToEulerAngles();
			assert(std::abs(eulerAngles.x - targetXRotationDegrees) < 0.001f);  // Close to 45 degrees in radians
			assert(std::abs(eulerAngles.y - 0) < 0.001f);  // No rotation around Y
			assert(std::abs(eulerAngles.z - 0) < 0.001f);  // No rotation around Z

			std::cout << "SetRotationX Test Passed" << std::endl;
		};

		auto testDecomposeRecompose = [&]() {
			Matrix4x4 original;
			original.SetScale(1.0f, 2.0f, 3.0f);
			original.RotateY(DirectX::XM_PI / 4); // Rotate 45 degrees
			original.SetTranslation(1.0f, 2.0f, 3.0f);

			Vector3f scale;
			Quaternion rotation;
			Vector3f translation;
			bool result = original.Decompose(scale, rotation, translation);
			assert(result);

			Matrix4x4 recomposed;
			recomposed.Recompose(scale, rotation, translation);

			// Compare original and recomposed matrices
			assert(std::abs(original.M._11 - recomposed.M._11) < 0.001f && std::abs(original.M._44 - recomposed.M._44) < 0.001f);
			std::cout << "Decompose and Recompose Test Passed" << std::endl;
		};

		auto testIdentityMatrix = [&]() {
			Matrix4x4 matrix;
			assert(matrix.M._11 == 1.0f && matrix.M._22 == 1.0f && matrix.M._33 == 1.0f && matrix.M._44 == 1.0f);
			assert(matrix.M._12 == 0.0f && matrix.M._13 == 0.0f && matrix.M._14 == 0.0f);
			assert(matrix.M._21 == 0.0f && matrix.M._23 == 0.0f && matrix.M._24 == 0.0f);
			assert(matrix.M._31 == 0.0f && matrix.M._32 == 0.0f && matrix.M._34 == 0.0f);
			assert(matrix.M._41 == 0.0f && matrix.M._42 == 0.0f && matrix.M._43 == 0.0f && matrix.M._44 == 1.0f);
			std::cout << "Identity Matrix Test Passed" << std::endl;
		};

		// Test Translations
		auto testTranslations = [&]() {
			Matrix4x4 matrix;
			matrix.SetTranslation(3.0f, 4.0f, 5.0f);
			assert(matrix.M._41 == 3.0f && matrix.M._42 == 4.0f && matrix.M._43 == 5.0f);
			std::cout << "Translation Test Passed" << std::endl;
		};

		// Test Rotation X
		auto testRotationX = [&]() {
			Matrix4x4 matrix;
			float angleRadians = DirectX::XM_PI / 4; // 45 degrees
			matrix.RotateX(angleRadians);
			assert(std::abs(matrix.M._22 - cos(angleRadians)) < 0.0001f);
			assert(std::abs(matrix.M._23 - sin(angleRadians)) < 0.0001f); // Note possible sign inversion depending on matrix layout
			std::cout << "X Rotation Test Passed" << std::endl;
		};

		// Test Scaling
		auto testScaling = [&]() {
			Matrix4x4 matrix;
			matrix.SetScale(2.0f, 3.0f, 4.0f);
			assert(matrix.M._11 == 2.0f && matrix.M._22 == 3.0f && matrix.M._33 == 4.0f);
			std::cout << "Scaling Test Passed" << std::endl;
		};

		testSetRotationX();
		testDecomposeRecompose();
		// Run tests
		testIdentityMatrix();
		testTranslations();
		testRotationX();
		testScaling();
	}

	void Application::OnStartUp() noexcept
	{
		Matrix4x4 mat = Matrix4x4::Identity();

		Matrix4x4 mat2;
		auto newM = mat * mat2;
		bool isIdentity = newM.IsIdentity();


		Vector3f scale;
		Vector3f translation;
		Quaternion quat;
		if (mat.Decompose(scale, quat, translation))
			const Vector3f euler = quat.ToEulerAngles();

		//run_vector3_tests();
		//run_quaternion_tests();
		run_matrix_tests();

		EventBus::Get().SetMainApplication(this);
		Log::Initialize();

		float dt = 0.0001f;
		float angleDeg = 0.0f;
		float speed = 10.0f;
		while (true)
		{
			Quaternion rotateY = Quaternion::AngleAxis(angleDeg, Vector3f::Up);
			//Quaternion rotateZ = Quaternion::AngleAxis(angleDeg, Vector3f::Forward);

			// Combine rotations: First rotate around Y, then around Z
			//Quaternion combinedRotation = rotateZ * rotateY;

			//Vector3f eulerAngles = combinedRotation.ToEulerAngles();
			//RLS_CORE_INFO("Euler Angles: ({0},{1},{2})", Math::NormalizeDegrees(eulerAngles.x), Math::NormalizeDegrees(eulerAngles.y), Math::NormalizeDegrees(eulerAngles.z));
			
			angleDeg += dt * speed;
		}

		SystemPaths::Initialize();
		D3D12Core::Initialize();
		MemoryManager::Get().Initialize();
		AssetManager::Initialize();
		AssetRegistry::RecursiveScanDirectoryForAssets(ENGINE_ASSET_DIRECTORY);
		AssetRegistry::RecursiveScanDirectoryForAssets(EDITOR_ASSET_DIRECTORY);
		UI::Initialize();

		const std::filesystem::path engineIni = SystemPaths::GetUserDocumentsDirectory() / "engine.ini";

		uint32_t windowWidth{ 1280u };
		uint32_t windowHeight{ 720u };
		if (std::filesystem::exists(engineIni))
		{
			std::ifstream inFile(engineIni);
			std::string s;
			while (inFile >> s)
			{
				if (s == "[RenderWindow][Dimensions]")
				{
					inFile.ignore(1);
					inFile >> windowWidth;
					inFile >> windowHeight;
					break;
				}
			}
			inFile.close();
		}
		else
		{
			std::ofstream outFile(engineIni);
			outFile << "[RenderWindow][Dimensions]\n";
			outFile << windowWidth << "\n";
			outFile << windowHeight;
			outFile.close();
		}

		Window::Initialize(m_ApplicationSpecification.Name, windowWidth, windowHeight);
		MasterRenderer::Initialize();
		PushOverlay(&m_ImGuiLayer);

		m_IsRunning = true;

		
	}

	void Application::ShutDown() noexcept
	{
		std::string engineDirectory = std::string(MAIN_ENGINE_DIRECTORY) + std::string("engine.ini");

		std::ofstream outFile(engineDirectory);
		outFile << "[RenderWindow][Dimensions]\n";
		outFile << Window::GetWidth() << "\n";
		outFile << Window::GetHeight();
		outFile.close();

		MasterRenderer::OnShutDown();

		m_IsRunning = false;
	}

	void Application::ExecuteMainThreadQueue() noexcept
	{
		std::scoped_lock(m_MainThreadFunctionQueueMutex);

		while (m_MainThreadFunctionQueue.size() > 0)
		{
			auto& func = m_MainThreadFunctionQueue.front();
			func();

			m_MainThreadFunctionQueue.pop();
		}
	}

}