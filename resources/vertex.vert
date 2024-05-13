#version 330 core
layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec2 inOffset;
layout (location = 2) in vec4 inColor;

flat out int instanceId;

out vec4 color;


void main()  
{   
    instanceId = gl_InstanceID;
    color = inColor;
    gl_Position = vec4(inPosition.x + inOffset.x, inPosition.y + inOffset.y, inPosition.z, 1.0); 
}
