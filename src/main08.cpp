
// Renderer Abstraction 

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include <assert.h>
#include <vector>
#include <typeinfo>

// #include "res/shaders/Shader.h"

using namespace std;

struct ShaderProgramSource
{
	std::string VertexSource;
	std::string FragSource;
};

//Layout별로, 데이터를 어떻게 읽어와야 하는지에 대한 정보를 가지고있는 구조체
struct VertexBufferElement
{
	unsigned int type; //각 데이터 타입이 무엇인지 (ex, vertex의 위치면 float)
	unsigned int count; //데이터가 몇 개인지
	unsigned char normalized; //데이터의 normalization이 필요한지

	static unsigned int GetSizeOfType(unsigned int type) //타입별로 적절한 메모리 사이즈를 반환하는 static 함수
	{
		switch (type)
		{
			case GL_FLOAT: return 4;
			case GL_UNSIGNED_INT: return 4;
			case GL_UNSIGNED_BYTE: return 1;
		}
		assert(0);
		return 0;
	}
};


class IndexBuffer
{
private:
	unsigned int m_RendererID;
	unsigned int m_Count;
public:
	IndexBuffer(const unsigned int* data, unsigned int count); //index는 (대부분) unsigned int* 타입. count는 갯수(size와 다름)
	~IndexBuffer();

	void Bind() const;
	void Unbind() const;

	inline unsigned int GetCount() const { return m_Count; }
};

IndexBuffer::IndexBuffer(const unsigned int* data, unsigned int count)
	: m_Count{count}
{
	assert(sizeof(unsigned int) == sizeof(GLuint) && "if false, stop here");

	glGenBuffers(1, &m_RendererID); //1. 버퍼 생성
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID); //2. 바인딩("작업 상태")
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), data, GL_STATIC_DRAW);  //3. 작업 상태 버퍼에 데이터 전달
}

IndexBuffer::~IndexBuffer()
{
	glDeleteBuffers(1, &m_RendererID);
}

void IndexBuffer::Bind() const
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID); //바인딩("작업 상태")
}

void IndexBuffer::Unbind() const
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); //언바인딩
}


class VertexBuffer
{
private:
	unsigned int m_RendererID;
public:
	VertexBuffer(const void* data, unsigned int size); //size는 byte 사이즈, 데이터의 타입은 모르기 때문에 void*로.
	~VertexBuffer();

	void Bind() const;
	void Unbind() const;
};

VertexBuffer::VertexBuffer(const void* data, unsigned int size)
{
	glGenBuffers(1, &m_RendererID); //1. 버퍼 생성
	glBindBuffer(GL_ARRAY_BUFFER, m_RendererID); //2. 바인딩("작업 상태")
	glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);  //3. 작업 상태 버퍼에 데이터 전달
}

VertexBuffer::~VertexBuffer()
{
	glDeleteBuffers(1, &m_RendererID);
}

void VertexBuffer::Bind() const
{
	glBindBuffer(GL_ARRAY_BUFFER, m_RendererID); //바인딩("작업 상태")
}

void VertexBuffer::Unbind() const
{
	glBindBuffer(GL_ARRAY_BUFFER, 0); //언바인딩
}


class VertexBufferLayout
{
private:
	std::vector<VertexBufferElement> m_Elements; //하나의 layout은 여러개의 element를 갖고 있음(ex, position, normal, color, etc...)
	unsigned int m_Stride; //vertex하나당 데이터가 얼마나 떨어져있는지 stride를 멤버변수로 갖고 있음

public:
	VertexBufferLayout()
		: m_Stride{ 0 }
	{}

	void Push<float>(unsigned int count);
	
	// template<typename T>
	// void Push(unsigned int count)
	// {
	// 	static_assert(false);
	// }

	//template specializations
	// template<>
	// void Push(unsigned int count)
	// void Push<float>(unsigned int count)
	// {
	// 	m_Elements.push_back({ GL_FLOAT, count, GL_FALSE });
	// 	m_Stride += count * VertexBufferElement::GetSizeOfType(GL_FLOAT); //vertex 하나당 float 데이터가 count개 추가될수록, count * size(GL_FLOAT)씩 stride가 커져야 함
	// }

