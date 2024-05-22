
// 5 Uniform - Vertex Array Object 
// 6 Buffer Abstraction - VAO(Vertex Array Object)
// 7 Buffer Layout Abstraction

#include <GL/glew.h> // +1 glfw보다 먼저 include해야 함  
#include <GLFW/glfw3.h>
#include <iostream>  // +1

#include <fstream> // +5
#include <string>
#include <sstream>

#include <assert.h> // +7
#include <vector>
#include <typeinfo>
using namespace std;

// ------------- start of shader -------------------------------------

// +5 어떻게 그려질지 구현한 프로그램이 Shader임
struct ShaderProgramSource
{
	std::string VertexSource;
	std::string FragSource;
};

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


// +4 --------Shader 컴파일 함수----------//
static unsigned int CompileShader(unsigned int type, const std::string& source)
{
	unsigned int id = glCreateShader(type); //셰이더 객체 생성(마찬가지)
	const char* src = source.c_str();
	glShaderSource(id, // 셰이더 객체 id
					1, // 몇 개의 소스 코드를 명시할 것인지
					&src, // 실제 소스 코드가 들어있는 문자열의 주소값
					nullptr); // 해당 문자열 전체를 사용할 경우 nullptr입력, 아니라면 길이 명시
	glCompileShader(id); // id에 해당하는 셰이더 컴파일

	return id;
}

// +4 -5 --------Shader 프로그램 생성, 컴파일, 링크----------//
// static unsigned int CreateShader(const std::string& vertexShader, const std::string& fragShader)
// {
// 	unsigned int program = glCreateProgram(); //셰이더 프로그램 객체 생성(저장되는 것은 id)
// 	unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader); 
// 	unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragShader);

// 	//컴파일된 셰이더 코드를 program에 추가하고 링크
// 	glAttachShader(program, vs);
// 	glAttachShader(program, fs);
// 	glLinkProgram(program);
// 	glValidateProgram(program);

// 	//셰이더 프로그램을 생성했으므로 vs, fs 개별 프로그램은 더이상 필요 없음
// 	glDeleteShader(vs);
// 	glDeleteShader(fs);

// 	return program;
// }

// +5 Shader 프로그램 생성, 컴파일, 링크 
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

// ------------------ end of shader ---------------


// Layout별로, 데이터를 어떻게 읽어와야 하는지에 대한 정보를 가지고있는 구조체
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


// vertex buffer layout
class VertexBufferLayout
{
private:
	std::vector<VertexBufferElement> m_Elements; //하나의 layout은 여러개의 element를 갖고 있음(ex, position, normal, color, etc...)
	unsigned int m_Stride; //vertex하나당 데이터가 얼마나 떨어져있는지 stride를 멤버변수로 갖고 있음

public:
	VertexBufferLayout()
		: m_Stride{ 0 }
	{}

	
	// template<typename T>;
	// void Push(unsigned int count)
	// {
	// 	static_assert(false);
	// }

	// template specializations
	// template<>
	// void Push(unsigned int count);

	// void Push<float>&(unsigned int count)
	void Push (float&)(unsigned int count)
	{
		m_Elements.push_back({ GL_FLOAT, count, GL_FALSE });
		m_Stride += count * VertexBufferElement::GetSizeOfType(GL_FLOAT); //vertex 하나당 float 데이터가 count개 추가될수록, count * size(GL_FLOAT)씩 stride가 커져야 함
	}

	// template<>
	// // void Push<unsigned int>(unsigned int count)
	// void Push (unsigned int)(unsigned int count)
	// {
	// 	m_Elements.push_back({ GL_UNSIGNED_INT, count, GL_FALSE });
	// 	m_Stride += count * VertexBufferElement::GetSizeOfType(GL_UNSIGNED_INT); //위와 마찬가지
	// }

	// template<>
	// // void Push reinterpret_cast<unsigned char>(unsigned int count)
	// void Push (unsigned char)(unsigned int count)
	// {
	// 	m_Elements.push_back({ GL_UNSIGNED_BYTE, count, GL_TRUE });
	// 	m_Stride += count * VertexBufferElement::GetSizeOfType(GL_UNSIGNED_BYTE);
	// }

	inline const std::vector<VertexBufferElement>& GetElement() const { return m_Elements; }
	inline unsigned int GetStride() const { return m_Stride; }
};



// Vertex Buffer
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



// Vertex Arrary
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




// index buffer
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



