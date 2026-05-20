/* Hello Triangle - código adaptado de https://learnopengl.com/#!Getting-started/Hello-Triangle
 *
 * Adaptado por Rossana Baptista Queiroz
 * para as disciplinas de Processamento Gráfico/Computação Gráfica - Unisinos
 * Versão inicial: 7/4/2017
 * Última atualização em 07/03/2025
 */

#include <iostream>
#include <string>
#include <assert.h>
#include <vector>
#include <fstream>
#include <sstream>

using namespace std;

// GLAD
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

//GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Protótipo da função de callback de teclado
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

// Protótipos das funções
int setupShader();

// Dimensões da janela
const GLuint WIDTH = 1000, HEIGHT = 1000;

// Código fonte do Vertex Shader
const GLchar* vertexShaderSource = "#version 330\n"
"layout (location = 0) in vec3 position;\n"
"layout (location = 1) in vec3 color;\n"
"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"out vec4 finalColor;\n"
"void main()\n"
"{\n"
"gl_Position = projection * view * model * vec4(position, 1.0);\n"
"finalColor = vec4(color, 1.0);\n"
"}\0";

// Código fonte do Fragment Shader
const GLchar* fragmentShaderSource = "#version 330\n"
"in vec4 finalColor;\n"
"out vec4 color;\n"
"void main()\n"
"{\n"
"color = finalColor;\n"
"}\n\0";

struct OBJModel
{
	GLuint VAO;
	GLuint VBO;

	vector<float> vertices;

	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;
};

vector<OBJModel> objetos;

int objetoSelecionado = 0;

bool modoRotacao = false;
bool modoTranslacao = false;
bool modoEscala = false;

GLuint shaderID;
GLint modelLoc;
GLint viewLoc;
GLint projectionLoc;

bool loadOBJ(string path, OBJModel& model)
{
	vector<glm::vec3> temp_vertices;

	ifstream file(path);

	if (!file.is_open())
	{
		cout << "Erro ao abrir OBJ!" << endl;
		return false;
	}

	string line;

	while (getline(file, line))
	{
		stringstream ss(line);

		string type;
		ss >> type;

		// Vértices
		if (type == "v")
		{
			glm::vec3 vertex;

			ss >> vertex.x >> vertex.y >> vertex.z;

			temp_vertices.push_back(vertex);
		}

		// Faces
		else if (type == "f")
		{
			vector<int> indices;
			string vertexString;

			while (ss >> vertexString)
			{
				stringstream vertexStream(vertexString);

				string indexString;

				getline(vertexStream, indexString, '/');

				int index = stoi(indexString) - 1;

				indices.push_back(index);
			}

			// Triangulação automática
			for (int i = 1; i < indices.size() - 1; i++)
			{
				glm::vec3 v1 = temp_vertices[indices[0]];
				glm::vec3 v2 = temp_vertices[indices[i]];
				glm::vec3 v3 = temp_vertices[indices[i + 1]];

				float r = (float)rand() / RAND_MAX;
				float g = (float)rand() / RAND_MAX;
				float b = (float)rand() / RAND_MAX;

				vector<glm::vec3> triangle = { v1, v2, v3 };

				for (auto& v : triangle)
				{
					model.vertices.push_back(v.x);
					model.vertices.push_back(v.y);
					model.vertices.push_back(v.z);

					model.vertices.push_back(r);
					model.vertices.push_back(g);
					model.vertices.push_back(b);
				}
			}
		}
	}

	glGenVertexArrays(1, &model.VAO);
	glGenBuffers(1, &model.VBO);

	glBindVertexArray(model.VAO);

	glBindBuffer(GL_ARRAY_BUFFER, model.VBO);

	glBufferData(
		GL_ARRAY_BUFFER,
		model.vertices.size() * sizeof(float),
		model.vertices.data(),
		GL_STATIC_DRAW
	);

	glVertexAttribPointer(
		0,
		3,
		GL_FLOAT,
		GL_FALSE,
		6 * sizeof(float),
		(GLvoid*)0
	);

	glEnableVertexAttribArray(0);

	glVertexAttribPointer(
		1,
		3,
		GL_FLOAT,
		GL_FALSE,
		6 * sizeof(float),
		(GLvoid*)(3 * sizeof(float))
	);

	glEnableVertexAttribArray(1);

	glBindVertexArray(0);

	model.position = glm::vec3(0.0f);
	model.rotation = glm::vec3(0.0f);
	model.scale = glm::vec3(0.5f);

	cout << "Modelo carregado com "
		 << model.vertices.size() / 6
		 << " vertices." << endl;

	return true;
}

