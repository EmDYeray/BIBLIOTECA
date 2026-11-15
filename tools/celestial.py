"""Planets, atmospheric detail and compact pieces of lunar and Martian terrain."""
from math import cos, exp, hypot, pi, sin, sqrt
from random import Random

from mesh_builder import Mesh, add, blend, mul, rgb, unit


def limit(x):
    return max(0, min(1, x))


def waves(v):
    x,y,z = v
    return sin(12*x+7*y+13*z)*cos(17*y+13*z) + .42*sin(33*x-24*y+21*z)


# Stylized coastlines sampled on the sphere using a signed distance, in degrees.
CONTINENTS = (
    ((-168,65),(-139,70),(-117,73),(-94,70),(-83,60),(-64,58),(-53,49),
     (-69,45),(-80,29),(-94,18),(-83,10),(-92,15),(-110,24),(-117,32),(-126,49),(-151,60)),
    ((-80,9),(-64,12),(-50,6),(-35,-7),(-41,-21),(-52,-35),(-68,-55),
     (-74,-47),(-71,-25),(-81,-6)),
    ((-73,76),(-58,83),(-26,83),(-18,74),(-43,60),(-57,64)),
    ((-12,36),(-17,18),(-13,9),(8,4),(10,-6),(16,-17),(18,-33),
     (32,-32),(41,-14),(52,12),(43,12),(33,29),(24,33),(10,37)),
    ((-9,43),(1,54),(10,58),(13,70),(38,71),(59,72),(88,76),(117,73),
     (151,72),(169,65),(160,56),(145,51),(136,37),(122,29),(112,13),
     (101,3),(96,18),(80,7),(72,23),(58,26),(51,16),(43,15),(33,32),
     (22,40),(10,43),(3,36),(-9,37)),
    ((113,-23),(115,-33),(132,-32),(145,-43),(153,-30),(150,-19),(136,-13),(126,-15)),
    ((-10,51),(-8,59),(0,58),(2,51)),
    ((47,-13),(51,-16),(48,-25),(43,-25)),
    ((96,3),(112,6),(125,-3),(143,-5),(146,-9),(133,-9),(116,-7),(106,-6)),
)


def coast_distance(x, y, poly):
    inside = False
    distance = 360.
    for (ax,ay),(bx,by) in zip(poly, poly[1:]+poly[:1]):
        dx,dy = bx-ax, by-ay
        t = limit(((x-ax)*dx+(y-ay)*dy)/(dx*dx+dy*dy))
        distance = min(distance, (x-ax-dx*t)**2 + (y-ay-dy*t)**2)
        if (ay>y)!=(by>y) and x < ax + (y-ay)*dx/dy:
            inside = not inside
    return sqrt(distance) * (1 if inside else -1)


def earth(lat, lon, v, night=False):
    x,y,z = v
    latitude, longitude = lat*180/pi, lon*180/pi-28
    longitude = (longitude+180)%360-180
    land = max(coast_distance(longitude, latitude, shape) for shape in CONTINENTS)
    noise = waves(v)
    forest = rgb('347958' if not night else '253E56')
    sand = rgb('CFAB6E' if not night else '5D625E')
    desert = exp(-((latitude-24)/10)**2)*limit(.78+.21*noise)
    ground = blend(forest, sand, limit(desert+.22*noise+.12*abs(y)))
    ocean = blend(rgb('134987' if not night else '0D2247'),
                  rgb('34ADC1' if not night else '243E67'),limit(.68-.19*abs(land/7)+.14*noise))
    color = blend(ocean,ground,limit(.5+(land+noise*.74)/4))
    color = blend(color,rgb('D4EFFF' if not night else '8BB4D1'),limit((abs(y)-.91)*18))
    cloud = sin(8*x+9*y+9*z+sin(y*11-z*6)*1.64)+.35*noise
    return 0,blend(color,rgb('F0FBF7' if not night else '9AB7CE'),
                    limit((cloud-.72)*1.78)*(.80 if not night else .41))


# Bright crater rims and dark basins are etched as bands instead of smooth plastic.
LUNAR_BASINS = ((.37,-.44,.101),(.17,.29,.084),(-.44,1.12,.073),
                (.61,1.61,.087),(-.61,-.022,.121),(-.32,-1.73,.151),
                (.24,-2.41,.089),(.67,2.65,.106),(-.73,2.45,.073))