	// template<>
	// void Push<unsigned int>(unsigned int count)
	// {
	// 	m_Elements.push_back({ GL_UNSIGNED_INT, count, GL_FALSE });
	// 	m_Stride += count * VertexBufferElement::GetSizeOfType(GL_UNSIGNED_INT); //위와 마찬가지
	// }

	// template<>
	// void Push<unsigned char>(unsigned int count)
	// {
	// 	m_Elements.push_back({ GL_UNSIGNED_BYTE, count, GL_TRUE });
	// 	m_Stride += count * VertexBufferElement::GetSizeOfType(GL_UNSIGNED_BYTE);
	// }

	inline const std::vector<VertexBufferElement>& GetElement() const { return m_Elements; }
	inline unsigned int GetStride() const { return m_Stride; }
};

	void vertexBufferLayout::Push<float>(unsigned int count)
	{
		m_Elements.push_back({ GL_FLOAT, count, GL_FALSE });
		m_Stride += count * VertexBufferElement::GetSizeOfType(GL_FLOAT); //vertex 하나당 float 데이터가 count개 추가될수록, count * size(GL_FLOAT)씩 stride가 커져야 함
	}


class VertexArray
{
private:
	unsigned int m_RendererID;
public:
	VertexArray();
	~VertexArray();

	void AddBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout);

	void Bind() const;
	void Unbind() const;
};

VertexArray::VertexArray()
{
	glGenVertexArrays(1, &m_RendererID); //vao 생성
	//glBindVertexArray(m_RendererID); //vao 바인딩(="작업 상태") <-- 바인딩은 AddBuffer 직전에 수행하도록 함
}

VertexArray::~VertexArray()
{
	glDeleteVertexArrays(1, &m_RendererID);
}

void VertexArray::AddBuffer(const VertexBuffer & vb, const VertexBufferLayout & layout)
{
	Bind(); //vao를 바인딩

	vb.Bind(); //Vertex Buffer를 바인딩

	const auto& elements = layout.GetElement();
	unsigned int offset = 0;
	for (int i=0;i<elements.size();i++)
	{
		const auto& element = elements[i];
		glEnableVertexAttribArray(i); //기존에는 0번만 존재했으나, position/normal/color등 여러 attribute가 생기면, 여러 attribute를 enable해야함
		glVertexAttribPointer(i, element.count, element.type, element.normalized, layout.GetStride(), (const void*)offset); //layout별로 데이터를 어떻게 읽어와야하는지를 element구조체로 가지고 있을 예정. 이를 활용함.
		offset += element.count * VertexBufferElement::GetSizeOfType(element.type);
	}

}

void VertexArray::Bind() const
{
	glBindVertexArray(m_RendererID);
}

void VertexArray::Unbind() const
{
	glBindVertexArray(0);
}


//셰이더 파일 파싱 함수
static ShaderProgramSource ParseShader(const std::string& filepath)
{
	std::ifstream stream(filepath);

	enum class ShaderType
	{
		NONE = -1, VERTEX = 0, FRAGMENT = 1
	};
	
	std::string line;
	std::stringstream ss[2];
	ShaderType type = ShaderType::NONE;

	while (getline(stream, line))
	{
		if (line.find("#shader") != std::string::npos)
		{
			if (line.find("vertex") != std::string::npos) //vertex 셰이더 섹션
			{
				type = ShaderType::VERTEX;
			}
			else if (line.find("fragment") != std::string::npos) //fragment 셰이더 섹션
			{
				type = ShaderType::FRAGMENT;
			}
		}
		else
		{
			ss[(int)type] << line << '\n'; //코드를 stringstream에 삽입
		}
	}

	return { ss[0].str(), ss[1].str() };
}

static unsigned int CompileShader(unsigned int type, const std::string& source)
{
	unsigned int id = glCreateShader(type); //셰이더 객체 생성(마찬가지)
	const char* src = source.c_str();
	glShaderSource(id, 1, &src, nullptr); // 셰이더의 소스 코드 명시
	glCompileShader(id); // id에 해당하는 셰이더 컴파일
	
	// Error Handling(없으면 셰이더 프로그래밍할때 괴롭다...)
	int result;
	glGetShaderiv(id, GL_COMPILE_STATUS, &result); //셰이더 프로그램으로부터 컴파일 결과(log)를 얻어옴
	if (result == GL_FALSE) //컴파일에 실패한 경우
	{
		int length;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length); //log의 길이를 얻어옴
		char* message = (char*)alloca(length * sizeof(char)); //stack에 동적할당
		glGetShaderInfoLog(id, length, &length, message); //길이만큼 log를 얻어옴
		std::cout << "셰이더 컴파일 실패! " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << std::endl;
		std::cout << message << std::endl;
		glDeleteShader(id); //컴파일 실패한 경우 셰이더 삭제
		return 0;
	}

	return id;
}

