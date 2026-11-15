"""Spacecraft, field equipment and infrastructure built from solid components."""
from math import cos, pi, sin

from mesh_builder import Mesh

silver = 'D2C9B4_metal'
steel = '455A66_metal'
warm = 'D4A078_metal'
copper = 'B25E45_metal'
white = 'EBE0C6'
blue = '375975_metal'
black = '303747_rubber'
glass = '7CC3CA_solar'
signal = '8BE8D8_lamp'
orange = 'E68A58'
grid = ('234760_solar','2d6e84_solar','335271_solar','477e9b_solar')


def solar(mesh, x, y, z, w, h, columns=5, rows=3):
    mesh.box((x,y,z),(w,h,.022),silver,.007)
    mesh.box((x,y,z+.014),(w-.018,h-.018,.012),blue,.004)
    dx,dy=(w-.034)/columns,(h-.034)/rows
    for i in range(columns):
        px=x+(i-(columns-1)/2)*dx
        for j in range(rows):
            py=y+(j-(rows-1)/2)*dy
            a,b = (dx-.004)/2,(dy-.004)/2
            face=((px-a,py-b,z+.021),(px+a,py-b,z+.021),
                  (px+a,py+b,z+.021),(px-a,py+b,z+.021))
            mesh.face(grid[(i+2*j)%4],face)
            for k in (-1,1):
                q=py+k*b*.40
                mesh.face('829AA9_solar',((px-a,q,z+.0215),(px+a,q,z+.0215),
                                          (px+a,q+.0014,z+.0215),(px-a,q+.0014,z+.0215)))
        mesh.box((px,y-h*.31,z-.016),(dx-.004,.013,.007),warm,.002)
    for edge in (-1,1):
        mesh.box((x,y+edge*(h/2-.006),z+.017),(w-.012,.010,.008),'BBA688_metal',.003)
        mesh.box((x+edge*(w/2-.009),y,z+.017),(.011,h-.021,.008),silver,.002)
    for xx in (-.33,.33):
        mesh.box((x+xx*w,y,z-.014),(.009,h*.84,.010),'667785_metal',.002)


def reflector(mesh, x, y, z, radius):
    def r(offset):
        return (x,y,z+offset)
    mesh.tube(((r(-.070),radius*.16),(r(-.064),radius*.44),(r(-.028),radius*.82),
               (r(.003),radius)),steel,28,False)
    # The second shell is viewed from inside the dish.
    mesh.tube(((r(.009),radius),(r(-.018),radius*.82),(r(-.054),radius*.44),
               (r(-.060),radius*.07)),white,28,False)
    mesh.tube(((r(.004),radius),(r(.014),radius*.965)),silver,32,False)
    for i in range(12):
        a=i*2*pi/12
        for a0,a1,d0,d1 in ((.45,.80,-.050,-.018),(.80,.948,-.018,.003)):
            p=(x+a0*radius*cos(a),y+a0*radius*sin(a),z+d0)
            q=(x+a1*radius*cos(a),y+a1*radius*sin(a),z+d1)
            mesh.rod(p,q,.0017,'B69D85_metal',5)
    for a in (.55,2.65,4.75):
        mesh.rod((x+radius*.87*cos(a),y+radius*.87*sin(a),z-.004),(x,y,z+radius*.37),.008,steel,8)
    mesh.rod(r(-.047),r(radius*.43),radius*.073,warm,12)
    mesh.tube(((r(radius*.32),radius*.091),(r(radius*.39),radius*.112),
               (r(radius*.42),radius*.083)),'D5AE70_metal',16,False)
    mesh.sphere(r(radius*.43),radius*.083,signal,bands=5,segments=10)


def camera(mesh, x, y, z, radius):
    def r(offset):
        return (x,y,z+offset)
    mesh.tube(((r(-.035),radius*.86),(r(.026),radius),(r(.056),radius),
               (r(.076),radius*.78)),steel,24,False)
    mesh.tube(((r(.076),radius*.78),(r(.065),radius*.72)),warm,28,False)
    mesh.tube(((r(.065),radius*.72),(r(.034),radius*.61)),blue,28,False)
    mesh.sphere(r(-.021),radius*.63,glass,bands=10,segments=24)


