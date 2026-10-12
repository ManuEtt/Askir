"""Original low-poly skinned character sources. Run: blender -b --python Tools/build_animated_characters_v78.py."""
from pathlib import Path
import json
import math
import bpy
import bmesh
from mathutils import Vector, Quaternion, Euler

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "ArtSource/Animated_V78"
OUT.mkdir(parents=True, exist_ok=True)
CLIPS = {"Idle": 60, "Walk": 30, "Run": 22, "Sprint": 18, "Jump": 30,
         "Dodge": 24, "LightAttack": 24, "HeavyAttack": 39, "Hit": 18,
         "Death": 45, "Block": 60, "Cast": 36}
BONES = [
    ("root", None, (0,0,0), (0,0,.1)),
    ("pelvis", "root", (0,0,.92), (0,0,1.07)),
    ("spine", "pelvis", (0,0,1.07), (0,0,1.28)),
    ("chest", "spine", (0,0,1.28), (0,0,1.50)),
    ("neck", "chest", (0,0,1.50), (0,0,1.61)),
    ("head", "neck", (0,0,1.61), (0,0,1.80)),
]
for side, sign in (("l", 1), ("r", -1)):
    BONES.extend([
        ("upperarm_"+side, "chest", (0,sign*.22,1.45), (0,sign*.39,1.20)),
        ("forearm_"+side, "upperarm_"+side, (0,sign*.39,1.20), (0,sign*.43,.98)),
        ("hand_"+side, "forearm_"+side, (0,sign*.43,.98), (.035,sign*.44,.90)),
        ("thigh_"+side, "pelvis", (0,sign*.11,.92), (0,sign*.12,.52)),
        ("shin_"+side, "thigh_"+side, (0,sign*.12,.52), (0,sign*.12,.10)),
        ("foot_"+side, "shin_"+side, (0,sign*.12,.10), (.17,sign*.12,.06)),
    ])


def reset():
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)
    for data in (bpy.data.actions, bpy.data.meshes, bpy.data.armatures, bpy.data.materials):
        for item in list(data):
            if item.users == 0:
                data.remove(item)
    scene = bpy.context.scene
    scene.unit_settings.system = "METRIC"
    scene.unit_settings.scale_length = 1.0
    scene.render.fps = 30
    bpy.context.preferences.filepaths.save_version = 0


def material(name, color, metallic=0.0):
    mat = bpy.data.materials.new(name)
    mat.diffuse_color = (*color, 1)
    mat.use_nodes = True
    shader = mat.node_tree.nodes.get("Principled BSDF")
    shader.inputs["Base Color"].default_value = (*color, 1)
    shader.inputs["Metallic"].default_value = metallic
    shader.inputs["Roughness"].default_value = .65
    return mat


