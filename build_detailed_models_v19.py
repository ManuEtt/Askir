"""Rebuild enriched static GLB/OBJ assets from existing geometry. Requires trimesh and numpy."""
from pathlib import Path
import numpy as np, trimesh, json
ROOT=Path(__file__).resolve().parents[1]
SRC=ROOT/'ArtSource/Realistic3D_V1'
OUT=ROOT/'ArtSource/Enhanced3D_V19'
OUT.mkdir(parents=True,exist_ok=True)
COLORS={'wood':[109,69,39,255],'wood_light':[164,115,69,255],'iron':[115,124,131,255],'gold':[186,143,65,255],'stone':[116,119,115,255],'cloth':[65,76,101,255],'leather':[86,52,35,255],'leaf':[49,91,50,255],'rope':[175,147,99,255],'skin':[177,132,104,255]}
def color(m,c):m.visual.vertex_colors=np.tile(COLORS[c],(len(m.vertices),1));return m
def box(size,pos,c):m=trimesh.creation.box(extents=size);m.apply_translation(pos);return color(m,c)
def cylinder(r,h,pos,c,sections=12):m=trimesh.creation.cylinder(radius=r,height=h,sections=sections);m.apply_translation(pos);return color(m,c)
def rod(a,b,r,c,sections=10):
 a=np.array(a);b=np.array(b);d=b-a;length=np.linalg.norm(d)
 if length<1e-5:return box((.01,.01,.01),a,c)
 m=trimesh.creation.cylinder(radius=r,height=length,sections=sections)
 m.apply_transform(trimesh.geometry.align_vectors([0,0,1],d/length));m.apply_translation((a+b)/2);return color(m,c)
def base(name):
 scene=trimesh.load(SRC/(name+'.glb'),force='scene');return [m.copy() for m in scene.dump() if isinstance(m,trimesh.Trimesh)]
def dims(meshes):
 v=np.vstack([m.vertices for m in meshes]);return v.min(axis=0),v.max(axis=0)