def radiator(mesh, x, y, z, w, h):
    mesh.box((x,y,z),(w,h,.018),blue,.006)
    for i in range(8):
        mesh.box((x+((i+.5)/8-.5)*w*.91,y,z+.012),(.007,h*.86,.014),silver,.002)


def antenna(mesh, a, b, lit=False):
    mesh.rod(a,b,.008,silver,8)
    if lit:
        mesh.sphere(b,.015,signal,bands=4,segments=8)


def satellite(kind='comms'):
    m = Mesh()
    if kind in ('imager','telescope','weather'):
        r = .118 if kind=='weather' else .100
        m.tube((((0,.13,-.119),r*.72),((0,.13,-.087),r),((0,.13,.132),r),
                ((0,.13,.16),r*.88)),white,24)
        for z in (-.081,.112):
            m.rod((0,.13,z),(0,.13,z+.018),r+.004,silver,22)
            m.rod((0,.13,z+.019),(0,.13,z+.025),r+.005,warm,22)
        for angle in (.53,2.61,4.70):
            x,y=r*cos(angle),.13+r*sin(angle)
            m.rod((x,y,-.066),(x,y,.098),.006,blue,7)
        camera(m,0,.13,.156,r*.88)
        radiator(m,-.073,.17,-.098,.105,.055)
        m.box((.124,.18,-.047),(.088,.101,.112),warm)
        antenna(m,(.122,.20,-.043),(.215,.33,-.093),True)
        if kind == 'telescope':
            m.rod((-.23,.123,0),(.23,.123,0),.025,steel)
            solar(m,-.265,.135,-.025,.269,.17,4)
            solar(m,.265,.135,-.025,.269,.17,4)
            m.box((.045,.246,-.105),(.109,.054,.061),steel)
        elif kind=='weather':
            solar(m,-.271,.14,.006,.27,.197,4)
            solar(m,.253,.17,.031,.23,.153,4)
            m.rod((-.106,.22,0),(-.135,.307,0),.022,copper)
            reflector(m,-.14,.304,.02,.074)
        else:
            solar(m,.28,.119,-.018,.29,.16)
            m.rod((.092,.12,-.018),(.26,.12,-.018),.019,warm)
            antenna(m,(-.06,.203,-.06),(-.183,.314,-.018))
        return m
    if kind == 'shield':
        m.tube((((0,.14,-.112),.094),((0,.14,.13),.103)),warm,10,smooth=False)
        solar(m,-.211,.121,.046,.236,.149,4)
        solar(m,.211,.121,.046,.236,.149,4)
        m.tube((((0,.148,.16),.169),((0,.148,.191),.175),((0,.148,.197),.157)),
               'F1DAB3_planet',32)
        m.tube((((0,.148,.198),.121),((0,.148,.204),.091)),copper,30,False)
        for a in (0,2*pi/3,4*pi/3):
            m.rod((.082*cos(a),.148+.082*sin(a),.205),(.14*cos(a),.148+.14*sin(a),.208),.008,steel)
        radiator(m,0,.151,-.128,.129,.148)
        return m
    if kind == 'radar':
        m.tube((((0,.147,-.096),.111),((0,.147,.09),.098)),warm,8,smooth=False)
        for x,y in ((-.238,.135),(.242,.187)):
            m.rod((0,.145,0),(x,y,0),.022,silver)
            solar(m,x,y,0,.28,.152)
        m.box((0,.096,.13),(.412,.069,.042),'C8A07C_metal')
        for i in range(6):
            m.box(((i-2.5)*.067,.099,.155),(.049,.039,.013),glass,.005)
        antenna(m,(-.065,.209,-.045),(-.194,.32,-.097),True)
        antenna(m,(.068,.219,-.021),(.212,.288,-.08))
        return m
    if kind == 'cubesat':
        m.box((0,.136,0),(.177,.235,.164),silver)
        solar(m,0,.136,.084,.169,.195,3,4)
        for x in (-.224,.184):
            m.rod((0,.136,0),(x,.136,.019),.014,steel)
            solar(m,x,.145,.019,.238,.141,4)
        for x in (-.069,.069):
            m.box((x,.136,.081),(.012,.238,.014),white,.003)
            antenna(m,(x,.255,-.062),(x*2.82,.329,-.012),True)
        camera(m,.013,.213,-.088,.041)
        return m
    if kind == 'probe':
        m.tube((((0,.125,-.121),.102),((0,.125,-.071),.125),
                ((0,.125,.112),.122)),warm,7,smooth=False)
        m.rod((0,.173,.026),(0,.278,.095),.028,steel)
        reflector(m,.016,.306,.108,.191)
        antenna(m,(.075,.164,.036),(.398,.115,.067),True)
        m.rod((-.096,.166,-.065),(-.297,.285,-.095),.017,silver)
        m.sphere((-.310,.29,-.098),.043,'708DB4_metal',bands=7,segments=14)
        camera(m,-.054,.073,.115,.044)
        return m
    m.box((0,.135,0),(.205,.217,.180),warm)
    m.rod((-.269,.139,-.032),(.266,.139,-.032),.021,steel)
    solar(m,-.266,.142,-.023,.306,.152)
    solar(m,.266,.142,-.023,.306,.152)
    radiator(m,0,.133,.099,.152,.145)
    for x in (-.096,.096):
        m.box((x,.132,.073),(.015,.217,.035),white,.003)
        antenna(m,(x,.216,-.061),(x*1.65,.325,-.1),True)
    reflector(m,0,.301,.066,.113)
    return m