// Função MAIN
int main()
{
	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Vivencial 1", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	glfwSetKeyCallback(window, key_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
	}

	glViewport(0, 0, WIDTH, HEIGHT);

	shaderID = setupShader();

	glUseProgram(shaderID);

	modelLoc = glGetUniformLocation(shaderID, "model");
	viewLoc = glGetUniformLocation(shaderID, "view");
	projectionLoc = glGetUniformLocation(shaderID, "projection");

	glEnable(GL_DEPTH_TEST);

	OBJModel obj1;
	OBJModel obj2;

	loadOBJ("../assets/Modelos3D/suzanne.obj", obj1);
	loadOBJ("../assets/Modelos3D/suzanne.obj", obj2);

	obj1.position = glm::vec3(-1.5f, 0.0f, 0.0f);
	obj2.position = glm::vec3(1.5f, 0.0f, 0.0f);

	objetos.push_back(obj1);
	objetos.push_back(obj2);

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 view = glm::mat4(1.0f);
		view = glm::translate(view, glm::vec3(0.0f, 0.0f, -5.0f));

		glm::mat4 projection = glm::perspective(
			glm::radians(45.0f),
			(float)WIDTH / (float)HEIGHT,
			0.1f,
			100.0f
		);

		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

		for (int i = 0; i < objetos.size(); i++)
		{
			glm::mat4 model = glm::mat4(1);

			model = glm::translate(model, objetos[i].position);

			model = glm::rotate(model,
				glm::radians(objetos[i].rotation.x),
				glm::vec3(1.0f, 0.0f, 0.0f));

			model = glm::rotate(model,
				glm::radians(objetos[i].rotation.y),
				glm::vec3(0.0f, 1.0f, 0.0f));

			model = glm::rotate(model,
				glm::radians(objetos[i].rotation.z),
				glm::vec3(0.0f, 0.0f, 1.0f));

			model = glm::scale(model, objetos[i].scale);

			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

			glBindVertexArray(objetos[i].VAO);

			glDrawArrays(
				GL_TRIANGLES,
				0,
				objetos[i].vertices.size() / 6
			);

			glBindVertexArray(0);
		}

		glfwSwapBuffers(window);
	}

	glfwTerminate();

	return 0;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	// Seleção de objeto
	if (key == GLFW_KEY_TAB && action == GLFW_PRESS)
	{
		objetoSelecionado++;

		if (objetoSelecionado >= objetos.size())
		{
			objetoSelecionado = 0;
		}

		cout << "Objeto selecionado: "
			 << objetoSelecionado
			 << endl;
	}

	// Modos
	if (key == GLFW_KEY_R && action == GLFW_PRESS)
	{
		modoRotacao = true;
		modoTranslacao = false;
		modoEscala = false;

		cout << "Modo Rotacao" << endl;
	}

	if (key == GLFW_KEY_T && action == GLFW_PRESS)
	{
		modoRotacao = false;
		modoTranslacao = true;
		modoEscala = false;

		cout << "Modo Translacao" << endl;
	}

	if (key == GLFW_KEY_S && action == GLFW_PRESS)
	{
		modoRotacao = false;
		modoTranslacao = false;
		modoEscala = true;

		cout << "Modo Escala" << endl;
	}

	OBJModel& obj = objetos[objetoSelecionado];

	// =========================
	// ROTAÇÃO
	// =========================
	if (modoRotacao)
	{
		if (key == GLFW_KEY_X &&
			(action == GLFW_PRESS || action == GLFW_REPEAT))
		{
			obj.rotation.x += 2.0f;
		}

		if (key == GLFW_KEY_Y &&
			(action == GLFW_PRESS || action == GLFW_REPEAT))
		{
			obj.rotation.y += 2.0f;
		}

		if (key == GLFW_KEY_Z &&
			(action == GLFW_PRESS || action == GLFW_REPEAT))
		{
			obj.rotation.z += 2.0f;
		}
	}

	// =========================
	// TRANSLAÇÃO
	// =========================
	if (modoTranslacao)
	{
		if (key == GLFW_KEY_W &&
			(action == GLFW_PRESS || action == GLFW_REPEAT))
		{
			obj.position.y += 0.05f;
		}

		if (key == GLFW_KEY_S &&
			(action == GLFW_PRESS || action == GLFW_REPEAT))
		{
			obj.position.y -= 0.05f;
		}

		if (key == GLFW_KEY_A &&
			(action == GLFW_PRESS || action == GLFW_REPEAT))
		{
			obj.position.x -= 0.05f;
		}

		if (key == GLFW_KEY_D &&
			(action == GLFW_PRESS || action == GLFW_REPEAT))
		{
			obj.position.x += 0.05f;
		}

		if (key == GLFW_KEY_Q &&
			(action == GLFW_PRESS || action == GLFW_REPEAT))
		{
			obj.position.z += 0.05f;
		}

		if (key == GLFW_KEY_E &&
			(action == GLFW_PRESS || action == GLFW_REPEAT))
		{
			obj.position.z -= 0.05f;
		}
	}

	// =========================
	// ESCALA
	// =========================
	if (modoEscala)
	{
		if (key == GLFW_KEY_UP &&
			(action == GLFW_PRESS || action == GLFW_REPEAT))
		{
			obj.scale += glm::vec3(0.02f);
		}

		if (key == GLFW_KEY_DOWN &&
			(action == GLFW_PRESS || action == GLFW_REPEAT))
		{
			obj.scale -= glm::vec3(0.02f);
		}
	}
}

// Shader
int setupShader()
{
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	GLuint shaderProgram = glCreateProgram();

	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);

	glLinkProgram(shaderProgram);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return shaderProgram;
}

