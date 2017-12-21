#ifdef GL_ES
    #define LOWP lowp
    precision mediump float;
#else
    #define LOWP
#endif

in vec2 v_texcoord0;
in vec3 v_normal;
out vec4 out_color;

uniform sampler2D tex;
//uniform vec4 u_emission;
uniform vec4 u_diffuse;
uniform vec3 u_light_dir;

void main()
{
    vec4 diffuse = u_diffuse;
    //vec3 emission = u_emission.rgb;
    vec3 ambient = diffuse.rgb;
    vec3 normal = normalize(v_normal);
    vec3 diffuseLight = vec3(0.0, 0.0, 0.0);
    vec3 ambientLight = vec3(0.0, 0.0, 0.0);
    ambientLight += vec3(0.2, 0.2, 0.2);
    vec3 l = vec3(0.0, 0.0, 1.0);
    diffuseLight += vec3(1.0, 1.0, 1.0) * max(dot(normal, l), 0.);
    vec3 color = vec3(0.0, 0.0, 0.0);
    color += diffuse.rgb * diffuseLight;
    //color += emission;
    color += ambient * ambientLight;
    out_color = texture(tex, v_texcoord0) * vec4(color,1.0f);
}