def pedestal(base='284253_metal', height=.11):
    m = Mesh()
    m.box((0,.020,0),(.243,.04,.178),base)
    m.tube((((0,.035,0),.045),((0,height,0),.025)),steel,12)
    return m


def ground_dish(size=.22):
    m = Mesh()
    m.include(pedestal('546982_metal',.21))
    m.tube((((0,.031,0),.073),((0,.076,0),.066),((0,.190,0),.049)),silver,14)
    for x in (-.081,.081):
        m.rod((x,.026,.071),(0,.196,-.021),.011,warm)
    head=len(m.faces)
    m.box((0,.228,-.041),(.124,.086,.108),blue)
    m.rod((-.074,.230,-.042),(.074,.230,-.042),.026,warm,14)
    m.rod((0,.21,-.068),(0,.356,-.065),.034,steel)
    reflector(m,0,.392,0,size)
    m.animate(head,'scan',(0,.21,0))
    return m


def wheel(mesh, x, y, z, radius):
    def r(v):
        return (x,y,z+v)
    mesh.tube(((r(-.041),radius*.85),(r(-.033),radius),(r(.033),radius),
               (r(.041),radius*.85)),black,22)
    for zoff in (-.045,.045):
        mesh.rod(r(zoff-.004),r(zoff+.004),radius*.67,steel,18)
        mesh.rod(r(zoff-.005),r(zoff+.005),radius*.34,'809BB6_metal',14)
        for i in range(5):
            a=i*2*pi/5
            mesh.sphere((x+radius*.48*cos(a),y+radius*.48*sin(a),z+zoff),.005,
                        silver,bands=3,segments=6)
    for i in range(16):
        a=2*pi*i/16
        px,py=x+radius*cos(a),y+radius*sin(a)
        dz=.004 if i%2 else -.004
        mesh.rod((px,py,z-.032+dz),(px,py,z+.032+dz),.006,'414B60_rubber',6)


