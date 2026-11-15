"""Layout of the independent pieces on every double-page diorama."""
from functools import cache
from math import cos, exp, pi, sin
from pathlib import Path

from celestial import globe, ridge, rock, saturn, terrain
from mesh_builder import Mesh, blend, load_open_model, rgb
from spacecraft import (beacon, capsule, crate, gantry, ground_dish, habitat, lander,
                        launch_tower, pedestal, reflector, rocket, rover, satellite, solar, station)


@cache
def model(name):
    name, _, variant = name.partition(':')
    open_root=Path(__file__).resolve().parents[1]/'assets'/'open_models'
    if name=='astronaut':
        return load_open_model(open_root/'nasa_astronaut.obj',.60)
    if name=='satellite' and variant in {'comms','imager','weather','telescope','radar','probe'}:
        return load_open_model(open_root/'nasa_acrimsat.obj',.62)
    if name=='lander' and variant in {'','moon','ice'}:
        lander_mesh=load_open_model(open_root/'nasa_viking_lander.obj',.54)
        lander_mesh.rod((.106,.252,.030),(.106,.352,.030),.008,'859AB0_metal',10)
        lander_mesh.sphere((.106,.358,.030),.017,'EEA95F_lamp',bands=8,segments=12)
        return lander_mesh
    builders = {'satellite':satellite,'rover':rover,'lander':lander,'rocket':rocket,'station':station}
    if name in builders:
        return builders[name](variant) if variant else builders[name]()
    builders = {'dish':ground_dish,'habitat':habitat,'tower':launch_tower,'gantry':gantry,
                'beacon':beacon,'crate':crate,'capsule':capsule}
    return builders[name]()


@cache
def cutout_bounds(item, yaw):
    c, s = cos(yaw), sin(yaw)
    lateral = [x*c+z*s for _, face in item.faces for (x, _, z), _, _ in face]
    return min(lateral), max(lateral)


def volume_bounds(item, at, scale, yaw):
    c, s = cos(yaw), sin(yaw)
    points=[]
    for _, face in item.faces:
        for (x,y,z),_,_ in face:
            points.append((at[0]+(x*c+z*s)*scale, at[1]+y*scale,
                           at[2]+(z*c-x*s)*scale))
    return (min(p[0] for p in points),max(p[0] for p in points),
            min(p[1] for p in points),max(p[1] for p in points),
            min(p[2] for p in points),max(p[2] for p in points))


def volumes_overlap(a, b, pad=.012):
    return (a[0] < b[1]-pad and a[1] > b[0]+pad and
            a[2] < b[3]-pad and a[3] > b[2]+pad and
            a[4] < b[5]-pad and a[5] > b[4]+pad)


def place_cutout(mesh, item, x, y, z, scale, yaw=0):
    # Keep each object in one leaf so neither edge of its silhouette is cut by the gutter.
    lo, hi = cutout_bounds(item, yaw)
    near, far = (.018,.926) if x >= 0 else (-.926,-.018)
    scale = min(scale,(far-near)/(hi-lo))
    x = min(max(x,near-lo*scale),far-hi*scale)
    # Resolve overlaps against previously authored pieces. Try lateral moves
    # first, then depth moves, and finally a small uniform reduction. This is
    # deterministic and keeps every actor on its own paper leaf.
    chosen=None
    for reduction in (1.,.94,.88,.82):
        fit=scale*reduction
        px=min(max(x,near-lo*fit),far-hi*fit)
        for _ in range(6):
            bounds=volume_bounds(item,(px,y,z),fit,yaw)
            changed=False
            for other in mesh.occupied:
                if not volumes_overlap(bounds,other):
                    continue
                if (bounds[0]+bounds[1])*.5 <= (other[0]+other[1])*.5:
                    px-=bounds[1]-other[0]+.018
                else:
                    px+=other[1]-bounds[0]+.018
                px=min(max(px,near-lo*fit),far-hi*fit)
                bounds=volume_bounds(item,(px,y,z),fit,yaw)
                changed=True
            if not changed or not any(volumes_overlap(bounds,o) for o in mesh.occupied):
                chosen=(px,y,z,fit,bounds)
                break
        if chosen:
            break
    if chosen is None:
        # Last resort: move the piece backward on the page instead of letting
        # it intersect a neighboring actor at the same depth.
        fit=scale*.78
        px=min(max(x,near-lo*fit),far-hi*fit)
        for dz in (-.16,.16,-.28,.28):
            bounds=volume_bounds(item,(px,y,z+dz),fit,yaw)
            if not any(volumes_overlap(bounds,o) for o in mesh.occupied):
                chosen=(px,y,z+dz,fit,bounds);break
    if chosen is None:
        chosen=(x,y,z,scale,volume_bounds(item,(x,y,z),scale,yaw))
    px,py,pz,fit,bounds=chosen
    mesh.include(item,(px,py,pz),fit,yaw)
    mesh.occupied.append(bounds)
    return (px,py,pz),fit


