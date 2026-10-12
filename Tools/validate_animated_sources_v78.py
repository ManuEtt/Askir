"""Independent GLB binary and CPU skin checks. Requires Python 3 and NumPy; no Blender/Unreal API."""
from pathlib import Path
import argparse
import copy
import json
import math
import struct
import sys
import numpy as np

CLIPS = {"Idle","Walk","Run","Sprint","Jump","Dodge","LightAttack","HeavyAttack","Hit","Death","Block","Cast"}
DTYPES = {5120:np.int8,5121:np.uint8,5122:np.int16,5123:np.uint16,5125:np.uint32,5126:np.float32}
WIDTHS = {"SCALAR":1,"VEC2":2,"VEC3":3,"VEC4":4,"MAT4":16}


class GLB:
    def __init__(self,path):
        raw=Path(path).read_bytes()
        magic,version,length=struct.unpack_from("<4sII",raw)
        if magic!=b"glTF" or version!=2 or length!=len(raw):
            raise ValueError("Invalid actual GLB header")
        chunks={}
        offset=12
        while offset<len(raw):
            size,kind=struct.unpack_from("<I4s",raw,offset)
            offset+=8
            chunks[kind]=raw[offset:offset+size]
            offset+=size
        self.data=json.loads(chunks[b"JSON"])
        self.binary=chunks[b"BIN\x00"]

    def accessor(self,index):
        accessor=self.data["accessors"][index]
        if "sparse" in accessor:
            raise ValueError("Sparse accessor unsupported in this source validator")
        view=self.data["bufferViews"][accessor["bufferView"]]
        dtype=np.dtype(DTYPES[accessor["componentType"]]).newbyteorder("<")
        width=WIDTHS[accessor["type"]]
        stride=view.get("byteStride",dtype.itemsize*width)
        result=np.ndarray((accessor["count"],width),dtype=dtype,buffer=self.binary,
                          offset=view.get("byteOffset",0)+accessor.get("byteOffset",0),
                          strides=(stride,dtype.itemsize)).copy()
        if accessor.get("normalized") and accessor["componentType"]!=5126:
            result=result.astype(float)/np.iinfo(dtype).max
        if not np.isfinite(result).all():
            raise ValueError("Nonfinite GLB accessor")
        return result


def quaternion_matrix(q):
    x,y,z,w=q/np.linalg.norm(q)
    return np.array([[1-2*y*y-2*z*z,2*x*y-2*z*w,2*x*z+2*y*w,0],
                     [2*x*y+2*z*w,1-2*x*x-2*z*z,2*y*z-2*x*w,0],
                     [2*x*z-2*y*w,2*y*z+2*x*w,1-2*x*x-2*y*y,0],[0,0,0,1]])


def local_matrix(node):
    if "matrix" in node:
        return np.array(node["matrix"]).reshape(4,4).T
    rotation=quaternion_matrix(np.array(node.get("rotation",[0,0,0,1]),dtype=float))
    rotation[:3,:3]=rotation[:3,:3]@np.diag(node.get("scale",[1,1,1]))
    rotation[:3,3]=node.get("translation",[0,0,0])
    return rotation


def interpolate(times,values,time,path):
    upper=int(np.searchsorted(times,time,side="right"))
    if upper==0:
        return values[0]
    if upper>=len(times):
        return values[-1]
    lower=upper-1
    fraction=(time-times[lower])/(times[upper]-times[lower])
    a,b=values[lower].astype(float),values[upper].astype(float)
    if path=="rotation":
        dot=float(np.dot(a,b))
        if dot<0:
            b=-b
            dot=-dot
        if dot<.9995:
            theta=math.acos(max(-1,min(1,dot)))
            return (math.sin((1-fraction)*theta)*a+math.sin(fraction*theta)*b)/math.sin(theta)
        result=a+(b-a)*fraction
        return result/np.linalg.norm(result)
    return a+(b-a)*fraction


def global_matrices(nodes):
    parents={child:i for i,node in enumerate(nodes) for child in node.get("children",[])}
    results={}
    def visit(index):
        if index not in results:
            local=local_matrix(nodes[index])
            results[index]=visit(parents[index])@local if index in parents else local
        return results[index]
    return {index:visit(index) for index in range(len(nodes))}