def rover(kind='science'):
    m = Mesh()
    cargo = kind=='cargo'
    drill = kind=='drill'
    shell = 'D6AD74_metal' if kind=='science' else 'CBD7D9_metal' if drill else orange
    for z in (-.126,.126):
        for x in ((-.157,.157) if cargo else (-.217,-.015,.201)):
            wheel(m,x,.065,z,.061)
            m.rod((x,.07,z),(x*.66,.18,z*.76),.013,steel)
    m.box((0,.189,0),(.472,.144,.235),shell,.035)
    m.box((0,.134,0),(.418,.037,.196),blue,.013)
    m.box((0,.265,0),(.277,.033,.172),warm if cargo else silver)
    radiator(m,-.063,.19,.126,.185,.081)
    m.box((-.214,.194,.115),(.039,.049,.035),signal,.009)
    m.box((.108,.183,.124),(.082,.051,.018),blue,.007)
    for x in (.082,.108,.134):
        m.box((x,.184,.135),(.012,.020,.004),white,.001)
    for z in (-.096,.096):
        m.rod((-.264,.209,z),(.249,.209,z),.009,steel,8)
    for x in (-.142,-.087,-.032,.023):
        m.box((x,.275,-.012),(.024,.009,.115),blue,.003)
    for y in (.164,.208):
        for z in (-.065,.065):
            m.box((.231,y,z),(.026,.019,.037),warm,.004)
    if cargo:
        m.box((.014,.337,-.016),(.225,.122,.163),warm)
        m.box((.014,.337,.074),(.101,.052,.025),white,.004)
    elif drill:
        a,b,c = (.161,.238,.096),(.285,.353,.148),(.370,.178,.170)
        m.rod(a,b,.022,steel);m.rod(b,c,.017,orange)
        m.sphere(b,.030,silver,bands=6,segments=12)
        m.tube(((c,.026),((.37,.139,.17),.022),((.37,.083,.17),.005)),steel,10)
        antenna(m,(-.14,.267,-.040),(-.196,.390,-.085))
    else:
        m.rod((.115,.257,-.043),(.115,.421,-.043),.025,steel)
        m.box((.116,.427,-.04),(.148,.058,.093),silver)
        camera(m,.080,.429,.01,.027)
        camera(m,.151,.429,.011,.025)
        antenna(m,(-.164,.223,-.044),(-.233,.395,-.104),True)
        m.rod((.224,.196,.061),(.327,.224,.077),.025,steel)
        m.rod((.327,.224,.077),(.361,.156,.115),.015,silver)
    return m


def lander(kind='moon'):
    m = Mesh()
    color = 'CDA166_metal' if kind=='moon' else '8FBBC9_metal'
    m.tube((((0,.127,0),.152),((0,.17,0),.177),
            ((0,.294,0),.169),((0,.328,0),.119)),color,8,smooth=False)
    for a in (pi/4,3*pi/4,5*pi/4,7*pi/4):
        x,z = cos(a),sin(a)
        foot = (x*.261,.019,z*.261)
        m.rod((x*.120,.234,z*.120),foot,.018,steel)
        m.rod((x*.101,.143,z*.101),(x*.204,.078,z*.204),.012,silver)
        m.rod((foot[0],.005,foot[2]),(foot[0],.031,foot[2]),.049,silver,14)
    m.tube((((0,.063,0),.065),((0,.151,0),.036)),steel,18)
    if kind=='moon':
        m.box((0,.397,.013),(.212,.155,.204),white,.039)
        for x in (-.049,.049):
            m.box((x,.410,.119),(.066,.048,.026),blue,.007)
            m.box((x,.42,.135),(.051,.015,.01),glass,.004)
        m.box((.0,.34,.122),(.088,.083,.036),steel,.008)
        m.box((.0,.34,.144),(.065,.057,.010),silver,.004)
        for i in range(4):
            a=(i+.5)*pi/2
            x,z=cos(a),sin(a)
            m.rod((x*.153,.178,z*.153),(x*.150,.273,z*.150),.010,warm,9)
        for y in (.094,.132,.17,.208,.246):
            m.rod((-.030,y,.186),(.030,y,.186),.006,silver)
        for x in (-.034,.034):
            m.rod((x,.075,.186),(x,.286,.186),.006,steel)
        antenna(m,(.027,.443,-.031),(.123,.563,-.028),True)
    else:
        radiator(m,0,.241,.161,.15,.103)
        m.rod((0,.315,-.022),(0,.377,-.022),.021,steel)
        reflector(m,0,.427,-.015,.114)
        for x in (-.215,.215):
            solar(m,x,.299,-.035,.16,.10,3,2)
    return m