class Body:
    def __init__(self, mats):
        self.vertices, self.faces, self.weights, self.material_ids = [], [], [], []
        self.mats = mats

    def vertex(self, position, weights):
        total = sum(weights.values())
        if total <= 0:
            raise RuntimeError("Unweighted source vertex")
        self.vertices.append(tuple(position))
        self.weights.append({key: value/total for key, value in weights.items() if value > 0})
        return len(self.vertices)-1

    def face(self, indices, mat):
        self.faces.append(indices)
        self.material_ids.append(mat)

    def rings(self, spec, mat, sides=12):
        previous = None
        for center, radius_x, radius_y, weights in spec:
            ring = [self.vertex(Vector(center)+Vector((radius_x*math.cos(j*2*math.pi/sides),
                                                      radius_y*math.sin(j*2*math.pi/sides),0)), weights)
                    for j in range(sides)]
            if previous:
                for j in range(sides):
                    self.face((previous[j],previous[(j+1)%sides],ring[(j+1)%sides],ring[j]),mat)
            else:
                self.face(tuple(reversed(ring)),mat)
            previous = ring
        self.face(tuple(previous),mat)

    def limb(self, points, radii, weights, mat, sides=10):
        previous = None
        for i, center in enumerate(points):
            center = Vector(center)
            tangent = Vector(points[min(i+1,len(points)-1)])-Vector(points[max(0,i-1)])
            tangent.normalize()
            u = Vector((1,0,0))
            v = tangent.cross(u).normalized()
            ring = [self.vertex(center + radii[i]*(u*math.cos(j*math.tau/sides)+v*math.sin(j*math.tau/sides)), weights[i])
                    for j in range(sides)]
            if previous:
                for j in range(sides):
                    self.face((previous[j],previous[(j+1)%sides],ring[(j+1)%sides],ring[j]),mat)
            else:
                self.face(tuple(reversed(ring)),mat)
            previous = ring
        self.face(tuple(previous),mat)

    def ellipsoid(self, center, scale, bone, mat, sides=12, rings=6):
        rows = []
        for i in range(rings+1):
            phi = math.pi*i/rings
            rows.append([self.vertex(Vector(center)+Vector((scale[0]*math.sin(phi)*math.cos(j*math.tau/sides),
                         scale[1]*math.sin(phi)*math.sin(j*math.tau/sides), scale[2]*math.cos(phi))),{bone:1})
                         for j in range(sides)])
        for i in range(rings):
            for j in range(sides):
                self.face((rows[i][j],rows[i+1][j],rows[i+1][(j+1)%sides],rows[i][(j+1)%sides]),mat)

    def box(self, center, scale, bone, mat):
        verts = [self.vertex(Vector(center)+Vector((x*scale[0],y*scale[1],z*scale[2])),{bone:1})
                 for x,y,z in ((-1,-1,-1),(1,-1,-1),(1,1,-1),(-1,1,-1),(-1,-1,1),(1,-1,1),(1,1,1),(-1,1,1))]
        for face in ((0,3,2,1),(4,5,6,7),(0,1,5,4),(1,2,6,5),(2,3,7,6),(3,0,4,7)):
            self.face(tuple(verts[i] for i in face),mat)

    def finish(self, role, rig):
        data = bpy.data.meshes.new("Askir_"+role+"_Skin")
        data.from_pydata(self.vertices, [], self.faces)
        data.update()
        editable = bmesh.new()
        editable.from_mesh(data)
        bmesh.ops.recalc_face_normals(editable, faces=list(editable.faces))
        editable.to_mesh(data)
        editable.free()
        mesh = bpy.data.objects.new("Askir_"+role+"_Body",data)
        bpy.context.collection.objects.link(mesh)
        for mat in self.mats:
            data.materials.append(mat)
        for polygon, index in zip(data.polygons,self.material_ids):
            polygon.material_index = index
        groups = {name:mesh.vertex_groups.new(name=name) for name,*_ in BONES}
        for index, weights in enumerate(self.weights):
            for bone, weight in weights.items():
                groups[bone].add([index],weight,"REPLACE")
        mesh.parent = rig
        modifier = mesh.modifiers.new("AskirDeformation","ARMATURE")
        modifier.object = rig
        return mesh


