/*
Author: Jake Baranowski
*/


#pragma once

namespace jb {

	class FrameBuffer
	{
	public:
		FrameBuffer();
		~FrameBuffer();

		unsigned int fbo;
		unsigned int width;
		unsigned int height;
		unsigned int colorBuffers[3];
	};
}

