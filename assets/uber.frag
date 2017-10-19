#ifdef GL_ES
    #define LOWP lowp
    precision mediump float;
#else
    #define LOWP
#endif

//Passed in from vertex shader.
in LOWP vec4 v_color;
in vec2 v_texCoord0;

//Passed out to final stage.
out vec4 out_color;

//Uniforms: set by application.
uniform bool hasColor,hasTex;
uniform sampler2D tex;

void main()
{
    if(hasTex) out_color = texture(tex, v_texCoord0) * v_color;
    else out_color = v_color; 
}