def rigged_body(role):
    tones = {"Hero":((.09,.19,.22),(.23,.075,.045)),
             "Guard":((.12,.16,.34),(.33,.38,.43)),
             "Raider":((.24,.055,.045),(.16,.14,.13))}
    cloth, metal = tones[role]
    mats = [material(role+"_Cloth",cloth), material(role+"_Skin",(.52,.31,.19)),
            material(role+"_Leather",(.095,.055,.025)), material(role+"_Metal",metal,.65),
            material(role+"_Eyes",(.008,.012,.01)), material(role+"_Accent",(.62,.43,.11),.25)]
    armature = bpy.data.armatures.new("AskirHumanoid18")
    rig = bpy.data.objects.new("Askir_"+role+"_Rig",armature)
    bpy.context.collection.objects.link(rig)
    bpy.context.view_layer.objects.active = rig
    rig.select_set(True)
    bpy.ops.object.mode_set(mode="EDIT")
    for name,parent,head,tail in BONES:
        bone = armature.edit_bones.new(name)
        bone.head, bone.tail = head, tail
        if parent:
            bone.parent = armature.edit_bones[parent]
    bpy.ops.object.mode_set(mode="OBJECT")
    body = Body(mats)
    body.rings([((0,0,.84),.14,.20,{"pelvis":1}),((0,0,.94),.13,.17,{"pelvis":.7,"spine":.3}),
                ((0,0,1.10),.12,.16,{"spine":1}),((0,0,1.30),.14,.21,{"spine":.2,"chest":.8}),
                ((0,0,1.46),.115,.235,{"chest":1}),((0,0,1.52),.09,.12,{"chest":.6,"neck":.4})],0)
    body.rings([((0,0,.93),.137,.184,{"pelvis":1}),((0,0,.975),.137,.184,{"pelvis":1})],2)
    body.box((.143,0,.951),(.015,.045,.025),"pelvis",5)
    body.ellipsoid((0,0,1.555),(.065,.07,.065),"neck",1)
    body.ellipsoid((0,0,1.687),(.105,.102,.113),"head",1)
    body.ellipsoid((.101,0,1.67),(.031,.022,.035),"head",1)
    for sign in (-1,1):
        body.ellipsoid((.094,sign*.044,1.707),(.012,.015,.010),"head",4,8,4)
    # A shoulder-to-forearm tube shares its elbow ring; weights interpolate on the actual surface.
    for side, sign in (("l",1),("r",-1)):
        upper, fore, hand = "upperarm_"+side,"forearm_"+side,"hand_"+side
        body.limb([(0,sign*.225,1.45),(0,sign*.30,1.345),(0,sign*.39,1.20),
                   (0,sign*.415,1.085),(0,sign*.43,.985)], [.08,.073,.057,.055,.045],
                  [{"chest":.25,upper:.75},{upper:1},{upper:.5,fore:.5},{fore:1},{fore:.65,hand:.35}],0)
        body.ellipsoid((.018,sign*.445,.94),(.050,.05,.069),hand,1)
        thigh, shin, foot = "thigh_"+side,"shin_"+side,"foot_"+side
        body.limb([(0,sign*.11,.90),(0,sign*.115,.73),(0,sign*.12,.52),
                   (0,sign*.12,.32),(0,sign*.12,.10)], [.10,.092,.072,.068,.055],
                  [{"pelvis":.15,thigh:.85},{thigh:1},{thigh:.5,shin:.5},{shin:1},{shin:.5,foot:.5}],2)
        body.box((.072,sign*.12,.062),(.12,.07,.062),foot,2)
        body.ellipsoid((0,sign*.23,1.44),(.095,.108,.078),upper,3)
        body.box((.046,sign*.12,.34),(.027,.079,.16),shin,3)
    # Bound equipment uses the same skin, rather than unparented preview objects.
    body.box((.23,-.445,.925),(.22,.015,.014),"hand_r",3)
    body.box((.06,-.445,.925),(.025,.08,.025),"hand_r",5)
    body.box((-.02,-.445,.925),(.06,.023,.025),"hand_r",2)
    if role == "Guard":
        body.ellipsoid((0,0,1.706),(.113,.112,.094),"head",3)
        body.box((.114,0,1.727),(.014,.105,.022),"head",3)
        body.ellipsoid((.09,.455,1.05),(.045,.18,.245),"hand_l",3)
        body.box((.14,.455,1.05),(.013,.03,.18),"hand_l",5)
    elif role == "Raider":
        body.ellipsoid((-.025,0,1.724),(.105,.115,.075),"head",2)
        body.box((.1,0,1.63),(.045,.097,.046),"head",2)
        body.box((.154,0,1.29),(.026,.11,.16),"chest",3)
    else:
        # Closed cape bands; skin follows chest, spine and pelvis without a cloth dependency.
        body.rings([((-.12,0,1.45),.035,.23,{"chest":1}),((-.14,0,1.20),.035,.21,{"spine":.65,"chest":.35}),
                    ((-.16,0,.86),.035,.20,{"pelvis":.8,"spine":.2})],0)
        body.ellipsoid((-.034,0,1.745),(.105,.108,.055),"head",2)
        body.box((.137,0,1.41),(.027,.022,.023),"chest",5)
    return rig, body.finish(role,rig)


