in vec3 a_position;
//in vec4 a_color;

in vec2 a_texcoord0;
out vec2 v_texcoord0;

uniform mat4 u_projView;

void main()
{
    //v_color = a_color;
    v_texcoord0 = a_texcoord0;
    gl_Position =  u_projView * vec4(a_position,1);
}
