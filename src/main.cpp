#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <openglDebug.h>
#include <demoShader.h>
#include <iostream>
#include <algorithm>
#include <queue>

//#include "renderdoc_app.h"


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);


#define USE_GPU_ENGINE 0
extern "C"
{
	__declspec(dllexport) unsigned long NvOptimusEnablement = USE_GPU_ENGINE;
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = USE_GPU_ENGINE;
}


const unsigned int SCR_WIDTH = 800;


const unsigned int SCR_HEIGHT = 800;

const unsigned int ROWS = 4;
const unsigned int COLUMNS = 4;


#pragma pack(push, 1)
struct instance_data {
    float x_offset;
    float y_offset;

    float r;
    float g;
    float b;
    float a;
};
#pragma pack(pop)



std::queue<std::pair<int, int>> indexes_to_update;


instance_data data[16]{};




int main(void)
{

    //RENDERDOC_API_1_1_2* rd = nullptr;
    //if (const HMODULE mod = LoadLibrary(L"renderdoc.dll")) {
    //    const auto RENDERDOC_GetAPI = reinterpret_cast<pRENDERDOC_GetAPI>(GetProcAddress(mod, "RENDERDOC_GetAPI"));
    //    assert(RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_1_2, reinterpret_cast<void**>(&rd)));
    //}
    //rd->StartFrameCapture(nullptr, nullptr);


	if (!glfwInit())
		return -1;


#pragma region report opengl errors to std
	//enable opengl debugging output.
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
#pragma endregion


	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //you might want to do this when testing the game for shipping


	GLFWwindow *window = window = glfwCreateWindow(640, 480, "Simple example", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	glfwSetKeyCallback(window, key_callback);

	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	glfwSwapInterval(1);


#pragma region report opengl errors to std
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(glDebugOutput, 0);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
#pragma endregion


    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

	//shader loading example
	Shader shader;
	shader.loadShaderProgramFromFile(RESOURCES_PATH "vertex.vert", RESOURCES_PATH "fragment.frag");
	shader.bind();


    float length = 0.08;


    float vertices[] = {
        -length / 2, length / 2, 1.0f, //-+, 0
        -length / 2, -length / 2, 0.0f, // -, - 1
        length / 2, -length / 2, 0.0f, //+- 2
        length / 2, length / 2, 0.0f,
    };
    unsigned int indices[] = {
        0, 1, 3,  // first Triangle
        1, 2, 3   // second Triangle
    };


    for (int row = 0; row < ROWS; row++) {

        for (int column = 0; column < COLUMNS; column++) {
            float ylocal = -(row * length + (row == 0 ? 0 : ((row) * 0.01)));
            float xlocal = (column * length + (column == 0 ? 0 : ((column) * 0.01)));

            auto writeindex = (row * COLUMNS) + column;


            data[writeindex] = {
                .x_offset = xlocal,
                .y_offset = ylocal,
                .r = (float)writeindex / (ROWS * COLUMNS - 1.f),
                .g = 0,
                .b = 0,
                .a = (float)!(row == 3 && column == 3)

            };

        }

    }


    unsigned int VBO, VAO, EBO, VBOinstance;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glGenBuffers(1, &VBOinstance);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBOinstance);
    glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);


    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    auto positionAttrib = glGetAttribLocation(shader.id, "inPosition");

    glVertexAttribPointer(positionAttrib, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(positionAttrib);


    auto offsetAttrib = glGetAttribLocation(shader.id, "inOffset");

    glEnableVertexAttribArray(offsetAttrib);
    glBindBuffer(GL_ARRAY_BUFFER, VBOinstance);
    glVertexAttribPointer(offsetAttrib, 2, GL_FLOAT, GL_FALSE, sizeof(instance_data), (void*)offsetof(instance_data, x_offset));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribDivisor(offsetAttrib, 1);


    auto colorAttrib = glGetAttribLocation(shader.id, "inColor");

    glEnableVertexAttribArray(colorAttrib);
    glBindBuffer(GL_ARRAY_BUFFER, VBOinstance);
    glVertexAttribPointer(colorAttrib, 4, GL_FLOAT, GL_FALSE, sizeof(instance_data), (void*)offsetof(instance_data, r));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribDivisor(colorAttrib, 1);


    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);



	while (!glfwWindowShouldClose(window))
	{
        

            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);



            bool was_update = false;

            if (!indexes_to_update.empty()) {
                glBindBuffer(GL_ARRAY_BUFFER, VBOinstance);


                while (!indexes_to_update.empty()) {
                    auto indexes = indexes_to_update.front();

                    auto ordered_indexes = std::minmax(indexes.first, indexes.second);


                    glBufferSubData(GL_ARRAY_BUFFER, ordered_indexes.first * sizeof(instance_data), (ordered_indexes.second - ordered_indexes.first + 1) * sizeof(instance_data), &data[ordered_indexes.first]);
                    // does not work

                    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(data), data);
                    // does not work too


                    indexes_to_update.pop();

                    was_update = true;
                }

                glBindBuffer(GL_ARRAY_BUFFER, 0);
            }




            glBindVertexArray(VAO);

            glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, 16);

            //if (was_update) {
            //    Sleep(1000);
            //}

            glfwSwapBuffers(window);
            glfwPollEvents();
	}

	//there is no need to call the clear function for the libraries since the os will do that for us.
	//by calling this functions we are just wasting time.
	//glfwDestroyWindow(window);


    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteBuffers(1, &VBOinstance);
    shader.clear();


	glfwTerminate();


    //rd->EndFrameCapture(nullptr, nullptr);
	return 0;
}



int locate_empty_cell() {
    for (int i = 0; i < sizeof(data) / sizeof(instance_data); i++) {
        if (data[i].a < 0.01)
        {
            return i;
        }
    }

    return -1;
}

void relocate_empty_cell_if_posssible(int row_offset, int col_offset) {
    auto empty_cell_index = locate_empty_cell();

    auto row = empty_cell_index / COLUMNS;
    auto column = empty_cell_index % ROWS;

    auto target_cell_index = ((row + row_offset) * ROWS + column + col_offset);


    if (target_cell_index < 0 || target_cell_index >= (ROWS * COLUMNS)) {
        return;
    }

    std::swap(data[empty_cell_index], data[target_cell_index]);



    indexes_to_update.push({ target_cell_index, empty_cell_index });

    return;
}



void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
        return;
    }

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        relocate_empty_cell_if_posssible(+1, 0);
        return;
    }

    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        relocate_empty_cell_if_posssible(-1, 0);
        return;
    }

    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        relocate_empty_cell_if_posssible(0, -1);
        return;
    }

    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        relocate_empty_cell_if_posssible(0, +1);
        return;
    }

}

