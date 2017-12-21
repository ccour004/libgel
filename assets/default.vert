in vec3 a_position;

in vec4 a_color;
out vec4 v_color;

in vec2 a_texcoord0;
out vec2 v_texcoord0;

in vec3 a_normal;
out vec3 v_normal;

uniform mat4 u_modelViewMatrix;
uniform mat4 u_projectionMatrix;
uniform mat3 u_normalMatrix;

void main()
{
    v_color = a_color;
    v_texcoord0 = a_texcoord0;
    v_normal = /*u_normalMatrix * */a_normal;
    gl_Position =  u_projectionMatrix * u_modelViewMatrix * vec4(a_position,1);
}