//--------Shader 프로그램 생성, 컴파일, 링크----------//
static unsigned int CreateShader(const std::string& vertexShader, const std::string& fragShader)
{
	unsigned int program = glCreateProgram(); //셰이더 프로그램 객체 생성(int에 저장되는 것은 id)
	unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader); 
	unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragShader);

	//컴파일된 셰이더 코드를 program에 추가하고 링크
	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);
	glValidateProgram(program);

	//셰이더 프로그램을 생성했으므로 vs, fs 개별 프로그램은 더이상 필요 없음
	glDeleteShader(vs);
	glDeleteShader(fs);

	return program;
}

int main(void)
{
	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(640, 480, "Buffer Layout Abstraction", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	glfwSwapInterval(1); //1이면 vsync rate와 같은 속도로 화면 갱신

	// glfwMakeContextCurrent가 호출된 후에 glewInit이 수행되어야 함
	if (glewInit() != GLEW_OK)
	{
		std::cout << "Error\n";
	}

	std::cout << glGetString(GL_VERSION) << std::endl; //내 플랫폼의 GL_Version 출력해보기

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); //Opengl 3.3 버전 사용
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE); //Compatability 버전일때는 VAO를 안만들어도 동작
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


	float positions[] = { //사각형을 그리기 위해 2차 수정
		-0.5f, -0.5f, //0
		 0.5f, -0.5f, //1
		 0.5f,  0.5f, //2
		-0.5f,  0.5f, //3
	};

	unsigned int indices[] = { //index buffer를 함께 사용(index는 unsigned 타입임에 유의)
		0, 1, 2, //vertex 012로 이루어진 삼각형
		2, 3, 0  //vertex 230로 이루어진 삼각형
	};

	//loop에 적어놓은 반복적 작업을 한 그룹으로 묶기 위해 VAO(Vertex Array Object)를 사용해야 함
	//VAO는 아래 데이터 전달 과정 및 해석 과정을 담고있는 데이터라고 생각하면 됨
	// unsigned int vao;
	// glGenVertexArrays(1, &vao); //vao 생성
	// glBindVertexArray(vao); //vao 바인딩(="작업 상태")

	//vao 생성 VertexArray가 담당
	VertexArray va; 
	VertexBuffer vb{ positions, 4 * 2 * sizeof(float) }; 
	VertexBufferLayout layout;
	layout.Push<float>(2); //vertex당 2개의 위치를 표현하는 float 데이터
	//layout.Push<float>(3); //만일 vertex당 색상을 표현하는 3개의 rgb데이터가 더 있었으면 왼쪽과 같이 추가하면, layout에서 알아서 stride 계산
	va.AddBuffer(vb, layout);

	//---------데이터를 전달하는 과정--------//
	// unsigned int bufferID;
	// glGenBuffers(1, &bufferID); //1. 버퍼 생성
	// glBindBuffer(GL_ARRAY_BUFFER, bufferID); //2. 바인딩("작업 상태")

	// Vertex Buffer 코드 VertexBuffer로 이동
	// VertexBuffer vb{ positions, 4 * 2 * sizeof(float) };

	// unsigned int vbo; //개념적으로 buffer ID라 불렀고, 실제로는 vbo(vertex buffer object)라는 용어를 많이 씀
	// glGenBuffers(1, &vbo); //1. 버퍼 생성
	// glBindBuffer(GL_ARRAY_BUFFER, vbo); //2. 바인딩("작업 상태")
	// glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float), positions, GL_STATIC_DRAW);  //3. 작업 상태 버퍼에 데이터 전달

	//---------데이터를 해석하는(법을 정의하는) 과정--------//
	// glEnableVertexAttribArray(0); //1. 몇 번째 Location의 attribute를 활성화(enable)
	// glVertexAttribPointer(0, 2,	GL_FLOAT, GL_FALSE, sizeof(float)*2, 0); //2. 데이터 해석 방법을 전달.

	// Index Buffer 코드 IndexBuffer로 이동
	IndexBuffer ib{ indices, 6 };

	// unsigned int ibo; //index buffer object
	// glGenBuffers(1, &ibo); //1. 인덱스 버퍼 생성
	// glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo); //2. 바인딩("작업 상태"), 이번에는 ARRAY_BUFFER가 아니라 ELEMENT_ARRAY_BUFFER
	// glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), indices, GL_STATIC_DRAW);  //3. 작업 상태 버퍼에 데이터 전달
	   
	//---------Shader 생성---------------//
	// ShaderProgramSource source = ParseShader("res/shaders/Basic.shader"); //셰이더 코드 파싱

	// unsigned int shader = CreateShader(source.VertexSource, source.FragSource);
	// glUseProgram(shader); //BindBuffer와 마찬가지로, 현재 셰이더 프로그램을 "작업 상태"로 놓음
						  //draw call은 작업 상태인 셰이더 프로그램을 사용하여 작업 상태인 버퍼 데이터를 그림

	//--------Uniform 데이터 전달--------//
	// *주의* 이 부분도 당연히 shader가 바인딩("작업 상태")된 상태에서 수행해야 함
	// int location = glGetUniformLocation(shader, "u_Color"); //셰이더 프로그램에 uniform 데이터를 전달하기 위해서는 uniform의 위치 정보가 필요. 
	// 														//셰이더 객체와 변수 이름을 인자로 넘겨주어 얻어올 수 있음
	// assert(location != 1);
	// glUniform4f(location, 0.2f, 0.3f, 0.8f, 1.0f);

	// glBindVertexArray(0); //vao unbind
	// glBindBuffer(GL_ARRAY_BUFFER, 0); //객체 인자를 0으로 바인딩하면, 현재 작업 상태를 해제한다는 의미(unbind)
	// glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); //객체 인자를 0으로 바인딩하면, 현재 작업 상태를 해제한다는 의미(unbind)
	// glUseProgram(0); //객체 인자를 0으로 바인딩하면, 현재 작업 상태를 해제한다는 의미(unbind)

	Shader shader{ "res/shaders/Basic02.shader" };
	shader.Bind();
	shader.SetUniform4f("u_Color", 0.2f, 0.3f, 0.8f, 1.0f);

	va.Unbind();
	vb.Unbind();
	ib.Unbind();
	shader.Unbind();

	// int location = glGetUniformLocation(shader, "u_Color"); //셰이더 프로그램에 uniform 데이터를 전달하기 위해서는 uniform의 위치 정보가 필요. 

	// glUniform4f(location, 0.2f, 0.3f, 0.8f, 1.0f);

	// glBindVertexArray(0); //vao unbind
	// glBindBuffer(GL_ARRAY_BUFFER, 0); //객체 인자를 0으로 바인딩하면, 현재 작업 상태를 해제한다는 의미(unbind)
	// glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); //객체 인자를 0으로 바인딩하면, 현재 작업 상태를 해제한다는 의미(unbind)
	// glUseProgram(0); //객체 인자를 0으로 바인딩하면, 현재 작업 상태를 해제한다는 의미(unbind)

	float r = 0.0f;
	float increment = 0.05f;

	assert(sizeof(unsigned int) == 4);  // if true, do nothing

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		/* Render here */
		glClear(GL_COLOR_BUFFER_BIT);

		
		//실시간으로 데이터를 변경하고 싶다면, 매 frame draw call이 호출되기 이전에 uniform 데이터를 변경해서 전달해주면 됨
		
		//1. 셰이더 바인딩, uniform 데이터 전달
		glUseProgram(shader);
		glUniform4f(location, r, 0.3f, 0.8f, 1.0f);

		// glBindVertexArray(vao);
		va.Bind();
		ib.Bind(); //한 모델이 다른 Material을 사용할 경우 여러 Draw call에 걸쳐 그려지는 경우가 많고, 이러한 경우 index buffer로 모델의 부분을 구분하는 경우가 많음

		glDrawElements(GL_TRIANGLES, 6,	GL_UNSIGNED_INT, nullptr); //Draw call, 강제로 오류를 만들어 보자

		if (r > 1.0f)
			increment = -0.05f;
		if (r < 0.0f)
			increment = 0.05f;

		r += increment;

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	glDeleteProgram(shader); //셰이더 삭제

	glfwTerminate();
	return 0;
}