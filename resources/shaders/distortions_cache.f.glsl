#version 120

#define highp

//precision highp float;

//uniform sampler2D texture;

uniform vec2 LensCenter;
uniform vec2 ScreenCenter;
uniform vec2 Scale;
uniform vec2 ScaleIn;
uniform vec4 HmdWarpParam;

varying vec2 texcoord;

vec2 HmdWarp(vec2 texIn) 
{ 
   vec2  theta  = (texIn - LensCenter) * ScaleIn;
   float rSq = dot(theta,theta);
   vec2  theta1 = theta * (HmdWarpParam.x + HmdWarpParam.y * rSq + HmdWarpParam.z * rSq * rSq + HmdWarpParam.w * rSq * rSq * rSq);
   
   return LensCenter + Scale * theta1;
}

void main()
{
   vec2 tc = HmdWarp(texcoord);
   if (any(notEqual(clamp(tc, ScreenCenter-vec2(0.25,0.5), ScreenCenter+vec2(0.25, 0.5)) - tc, vec2(0.0, 0.0))))
       gl_FragColor.rgba = vec4(-1.0, -1.0, 0.0, 0.0);
   else
       gl_FragColor.rgba = vec4(tc.r, tc.g, 0,0); //texture2D(texture, tc);
}