def evaluate(glb,animation,time):
    nodes=copy.deepcopy(glb.data["nodes"])
    for channel in animation["channels"]:
        sampler=animation["samplers"][channel["sampler"]]
        if sampler.get("interpolation","LINEAR")!="LINEAR":
            raise ValueError("Unexpected animation interpolation")
        path=channel["target"]["path"]
        times=glb.accessor(sampler["input"]).reshape(-1)
        values=glb.accessor(sampler["output"])
        if len(times)!=len(values) or np.any(np.diff(times)<=0):
            raise ValueError("Unordered/mismatched animation keys")
        nodes[channel["target"]["node"]][path]=interpolate(times,values,time,path).tolist()
    matrices=global_matrices(nodes)
    result=[]
    for index,node in enumerate(nodes):
        if "mesh" not in node or "skin" not in node:
            continue
        skin=glb.data["skins"][node["skin"]]
        bind=glb.accessor(skin["inverseBindMatrices"]).reshape(-1,4,4).transpose(0,2,1)
        inverse_mesh=np.linalg.inv(matrices[index])
        joints=np.stack([inverse_mesh@matrices[joint]@inv for joint,inv in zip(skin["joints"],bind)])
        for primitive in glb.data["meshes"][node["mesh"]]["primitives"]:
            attrs=primitive["attributes"]
            positions=glb.accessor(attrs["POSITION"])
            homogeneous=np.column_stack((positions,np.ones(len(positions))))
            indices=glb.accessor(attrs["JOINTS_0"]).astype(int)
            weights=glb.accessor(attrs["WEIGHTS_0"])
            posed=np.zeros_like(homogeneous,dtype=float)
            for influence in range(4):
                transformed=np.einsum("nij,nj->ni",joints[indices[:,influence]],homogeneous)
                posed+=transformed*weights[:,influence,None]
            result.append((posed@matrices[index].T)[:,:3])
    if not result:
        raise ValueError("GLB contains no actual skinned mesh")
    return np.concatenate(result)


def validate(path):
    glb=GLB(path)
    skins=glb.data.get("skins",[])
    animations=glb.data.get("animations",[])
    if len(skins)!=1 or len(skins[0]["joints"])!=18:
        raise ValueError("Expected one actual 18-joint skin")
    names={entry.get("name") for entry in animations}
    if names!=CLIPS or len(animations)!=12:
        raise ValueError("Expected exactly the twelve required animation names: "+str(names))
    matrices=glb.accessor(skins[0]["inverseBindMatrices"]).reshape(-1,4,4)
    if len(matrices)!=18 or np.any(np.abs(np.linalg.det(matrices))<1e-10):
        raise ValueError("Missing or singular inverse bind matrices")
    vertices=0
    for mesh in glb.data["meshes"]:
        for primitive in mesh["primitives"]:
            attrs=primitive["attributes"]
            indices=glb.accessor(attrs["JOINTS_0"])
            weights=glb.accessor(attrs["WEIGHTS_0"])
            if len(indices)!=len(weights) or np.any(indices<0) or np.any(indices>=18):
                raise ValueError("Invalid actual vertex joint indices")
            if np.any(weights<0) or np.any(np.abs(np.sum(weights,axis=1)-1)>1e-5):
                raise ValueError("Unnormalized actual vertex skin weights")
            vertices+=len(weights)
    clips={}
    for animation in animations:
        all_times=np.concatenate([glb.accessor(sampler["input"]).reshape(-1) for sampler in animation["samplers"]])
        start,end=float(np.min(all_times)),float(np.max(all_times))
        if end<=start:
            raise ValueError("Animation has no positive duration")
        baseline=evaluate(glb,animation,start)
        maximum=0
        for fraction in (.125,.25,.5,.75,1):
            skin=evaluate(glb,animation,start+(end-start)*fraction)
            maximum=max(maximum,float(np.max(np.linalg.norm(skin-baseline,axis=1))))
        if maximum<=.0001:
            raise ValueError("Exported skin animation does not deform vertices: "+animation["name"])
        clips[animation["name"]]={"duration_seconds":round(end-start,6),
            "maximum_sampled_vertex_displacement_m":round(maximum,6),"status":"Bestanden"}
    return {"status":"Bestanden","file":str(path),"skin_joints":18,"exported_primitive_vertices":vertices,
            "animations":clips,"coverage":"independent GLB binary accessors and CPU linear-blend skin evaluation"}


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--project-root",type=Path,default=Path(__file__).resolve().parents[1])
    args=parser.parse_args()
    report={"status":"Nicht ausgeführt","characters":{},"unreal_import":"Nicht ausgeführt","runtime_animation":"Nicht ausgeführt"}
    code=0
    try:
        for role in ("Hero","Guard","Raider"):
            report["characters"][role]=validate(args.project_root/"ArtSource/Animated_V78"/("Askir_"+role+".glb"))
        report["status"]="Bestanden"
    except Exception as exc:
        report["status"]="Fehlgeschlagen"
        report["error"]=str(exc)
        code=1
    path=args.project_root/"Reports/V78_CHARACTER_SOURCE_BINARY.json"
    path.parent.mkdir(exist_ok=True)
    path.write_text(json.dumps(report,indent=2)+"\n",encoding="utf-8")
    print("ASKIR V78 GLB source verification:",report["status"],path)
    return code


if __name__=="__main__":
    sys.exit(main())