def mark(mesh, item, x, y, z, scale, yaw, motion, center):
    start = len(mesh.faces)
    (x,y,z),scale = place_cutout(mesh,item,x,y,z,scale,yaw)
    mesh.animate(start,motion,(x,y+center*scale,z))


def stage(mesh, *parts):
    for item, x, y, z, scale, yaw in parts:
        if item.startswith('rocket'):
            mark(mesh,model(item),x,y,z,scale,yaw,'launch',.26)
        elif item.startswith(('capsule','satellite','station')):
            mark(mesh,model(item),x,y,z,scale,yaw,'fly',.26)
        elif item.startswith('rover'):
            mark(mesh,model(item),x,y,z,scale,yaw,'roll',.10)
        else:
            place_cutout(mesh,model(item),x,y,z,scale,yaw)


def mount(mesh, item, x, y, z, scale=1., yaw=0):
    spacecraft=Mesh()
    spacecraft.include(model(item),(0,y,0),scale,yaw)
    start=len(mesh.faces)
    (px,_,pz),fit=place_cutout(mesh,spacecraft,x,0,z,1)
    mesh.animate(start,'spin',(px,y*fit,pz))
    mesh.box((px,.009,pz),(.225,.018,.166),'526f76',.008)
    mesh.box((px,.019,pz),(.177,.007,.124),'D5AA70',.004)
    mesh.box((px,.025,pz+.038),(.128,.005,.018),'EBCC96_metal',.002)


def planet(mesh, body, x,y,z,r):
    mesh.box((x,.010,z),(.235,.019,.175),'49656B',.008)
    mesh.box((x,.022,z),(.185,.006,.122),'B99565',.003)
    start=len(mesh.faces)
    mesh.include(globe(body,r),(x,y,z))
    mesh.animate(start,'spin',(x,y,z))


def saturn_scene(mesh, x,y,z,r, tilt,spin, icy_moon):
    assembly=Mesh()
    assembly.include(saturn(r,tilt,spin),(0,y,0))
    mark(mesh,assembly,x,0,z,1,0,'ringed',y)
    mx,my,mz,mr = icy_moon
    small=Mesh()
    small.include(globe('moon',mr),(0,my,0))
    mark(mesh,small,mx,0,mz,1,0,'spin',my)


def ground(mesh, kind, x,z,w,d,seed):
    mesh.include(terrain(kind,w,d,seed),(x,0,z))


def orbs(mesh,x,z,scale=1):
    for i,(dx,dz,r) in enumerate(((-.094,.035,.059),(.035,.022,.037),(.11,-.04,.045))):
        mesh.include(rock(r,i+4),(x+dx*scale,.043*scale,z+dz*scale),scale)


def solar_arc(mesh, x,y,z, radius, reverse):
    for i in range(3):
        points = [(x+reverse*(radius*.39+(radius*.48+i*.022)*cos(a)),
                   y+radius*.65+(.16-i*.026)*sin(a),
                   z+.022*sin(a+i)) for a in (pi*.08+j*.86*pi/23 for j in range(24))]
        for a,b in zip(points,points[1:]):
            mesh.rod(a,b,.005,'FDC26B_lamp',6)


def pioneer(mesh,x,y,z,scale=1,yaw=0):
    craft=Mesh()
    craft.tube((((0,.25,0),.085),((0,.342,0),.094)), '9D8F77_metal',10)
    craft.box((0,.356,-.055),(.233,.133,.172),'A99067_metal',.018)
    reflector(craft,0,.503,.068,.200)
    craft.rod((.052,.331,-.02),(.307,.468,.073),.012,'A6B0C4_metal',8)
    craft.rod((-.073,.33,-.024),(-.253,.278,.114),.011,'A6B0C4_metal',8)
    craft.box((.05,.279,.092),(.106,.069,.019),'CDAF69_metal',.008)
    mark(mesh,craft,x,y,z,scale,yaw,'fly',.38)


def apollo_instruments(mesh,x,z):
    mesh.box((x,.148,z),(.27,.15,.19),'C6A36F_metal',.02)
    mesh.box((x,.228,z),(.225,.018,.154),'AAB1C1_metal',.008)
    mesh.rod((x,.192,z),(x,.363,z),.014,'728999_metal',8)
    scan=len(mesh.faces)
    reflector(mesh,x,.427,z,.096)
    mesh.animate(scan,'scan',(x,.427,z))
    for i in range(2):
        off=x+.19+i*.11
        mesh.box((off,.105,z+.075),(.075,.068,.090),'8994AA_metal',.012)
        mesh.rod((x,.13,z),(off,.10,z+.075),.004,'474B60_metal',6)


