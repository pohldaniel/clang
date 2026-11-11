#include <States/StateMachine.h>
#include <WebGPU/WgpTexture.h>
#include <WebGPU/WgpBuffer.h>
#include <WebGPU/WgpMesh.h>
#include <WebGPU/WgpData.h>
#include <WebGPU/WgpModel.h>

class Compute : public State {

	enum class FilterType {
		Sum,
		Maximum,
		Minimum,
	};

	struct Parameters {
		FilterType filterType = FilterType::Sum;
		glm::mat3x4 kernel = glm::mat4(1.0f);
		bool normalize = true;
	};

	struct UniformsCompute {
		glm::mat3x4 kernel = glm::mat3x4(0.0);
		float test = 0.5f;
		uint32_t filterType = 0;
		float _pad[2];
	};
public:    

    Compute(StateMachine& machine);
	~Compute();

	void fixedUpdate() override;
	void update() override;
	void render() override;
	void OnDraw(const WGPUCommandEncoder& commandEncoder, const WGPURenderPassDescriptor& renderPassDescriptor);
	void compute(const WGPUCommandEncoder& commandEncoder);

	void OnMouseMotion(const Event::MouseMoveEvent& event) override;
	void OnMouseButtonDown(const Event::MouseButtonEvent& event) override;
	void OnMouseButtonUp(const Event::MouseButtonEvent& event) override;
	void OnKeyDown(const Event::KeyboardEvent& event) override;
	void OnKeyUp(const Event::KeyboardEvent& event) override;
    void resize(int deltaW, int deltaH) override;
	
private:

	std::vector<WGPUBindGroupLayout> OnBindGroupLayouts();
	void renderUi(const WGPURenderPassEncoder& renderPassEncoder, bool force = false);
	WGPUBindGroup createBindGroup();

	bool m_initUi = true;
	bool m_drawUi = true;
	bool m_shouldCompute = true;
	bool m_force = true;


	UniformsCompute m_uniforms;
	Parameters m_parameters;
	float m_scale = 0.5f;
	WgpBuffer m_uniformBuffer;
	WgpTexture m_inputTexture, m_outputTexture;
	WGPUBindGroup m_bindGroup;
};