def set_pose(rig, poses, translations):
    for bone in rig.pose.bones:
        bone.rotation_mode = "QUATERNION"
        rest = bone.bone.matrix_local.to_quaternion()
        world = Euler(tuple(math.radians(x) for x in poses.get(bone.name,(0,0,0))),"XYZ").to_quaternion()
        bone.rotation_quaternion = rest.inverted() @ world @ rest
        bone.location = bone.bone.matrix_local.to_3x3().inverted() @ Vector(translations.get(bone.name,(0,0,0)))
        bone.scale = (1,1,1)


def clip_pose(name, t):
    s, c = math.sin(math.tau*t), math.cos(math.tau*t)
    arch = math.sin(math.pi*t)
    poses, translations = {}, {}
    if name in ("Walk","Run","Sprint"):
        stride = {"Walk":24,"Run":42,"Sprint":56}[name]
        for side, sign in (("l",1),("r",-1)):
            poses["thigh_"+side]=(0,-sign*stride*s,0)
            poses["shin_"+side]=(0,max(0,sign*s)*stride*.75,0)
            poses["foot_"+side]=(0,-max(0,sign*s)*stride*.22,0)
            poses["upperarm_"+side]=(0,sign*stride*.6*s,0)
            poses["forearm_"+side]=(0,-(20 if name!="Walk" else 5),0)
        poses["chest"]=(0,0 if name=="Walk" else 10,4*s)
        translations["pelvis"]=(0,0,.018*(1-math.cos(2*math.tau*t)))
    elif name == "Idle":
        poses["chest"]=(0,1.6*s,0)
        poses["head"]=(0,0,2*s)
        translations["chest"]=(0,0,.003*(1-c))
    elif name == "Jump":
        poses.update({"thigh_l":(0,-45*arch,0),"thigh_r":(0,-35*arch,0),
                      "shin_l":(0,80*arch,0),"shin_r":(0,75*arch,0),
                      "upperarm_l":(0,-60*arch,-15*arch),"upperarm_r":(0,-60*arch,15*arch),"chest":(0,12*arch,0)})
        translations["pelvis"]=(0,0,-.12*arch)
    elif name == "Dodge":
        poses.update({"chest":(20*arch,55*arch,0),"thigh_l":(0,-65*arch,0),
                      "thigh_r":(0,-45*arch,0),"shin_l":(0,100*arch,0),"shin_r":(0,85*arch,0),
                      "upperarm_l":(0,-65*arch,0),"upperarm_r":(0,-55*arch,0)})
        translations["pelvis"]=(0,0,-.25*arch)
    elif name in ("LightAttack","HeavyAttack"):
        strength = 1.0 if name=="LightAttack" else 1.45
        # Raise into a readable windup, then slash across the front before returning to rest.
        windup = math.sin(math.pi*min(t/.42,1)) if t<.42 else 0
        slash = math.sin(math.pi*(t-.30)/.70) if t>.30 else 0
        poses.update({"upperarm_r":(15*slash,-85*windup-45*slash,50*slash),
                      "forearm_r":(0,-35*windup+25*slash,0),"chest":(0,8*arch,-25*windup+35*slash),
                      "hand_r":(0,0,15*slash),"thigh_l":(0,-12*arch,0)})
        if name=="HeavyAttack":
            poses["upperarm_l"]=(0,-70*arch,-15*arch)
            translations["pelvis"]=(0,0,-.06*arch)
        poses["chest"] = tuple(v*strength for v in poses["chest"])
    elif name == "Hit":
        poses.update({"chest":(0,-24*arch,12*arch),"head":(0,-18*arch,0),
                      "upperarm_l":(0,20*arch,0),"upperarm_r":(0,20*arch,0)})
        translations["pelvis"]=(-.07*arch,0,-.025*arch)
    elif name == "Death":
        ease = t*t*(3-2*t)
        poses.update({"root":(0,86*ease,0),"chest":(0,-5*ease,0),
                      "upperarm_l":(0,-20*ease,12*ease),"upperarm_r":(0,-15*ease,-12*ease)})
        translations["root"]=(0,0,.075*ease)
    elif name == "Block":
        poses.update({"upperarm_l":(0,-75+1.5*s,-15),"forearm_l":(0,-20,0),
                      "hand_l":(0,95-1.5*s,0),"upperarm_r":(0,-45+s,20),
                      "forearm_r":(0,-45,0),"chest":(0,4+s,-10)})
    elif name == "Cast":
        poses.update({"upperarm_l":(0,-85*arch,-20*arch),"upperarm_r":(0,-95*arch,20*arch),
                      "forearm_l":(0,20*arch,0),"forearm_r":(0,25*arch,0),
                      "hand_l":(10*arch,0,35*arch),"hand_r":(-10*arch,0,-35*arch),"head":(0,-8*arch,0)})
    return poses, translations