def surface(kind, lat, lon, v):
    x,y,z = v
    noise = waves(v)
    if kind == 'mars':
        dust = .45+.25*sin(5*x-4*z+6*y)*cos(7*y-7*z)+.16*noise
        escarpment=exp(-26*(sin(2.9*x+4.2*y-1.7*z)+.18*sin(11*x))**2)
        polar=limit((abs(y)-.91)*14)
        ground=blend(rgb('743A36'),rgb('E9A36B'),limit(dust-.22*escarpment))
        return .011*noise-.004*escarpment,blend(ground,rgb('F1D7B1'),polar*.75)
    if kind == 'moon':
        basin,border=0.,0.
        for clat,clon,radius in LUNAR_BASINS:
            delta=(lon-clon+pi)%(2*pi)-pi
            d=hypot(lat-clat,delta*cos(clat))/radius
            basin=max(basin,exp(-d*d*1.42))
            border=max(border,exp(-17*(d-1)**2))
        a=limit(.59+.16*noise+.22*border-.42*basin)
        return .004*noise+.006*border-.009*basin,blend(rgb('525B73'),rgb('DDD0B2'),a)
    if kind == 'europa':
        fracture = limit((.25-abs(sin(13*x+12*y+8*z+1.8*sin(11*y-7*z))))*2.8)
        color = blend(rgb('77B9D0'),rgb('D7ECE5'),limit(.67+.23*noise))
        return .008*noise, blend(color,rgb('836584'),fracture*.82)
    if kind == 'saturn':
        a = limit(.60+.16*sin(y*24)+.08*sin(y*56+noise*.6)+.035*noise)
        return 0, blend(rgb('9D6957'),rgb('EED094'),a)
    a = limit(.55+.22*noise+.12*sin(46*x+54*y+37*z))
    return .006*noise, blend(rgb('E64A22'),rgb('FFCF63'),a)


def globe(kind, radius):
    mesh = Mesh()
    if kind in ('earth','night'):
        sampler = lambda lat,lon,v: earth(lat,lon,v,kind=='night')
        mesh.sphere((0,0,0),radius,'FFFFFF_planet',sampler,38,76)
        if kind == 'night':
            for lon,lat in ((-121,37),(-111,33),(-99,19),(-88,42),(-77,40),(-74,44),
                            (-73,-13),(-46,-22),(-58,-34),(-3,42),(1,50)):
                lon,lat = (lon+28)*pi/180,lat*pi/180
                v = (cos(lat)*sin(lon),sin(lat),cos(lat)*cos(lon))
                mesh.sphere(mul(v,radius*1.009),.004,'D8A95B_lamp',bands=3,segments=6)
    else:
        mesh.sphere((0,0,0),radius,'FFFFFF_lamp' if kind=='sun' else 'FFFFFF_planet',
                    lambda lat,lon,v: surface(kind,lat,lon,v),24,48)
    return mesh


def orbital_ring(mesh, radius, tilt, palette, spin=0):
    """Several solid annuli, with edges, seen at the same tilt as the planet."""
    n = 72
    c,s = cos(tilt),sin(tilt)
    cs,ss = cos(spin),sin(spin)
    def point(r, t, h):
        x,y,z = r*cos(t), h*c+r*sin(t)*s, r*sin(t)*c-h*s
        return x*cs-y*ss,x*ss+y*cs,z
    for near,far,material in palette:
        a,b = radius*near,radius*far
        for i in range(n):
            u,v = i*2*pi/n,(i+1)*2*pi/n
            an,af,bn,bf = point(a,u,.002),point(a,v,.002),point(b,u,.002),point(b,v,.002)
            cn,cf,dn,df = point(a,u,-.002),point(a,v,-.002),point(b,u,-.002),point(b,v,-.002)
            mesh.face(material,(an,af,bf,bn))
            mesh.face(material,(dn,df,cf,cn))
            mesh.face(material,(bn,bf,df,dn))
            mesh.face(material,(cn,cf,af,an))


def saturn(radius, tilt=.28, spin=-.16):
    mesh = globe('saturn',radius)
    orbital_ring(mesh,radius,tilt,((1.19,1.30,'AF8370_planet'),(1.33,1.49,'D1AB88_planet'),
                                  (1.52,1.71,'DEC295_planet'),(1.77,1.91,'AE8F79_planet')),
                 spin)
    return mesh


