#pragma once
#include "UncheckedUniformBuffer.h"

template<typename T>
class UniformBuffer : UncheckedUniformBuffer
{
public:
	unsigned id = 0;
	void create();
	void create(unsigned);
	void update(T*);
	void update(unsigned, T*);
	void bind();
	void bind(unsigned);

	UniformBuffer();

private:
	static constexpr unsigned STANDARD_BUFFER_SIZE = 1024;
	T* data;

	void checkCreated_(const char* file, int line);

#define checkCreated checkCreated_(__FILE__, __LINE__);

};

