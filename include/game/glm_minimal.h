#pragma once

// Only include what we absolutely need
#define GLM_FORCE_RADIANS
#define GLM_FORCE_INLINE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES

// Core types only
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

// Essential functions only - include individually
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Do NOT include these heavy headers:
// #include <glm/glm.hpp>  // TOO HEAVY
// #include <glm/gtc/quaternion.hpp>  // Only if needed