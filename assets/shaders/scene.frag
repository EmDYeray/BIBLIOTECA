#version 330 core
in vec3 world,local,n,localN,c;
in vec2 uv;
in float trailAlpha;
flat in int mat;
uniform int mode,material,shaded;
uniform sampler2D frontImage,backImage,maskImage,materialGrain;
uniform vec3 tint,eye,hearth;
uniform vec4 furnitureFloor[14],readingLamps[4];
uniform float dim,fireTime,readerLight,popColor,popupLightMotion;
out vec4 pixel;

vec2 surfacePoint(vec3 p,vec3 normal){
    vec3 axis=abs(normal);
    if(axis.y>=max(axis.x,axis.z))return p.xz;
    return axis.x>axis.z?p.zy:p.xy;
}
float emberPulse(){
    return .87+.074*sin(fireTime*4.9)+.047*sin(fireTime*8.7+1.1)+.030*sin(fireTime*13.1+.7);
}
void main(){
    if(mode==3){pixel=texture(frontImage,uv);return;}
    if(mode==4){pixel=vec4(1);return;}
    if(mode==6){
        float cloud=1.0-smoothstep(.06,1.0,dot(uv,uv));
        pixel=vec4(c,trailAlpha*cloud*cloud);
        return;
    }
    vec3 normal=normalize(n)*(gl_FrontFacing?1.0:-1.0);
    vec3 view=normalize(eye-world);
    if(mode==5){
        float edge=pow(max(0.0,dot(normal,view)),.64);
        float tip=1.0-smoothstep(.48,1.0,uv.x);
        float root=smoothstep(.0,.19,uv.x);
        float curl=.74+.26*sin(uv.x*12.2-fireTime*9.0+uv.y*2.7);
        vec3 flame=mix(vec3(1.0,.82,.43),vec3(1.0,.165,.026),smoothstep(.14,.88,uv.x));
        pixel=vec4(flame,dim*c.x*edge*tip*root*(.48+.22*curl));
        return;
    }

    vec3 base=c*tint;
    bool ink=mode>0 && (mat==10 || mat==11);
    if(ink)base=mat==11?texture(backImage,uv).rgb:texture(frontImage,uv).rgb;
    bool cutout=material==7;
    int kind=(material<0 || cutout)?mat:material;
    vec2 surface=surfacePoint(local,localN);
    vec2 grainAt=surface*1.09;
    if(kind==0 || kind==5)grainAt=(kind==5?world.xz:surface)*vec2(.27,.44);
    else if(kind==4)grainAt=surfacePoint(world,normal)*.50;
    else if(kind==2)grainAt=surface*1.9;
    else if(kind==3 || kind==15)grainAt=surface*vec2(1.05,1.39);
    else if(kind==12 || kind==1)grainAt=surface.yx*vec2(.42,.86);
    else if(kind==21)grainAt=surface*vec2(.61,1.23);
    vec3 grain=texture(materialGrain,grainAt).rgb;
    if(kind==0 || kind==5)base*=.85+.27*grain.r;
    if(kind==4)base*=.96+.081*grain.b;
    if(kind==2)base*=ink?.994+.012*grain.g:.93+.13*grain.g;
    if(kind==3 || kind==15)base*=.91+.18*grain.g;
    if(cutout)base*=.96+.08*grain.g;
    if(kind==12 || kind==1)base*=.96+.087*grain.r;
    if(kind==13 || kind==16)base*=.97+.06*grain.g;
    if(kind==20)base=base*(.915+.16*grain.b)+grain.b*vec3(.053,.048,.035);
    if(kind==21)base*=.97+.062*grain.g;

    if(cutout && popColor>0.){
        float luma=dot(base,vec3(.2126,.7152,.0722));
        base=clamp(mix(vec3(luma),base,1.+.19*popColor)*1.035,0.,1.);
    }

#ifdef BIBLIOTECA_MOBILE
    // Broad, warm lighting with a small tile-GPU budget: no per-pixel lamp
    // or furniture loops and no exponential falloff.
    float adult=smoothstep(16.96,17.14,world.x);
    vec3 toFire=hearth+vec3(0.,.80,0.)-world;
    float fireLight=(1.-adult)*2.35/(1.+.72*dot(toFire,toFire));
    float overhead=max(0.,normal.y);
    float shelfWash=max(0.,1.-abs(world.y-5.13)*.38-max(0.,world.z+.06)*.18);
    float cove=max(0.,1.-abs(world.y-6.)*.30-max(0.,world.z)*.09);
    vec3 light=mix(vec3(.77,.82,.85),vec3(.86,.79,.69),adult)*(.77+.13*overhead)
        +shelfWash*vec3(.15,.16,.13)+cove*vec3(.16,.14,.10)
        +fireLight*vec3(.70,.33,.14);
    float readerDiffuse=max(0.,dot(normal,normalize(vec3(-.42,.81,.39))));
    light=mix(light,vec3(.83,.79,.72)+readerDiffuse*vec3(.31,.27,.21),readerLight*.90);
    vec3 outColor=base*light;
    if(cutout || kind==12 || kind==13 || kind==17){
        float face=max(0.,dot(normal,view));
        float rim=1.-face;
        outColor+=vec3(.13,.17,.19)*rim*rim*(.35+.65*readerLight);
        outColor+=base*readerDiffuse*.14*readerLight;
    }
#else
    // Broad gradients approximate bounced light from the perimeter coves.
    float adult=smoothstep(16.96,17.14,world.x);
    float sideDepth=adult>.5?min(world.x-17.05,36.80-world.x):17.05-abs(world.x);
    float wallDepth=min(sideDepth,15.20-world.z);
    float coveHeight=world.y-6.0;
    float shelfHeight=world.y-5.13;
    float shelfDepth=max(0.0,world.z+.06);
    float cove=exp(-.21*max(wallDepth,0.0)-.182*coveHeight*coveHeight);
    float shelfWash=.83*exp(-.23*shelfHeight*shelfHeight-.36*shelfDepth*shelfDepth);
    float shelfSpill=.14*exp(-.13*shelfDepth*shelfDepth);
    float bounce=.52+.13*max(0.0,normal.y)+.04*max(0.0,-normal.z);
    vec3 light=mix(vec3(.90,1.01,1.03),vec3(.99,.99,.87),adult)*bounce
        +(cove+shelfWash+shelfSpill)*vec3(.54,.48,.374);
    float washPos=.5+.5*cos(world.x*.79+.44);
    light+=(.09+.18*washPos)*exp(-.22*shelfDepth*shelfDepth)*vec3(.89,.96,.99);
    if(adult>0.0){
        float downlight=0.0;
        for(int i=0;i<4;++i){
            vec3 delta=readingLamps[i].xyz-world;
            float range=dot(delta,delta);
            float f=max(0.,dot(normal,delta*inversesqrt(max(range,.002))));
            downlight+=(.27+.73*f)*1.47/(1.0+.39*range);
        }
        light+=adult*downlight*vec3(.90,.63,.40);
    }
    float frontRecess=(1.0-smoothstep(-.60,.31,world.z))*smoothstep(-1.47,-.69,world.z);
    float perimeterRecess=1.0-smoothstep(.14,.87,wallDepth);
    float shelfRecess=max(frontRecess,perimeterRecess);
    if(kind==0 || kind==2 || kind==3 || kind==21)light*=1.0-.21*shelfRecess;
    if(kind==5 || (kind==4 && abs(world.y-hearth.y)<.027)){
        float edge=max(0.0,length(world.xz-hearth.xz)-1.31);
        light*=1.0-.24*exp(-edge*8.1);
    }
    if(kind==5){
        float contact=0.;
        for(int i=0;i<14;++i){
            vec4 box=furnitureFloor[i];
            vec2 d=max(abs(world.xz-box.xy)-box.zw*.84,vec2(0.));
            contact=max(contact,1.0-smoothstep(.0,.32,dot(d,d)));
        }
        light*=1.0-.20*contact;
    }
    vec3 toFire=hearth+vec3(.07*emberPulse(),.80,0.0)-world;
    vec3 fireDir=normalize(toFire);
    float falloff=2.44/(1.0+.72*dot(toFire,toFire));
    float warm=falloff*emberPulse()*(1.0-adult);
    light+=vec3(1.0,.50,.24)*warm*(.23+.77*max(0.,dot(normal,fireDir)));

    // Soft light on the object being read also keeps the printed pages legible.
    vec3 readingDir=normalize(vec3(-.46,.79,.41)+.44*view);
    float readerDiffuse=max(0.,dot(normal,readingDir));
    light=mix(light,vec3(.69,.71,.75)+vec3(.53,.42,.34)*readerDiffuse,readerLight*.92);
    vec3 halfway=normalize(fireDir+view);
    float reflectance=kind==1?.32:kind==12?.24:kind==20?.145:kind==5?.09:kind==13?.25:.030;
    float shine=pow(max(0.,dot(normal,halfway)),kind==5?12.0:26.0);
    vec3 outColor=base*light+vec3(.95,.48,.19)*reflectance*shine*warm;
    if(kind==12 || kind==13){
        float edge=pow(1.-max(0.,dot(normal,view)),3.);
        float highlight=pow(max(0.,dot(normal,normalize(readingDir+view))),38.);
        outColor+=readerLight*(vec3(.19,.23,.28)*edge+vec3(.38,.40,.45)*highlight)
            *reflectance*(cutout?.30:1.);
    }
    if(kind==1){
        float contour=pow(1.0-max(0.0,dot(normal,view)),2.2);
        outColor+=contour*vec3(.18,.137,.076);
    }
    if(cutout){
        float contour=1.-max(0.,dot(normal,view));
        outColor+=.022*contour*contour*vec3(.98,.85,.64);
    }
    if(kind==17 && readerLight>0.){
        float face=max(0.,dot(normal,view));
        outColor=base*(.42+.65*readerDiffuse+.23*face)+.042*pow(1.-face,3.1)*vec3(.52,.70,1.);
    }
#endif
#ifdef BIBLIOTECA_MOBILE
    if(ink)outColor=mix(outColor,base,.78*readerLight+.13);
#else
    if(ink)outColor=mix(outColor,base,.56*readerLight+.07);
#endif
    if(shaded!=0 && ink){
        float pageX=shaded==1 ? .5+.5*uv.x : .5*uv.x;
        vec2 q=vec2(pageX,1.0-uv.y),d=vec2(.005,.0065);
        float contact=.34*texture(maskImage,q).r;
        contact+=.165*(texture(maskImage,q+d).r+texture(maskImage,q-d).r
            +texture(maskImage,q+vec2(-d.x,d.y)).r+texture(maskImage,q+vec2(d.x,-d.y)).r);
        outColor*=1.0-contact*vec3(.27,.28,.31);
    }
    if(kind==14){
        float pulse=.5+.5*sin(fireTime*3.4+local.y*3.1+local.x*7.0);
        outColor=cutout?base*(.92+popupLightMotion*(.34*pulse-.15))+vec3(.048,.034,.018)
                       :base*1.08+vec3(.10,.059,.032);
    }
    if(kind==18)outColor=base*.98;
    if(kind==19)outColor=base*(.49+.43*emberPulse());
    if(kind==22)outColor=base*(.83+.20*max(0.,dot(normal,view)))+vec3(.18,.111,.055);
    pixel=vec4(clamp(outColor*dim,0.,1.),1.);
}
