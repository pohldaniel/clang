#pragma once
#include <array>
#include <glm/glm.hpp>

struct Uniforms {
  glm::mat4 projectionMatrix;
  glm::mat4 viewMatrix;
  glm::mat4 envMatrix;
  glm::mat4 modelMatrix;
  glm::mat4 normalMatrix;
  std::array<float, 4> color;
  glm::vec3 camPosition;
  float _pad[1];
};

struct ComputeUniforms {
  glm::mat3x4 kernel = glm::mat3x4(0.0);
  float test = 0.5f;
  uint32_t filterType = 0;
  float _pad[2];
};

struct LightingUniforms {
	std::array<glm::vec4, 2> directions;
	std::array<glm::vec4, 2> colors;
	float hardness = 32.0f;
	float kd = 1.0f;
	float ks = 0.5f;
	float _pad[1];
};

struct NormalUniforms {
	std::array <float, 3> light_pos_vs;
	uint32_t mode;
	float light_intensity;
	float depth_scale;
	float depth_layers;
	float padding;
};