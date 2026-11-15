#version 330 core
layout(location=0) in vec3 pos;
layout(location=1) in vec3 norm;
layout(location=2) in vec3 pigment;
layout(location=3) in vec2 texcoord;
layout(location=4) in float surface;
uniform mat4 vp,model;
uniform int mode;
uniform float fireTime;
out vec3 world,local,n,localN,c;
out vec2 uv;
out float trailAlpha;
flat out int mat;
void main(){
    uv=texcoord;
    if(mode==3){gl_Position=vec4(pos.xy*2.0,0,1);return;}
    vec3 q=pos;
    if(mode==4){gl_Position=model*vec4(q,1);return;}
    if(mode==6){
        c=pigment;
        trailAlpha=surface;
        gl_Position=vp*vec4(q,1);
        return;
    }
    if(mode==5){
        float h=uv.x,phase=uv.y;
        float sway=h*h;
        q.x+=sway*(.139*sin(fireTime*4.7+phase+h*5.)+.048*sin(fireTime*9.7+phase-h*6.));
        q.z+=sway*.116*sin(fireTime*5.9+phase+h*7.4);
        q.y+=h*pigment.y*(.105*sin(fireTime*6.6+phase)+.050*sin(fireTime*11.1+h*3.));
    }
    vec4 p=model*vec4(q,1);
    world=p.xyz;local=q;n=mat3(transpose(inverse(model)))*norm;
    localN=norm;c=pigment;mat=int(surface+.5);
    gl_Position=vp*p;
}