def archive_scene(m,name):
    if name=='pioneer-salida':
        planet(m,'earth',-.50,.38,-.02,.221)
        stage(m,('rocket',.368,.030,-.025,.80,.15),('tower',.674,.017,-.113,.66,.06))
        m.box((.39,.023,.025),(.79,.044,.33),'5F6271_metal',.025)
    elif name=='pioneer-asteroides':
        pioneer(m,.343,.015,.051,.96,.28)
        for i,(x,y,z,r) in enumerate(((-.68,.24,-.02,.13),(-.35,.48,.019,.15),(-.12,.26,.13,.11),(-.58,.63,-.11,.083))):
            mark(m,rock(r,i+36),x,y,z,1,(i+1)*.29,'drift',r*.53)
    elif name=='pioneer-placa':
        pioneer(m,-.49,.011,.016,.89,-.19)
        m.box((.42,.39,.073),(.674,.420,.038),'C89B5D_metal',.023)
        m.box((.417,.014,.071),(.687,.028,.213),'6E786E_metal',.010)
        for i in range(9):
            a=i*2*pi/9
            m.rod((.25,.424,.095),(.25+.090*cos(a),.424+.080*sin(a),.095),.0035,'6D5639_metal',6)
        for x in (.484,.599):
            m.sphere((x,.486,.099),.023,'755F43_metal',bands=8,segments=16)
            m.rod((x,.356,.1),(x,.462,.1),.011,'755F43_metal',7)
            m.rod((x-.035,.417,.1),(x+.033,.428,.1),.006,'755F43_metal',6)
        for i in range(6):
            m.sphere((.17+i*.094,.247,.098),.01+(i%3)*.002,'624C3A_metal',bands=6,segments=12)
    elif name=='pioneer-senal':
        stage(m,('dish',-.46,.011,.044,.98,-.38))
        pioneer(m,.52,.027,-.071,.80,.35)
        for row in range(3):
            points=[(-.16+i*.038,.33+row*.08+.055*sin(i*.53),.163) for i in range(13)]
            for a,b in zip(points,points[1:]):m.rod(a,b,.006,'486D82_metal',6)
    elif name=='kepler-equilibrio':
        planet(m,'sun',-.542,.452,-.045,.204)
        mount(m,'satellite:telescope',.43,.294,.046,1.22,.58)
        for y in (.22,.31,.40):
            m.rod((-.21,y,.13),(.131,y,.13),.008,'E8AA5D_metal',7)
            m.rod((.092,y+.026,.13),(.131,y,.13),.008,'E8AA5D_metal',7)
    elif name=='kepler-campo':
        mount(m,'satellite:imager',-.492,.252,.018,1.13,-.23)
        for i,(x,y,r) in enumerate(((.24,.33,.057),(.5,.59,.083),(.711,.37,.062),(.24,.724,.033),(.76,.672,.035))):
            color='E3B276_lamp' if i%2 else '7A98B6_planet'
            planetlet=Mesh()
            planetlet.sphere((0,0,0),r,color,bands=12,segments=24)
            mark(m,planetlet,x,y,-.038,1,0,'drift',0)
        m.box((.48,.023,-.034),(.79,.045,.141),'626B83_metal',.010)
    elif name=='apolo-muestras':
        ground(m,'moon',-.44,.075,.80,.51,19)
        ground(m,'moon',.44,.075,.80,.51,23)
        place_cutout(m,model('astronaut'),-.48,.074,.076,1,.32)
        m.rod((-.181,.304,.22),(-.206,.086,.22),.008,'758B99_metal',12)
        m.box((-.207,.088,.222),(.073,.031,.051),'B1A08E_metal',.006)
        m.box((.43,.176,.123),(.46,.087,.274),'A0ABB2_metal',.019)
        indicator=(.61,.223,.219)
        start=len(m.faces)
        m.sphere(indicator,.024,'DF9854_lamp',bands=8,segments=16)
        m.animate(start,'pulse',indicator)
        for x,z in ((.24,.112),(.45,.153),(.58,.102)):
            m.include(rock(.072,round(x*100)),(x,.22,z),.87,.27)
        for x in (.241,.62):
            m.rod((x,.09,.117),(x,.158,.117),.016,'8E9CAD_metal',8)
        m.rod((.68,.085,-.042),(.68,.585,-.053),.012,'B9B4AE_metal',8)
        m.box((.572,.48,-.05),(.217,.136,.015),'BB9273_metal',.004)
    elif name=='apolo-easep':
        ground(m,'moon',-.43,.084,.81,.52,39)
        ground(m,'moon',.43,.084,.81,.52,43)
        apollo_instruments(m,.269,.063)
        m.box((-.46,.182,.119),(.472,.046,.305),'8B8F9A_metal',.011)
        for row in range(4):
            for col in range(6):
                m.box((-.653+col*.077,.210,.013+row*.065),(.059,.008,.049),'B6B9BD_metal',.003)
        for x in (-.63,-.29):
            m.rod((x,.061,.121),(x,.165,.121),.021,'6D7284_metal',8)
        stage(m,('beacon',-.678,.077,-.065,.66,.14))
    elif name=='apolo-red':
        ground(m,'moon',-.13,.07,1.48,.5,40)
        apollo_instruments(m,-.50,.04)
        stage(m,('lander:moon',.425,.073,.029,.97,.40))
    elif name=='apolo-surveyor':
        ground(m,'moon',-.06,.075,1.56,.53,11)
        stage(m,('dish',-.52,.075,.027,.90,-.31),('crate',.55,.087,.193,.94,-.12))
        m.tube((((.396,.092,-.031),.104),((.396,.325,-.031),.113)), '96A5A6_metal',10)
        for x,z in ((.25,-.15),(.61,.015),(.27,.215)):
            m.rod((.4,.305,-.032),(x,.093,z),.016,'C1AC7E_metal',8)
        start=len(m.faces)
        reflector(m,.389,.495,-.028,.157)
        m.animate(start,'scan',(.389,.495,-.028))
        m.rod((.4,.313,-.032),(.397,.427,-.032),.018,'9FACB5_metal',8)
    elif name=='apolo-impacto':
        ground(m,'moon',.08,.03,1.56,.56,8)
        apollo_instruments(m,-.52,.056)
        stage(m,('capsule',.441,.278,.111,.97,-.35))
        for i in range(3):
            for j in range(7):
                a=.44+j*.25;b=a+.25;r=.112+i*.072
                m.rod((.46+cos(a)*r,.085,.071+sin(a)*r),(.46+cos(b)*r,.085,.071+sin(b)*r),.004,'AB7F61_metal',6)
    elif name=='apolo-hadley':
        ground(m,'moon',-.03,.089,1.68,.53,61)
        m.include(ridge('mars',1.52,.438,45),(-.03,.061,-.149),yaw=.02)
        stage(m,('rover:science',-.42,.085,.181,1.14,-.12),('beacon',.571,.063,.187,.77,.36))
    elif name=='apolo-caida':
        ground(m,'moon',0,.088,1.65,.50,27)
        stage(m,('lander:moon',-.53,.071,-.022,.97,-.31))
        m.box((.442,.568,.061),(.619,.049,.064),'A8B3B5_metal',.007)
        for x in (.24,.61):
            m.rod((x,.09,.062),(x,.54,.061),.008,'7E8793_metal',8)
        start=len(m.faces)
        m.rod((.319,.337,.093),(.319,.451,.093),.018,'8F6554_metal',10)
        m.box((.319,.463,.093),(.102,.053,.054),'8B96AB_metal',.009)
        m.rod((.567,.336,.093),(.567,.458,.093),.005,'C9C6B8_metal',6)
        for i in range(5):
            m.rod((.567,.36+i*.021,.093),(.606,.398+i*.02,.093),.005,'DFCFB1_metal',6)
        m.animate(start,'drop',(.45,.396,.093))
    elif name in ('insight-ondas','insight-taladro','insight-polvo'):
        ground(m,'mars',.02,.091,1.64,.51,{'insight-ondas':77,'insight-taladro':32,'insight-polvo':81}[name])
        if name=='insight-ondas':
            m.include(ridge('mars',1.40,.452,41),(.02,.047,-.109),yaw=.04)
            m.sphere((-.498,.151,.157),.101,'C3B7A9_metal',bands=11,segments=22,pole=0)
            points=[(-.45+i*.067,.259+.06*sin(i*.89),.081) for i in range(14)]
            for a,b in zip(points,points[1:]):m.rod(a,b,.011,'F0BE6F_lamp',7)
        elif name=='insight-taladro':
            m.include(ridge('mars',.87,.275,31),(.416,.065,-.031),yaw=.06)
            for x in (-.59,-.207):
                m.rod((x,.077,.019),(x,.545,.019),.023,'8B9EA8_metal',9)
            m.box((-.40,.541,.021),(.443,.095,.178),'A9997B_metal',.023)
            bit=len(m.faces)
            m.rod((-.39,.167,.071),(-.39,.504,.071),.032,'C4A575_metal',12)
            m.animate(bit,'drop',(-.39,.31,.071))
            m.box((.32,.117,.208),(.28,.078,.119),'767688_metal',.017)
        else:
            for x in (-.49,.47):
                m.rod((x,.086,.046),(x,.36,.046),.027,'9F9387_metal',8)
                solar(m,x,.428,.013,.58,.221,5)
                start=len(m.faces)
                for i in range(6):
                    m.box((x-.235+i*.09,.388+(i%3)*.044,.039),(.051,.023,.009),'BE8A5E_metal',.005)
                m.animate(start,'drift',(x,.420,.044))
            orbs(m,0,.25,.78)
    elif name=='insight-capas':
        ground(m,'mars',-.44,.084,.81,.51,67)
        m.box((-.48,.14,.123),(.35,.129,.208),'6A7E8B_metal',.020)
        m.box((-.48,.25,.043),(.456,.268,.047),'B9A28C_metal',.013)
        m.box((-.48,.25,.068),(.389,.211,.018),'465B74_metal',.008)
        for row in range(3):
            points=[(-.637+i*.025,.190+row*.057+.023*sin(i*.47+row),.081) for i in range(13)]
            for a,b in zip(points,points[1:]):m.rod(a,b,.004,'EAD397_lamp',6)
        x,y,z=.462,.387,-.039
        r=.353
        bands=((1.,'A35F4E_planet'),(.91,'C68654_planet'),(.72,'E0A260_planet'),
               (.48,'AF6B4F_planet'),(.27,'F7BD73_planet'),(0.,'F7BD73_planet'))
        m.rod((x,.047,z),(x,y-.285,z),.021,'876D6C_metal',8)
        for (outer,material),(inner,_) in zip(bands,bands[1:]):
            for i in range(32):
                a,b=i*2*pi/32,(i+1)*2*pi/32
                m.face(material,((x+r*outer*cos(a),y+r*outer*sin(a),z),
                                 (x+r*outer*cos(b),y+r*outer*sin(b),z),
                                 (x+r*inner*cos(b),y+r*inner*sin(b),z),
                                 (x+r*inner*cos(a),y+r*inner*sin(a),z)))
        m.box((x,.025,z),(.43,.049,.27),'61576B_metal',.014)
    elif name=='marte-travesia':
        ground(m,'mars',.025,.096,1.65,.51,26)
        m.include(ridge('mars',1.37,.51,99),(.07,.08,-.16),yaw=-.03)
        stage(m,('rover:science',.213,.091,.111,1.04,-.37),('crate',-.597,.085,.218,.79,.13))
        for i in range(6):
            for z in (.183,.277):
                m.box((-.64+i*.094,.079,z),(.053,.018,.049),'815450_metal',.008)
    elif name=='orbita-sistemas':
        stage(m,('station',-.46,.061,-.013,.92,.20))
        for i,h in enumerate((.48,.42,.573)):
            x=.19+i*.258
            m.tube((((x,.087,.04),.088),((x,h,.04),.088)),
                   ('87A6B1_metal','829EA0_metal','A39172_metal')[i],18)
            m.sphere((x,h,.04),.086,'B4B8AB_metal',bands=12,segments=20,pole=0)
            m.sphere((x,h-.10,.118),.017,'68BDAD_lamp',bands=8,segments=12)
        m.rod((.19,.22,.137),(.7,.22,.137),.021,'A8B9B1_metal',9)
        m.box((.448,.026,0),(.82,.051,.285),'646F81_metal',.015)
    else:
        return False
    return True