def habitat():
    m = Mesh()
    m.box((0,.075,0),(.43,.15,.297),steel)
    m.sphere((0,.12,0),.158,'DADBD8_planet',bands=12,segments=24,pole=0)
    m.tube((((0,.141,0),.160),((0,.153,0),.157)),warm,32,False)
    for angle in (0,pi*.5,pi,pi*1.5):
        x,z=.161*cos(angle),.161*sin(angle)
        m.rod((x,.143,z),(x*.84,.207,z*.84),.005,steel,6)
    for x in (-.154,.154):
        m.box((x,.093,.17),(.081,.081,.085),white)
        m.sphere((x,.102,.211),.022,glass,bands=6,segments=12)
    m.box((0,.100,.188),(.105,.133,.092),silver)
    m.box((0,.104,.237),(.057,.096,.012),blue,.009)
    m.box((0,.151,.246),(.031,.011,.009),'61C5D0_lamp',.004)
    for x in (-.090,.090):
        m.rod((x,.034,.194),(x,.176,.166),.009,warm)
    m.box((0,.022,.243),(.093,.024,.055),silver,.006)
    for x in (-.22,.22):
        antenna(m,(x*.86,.086,-.04),(x,.302,-.08),True)
    return m


def station(kind='science'):
    m = Mesh()
    m.tube((((0,.134,-.16),.080),((0,.134,-.112),.115),((0,.134,.112),.116),
            ((0,.134,.174),.067)),white,20)
    m.tube((((-.23,.134,.058),.038),((-.186,.134,.058),.072),((.185,.134,.058),.072),
            ((.246,.134,.058),.04)),silver,16)
    m.rod((-.423,.207,-.025),(.423,.207,-.025),.02,steel)
    for x in (-.340,.340):
        solar(m,x,.147,-.017,.137,.279,3,5)
        m.rod((x,.272,-.019),(x,.332,-.019),.010,warm)
    camera(m,0,.134,.174,.061)
    for z in (-.09,.075):
        m.rod((0,.134,z),(0,.134,z+.012),.119,'987D62_metal',20)
    for x in (-.115,.115):
        m.box((x,.089,.096),(.06,.122,.077),blue)
    if kind=='lab':
        m.tube((((0,.183,-.061),.03),((0,.279,-.061),.052),((0,.347,-.061),.052)),warm,12)
        radiator(m,.104,.209,.125,.094,.096)
        antenna(m,(-.144,.216,-.012),(-.2,.355,-.043),True)
    return m


def rocket(kind='lift'):
    m = Mesh()
    main = 'F7EBD9' if kind=='lift' else 'C7D7DB_metal'
    m.tube((((0,.075,0),.071),((0,.127,0),.072),((0,.151,0),.096),
            ((0,.576,0),.096),((0,.631,0),.069),((0,.72,0),.055),
            ((0,.765,0),.023),((0,.795,0),.004)),main,40)
    for y, height, material in ((.151,.022,'244761_metal'),(.304,.036,'D75D3B_metal'),
                                (.564,.016,'244761_metal')):
        m.rod((0,y,0),(0,y+height,0),.100,material,40)
    for y in (.181,.552):
        m.rod((0,y,0),(0,y+.007,0),.097,'D7AE60_metal',40)
    for side in (-1,1):
        z=.095*side
        m.box((0,.421,z),(.072,.106,.016),'244761_metal',.008)
        m.box((0,.442,z+side*.009),(.037,.037,.012),'4BBBC5_solar',.008)
        for x in (-.023,0,.023):
            m.box((x,.394,z+side*.010),(.011,.012,.006),'F9BF58_lamp',.003)
    m.tube((((0,.018,0),.072),((0,.054,0),.053),((0,.087,0),.044)),steel,20,False)
    if kind=='lift':
        for x in (-.128,.128):
            m.tube((((x,.075,0),.047),((x,.284,0),.047),((x,.377,0),.033),((x,.451,0),.004)),
                   'D75D3B_metal',28)
            m.rod((x,.082,0),(x,.112,0),.050,steel,28)
            m.rod((x,.250,0),(x,.274,0),.049,'F7EBD9',28)
            m.tube((((x,.030,0),.038),((x,.071,0),.026),((x,.086,0),.022)),
                   '354A61_metal',24,False)
    else:
        for x in (-.119,.119):
            m.rod((x,.050,0),(x*.64,.178,0),.032,blue,12)
            m.rod((x,.043,0),(x,.060,0),.043,steel,12)
    return m


