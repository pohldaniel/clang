#pragma once
#include <array>
#include <glm/glm.hpp>

struct Uniforms {
  glm::mat4 projection;
  glm::mat4 view;
  glm::mat4 env;
  glm::mat4 model;
  glm::mat4 normal;
  std::array<float, 4> color;
  glm::vec3 camPosition;
  float _pad0;
  glm::mat4 lightVP;
  glm::mat4 shadow;
  glm::vec3 lightPosition;
  float _pad1;
};

struct LightingUniforms {
	std::array<glm::vec4, 2> directions;
	std::array<glm::vec4, 2> colors;
	float hardness = 32.0f;
	float kd = 1.0f;
	float ks = 0.5f;
	float _pad[1];
};

struct PBRLightingUniforms {
	std::array <float, 3> position;
	float padding1;
	std::array <float, 3> color;
	float padding2;
};

struct MaterialUniforms {
	std::array <float, 4> baseColorFactor;
	float alphaCutoff;
	float padding[3];
};