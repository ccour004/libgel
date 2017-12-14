#ifdef GL_ES
    #define LOWP lowp
    precision mediump float;
#else
    #define LOWP
#endif

in vec2 v_texcoord0;
out vec4 out_color;

uniform sampler2D tex;
uniform vec4 u_diffuse;

void main()
{
    out_color = /*u_diffuse + */texture(tex, v_texcoord0);
}
