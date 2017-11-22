in vec3 a_position;
out vec4 v_color;
in vec4 a_color;
in float time;

uniform mat4 u_projView;

void main()
{
    v_color = a_color;
    gl_Position =  u_projView * vec4(a_position.x+sin(time),a_position.y+sin(time),a_position.z,1);
}