def launch_tower():
    m=Mesh()
    for x in (-.075,.075):
        for z in (-.065,.065):
            m.rod((x,0,z),(x,.59,z),.013,copper,8)
    for y in (.035,.179,.324,.485,.587):
        m.box((0,y,0),(.179,.026,.157),steel,.004)
        if y<.48:
            for z in (-.066,.066):
                m.rod((-.073,y,z),(.073,y+.144,z),.009,warm,8)
    m.box((.105,.424,0),(.258,.03,.079),copper)
    antenna(m,(0,.586,0),(.0,.685,0),True)
    m.sphere((0,.626,.061),.023,'F5B85E_lamp',bands=8,segments=12)
    return m


def gantry():
    m=Mesh()
    for x in (-.25,.25):
        m.box((x,.039,0),(.143,.078,.189),blue)
        for z in (-.066,.066):
            m.rod((x,.056,z),(x,.525,z),.017,silver)
        for y in (.10,.25,.40):
            m.rod((x,y,-.065),(x,y+.12,.065),.009,warm,8)
    for z in (-.069,.069):
        m.rod((-.271,.50,z),(.271,.50,z),.026,blue,8)
        for x in (-.216,-.108,0,.108,.216):
            m.box((x,.525,z),(.059,.010,.048),'DDB268_metal',.003)
    carriage=len(m.faces)
    m.box((.070,.47,0),(.126,.087,.132),copper)
    m.box((.070,.463,.068),(.083,.045,.011),'2F5263_metal',.007)
    for x, material in ((.045,'50B7AC_lamp'),(.070,'EBC15E_lamp'),(.095,'D76149_lamp')):
        m.sphere((x,.463,.077),.007,material,bands=5,segments=10)
    m.rod((.070,.439,0),(.070,.302,0),.013,steel)
    m.box((.070,.297,0),(.061,.030,.08),warm)
    m.animate(carriage,'slide',(.070,.47,0))
    return m


def beacon():
    m=Mesh()
    m.include(pedestal('755A64_metal',.18))
    m.box((0,.068,.017),(.148,.089,.123),steel)
    m.rod((0,.171,0),(0,.298,0),.012,silver)
    for a in (0,2*pi/3,4*pi/3):
        p=(cos(a)*.104,.277,sin(a)*.104)
        m.rod((0,.277,0),p,.009,silver)
        m.sphere(p,.026,'E5A85D_lamp',bands=5,segments=10)
    m.sphere((0,.314,0),.016,'63CBD1_lamp',bands=4,segments=9)
    return m


def crate():
    m=Mesh()
    m.box((0,.062,0),(.207,.124,.151),blue)
    m.box((0,.126,0),(.191,.014,.138),silver)
    for x in (-.065,.065):
        m.box((x,.066,.077),(.024,.099,.013),warm,.002)
    m.box((-.007,.082,.08),(.052,.028,.010),white,.003)
    m.box((.013,.108,.084),(.021,.008,.008),'79D0B8_lamp',.002)
    return m


def capsule():
    m=Mesh()
    m.tube((((0,.025,0),.145),((0,.073,0),.153),((0,.197,0),.112),
            ((0,.296,0),.056),((0,.342,0),.054)),white,26)
    m.rod((0,.028,0),(0,.065,0),.155,'534751_planet',28)
    m.tube((((0,.070,0),.153),((0,.081,0),.151)),warm,28,False)
    m.box((0,.197,.106),(.143,.058,.027),blue,.021)
    for x in (-.034,.034):
        m.sphere((x,.207,.128),.023,glass,bands=6,segments=14)
    antenna(m,(.047,.267,.031),(.143,.386,.040),True)
    return m