int main(void)
{
	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit()){}

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	// +1 glfwMakeContextCurrent가 호출된 후에 glewInit이 수행되어야 함
	if (glewInit() != GLEW_OK){}

	std::cout << glGetString(GL_VERSION) << std::endl; // +1

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

	// +2 -6 ---------데이터를 전달하는 과정--------//
	// Vertex Array는 GPU의 VRAM에 Buffer에 저장되는 데이터를 넘기는 방식
	// 데이터를 넘기고 나서 삼각형을 실제로 그리는 명령을 호출하는 것을 Draw call
	// 삼각형이 어떻게 그려질지 구현한 프로그램이 Shader

	// unsigned int bufferID;
	// glGenBuffers(1, &bufferID); //1. 버퍼 객체를 만들고(1개), 주소를 bufferID에 저장
	// glBindBuffer(GL_ARRAY_BUFFER, bufferID); //2. buffer를 "작업 상태"로 만듬
											
	// glBufferData(GL_ARRAY_BUFFER, //3. GPU에 데이터를 넘겨주는 함수를 호출 
	// 			6 * sizeof(float), //데이터의 크기를 전달
	// 			positions,		  //데이터 포인터 전달
	// 			GL_STATIC_DRAW);   //데이터 변경을 알려줌, GPU의 효율적인 동작을 위한 Hint

	// +3 -6 ---------데이터를 해석하는 과정--------//
	// 위에서는 Byte의 덩어리를 GPU로 전달했을 뿐, 그 데이터를 어떻게 나누어 사용할지는 알 수 없음
	// 어떻게 나누어서 사용해야 하는지를 아래 두 과정을 통해 알려주어야 함
	
	// glEnableVertexAttribArray(0); // 1. 몇 번째 Location의 attribute를 활성화(enable)
	// glVertexAttribPointer(0, // 2. 데이터 해석 방법을 전달. 몇 번째 location의 attribute의 데이터 해석 
	// 					2, // 각 데이터가 몇 개 단위로 이루어져 있는지 예제에서 각 점은 두 개의 float으로 표현
	// 					GL_FLOAT, // 데이터 타입
	// 					GL_FALSE, // 정규화가 필요한지
	// 					sizeof(float)*2, // (stride) (첫 데이터와 두 번째 데이터가 얼마나 떨어져있는지)
	// 					0); // offset 첫 데이터가 몇 바이트부터 시작하는지

	// +4 -5
	// unsigned int shader = CreateShader(vertexShader, fragShader);
	// glUseProgram(shader); // 현재 셰이더 프로그램을 "작업 상태"로 놓음
						  // draw call은 작업 상태인 셰이더 프로그램을 사용하여 작업 상태인 버퍼 데이터를 그림

	// +5
	ShaderProgramSource source = ParseShader("res/shaders/Basic.shader"); //셰이더 코드 파싱
	
	unsigned int shader = CreateShader(source.VertexSource, source.FragSource);
	glUseProgram(shader); // 현재 셰이더 프로그램을 "작업 상태"로 놓음
						  // draw call은 작업 상태인 셰이더 프로그램을 사용하여 작업 상태인 버퍼 데이터를 그림


	// +2 -6 현재 "작업 상태"-glBindBuffer()에 들어와 있는 대상을 그리는 것
	glDrawArrays(GL_TRIANGLES, //실제 draw call, 삼각형을 그릴 것이라고 명시
				0,				//몇 번째 데이터부터 그리려는지 명시(모두 그린다면 0)
				3);				//몇 개의 데이터를 그릴 것인지 명시
	//현재는 아무것도 화면에 나오지 않을 것임. 
	//왜냐하면 삼각형을 어떻게 그릴 것인지 쉐이더를 통해 알려주지 않았기 때문!

	// +6---------데이터를 전달하는 과정--------//
	unsigned int bufferID;
	glGenBuffers(1, &bufferID); //1. 버퍼 생성
	glBindBuffer(GL_ARRAY_BUFFER, bufferID); //2. 바인딩("activate")
	glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float), positions, GL_STATIC_DRAW);  //3. 데이터 전달

	//---------데이터를 해석하는(법을 정의하는) 과정--------//
	glEnableVertexAttribArray(0); //1. 몇 번째 Location의 attribute를 활성화(enable)
	glVertexAttribPointer(0, 2,	GL_FLOAT, GL_FALSE, sizeof(float)*2, 0); //2. 해석 방법을 전달.

	unsigned int ibo; //index buffer object
	glGenBuffers(1, &ibo); //1. 인덱스 버퍼 생성
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo); //2. 바인딩, ELEMENT_ARRAY_BUFFER
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), indices, GL_STATIC_DRAW);  //3. 작업 상태 버퍼에 데이터 전달

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		/* Render here */
		glClear(GL_COLOR_BUFFER_BIT);

		// +6 draw the object
		glDrawElements(GL_TRIANGLES, // drawelement함수
						6,			// 그릴 index의 갯수
						GL_UNSIGNED_INT, // index 타입 
						nullptr); // offset 포인터

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	// +4 셰이더 삭제
	glDeleteProgram(shader); 

	glfwTerminate();
	return 0;
} 