def scene(name):
    m = Mesh()
    if archive_scene(m,name):
        return m
    if name in ('apolo-eagle','apolo-intrepid','apolo-falcon'):
        seed={'apolo-eagle':26,'apolo-intrepid':19,'apolo-falcon':39}[name]
        ground(m,'moon',-.11,.070,1.62,.54,seed)
        stage(m,('lander:moon',-.30,.079,.031,1.22,-.28))
        if name=='apolo-intrepid':
            stage(m,('dish',.48,.076,-.031,.83,.31),('beacon',.69,.072,.162,.51,.12))
        elif name=='apolo-falcon':
            stage(m,('rover:science',.46,.083,.024,1.02,.29))
        else:
            stage(m,('beacon',.48,.075,.135,.77,.24),('crate',.64,.082,-.087,.92,.21))
    elif name in ('kepler-telescopio','kepler-transito'):
        if name=='kepler-telescopio':
            mount(m,'satellite:telescope',-.36,.29,.02,1.33,-.42)
            planet(m,'sun',.44,.43,-.02,.19)
            m.sphere((.41,.43,.20),.064,'22456B_planet',bands=12,segments=24)
        else:
            planet(m,'sun',-.42,.425,.015,.26)
            m.sphere((-.34,.44,.26),.072,'485A79_planet',bands=12,segments=24)
            mount(m,'satellite:imager',.42,.29,.02,1.20,.34)
            m.rod((-.72,.147,-.12),(.72,.147,-.12),.009,'D8B477_metal',6)
    elif name=='pioneer-jupiter':
        def bands(lat,lon,n):
            stripe=.5+.39*sin(lat*31+sin(lon*2)*.40)+.08*sin(lat*77)
            eye=exp(-((lat+.27)/.11)**2-((lon-.28)/.28)**2)
            base=blend(rgb('A35D4B'),rgb('EDD3A4'),stripe)
            return 0.,blend(base,rgb('A64335'),eye*.91)
        start=len(m.faces)
        m.sphere((-.38,.43,0),.30,'FFFFFF_planet',bands,32,64)
        m.animate(start,'spin',(-.38,.43,0))
        pioneer(m,.43,.039,.022,1.04,-.17)
    elif name=='insight-marte':
        ground(m,'mars',-.44,.09,.83,.54,19)
        ground(m,'mars',.46,.09,.81,.54,20)
        craft=Mesh()
        craft.box((-.18,.27,.055),(.29,.17,.233),'D7AE6F_metal',.027)
        for x in (-.29,-.07):
            craft.rod((x,.24,.056),(x-.07,.089,.225),.022,'465363_metal')
            craft.rod((x,.24,.04),(x+.02,.089,-.103),.02,'465363_metal')
        for x in (-.54,.18):
            craft.rod((-.18,.295,.052),(x,.265,.055),.013,'A4B6C8_metal',8)
            solar(craft,x,.264,.075,.31,.19,5)
        craft.rod((-.11,.315,.119),(.068,.518,.145),.021,'9A9A88_metal')
        craft.rod((.068,.518,.145),(.19,.23,.221),.019,'D3B279_metal')
        craft.sphere((-.11,.337,.119),.015,'66BCC9_lamp',bands=8,segments=12)
        place_cutout(m,craft,-.35,.024,0,.93)
        m.sphere((.46,.153,.216),.112,'C4B8AA_metal',bands=11,segments=22,pole=0)
        stage(m,('beacon',.64,.089,-.055,.70,.3))
        for i in range(4):
            m.box((.39+i*.107,.092,.08),(.055,.045+i*.026,.077),'B47158_metal',.007)
    elif name=='rovers-gemelos':
        ground(m,'mars',-.03,.077,1.64,.51,37)
        stage(m,('rover:science',-.43,.079,.092,.91,-.38),('rover:science',.44,.079,-.023,1.04,.23))
        m.include(ridge('mars',.84,.21,13),(.10,.047,-.13),yaw=.08)
        orbs(m,.22,.29,.56)
    elif name == 'tierra':
        planet(m,'earth',-.420,.409,-.012,.276)
        mount(m,'satellite:imager',.432,.279,.010,1.13,-.27)
        stage(m,('dish',.16,0,.275,.62,.36),('crate',.68,0,.199,.47,-.15))
    elif name == 'tierra-nocturna':
        planet(m,'night',.365,.410,-.045,.293)
        mount(m,'satellite:weather',-.433,.316,.050,1.12,.33)
        stage(m,('dish',-.18,0,.267,.63,-.10),('crate',.692,0,.22,.44,.20))
    elif name == 'tierra-incendio':
        planet(m,'earth',-.543,.312,-.107,.198)
        ground(m,'mars',.313,.154,.94,.39,27)
        m.include(ridge('mars',.65,.19,14),(.245,.042,.025))
        stage(m,('rover:cargo',.322,.077,.168,.94,-.32),('beacon',.653,.066,.189,.71,.20),
              ('crate',-.292,0,.230,.70,.22))
        orbs(m,.002,.279,.65)
    elif name == 'saturno':
        saturn_scene(m,-.395,.432,-.055,.251,.28,-.18,(.183,.202,.18,.054))
        mount(m,'satellite:probe',.465,.271,.083,1.11,-.33)
        stage(m,('crate',.360,0,.294,.49,.15))
    elif name == 'saturno-travesia':
        saturn_scene(m,.406,.458,-.018,.244,.49,.31,(-.108,.262,.176,.064))
        mount(m,'satellite:probe',-.527,.260,.075,.96,.24)
        stage(m,('satellite:cubesat',-.20,.094,.312,.55,-.35))
    elif name == 'saturno-lunas':
        saturn_scene(m,-.427,.375,-.085,.196,.16,-.25,(-.012,.195,.211,.067))
        planet(m,'moon',.56,.434,-.047,.127)
        mount(m,'satellite:radar',.224,.103,.222,.90,.33)
    elif name in ('marte','marte-cantera','marte-tormenta','marte-cumbre'):
        quarry,storm,summit = name=='marte-cantera',name=='marte-tormenta',name=='marte-cumbre'
        if quarry:
            ground(m,'mars',-.49,.184,.65,.37,10)
            ground(m,'mars',.42,.144,.82,.419,25)
            m.include(ridge('mars',.83,.44,21),(-.443,.064,-.12),yaw=-.20)
            m.include(ridge('mars',.49,.33,24),(.487,.066,-.131),yaw=.24)
            stage(m,('rover:drill',.326,.082,.14,1.16,-.23),('crate',-.437,.085,.227,.82,-.36))
            orbs(m,-.558,.242)
        elif storm:
            ground(m,'mars',-.432,.172,.82,.43,41)
            ground(m,'mars',.502,.133,.55,.32,42)
            stage(m,('rover:science',-.508,.092,.175,.94,.45),('habitat',.497,.082,.108,1.12,.29))
            m.include(ridge('mars',.80,.31,63),(-.40,.067,-.123),yaw=-.14)
            orbs(m,.006,.262,.82)
        elif summit:
            ground(m,'mars',-.464,.120,.72,.43,33)
            ground(m,'mars',.485,.181,.63,.36,14)
            m.include(ridge('mars',.67,.58,18),(-.496,.065,-.10),yaw=.13)
            stage(m,('rover:science',-.445,.085,.160,1.11,-.38),('dish',.514,.086,.158,.85,-.25))
            orbs(m,-.003,.282,.72)
        else:
            ground(m,'mars',.36,.152,.87,.411,12)
            planet(m,'mars',-.482,.365,-.088,.236)
            m.include(ridge('mars',.67,.25,18),(.306,.050,-.104),yaw=.26)
            stage(m,('rover:science',.317,.080,.154,1.17,-.41),('beacon',.727,.085,.159,.57,.26))
            orbs(m,-.537,.228,.9)
    elif name in ('europa','europa-crevasse','europa-descenso'):
        slope = name=='europa-crevasse'
        if name=='europa':
            planet(m,'europa',-.463,.379,-.040,.248)
            ground(m,'europa',.413,.147,.79,.45,12)
            stage(m,('lander:ice',.439,.069,.132,1.05,-.26),('beacon',.032,0,.325,.54,.34))
            m.include(ridge('europa',.41,.14,33),(.523,.045,-.102),yaw=.14)
        else:
            ground(m,'europa',-.479,.147,.70,.416,61 if slope else 38)
            ground(m,'europa',.488,.125,.72,.433,58 if slope else 28)
            m.include(ridge('europa',.66,.35,14 if slope else 19),(-.473,.06,-.108),yaw=-.39)
            if slope:
                mount(m,'satellite:probe',.476,.270,-.027,1,.26)
                stage(m,('rover:drill',-.474,.074,.173,.96,-.38),('beacon',.481,.051,.277,.71,.36))
            else:
                stage(m,('lander:ice',.486,.053,.140,1.16,.16),
                      ('habitat',-.537,.070,.123,.85,-.32),('crate',.064,0,.254,.64,-.2))
    elif name in ('luna','luna-puesto','luna-capsula'):
        if name=='luna':
            planet(m,'moon',-.484,.374,-.091,.251)
            ground(m,'moon',.405,.166,.89,.449,68)
            stage(m,('lander',.338,.090,.135,1.13,-.43),('beacon',.696,.063,.226,.71,.22))
            orbs(m,-.414,.266,.97)
        elif name=='luna-puesto':
            ground(m,'moon',-.449,.135,.77,.427,53)
            ground(m,'moon',.449,.135,.77,.437,66)
            stage(m,('lander',-.443,.080,.130,.94,.29),('habitat',.357,.084,.134,1.09,-.33),
                  ('dish',.738,.067,.109,.70,.34),('crate',-.009,0,.251,.78,-.28))
        else:
            ground(m,'moon',-.402,.146,.77,.43,32)
            stage(m,('capsule',-.484,.084,.132,1.42,.29),('beacon',-.119,.078,.186,.72,-.20))
            mount(m,'station',.532,.252,-.008,1.02,-.22)
    elif name in ('sol','sol-corona','sol-fulguracion'):
        if name=='sol-fulguracion':
            planet(m,'sun',.397,.389,-.053,.245)
            solar_arc(m,.410,.407,-.021,.266,-1)
            mount(m,'satellite:radar',-.428,.244,.078,1.08,.37)
            stage(m,('satellite:cubesat',-.273,.113,.302,.64,-.24))
        else:
            corona = name=='sol-corona'
            planet(m,'sun',-.451,.399,-.064,.256 if corona else .285)
            if corona:
                solar_arc(m,-.438,.438,-.027,.285,1)
            mount(m,'satellite:shield' if corona else 'satellite:telescope',
                  .438,.271,.041,1.17,-.30 if corona else .31)
            stage(m,('dish' if corona else 'beacon',.229,0,.270,.69,.28))
    elif name == 'satelite':
        mount(m,'satellite:comms',-.415,.290,.015,1.22,-.28)
        stage(m,('dish',.443,0,.090,.91,-.33),('crate',.742,0,.177,.69,.20))
    elif name == 'observador':
        mount(m,'satellite:imager',.404,.304,.031,1.37,-.43)
        stage(m,('gantry',-.454,0,-.045,.98,.15),('crate',-.503,0,.215,.74,-.21))
    elif name == 'cohete':
        stage(m,('rocket',-.366,.017,-.015,.99,-.11),('tower',.405,0,-.087,1.11,-.30),
              ('crate',.650,0,.236,.9,.37),('beacon',-.716,0,.187,.63,.23))
        m.box((-.366,.018,-.015),(.46,.034,.310),'4E596C_metal')
    elif name == 'orbita-atraque':
        mount(m,'station:lab',-.380,.266,-.023,1.17,-.23)
        stage(m,('capsule',.469,0,.133,1.19,-.47),('crate',.713,0,.221,.68,-.2))
    elif name == 'estacion':
        mount(m,'station',.269,.263,-.020,1.48,.19)
        stage(m,('habitat',-.533,0,.104,1.20,-.29),('crate',-.558,0,.314,.78,-.17))
    elif name == 'antena':
        stage(m,('dish',-.438,0,.085,1.32,-.18),('habitat',.432,0,.126,1.08,-.31),
              ('beacon',.726,0,-.056,.87,.12),('crate',.067,0,.238,.70,.2))
    elif name == 'vehiculo':
        ground(m,'mars',-.339,.161,.96,.41,81)
        stage(m,('rover:cargo',-.388,.093,.149,1.45,.19),('dish',.560,0,.014,.88,.34),
              ('crate',.355,0,.249,.99,-.3))
    elif name == 'sonda':
        mount(m,'satellite:probe',.403,.265,.026,1.36,-.32)
        mount(m,'satellite:cubesat',-.496,.168,.140,1.12,.29)
        stage(m,('crate',.446,0,.268,.64,.3))
    elif name == 'telescopio':
        mount(m,'satellite:telescope',-.411,.287,.043,1.30,.32)
        stage(m,('dish',.489,0,.096,1.03,-.36),('beacon',.778,0,.198,.70,.11))
    elif name == 'orbita-ensamble':
        stage(m,('gantry',.421,0,-.116,1.24,-.21),('satellite:radar',.497,.078,.160,.92,.22),
              ('rocket:light',-.445,.026,-.039,.88,.11),('crate',-.567,0,.243,.98,-.28))
    else:
        raise ValueError(f'Escena desconocida: {name}')
    return m


