/*
*	Author: Eric Winebrenner
*/

#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace ew {
	struct Camera {
		glm::vec3 position = glm::vec3(0.0f, 0.0f, 5.0f);
		glm::vec3 target = glm::vec3(0.0f);

		float fov = 60.0f;
		float nearPlane = 0.01f;
		float farPlane = 100.0f;
		bool orthographic = false;
		float orthoHeight = 6.0f;
		float aspectRatio = 1.77f;

		inline glm::mat4 viewMatrix()const {
			glm::vec3 toTarget = glm::normalize(target - position);
			glm::vec3 up = glm::vec3(0, 1, 0);
			//If camera is aligned with up vector, choose a new one
			if (glm::abs(glm::dot(toTarget, up)) >= 1.0f - glm::epsilon<float>()) {
				up = glm::vec3(0, 0, 1);
			}
			return glm::lookAt(position, target, up);
		}
		inline glm::mat4 projectionMatrix()const {

			if (orthographic) {
				
				float width = orthoHeight * aspectRatio;
				float r = width / 2;
				float l = -r;
				float t = orthoHeight / 2;
				float b = -t;
				return glm::ortho(l, r, b, t, nearPlane, farPlane);
			}
			else {
				return glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
			}
		}
	};

}
