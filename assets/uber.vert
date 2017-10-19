//Pass in to vertex shader.
in vec3 a_position;
in vec2 a_texCoord0;
in vec4 a_color;

//Pass out to fragment shader.
out vec2 v_texCoord0;
out vec4 v_color;

//Uniforms: set by application.
uniform bool hasColor,hasTex;
uniform mat4 u_projView;

void main()
{
    if(hasColor) v_color = a_color; else v_color = vec4(1.0f,1.0f,1.0f,1.0f);
    if(hasTex) v_texCoord0 = a_texCoord0;
    gl_Position =  u_projView * vec4(a_position,1);
}