SCENES = (
    'satelite','observador','cohete','estacion','antena','vehiculo','sonda','telescopio',
    'marte','europa','luna','saturno','tierra','sol','marte-cantera','europa-crevasse',
    'luna-puesto','saturno-travesia','tierra-nocturna','sol-corona','orbita-ensamble',
    'tierra-incendio','europa-descenso','orbita-atraque','marte-tormenta','luna-capsula',
    'saturno-lunas','sol-fulguracion','marte-cumbre',
    'apolo-eagle','apolo-intrepid','apolo-falcon','kepler-telescopio','kepler-transito',
    'pioneer-jupiter','insight-marte','rovers-gemelos',
    'pioneer-salida','pioneer-asteroides','pioneer-placa','pioneer-senal','kepler-equilibrio','kepler-campo',
    'apolo-muestras','apolo-red','apolo-surveyor','apolo-impacto','apolo-hadley','apolo-caida',
    'insight-ondas','insight-taladro','insight-polvo','marte-travesia','orbita-sistemas','apolo-easep','insight-capas',
)

FLIERS = {
    'repetidor-en-vuelo':'satellite:comms','sonda-en-vuelo':'satellite:probe',
    'cubesat-en-vuelo':'satellite:cubesat','capsula-en-vuelo':'capsule',
    'explorador-en-vuelo':'satellite:imager','escudo-en-vuelo':'satellite:shield',
    'radar-en-vuelo':'satellite:radar','meteorologico-en-vuelo':'satellite:weather',
    'laboratorio-en-vuelo':'station:lab','telescopio-en-vuelo':'satellite:telescope',
}