def animate(rig):
    actions = {}
    rig.animation_data_create()
    for clip, frames in CLIPS.items():
        action = bpy.data.actions.new(clip)
        action.use_fake_user = True
        rig.animation_data.action = action
        for step in range(9):
            t = step/8
            frame = 1+frames*t
            set_pose(rig,*clip_pose(clip,t))
            for bone in rig.pose.bones:
                for path in ("location","rotation_quaternion","scale"):
                    bone.keyframe_insert(data_path=path,frame=frame,group=bone.name)
        for curve in action.fcurves:
            for point in curve.keyframe_points:
                point.interpolation = "LINEAR"
        actions[clip] = action
    return actions


def evaluate_skin(rig, mesh, actions):
    result = {}
    for clip, action in actions.items():
        rig.animation_data.action=action
        baseline, maximum = None, 0
        for t in (0,.125,.25,.5,.75,1):
            bpy.context.scene.frame_set(1+int(CLIPS[clip]*t))
            evaluated = mesh.evaluated_get(bpy.context.evaluated_depsgraph_get())
            skin = evaluated.to_mesh()
            positions = [v.co.copy() for v in skin.vertices]
            if baseline is None:
                baseline = positions
            else:
                maximum=max(maximum,max((a-b).length for a,b in zip(baseline,positions)))
            evaluated.to_mesh_clear()
        if maximum < .0001:
            raise RuntimeError("Clip did not deform actual skinned vertices: "+clip)
        result[clip]={"maximum_sampled_vertex_displacement_m":round(maximum,6),"status":"Bestanden"}
    return result


def export_fbx(file, rig, mesh, action=None):
    bpy.ops.object.select_all(action="DESELECT")
    rig.select_set(True)
    mesh.select_set(True)
    bpy.context.view_layer.objects.active=rig
    rig.animation_data.action=action
    set_pose(rig,{}, {})
    bpy.context.scene.frame_set(1)
    if action:
        bpy.context.scene.frame_end=int(action.frame_range[1])
    bpy.ops.export_scene.fbx(filepath=str(file),use_selection=True,object_types={"ARMATURE","MESH"},
        add_leaf_bones=False,armature_nodetype="NULL",bake_anim=bool(action),
        bake_anim_use_all_actions=False,bake_anim_use_nla_strips=False,
        bake_anim_simplify_factor=0,apply_unit_scale=True,apply_scale_options="FBX_SCALE_UNITS",
        axis_forward="-Y",axis_up="Z",use_mesh_modifiers=True)


