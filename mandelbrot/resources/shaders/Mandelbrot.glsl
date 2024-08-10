// Mandelbrot Shader

#type vertex
#version 450 core

layout(location = 0) in vec3 iPosition;
layout(location = 1) in vec2 iResolution;

layout(std140, binding = 0) uniform Camera
{
	mat4 uViewProjection;
};

layout (location = 0) out vec2 oResolution;

void main()
{
	oResolution = iResolution;
	gl_Position = uViewProjection * vec4(iPosition, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 oColour;

layout (location = 0) in vec2 iResolution;

const int ITERATIONS = 10000;
const float LIMIT = 16.0;
const vec2 MIN = vec2(-3.0, -2.0);
const vec2 MAX = vec2(2.0, 2.0);

vec3 spectralColour(float l)        // RGB <0,1> <- lambda l <400,700> [nm]
{
    float t;  
    vec3 c = vec3(0.0,0.0,0.0);
    
    if ((l>=400.0)&&(l<410.0)) { t=(l-400.0)/(410.0-400.0); c.r=    +(0.33*t)-(0.20*t*t); }
    else if ((l>=410.0)&&(l<475.0)) { t=(l-410.0)/(475.0-410.0); c.r=0.14         -(0.13*t*t); }
    else if ((l>=545.0)&&(l<595.0)) { t=(l-545.0)/(595.0-545.0); c.r=    +(1.98*t)-(     t*t); }
    else if ((l>=595.0)&&(l<650.0)) { t=(l-595.0)/(650.0-595.0); c.r=0.98+(0.06*t)-(0.40*t*t); }
    else if ((l>=650.0)&&(l<700.0)) { t=(l-650.0)/(700.0-650.0); c.r=0.65-(0.84*t)+(0.20*t*t); }
         if ((l>=415.0)&&(l<475.0)) { t=(l-415.0)/(475.0-415.0); c.g=             +(0.80*t*t); }
    else if ((l>=475.0)&&(l<590.0)) { t=(l-475.0)/(590.0-475.0); c.g=0.8 +(0.76*t)-(0.80*t*t); }
    else if ((l>=585.0)&&(l<639.0)) { t=(l-585.0)/(639.0-585.0); c.g=0.84-(0.84*t)           ; }
         if ((l>=400.0)&&(l<475.0)) { t=(l-400.0)/(475.0-400.0); c.b=    +(2.20*t)-(1.50*t*t); }
    else if ((l>=475.0)&&(l<560.0)) { t=(l-475.0)/(560.0-475.0); c.b=0.7 -(     t)+(0.30*t*t); }
    return c;
}

int mandelbrot( vec2 c )
{
    vec2 z = vec2(0, 0);
    
    for ( int i = 0; i < ITERATIONS; ++i)
    {
        vec2 z_squared;

        z_squared.x = pow(z.x, 2.0) - pow(z.y, 2.0);
        z_squared.y = 2.0 * z.x * z.y;
        
        z = z_squared + c;
        
        if ( ( pow(z.x, 2.0) + pow(z.y, 2.0) ) > LIMIT )
        {
            return i;
        }
    }
    
    return 0;
}


void main()
{
    // Normalized pixel coordinates (from 0 to 1)
    vec2 uv = gl_FragCoord.xy/iResolution.xy;
    
    vec2 c = mix(MIN, MAX, uv);
    
    int iters = mandelbrot(c);
    
    vec3 col = vec3(0.0, 0.0, 0.0);
    if (iters != 0)
    {
        float q = float(iters) / float(ITERATIONS);
        q = pow(q,0.2);
        col = spectralColour( 400.0 + ( 300.0 * q ));
    }

	oColour = vec4(col, 1.0);
}
