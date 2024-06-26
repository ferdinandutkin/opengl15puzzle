#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <openglDebug.h>
#include <shader.h>
#include <iostream>
#include <algorithm>
#include <queue>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

//#include "C:\Program Files\RenderDoc\renderdoc_app.h"


#include <Windows.h>

#include <cassert>


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);


const unsigned int SCR_WIDTH = 800;


const unsigned int SCR_HEIGHT = 800;

const unsigned int ROWS = 4;
const unsigned int COLUMNS = 4;
const unsigned int MAX_INDEX = ROWS * COLUMNS - 1;


struct instance_data {
    glm::vec2 offset;
    glm::vec4 color;
};




std::queue<std::pair<int, int>> indexes_to_update;


instance_data data[16]{};




int main(void)
{

    //RENDERDOC_API_1_1_2* rd = nullptr;
    //if (const HMODULE mod = LoadLibrary("C:\\Program Files\\RenderDoc\\renderdoc.dll")) {
    //    const auto RENDERDOC_GetAPI = reinterpret_cast<pRENDERDOC_GetAPI>(GetProcAddress(mod, "RENDERDOC_GetAPI"));
    //    assert(RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_1_2, reinterpret_cast<void**>(&rd)));
    //}


    if (!glfwInit()) {
        return -1;
    }


	//enable opengl debugging output.
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);



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

	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(glDebugOutput, 0);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

	//shader loading example
	shader shader;
	shader.loadShaderProgramFromFile(RESOURCES_PATH "vertex.vert", RESOURCES_PATH "fragment.frag");
	shader.use();


    float length = 0.25;


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
                .offset = {
                    xlocal,
                    ylocal
                 },
                .color = {
                    (float)writeindex / (MAX_INDEX),
                     0,
                     0,
                    (float)(writeindex != MAX_INDEX)
            }
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
    glVertexAttribPointer(offsetAttrib, 2, GL_FLOAT, GL_FALSE, sizeof(instance_data), (void*)offsetof(instance_data, offset));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribDivisor(offsetAttrib, 1);


    auto colorAttrib = glGetAttribLocation(shader.id, "inColor");

    glEnableVertexAttribArray(colorAttrib);
    glBindBuffer(GL_ARRAY_BUFFER, VBOinstance);
    glVertexAttribPointer(colorAttrib, 4, GL_FLOAT, GL_FALSE, sizeof(instance_data), (void*)offsetof(instance_data, color));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribDivisor(colorAttrib, 1);


    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

  //  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glBindVertexArray(VAO);


    while (!glfwWindowShouldClose(window))
    {
        // Clear the color buffer
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Check if there are any updates to the buffer data
        if (!indexes_to_update.empty()) {
            glBindBuffer(GL_ARRAY_BUFFER, VBOinstance);

            // Process each update
            while (!indexes_to_update.empty()) {
                auto indexes = indexes_to_update.front();



          /*      glBufferSubData(GL_ARRAY_BUFFER, 0,
                    sizeof(data),
                    &data);*/


              /*  glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
                glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_DYNAMIC_DRAW);*/

    /*            glBufferSubData(GL_ARRAY_BUFFER, indexes.second * sizeof(instance_data),
                    sizeof(instance_data),
                    &data);

                glBufferSubData(GL_ARRAY_BUFFER, indexes.first * sizeof(instance_data),
                    sizeof(instance_data),
                    &data);*/



                void* ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
       
                memcpy(ptr, data, sizeof(data));
                glUnmapBuffer(GL_ARRAY_BUFFER);



                // Re-enable both color and offset attributes
                glEnableVertexAttribArray(colorAttrib);
                glEnableVertexAttribArray(offsetAttrib);

                // Remove processed update
                indexes_to_update.pop();
            }

            // Unbind the buffer
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }


        // Bind vertex array object


        // Draw the instances
        glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, 16);

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }




    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteBuffers(1, &VBOinstance);
    shader.clear();


    glfwDestroyWindow(window);
	glfwTerminate();


    //rd->EndFrameCapture(nullptr, nullptr);
	return 0;
}



void relocate_empty_cell_if_posssible(int row_offset, int col_offset) {

    auto empty_cell_iter = std::find_if(std::begin(data), std::end(data), [](const instance_data& entry) {return entry.color.a < 0.01; });

    if (empty_cell_iter == std::end(data))
    {
        return;
    }

    
    auto empty_cell_index = std::distance(std::begin(data), empty_cell_iter);

    auto row = empty_cell_index / COLUMNS;
    auto column = empty_cell_index % ROWS;


    if (column + col_offset >= COLUMNS || column + col_offset < 0) {
        return;
    }

    if (row + row_offset >= ROWS || row + row_offset < 0) {
        return;
    }

    auto target_cell_index = ((row + row_offset) * ROWS + column + col_offset);


    if (target_cell_index < 0 || target_cell_index >= (ROWS * COLUMNS)) {
        return;
    }

    std::swap(data[empty_cell_index].color, data[target_cell_index].color);



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
        relocate_empty_cell_if_posssible(0, +1);
        return;
    }

    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        relocate_empty_cell_if_posssible(0, -1);
        return;
    }

}