def add_details(name,meshes):
 lo,hi=dims(meshes);center=(lo+hi)/2;size=hi-lo
 x,y,z=center; sx,sy,sz=size
 if 'TimberHut' in name:
  for sign in [-1,1]:
   for t in np.linspace(-.42,.42,5):
    meshes.append(rod((x+t*sx,y+sign*.48*sy,lo[2]+.1*sz),(x+t*sx,y+sign*.48*sy,lo[2]+.76*sz),.035*max(sx,sy),'wood'))
   for h in [.22,.42,.62]:meshes.append(rod((lo[0],y+sign*.5*sy,lo[2]+h*sz),(hi[0],y+sign*.5*sy,lo[2]+h*sz),.022*sx,'wood_light'))
  meshes.append(box((.19*sx,.04*sy,.28*sz),(x,y-.52*sy,lo[2]+.34*sz),'wood'))
  for t in [-.2,.2]:meshes.append(box((.12*sx,.025*sy,.11*sz),(x+t*sx,y-.53*sy,lo[2]+.52*sz),'gold'))
 elif 'Barrel' in name:
  for frac in [.16,.35,.66,.84]:meshes.append(cylinder(.52*max(sx,sy),.022*sz,(x,y,lo[2]+frac*sz),'iron',20))
  for angle in np.linspace(0,2*np.pi,12,endpoint=False):
   px=x+.48*sx*np.cos(angle);py=y+.48*sy*np.sin(angle)
   meshes.append(rod((px,py,lo[2]+.09*sz),(px,py,hi[2]-.09*sz),.012*sz,'wood_light'))
 elif 'WoodenCrate' in name:
  for yy in [lo[1]-.004,hi[1]+.004]:
   for i in np.linspace(-.35,.35,4):meshes.append(box((.08*sx,.02*sy,.88*sz),(x+i*sx,yy,z),'wood_light'))
   meshes.append(rod((lo[0],yy,lo[2]),(hi[0],yy,hi[2]),.025*sx,'iron'))
 elif 'StoneWall' in name:
  for i in range(4):
   zz=lo[2]+(i+.5)*sz/4
   meshes.append(box((sx*1.01,.025*max(sy,.1),.012*sz),(x,lo[1]-.012,zz),'stone'))
  for i in range(3):
   for j in range(4):
    xx=lo[0]+((j+.5*(i%2))%4)*sx/4
    meshes.append(box((.012*sx,.025*max(sy,.1),.24*sz),(xx,lo[1]-.015,lo[2]+(i+.5)*sz/4),'stone'))
 elif 'StoneWell' in name:
  for a in np.linspace(0,2*np.pi,12,endpoint=False):
   meshes.append(box((.06*sx,.04*sy,.04*sz),(x+.5*sx*np.cos(a),y+.5*sy*np.sin(a),lo[2]+.38*sz),'stone'))
  meshes.append(rod((x-.35*sx,y,hi[2]-.13*sz),(x+.35*sx,y,hi[2]-.13*sz),.025*sz,'wood'))
  meshes.append(rod((x,y,hi[2]-.13*sz),(x,y,lo[2]+.36*sz),.008*sz,'rope'))
 elif 'PineTree' in name:
  for i in range(15):
   a=2*np.pi*i/15;h=.28+.55*((i*7)%15)/15
   start=(x,y,lo[2]+h*sz);end=(x+np.cos(a)*(.25*(1-h))*sx,y+np.sin(a)*(.25*(1-h))*sy,lo[2]+(h-.12)*sz)
   meshes.append(rod(start,end,.012*sz,'leaf',6))
 elif 'Campfire' in name:
  for a in np.linspace(0,2*np.pi,9,endpoint=False):
   meshes.append(box((.12*sx,.1*sy,.08*sz),(x+.43*sx*np.cos(a),y+.43*sy*np.sin(a),lo[2]+.07*sz),'stone'))
  for a in np.linspace(0,np.pi,5):
   meshes.append(rod((x-.27*sx*np.cos(a),y-.27*sy*np.sin(a),lo[2]+.12*sz),(x+.27*sx*np.cos(a),y+.27*sy*np.sin(a),lo[2]+.12*sz),.035*sz,'wood'))
 elif 'IronSword' in name:
  for frac in [.1,.14,.18]:meshes.append(cylinder(.027*max(sx,sy,sz),.008*sz,(x,y,lo[2]+frac*sz),'leather'))
  meshes.append(box((.07*max(sx,sy),.018*max(sx,sy),.012*sz),(x,y,lo[2]+.24*sz),'gold'))
 elif 'RoundShield' in name:
  for a in np.linspace(0,2*np.pi,16,endpoint=False):
   meshes.append(cylinder(.018*max(sx,sy,sz),.018*max(sx,sy,sz),(x+.43*sx*np.cos(a),y+.43*sy*np.sin(a),z),'iron'))
 elif 'Character' in name:
  # Rigid surface accessories are visual previews; do not use as skinned character geometry.
  if 'Hero' in name:accent='gold'
  elif 'Guard' in name:accent='iron'
  elif 'Merchant' in name:accent='leather'
  else:accent='iron'
  for zz in [.42,.62,.82]:meshes.append(box((.43*sx,.12*sy,.025*sz),(x,y,lo[2]+zz*sz),accent))
  for xx in [-.26,.26]:
   meshes.append(box((.1*sx,.18*sy,.08*sz),(x+xx*sx,y,lo[2]+.73*sz),accent))
  meshes.append(box((.18*sx,.05*sy,.1*sz),(x,y-.43*sy,lo[2]+.5*sz),'leather'))
  if 'Guard' in name or 'Raider' in name:
   meshes.append(rod((x+.5*sx,y,lo[2]+.42*sz),(x+.5*sx,y,lo[2]+.97*sz),.015*sz,'iron'))
  if 'Merchant' in name:
   meshes.append(box((.33*sx,.23*sy,.2*sz),(x+.25*sx,y,lo[2]+.35*sz),'leather'))
 return meshes
report=[]
for f in sorted(SRC.glob('SM_*.glb')):
 name=f.stem;meshes=add_details(name,base(name))
 scene=trimesh.Scene()
 for i,m in enumerate(meshes):scene.add_geometry(m,geom_name=f'{name}_{i:03d}')
 (OUT/(name+'.glb')).write_bytes(scene.export(file_type='glb'))
 # combined OBJ is a fallback for UE import without GLTF plugin; vertex colors/materials not guaranteed
 joined=trimesh.util.concatenate(meshes);joined.export(OUT/(name+'.obj'))
 report.append({'name':name,'parts':len(meshes),'triangles':sum(len(m.faces) for m in meshes),'glb_bytes':(OUT/(name+'.glb')).stat().st_size})
(OUT/'asset_manifest.json').write_text(json.dumps(report,indent=2),encoding='utf8')
print('Enhanced models:',len(report),'total triangles',sum(r['triangles'] for r in report))