def roundtrip_glb(file):
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)
    for action in list(bpy.data.actions):
        bpy.data.actions.remove(action)
    bpy.ops.import_scene.gltf(filepath=str(file))
    rigs=[obj for obj in bpy.context.scene.objects if obj.type=="ARMATURE"]
    # The glTF importer adds an Icosphere used as a custom bone display shape.
    meshes=[obj for obj in bpy.context.scene.objects if obj.type=="MESH" and
            any(mod.type=="ARMATURE" for mod in obj.modifiers)]
    if len(rigs)!=1 or len(rigs[0].data.bones)!=len(BONES) or not meshes:
        raise RuntimeError("GLB roundtrip did not reconstruct actual rig/mesh")
    if len(bpy.data.actions)!=len(CLIPS):
        raise RuntimeError("GLB roundtrip action count mismatch: "+str(len(bpy.data.actions)))
    if not all(any(mod.type=="ARMATURE" and mod.object==rigs[0] for mod in mesh.modifiers) for mesh in meshes):
        raise RuntimeError("Roundtrip mesh has no skeletal deformation modifier")
    return {"status":"Bestanden","bones":len(rigs[0].data.bones),"actions":len(bpy.data.actions),
            "meshes":len(meshes),"coverage":"Blender4.3 GLB import, actual bones/actions/ArmatureModifiers"}


report={"version":"V78","blender":bpy.app.version_string,"status":"Nicht ausgeführt",
        "license":"CC0-1.0","characters":{},"unreal_import":"Nicht ausgeführt","runtime_animation":"Nicht ausgeführt"}
for role in ("Hero","Guard","Raider"):
    reset()
    rig,mesh=rigged_body(role)
    actions=animate(rig)
    validation=evaluate_skin(rig,mesh,actions)
    rig.animation_data.action=actions["Idle"]
    bpy.context.scene.frame_start=1
    bpy.context.scene.frame_end=61
    bpy.context.scene.frame_set(1)
    source_vertices=len(mesh.data.vertices)
    source_faces=len(mesh.data.polygons)
    bpy.ops.wm.save_as_mainfile(filepath=str(OUT/("Askir_"+role+".blend")))
    bpy.ops.object.select_all(action="DESELECT")
    rig.select_set(True)
    mesh.select_set(True)
    glb=OUT/("Askir_"+role+".glb")
    bpy.ops.export_scene.gltf(filepath=str(glb),export_format="GLB",use_selection=True,
        export_animations=True,export_animation_mode="ACTIONS",export_skins=True,export_apply=False,
        export_anim_slide_to_zero=True,export_optimize_animation_size=False)
    export_fbx(OUT/("Askir_"+role+".fbx"),rig,mesh)
    for clip,action in actions.items():
        export_fbx(OUT/("Askir_"+role+"_"+clip+".fbx"),rig,mesh,action)
    roundtrip=roundtrip_glb(glb)
    report["characters"][role]={"source_vertices":source_vertices,"source_polygons":source_faces,
                               "bones":len(BONES),"clips":len(CLIPS),"deformation_tests":validation,
                               "glb_roundtrip":roundtrip,"height_m":1.8,"source_forward":"+X","source_up":"+Z"}
    print("ASKIR V78 CHARACTER SOURCE VERIFIED",role,source_vertices,len(BONES),len(CLIPS))
report["status"]="Bestanden"
report["coverage"]="original source generation and Blender skin/action/GLB roundtrip checks only"
(ROOT/"Reports").mkdir(exist_ok=True)
(ROOT/"Reports/V78_CHARACTER_SOURCE_BLENDER.json").write_text(json.dumps(report,indent=2)+"\n",encoding="utf-8")