def terrain(kind, width=.6, depth=.37, seed=12):
    mesh = Mesh()
    rng = Random(seed)
    x_count,z_count = 18,12
    crater = kind == 'moon'
    ice = kind == 'europa'
    palette = ('757386','D1C9B8') if crater else ('4B6790','BAEBE4') if ice else ('8F493B','ECAC6B')
    low,high = map(rgb,palette)
    def top(i,j):
        x,z = (i/x_count-.5)*width,(j/z_count-.5)*depth
        noise = sin(26*x+16*z)*sin(19*z-2*x)+rng.uniform(-.2,.2)
        d = hypot((x-width*.17)/width,(z+.027)/depth)
        trench = abs(x*.33+z+.035*sin(22*x))
        height = .068+.027*noise
        if crater:
            height += -.045*exp(-65*d*d)+.033*exp(-240*(d-.26)**2)
        if ice:
            height += -.037*exp(-1600*trench**2)
        color = blend(low,high,limit(.53+.26*noise+.20*sin(17*x-6*z)))
        if ice:
            color = blend(color,rgb('46446E'),.76*exp(-1400*trench**2))
        return (x,height,z),color
    grid = [[top(i,j) for j in range(z_count+1)] for i in range(x_count+1)]
    for i in range(x_count):
        for j in range(z_count):
            p,c = zip(grid[i][j],grid[i][j+1],grid[i+1][j+1],grid[i+1][j])
            mesh.face('FFFFFF_planet',p,colors=c)
    def edge(points):
        for (a,_),(b,_) in zip(points,points[1:]):
            foot_a,foot_b = (a[0],0,a[2]),(b[0],0,b[2])
            upper_a=(a[0],a[1]*.64,a[2])
            upper_b=(b[0],b[1]*.64,b[2])
            mid_a=(a[0],a[1]*.38,a[2])
            mid_b=(b[0],b[1]*.38,b[2])
            mesh.face(palette[0]+'_planet',(a,b,upper_b,upper_a))
            mesh.face('D69C67' if not ice else '8BAEB8',(upper_a,upper_b,mid_b,mid_a))
            mesh.face('BB9779' if not ice else 'C4D6CF',(mid_a,mid_b,foot_b,foot_a))
    edge(grid[0][::-1])
    edge(grid[x_count])
    edge([row[0] for row in grid])
    edge([row[z_count] for row in grid[::-1]])
    mesh.face(palette[0]+'_planet',((-width/2,0,-depth/2),(width/2,0,-depth/2),
                                   (width/2,0,depth/2),(-width/2,0,depth/2)))
    return mesh


def rock(radius, seed):
    mesh = Mesh()
    rng = Random(seed)
    def uneven(lat, lon, v):
        return rng.uniform(-.15,.15),blend(rgb('69424D'),rgb('C98966'),limit(.5+.3*waves(v)))
    mesh.sphere((0,radius*.53,0),radius,'FFFFFF_planet',uneven,5,9)
    return mesh


def ridge(kind, width, height, seed):
    mesh = Mesh()
    rng = Random(seed)
    palette = ('87463C_planet','D4613C_planet','A84C37_planet','E69358_planet') if kind=='mars' else (
        '687AAA_planet','9ECEDE_planet','67658D_planet','D1EDEF_planet')
    columns = 10
    top = [height*(.65+rng.random()*.35) for _ in range(columns+1)]
    def point(i,j,front):
        u = i/columns
        x = (u-.5)*width
        z = (.045 if front else -.045)*(1-.14*j)+.024*sin(4*pi*u)
        return x,j*top[i]/4,z
    for i in range(columns):
        for j,material in enumerate(palette):
            a,b,c,d = point(i,j,True),point(i+1,j,True),point(i+1,j+1,True),point(i,j+1,True)
            e,f,g,h = point(i,j,False),point(i+1,j,False),point(i+1,j+1,False),point(i,j+1,False)
            mesh.face(material,(a,b,c,d))
            mesh.face(material,(f,e,h,g))
            if i==0:
                mesh.face(material,(e,a,d,h))
            if i==columns-1:
                mesh.face(material,(b,f,g,c))
        mesh.face(material,(c,g,h,d))
        mesh.face(palette[0],(point(i,0,False),point(i+1,0,False),
                              point(i+1,0,True),point(i,0,True)))
    return mesh
